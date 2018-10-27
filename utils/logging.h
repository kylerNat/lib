#ifndef LOGGING
#define LOGGING

#include <stdlib.h>
#include <stdio.h>
#include <utils/misc.h>

struct print_format
{
    int argument;
};

struct printer
{
    int count;
    print_format format;
};

#define default_print_format {1}
printer __printer__ = {0, default_print_format};

printer operator,(printer p, print_format pf)
{
    p.format = pf;
    return p;
}

#define define_printer(arg, print_code)                         \
    printer operator,(printer p, arg)                           \
    {                                                           \
        for(int __i = 0; __i < p.format.argument; __i++)        \
        {                                                       \
            p.count += print_code;                              \
        }                                                       \
        p.format = default_print_format;                        \
        return p;                                               \
    }


define_printer(const char* s, printf("%s", s));

define_printer(char* s, printf("%s", s));

define_printer(char c, printf("%.1s", &c));

define_printer(int a, printf("%d", a));
define_printer(uint a, printf("%u", a));
define_printer(uint64 a, printf("%llu", a));

define_printer(float a, printf("%f", a));
define_printer(double a, printf("%f", a));

define_printer(DWORD a, printf("%li", a));

/* define_printer(bool a, printf("%s", a ? "true" : "false")); */

#define log_warning(...) (__printer__, "warning: ", __VA_ARGS__, "\n")

#define log_error_text(...) (__printer__, "error: ")
#define log_error(...) {(__printer__, "error: ", __VA_ARGS__, "\n"); exit(EXIT_FAILURE);}

#define log_output(...) (__printer__, __VA_ARGS__)

#define assert(this_is_true, ...) {if(!(this_is_true)) {log_error(__VA_ARGS__);}}

#endif //LOGGING
