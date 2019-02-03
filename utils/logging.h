#ifndef LOGGING
#define LOGGING

#include <stdlib.h>
#include <stdio.h>
#include <utils/misc.h>

struct print_format
{
    int argument;
};

typedef int (*printfn_t)(const char* format, ...);

struct printer
{
    int count;
    printfn_t print_function;
    print_format format;
};

#define default_print_format {1}
printer __printer__ = {0, printf, default_print_format};

printer operator,(printer p, print_format pf)
{
    p.format = pf;
    return p;
}

//TODO: there's probably a better way to do this
printer operator,(printer p, printfn_t f)
{
    __printer__.print_function = f;
    return __printer__;
}

#define define_printer(arg, print_code)                         \
    printer operator,(printer p, arg)                           \
    {                                                           \
        for(int __i = 0; __i < p.format.argument; __i++)        \
        {                                                       \
            p.count += p.print_function print_code;             \
        }                                                       \
        p.format = default_print_format;                        \
        return p;                                               \
    }


define_printer(const char* s, ("%s", s));

define_printer(char* s, ("%s", s));

define_printer(char c, ("%.1s", &c));

define_printer(int a, ("%d", a));
define_printer(uint a, ("%u", a));
define_printer(uint64 a, ("%llu", a));

define_printer(float a, ("%f", a));
define_printer(double a, ("%f", a));

/* define_printer(bool a, printf("%s", a ? "true" : "false")); */

#define log_warning(...) (__printer__, "warning: ", __VA_ARGS__, "\n")

#define log_error_text(...) (__printer__, "error: ")
#define log_error(...) {(__printer__, "error: ", __VA_ARGS__, "\n"); exit(EXIT_FAILURE);}

#define log_output(...) (__printer__, __VA_ARGS__)

#define assert(this_is_true, ...) {if(!(this_is_true)) {log_error(__VA_ARGS__);}}

#endif //LOGGING
