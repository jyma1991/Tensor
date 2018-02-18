/*
Copyright (c) 2017, LibMars Developers.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MARS_DEBUGGER_H
#define MARS_DEBUGGER_H 1

/*
#include <execinfo.h>
//#include <cxxabi.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>
*/

namespace mars{

void print_stack_trace_c()
{
    const int BT_BUF_SIZE = 1024;
    int j, nptrs;
    void *buffer[BT_BUF_SIZE];
    char **strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
    would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for(j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);
}

std::pair<std::string, std::string> caller_callee_symbol()
{
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    int count = 0;
    std::string calls[2];
    while (unw_step(&cursor) > 0)
    {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0)
        {
            break;
        }
        //printf("0x%lx:", pc);

        char sym[1024];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
        {
            //printf(" (%s+0x%lx)\n", sym, offset);
            calls[count] = std::string(sym);
            count ++;
            if( count == 2 )
                break;
        }
        else
        {
            printf(" -- error: unable to obtain symbol name for this frame\n");
        }
    }

    return std::pair<std::string, std::string>(calls[0], calls[1]);
}

void segmentation_fault_handler(int sig)
{
    void *array[100];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    // print out all the frames to stderr
    fprintf( stderr, "Error: signal %d:\n", sig );
    backtrace_symbols_fd( array, size, STDERR_FILENO );
    exit(1);
}

/*
// Print a demangled stack backtrace of the caller function to FILE* out.
void print_stack_trace_broken()
{
    FILE *out = stderr;
    int max_frames = 255;
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0)
    {
        fprintf(out, "  <empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*) malloc( funcnamesize );

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for( int i = 1; i < addrlen; i++ )
    {
        char* begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char* p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset)
            {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
            if (status == 0)
            {
                funcname = ret; // use possibly realloc()-ed string
                fprintf(out, "STACK: %s: %s+%s\n", symbollist[i], funcname, begin_offset);
            }
            else

            {
                // demangling failed. Output function name as a C function with
                // no arguments.
                fprintf(out, "STACK: %s: %s()+%s\n", symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? prin  the whole line.
            fprintf(out, "STACK: %s\n", symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
}
*/

class Debugger
{
private:
    void init()
    {
        signal( SIGSEGV, segmentation_fault_handler );   // install our handler
        //fprintf(stderr, "Segmentation fault (core dumped)\n");
        stderr = freopen( "./debug_error.log", "w", stderr );
        fprintf(stderr, "******\n" );
    }

public:
    Debugger() { init(); }

// Call this function to get a backtrace.
void print_stack_trace()
{
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0)
    {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0)
        {
            break;
        }
        printf("0x%lx:", pc);

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0)
        {
            printf(" (%s+0x%lx)\n", sym, offset);
        }
        else
        {
            printf(" -- error: unable to obtain symbol name for this frame\n");
        }
    }
}


};

}

#endif
