
#define _GNU_SOURCE

#include "opts.h"
#include <getopt.h>
#include <features.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _GNU_SOURCE
#  error "getopt.h doesn't have getopt_long."
#endif

struct internal_option_t {
    option_t opt;
    const char* value;
    int has;
};
static struct internal_option_t* opts_options = NULL;
static struct option* opts_gnu_options = NULL;
static char* opts_shorts = NULL;
static size_t opts_first_nonarg;
static size_t opts_argc;
static const char** opts_argv;

int opts_set(option_t* opts)
{
    size_t number, i;
    size_t shortln, pos;

    shortln = 0;
    for(i = 0; opts[i].name; ++i) {
        if(opts[i].letter != 0) {
            ++shortln;
            if(opts[i].value)
                ++shortln;
        }
    }
    shortln += 2;
    number = i;

    opts_options = malloc((number + 1) * sizeof(struct internal_option_t));
    if(!opts_options)
        return 0;
    opts_gnu_options = malloc((number + 1) * sizeof(struct option));
    if(!opts_gnu_options)
        return 0;

    if(shortln > 0) {
        opts_shorts = malloc(shortln);
        if(!opts_shorts)
            return 0;
        opts_shorts[0] = '\0';
    }

    pos = 1;
    opts_shorts[0] = '-';
    for(i = 0; i < number; ++i) {
        opts_options[i].opt   = opts[i];
        opts_options[i].value = NULL;
        opts_options[i].has   = 0;

        opts_gnu_options[i].name    = opts[i].name;
        opts_gnu_options[i].has_arg = opts[i].value;
        opts_gnu_options[i].flag    = NULL;
        opts_gnu_options[i].val     = 0;

        if(pos < shortln && opts[i].letter != 0) {
            opts_shorts[pos] = opts[i].letter;
            ++pos;
            if(pos < shortln && opts[i].value) {
                opts_shorts[pos] = '.';
                ++pos;
            }
        }
    }
    if(pos < shortln)
        opts_shorts[pos] = '\0';
    else
        opts_shorts[shortln - 1] = '\0';

    memset(&opts_gnu_options[number], 0, sizeof(struct option));
    memset(&opts_options[number], 0, sizeof(struct internal_option_t));
    return 1;
}

static struct internal_option_t* opts_opt_from_name(const char* name)
{
    size_t i;
    for(i = 0; opts_options[i].opt.name; ++i) {
        if(strcmp(opts_options[i].opt.name, name) == 0)
            return &opts_options[i];
    }
    return NULL;
}

static struct internal_option_t* opts_opt_from_short(char sh)
{
    size_t i;
    for(i = 0; opts_options[i].opt.name; ++i) {
        if(opts_options[i].opt.letter == sh)
            return &opts_options[i];
    }
    return NULL;
}

int opts_parse(int argc, char *argv[])
{
    int opt_id;
    int c;
    size_t capa;
    struct internal_option_t* opt;

    opts_argc = 0;
    opts_argv = malloc(sizeof(char*) * 10);
    if(!opts_argv)
        return 0;
    capa = 10;

    if(!opts_gnu_options)
        return 0;

    while((c = getopt_long(argc, argv, opts_shorts,
                    opts_gnu_options, &opt_id)) != -1) {
        if(c == 0) {
            opt = opts_opt_from_name(opts_gnu_options[opt_id].name);
            if(!opt)
                continue;
            if(optarg)
                opt->value = optarg;
            opt->has = 1;
        }
        else if(c == 1) {
            if(!optarg)
                continue;
            opts_argv[opts_argc] = optarg;
            ++opts_argc;
            if(opts_argc >= capa) {
                capa += 10;
                opts_argv = realloc(opts_argv, capa);
            }
        }
        else if(c != '?') {
            opt = opts_opt_from_short((char)c);
            if(!opt)
                continue;
            if(optarg)
                opt->value = optarg;
            opt->has = 1;
        }
    }

    opts_first_nonarg = 0;
    free(opts_gnu_options);
    opts_gnu_options = NULL;
    return 1;
}

const char* opts_next()
{
    if(opts_first_nonarg >= opts_argc)
        return NULL;
    else
        return opts_argv[opts_first_nonarg++];
}

const char* opts_as_string(const char* name)
{
    struct internal_option_t* opt = opts_opt_from_name(name);
    if(!opt)
        return "";
    return opt->value;
}

int opts_as_bool(const char* name)
{
    struct internal_option_t* opt = opts_opt_from_name(name);
    if(!opt)
        return 0;
    return opt->has;
}

int opts_as_int(const char* name)
{
    const char* str = opts_as_string(name);
    int ret;
    
    if(!str)
        return 0;
    sscanf(str, "%i", &ret);
    return ret;
}

void opts_display_help()
{
    printf("Options :\n");
    size_t i;
    for(i = 0; opts_options[i].opt.name; ++i) {
        printf("\t%s ", opts_options[i].opt.name);
        if(opts_options[i].opt.letter != 0)
            printf("[%c] ", opts_options[i].opt.letter);
        printf(": %s\n", opts_options[i].opt.desc);
    }
}

void opts_close()
{
    if(opts_options)
        free(opts_options);
    if(opts_gnu_options)
        free(opts_gnu_options);
    if(opts_shorts)
        free(opts_shorts);
}

