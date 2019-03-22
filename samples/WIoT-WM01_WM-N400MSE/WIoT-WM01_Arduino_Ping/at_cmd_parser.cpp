/* Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @section DESCRIPTION
 *
 * Parser for the AT command syntax
 *
 */
#include "at_cmd_parser.h"
#include "debug.h"
#include "stdarg.h"

#ifdef LF
#undef LF
#define LF  10
#else
#define LF  10
#endif

#ifdef CR
#undef CR
#define CR  13
#else
#define CR  13
#endif

static char *Advance( char *bufp );
int vsscanf_avr( char *buf, char *format, va_list argp );

//  /putc handling with timeouts
int ATCmdParser::p_putc(char c)
{
    return _serial->write(&c,1);
}

int ATCmdParser::p_getc()
{
    int recved_byte;

    unsigned long start = millis();

    while( (millis() - start < (unsigned long)_timeout) ) {
        if( _serial->available() ) {
            recved_byte = _serial->read();
            //LOGDEBUG0((char)recved_byte);
            return recved_byte;
        }
    }

    return -1;
}

void ATCmdParser::flush()
{
    while (_serial->available()) {
        unsigned char ch;
        ch = _serial->read();
    }
}

// read/write handling with timeouts
int ATCmdParser::write(const char *data, int size)
{
    int i = 0;
    for ( ; i < size; i++) {
        if (p_putc(data[i]) < 0) {
            return -1;
        }
    }
    return i;
}

int ATCmdParser::read(char *data, int size)
{
    int i = 0;
    for ( ; i < size; i++) {
        int c = p_getc();
        if (c < 0) {
            return -1;
        }
        data[i] = c;
    }
    return i;
}


// printf/scanf handling
int ATCmdParser::vprintf(const char *format, va_list args)
{
    if (vsprintf(_buffer, format, args) < 0) {
        return -1;
    }

    int i = 0;
    for ( ; _buffer[i]; i++) {
        if (p_putc(_buffer[i]) < 0) {
            return -1;
        }
    }
    return i;
}

int ATCmdParser::vscanf(const char *format, va_list args)
{
    // Since format is const, we need to copy it into our buffer to
    // add the line's null terminator and clobber value-matches with asterisks.
    //
    // We just use the beginning of the buffer to avoid unnecessary allocations.
    int i = 0;
    int offset = 0;

    while (format[i]) {
        if (format[i] == '%' && format[i+1] != '%' && format[i+1] != '*') {
            _buffer[offset++] = '%';
            _buffer[offset++] = '*';
            i++;
        } else {
            _buffer[offset++] = format[i++];
        }
    }

    // Scanf has very poor support for catching errors
    // fortunately, we can abuse the %n specifier to determine
    // if the entire string was matched.
    _buffer[offset++] = '%';
    _buffer[offset++] = 'n';
    _buffer[offset++] = 0;

    // To workaround scanf's lack of error reporting, we actually
    // make two passes. One checks the validity with the modified
    // format string that only stores the matched characters (%n).
    // The other reads in the actual matched values.
    //
    // We keep trying the match until we succeed or some other error
    // derails us.
    int j = 0;

    while (true) {
        // Ran out of space
        if (j+1 >= _buffer_size - offset) {
            return false;
        }
        // Recieve next character
        int c = p_getc();
        if (c < 0) {
            return -1;
        }
        _buffer[offset + j++] = c;
        _buffer[offset + j] = 0;

        // Check for match
        int count = -1;
        sscanf(_buffer+offset, _buffer, &count);

        // We only succeed if all characters in the response are matched
        if (count == j) {
            // Store the found results
            sscanf(_buffer+offset, format, args);
            return j;
        }
    }
}


// Command parsing with line handling
bool ATCmdParser::vsend(const char *command, va_list args)
{
    if (vsprintf(_buffer, command, args) < 0) {
        return false;
    }

    for (int i = 0; _buffer[i]; i++) {
        if (p_putc(_buffer[i]) < 0) {
            return false;
        }
    }

    // Finish with newline
    for (size_t i = 0; _output_delimiter[i]; i++) {
        if (p_putc(_output_delimiter[i]) < 0) {
            return false;
        }
    }

    LOGDEBUG1(F("AT> "),_buffer);
    return true;
}

