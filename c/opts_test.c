
#include "opts.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    option_t opts[] = {
        {"maildir", 'M', 1},
        {"home",    'H', 1},
        {"verbose", 'v', 1},
        {"mute",    'm', 0},
        {"help",     0 , 0},
        {NULL,       0,  0},
    };
    const char* str;

    if(!opts_set(opts))
        return 1;
    if(!opts_parse(argc, argv))
        return 1;

    printf("maildir => %s\n", opts_as_string("maildir"));
    printf("home    => %s\n", opts_as_string("home"));
    printf("verbose => %i\n", opts_as_int("verbose"));
    printf("mute    => %i\n", opts_as_bool("mute"));
    printf("help    => %i\n", opts_as_bool("help"));

    while((str = opts_next())) {
        printf(" - %s\n", str);
    }

    opts_close();
    return 0;
}

