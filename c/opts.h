
#ifndef DEF_OPTS_H
#define DEF_OPTS_H

typedef struct _option_t {
    const char* name; /** The name of the option. */
    char letter;      /** The short version, or 0 if none. */
    int value;        /** Boolean : has the argument a value or is it a flag. */
} option_t;

/** opts must end with an option with NULL as name. */
int opts_set(option_t* opts);
/** Parse the command line options. */
int opts_parse(int argc, char *argv[]);
/** Get the next non argument value. Return NULL when there is no one left */
const char* opts_next();
/** Get the value of the argument as a string. Arguments are designated by their long name */
const char* opts_as_string(const char* name);
/** Get the value of the argument as a boolean. */
int opts_as_bool(const char* name);
/** Get the value as an int. */
int opts_as_int(const char* name);
/** Free all memory used. */
void opts_close();

#endif