bool ATCmdParser::vrecv(const char *response, va_list args)
{
restart:
    _aborted = false;
    // Iterate through each line in the expected response
    while (response[0]) {
        // Since response is const, we need to copy it into our buffer to
        // add the line's null terminator and clobber value-matches with asterisks.
        //
        // We just use the beginning of the buffer to avoid unnecessary allocations.
        int i = 0;
        int offset = 0;
        bool whole_line_wanted = false;

        while (response[i]) {
            if (response[i] == '%' && response[i+1] != '%' && response[i+1] != '*') {
                _buffer[offset++] = '%';
                _buffer[offset++] = '*';
                i++;
            } else {
                _buffer[offset++] = response[i++];
                // Find linebreaks, taking care not to be fooled if they're in a %[^\n] conversion specification
                if (response[i - 1] == '\n' && !(i >= 3 && response[i-3] == '[' && response[i-2] == '^')) {
                    whole_line_wanted = true;
                    break;
                }
            }
        }

        // Scanf has very poor support for catching errors
        // fortunately, we can abuse the %n specifier to determine
        // if the entire string was matched.
        _buffer[offset++] = '%';
        _buffer[offset++] = 'n';
        _buffer[offset++] = 0;

        //LOGDEBUG1(F("AT? "),_buffer);
        // To workaround scanf's lack of error reporting, we actually
        // make two passes. One checks the validity with the modified
        // format string that only stores the matched characters (%n).
        // The other reads in the actual matched values.
        //
        // We keep trying the match until we succeed or some other error
        // derails us.
        int j = 0;

        while (true) {
            // Receive next character
            int c = (char)p_getc();
            if (c < 0) {
                LOGDEBUG(F("AT(Timeout)"));
                return false;
            }
            // Simplify newlines (borrowed from retarget.cpp)
            if ((c == CR && _in_prev != LF) ||
                (c == LF && _in_prev != CR)) {
                _in_prev = c;
                c = '\n';
            } else if ((c == CR && _in_prev == LF) ||
                       (c == LF && _in_prev == CR)) {
                _in_prev = c;
                // onto next character
                continue;
            } else {
                _in_prev = c;
            }
            _buffer[offset + j++] = c;
            _buffer[offset + j] = 0;

            // Check for oob data
//            for (struct oob *oob = _oobs; oob; oob = oob->next) {
//                if ((unsigned)j == oob->len && memcmp(
//                        oob->prefix, _buffer+offset, oob->len) == 0) {
//                    LOGDEBUG1(F("AT! "),oob->prefix);
//                    // oob->cb();
//
//                    if (_aborted) {
//                        LOGDEBUG(F("AT(Aborted)"));
//                        return false;
//                    }
//                    // oob may have corrupted non-reentrant buffer,
//                    // so we need to set it up again
//                    goto restart;
//                }
//            }

            // Check for match
            int count = -1;
            if (whole_line_wanted && c != '\n') {
                // Don't attempt scanning until we get delimiter if they included it in format
                // This allows recv("Foo: %s\n") to work, and not match with just the first character of a string
                // (scanf does not itself match whitespace in its format string, so \n is not significant to it)
            } else {
                sscanf(_buffer+offset, _buffer, &count);

            }

            // We only succeed if all characters in the response are matched
            if (count == j) {
                LOGDEBUG1(F("AT= "),_buffer+offset);
                // Reuse the front end of the buffer
                memcpy(_buffer, response, i);
                _buffer[i] = 0;

                // Store the found results
#ifdef __AVR__
                // kaizen modified
                // In AVR, vsscanf is not. so it can get only char array type
                // Max Parameter is 6
                char *ptr1 = va_arg(args,char*); char *ptr2 = va_arg(args,char*);
                char *ptr3 = va_arg(args,char*); char *ptr4 = va_arg(args,char*);
                char *ptr5 = va_arg(args,char*); char *ptr6 = va_arg(args,char*);

                sscanf(_buffer+offset,_buffer,ptr1,ptr2,ptr3,ptr4,ptr5,ptr6);
#else
                vsscanf(_buffer+offset,_buffer, args);
#endif

                // Jump to next line and continue parsing
                response += i;
                break;
            }

            // Clear the buffer when we hit a newline or ran out of space
            // running out of space usually means we ran into binary data
            if (c == '\n' || j+1 >= _buffer_size - offset) {
                LOGDEBUG1(F("AT< "),_buffer+offset);
                j = 0;
            }
        }
    }

    return true;
}

// Mapping to vararg functions
int ATCmdParser::printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vprintf(format, args);
    va_end(args);
    return res;
}

int ATCmdParser::scanf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vscanf(format, args);
    va_end(args);
    return res;
}

bool ATCmdParser::send(const __FlashStringHelper* command, ...)
{
    va_list args;
    va_start(args, command);

#ifdef __AVR__
    if( vsnprintf_P (_buffer, _buffer_size, (char*)command, args) < 0) {
        return false;
    }
#else
    if( vsnprintf (_buffer, _buffer_size, (char*)command, args) < 0) {
        return false;
    }
#endif
    va_end(args);

    _serial->print(_buffer);

    LOGDEBUG1(F("AT> "),_buffer);
    // Finish with newline
    for (size_t i = 0; _output_delimiter[i]; i++) {
        if (p_putc(_output_delimiter[i]) < 0) {
            return false;
        }
    }

    return true;
}

bool ATCmdParser::send(const char *command, ...)
{
    va_list args;
    va_start(args, command);
    bool res = vsend(command, args);
    va_end(args);
    return res;
}

bool ATCmdParser::recv(const __FlashStringHelper* response, ...)
{
    char *temp_buf = new char [_buffer_size];
    strcpy_P(temp_buf,(PGM_P)response);

    va_list args;
    va_start(args, temp_buf);
    bool res = vrecv(temp_buf, args);
    va_end(args);

    delete temp_buf;
    return res;
}


bool ATCmdParser::recv(const char *response, ...)
{
    va_list args;
    va_start(args, response);
    bool res = vrecv(response, args);
    va_end(args);
    return res;
}

// oob registration
void ATCmdParser::oob(const char *prefix, Parser_Callback cb)
{
    struct oob *oob = new struct oob;
    oob->len = strlen(prefix);
    oob->prefix = prefix;
    //memset(oob->prefix, 0, oob->len);
    //strncpy(oob->prefix,prefix,oob->len);

    oob->cb = cb;
    oob->next = _oobs;
    _oobs = oob;
}

void ATCmdParser::abort()
{
    _aborted = true;
}

bool ATCmdParser::process_oob()
{
    if (!_serial->available()) {
        return false;
    }

    int i = 0;
    while (true) {
        // Receive next character
        int c = p_getc();
        if (c < 0) {
            return false;
        }
        _buffer[i++] = (char)c;
        _buffer[i] = 0;

        // Check for oob data
        struct oob *oob = _oobs;
        while (oob) {
            //for(int j=0;j<i;j++){
            //    LOGDEBUG_LN_TYPE(_buffer[j],HEX);
            //}

            if (i == (int)oob->len && memcmp(
                    oob->prefix, _buffer, oob->len) == 0) {
                LOGDEBUG1("AT! ",oob->prefix);
                oob->cb();
                return true;
            }
            oob = oob->next;
        }

        // Clear the buffer when we hit a newline or ran out of space
        // running out of space usually means we ran into binary data
        if (i+1 >= _buffer_size ||
            strcmp(&_buffer[i-_output_delim_size], _output_delimiter) == 0) {

            //debug_if(_dbg_on, "AT< %s", _buffer);
            i = 0;
        }
    }
}


#define DEF_TYPE   0
#define LONG       1
#define SHORT      2
static char *Advance( char *bufp )
{
    char *new_bufp = bufp;

    /* Skip over nonwhite SPACE */
    while ((*new_bufp != ' ') && (*new_bufp != '\t') &&
            (*new_bufp != '\n') && (*new_bufp != '\0'))
        new_bufp++;

    /* Skip white SPACE */
    while ((*new_bufp == ' ') || (*new_bufp == '\t') ||
            (*new_bufp == '\n') || (*new_bufp == '\0'))
        new_bufp++;

    return new_bufp;
} /* Advance */

int vsscanf_avr( char *buf, char *format, va_list argp )
{
    char *fmtp;
    char *bufp;
    bool suppress;
    int mytype, width, n, k = 0;
    char lastchar;

    bufp = buf;

    for (fmtp = format; *fmtp; fmtp++)
    {
        if (*fmtp == '%')
        {
            mytype = DEF_TYPE;
            suppress = false;
            width = 0;
            lastchar = ' ';
        }
        else if (*fmtp == '*')
            suppress = true;
        else if (isspace(*fmtp));
        else if (isdigit(*fmtp))
        {
            if (lastchar != '.')
            {
                width *= 10;
                width += (*fmtp - '0');
            }
        }
        else if (*fmtp == 'l' || *fmtp == 'L')
            mytype = LONG;
        else if (*fmtp == 'h')
            mytype = SHORT;
        else if (*fmtp == 'i' || *fmtp == 'd')
        {
            if (suppress)
                bufp = Advance(bufp);
            else if (mytype == SHORT)
            {
                k+=sscanf(bufp,"%hd%n",va_arg(argp,short*),&n);
                bufp+=n;
            }
            else if (mytype == LONG)
            {
                k+=sscanf(bufp,"%ld%n",va_arg(argp,long*),&n);
                bufp+=n;
            }
            else
            {
                k+=sscanf(bufp,"%d%n",va_arg(argp, int*),&n);
                bufp+=n;
            }
        }
        else if (*fmtp == 'f')
        {
            if (suppress)
                bufp = Advance(bufp);
            else if (mytype == LONG)
            {
                k+=sscanf(bufp,"%f%n",va_arg(argp, double*),&n);
                bufp+=n;
            }
            else
            {
                k+=sscanf(bufp,"%f%n",va_arg(argp, float*),&n);
                bufp+=n;
            }
        }
        else if (*fmtp == 's')
        {
            if (suppress)
                bufp = Advance(bufp);
            else {
                k+=sscanf(bufp,"%s%n",va_arg(argp, char*),&n);
                bufp+=n;
            }
        }
        else if (*fmtp == 'c')
        {
            if (!suppress)
            {
                k+=sscanf(bufp,"%c%n",va_arg(argp, char*),&n);
                bufp+=n;
            }
            else bufp++;
        }
        else lastchar = *fmtp;
    } /* for */
    return k;
} /* vsscanf clone */



