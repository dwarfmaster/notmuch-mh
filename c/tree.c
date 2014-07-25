#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <notmuch.h>
#include "opts.h"

#define QUERY_LENGTH 256
#define MAX_DEPTH    256
#define COLUMN_WIDTH   0

static const char* utf8_symbols[] = {
    "\xe2\x94\x82", /* Vertical bar.   */
    "\xe2\x94\x80", /* Horizontal bar. */
    "\xe2\x94\x94", /* Angle.          */
    "\xe2\x94\x9c", /* New branch.     */
    "\xe2\x94\xac", /* New sub-branch. */
    ">",            /* Arrow.          */
    " ",            /* Empty.          */
    "\xe2\x97\x8f", /* Filled circle.  */
};
static const char* ascii_symbols[] = {
    "|",  /* Vertical bar.   */
    "-",  /* Horizontal bar. */
    "\\", /* Angle.          */
    "|",  /* New branch.     */
    "-",  /* New sub-branch. */
    ">",  /* Arrow.          */
    " ",  /* Empty.          */
    "X",  /* Filled circle.  */
};
static const char** symbols;

enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_CLEAR,
    COLOR_NB
};
static const char* colors[COLOR_NB] = {
    "\x1b[31m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[34m",
    "\x1b[35m",
    "\x1b[36m",
    "\x1b[37m",
    "\x1b[0m",
};

static void die(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static const char* print_line(notmuch_message_t* msg, const char* prevsubj)
{
    const char* subject = notmuch_message_get_header(msg, "Subject");
    const char* from    = notmuch_message_get_header(msg, "From");
    if(strlen(subject) > 4 && memcmp(subject, "Re: ", 4) == 0)
        subject += 4;

    char tags [QUERY_LENGTH];
    const char* tag;
    size_t length = QUERY_LENGTH;
    notmuch_tags_t* tgs;
    tags[0] = '\0';

    for(tgs = notmuch_message_get_tags(msg);
            notmuch_tags_valid(tgs);
            notmuch_tags_move_to_next(tgs)) {
        tag = notmuch_tags_get(tgs);
        strncat(tags, tag, length);
        length -= strlen(tag);
        strncat(tags, " ", length);
        length -= 1;
    }
    if(length < QUERY_LENGTH)
        tags[QUERY_LENGTH - length - 1] = '\0';

    if(strcmp(subject, prevsubj) == 0) {
        printf(" %s[%s] %s[%s]%s\n",
                colors[COLOR_GREEN], from,
                colors[COLOR_RED],   tags,
                colors[COLOR_CLEAR]);
    }
    else {
        printf("%s%s %s[%s] %s[%s]%s\n",
                colors[COLOR_CYAN],  subject,
                colors[COLOR_GREEN], from,
                colors[COLOR_RED],   tags,
                colors[COLOR_CLEAR]);
    }
    return subject;
}

static int count_subs(notmuch_message_t* msg)
{
    int match = opts_as_bool("matched");
    notmuch_messages_t* subs;
    int count;

    count = 0;
    for(subs = notmuch_message_get_replies(msg);
            notmuch_messages_valid(subs);
            notmuch_messages_move_to_next(subs)) {
        msg = notmuch_messages_get(subs);
        if(!match || notmuch_message_get_flag(msg, NOTMUCH_MESSAGE_FLAG_MATCH))
            ++count;
        else
            count += count_subs(msg);
    }
    notmuch_messages_destroy(subs);

    return count;
}

static void print_message(notmuch_message_t* msg, int new,
        int symbs[MAX_DEPTH], size_t dec, const char* prevsubj)
{
    size_t i, j;
    int cont, next;
    const char* subject;
    notmuch_messages_t* subs;
    subs = notmuch_message_get_replies(msg);

    if(!opts_as_bool("matched")
            || notmuch_message_get_flag(msg, NOTMUCH_MESSAGE_FLAG_MATCH))
    {
        if(opts_as_bool("mid"))
            printf("%s\t", notmuch_message_get_message_id(msg));

        if(opts_as_bool("hlmatch")) {
            if(notmuch_message_get_flag(msg, NOTMUCH_MESSAGE_FLAG_MATCH))
                printf("%s", symbols[7]);
            else
                printf(" ");
        }

        for(i = 0; dec > 0 && i < dec - 1; ++i) {
            printf("%s", symbols[symbs[i]]);
            for(j = 0; j < COLUMN_WIDTH; ++j)
                printf(" ");
        }

        if(dec >= 1) {
            printf("%s", symbols[new]);
            for(j = 0; j < COLUMN_WIDTH; ++j)
                printf("%s", symbols[1]);
        }

        if(count_subs(msg) > 0)
            printf("%s", symbols[4]);
        else
            printf("%s", symbols[1]);

        printf("%s", symbols[5]);
        subject = print_line(msg, prevsubj);

        symbs[dec] = 0;
        cont = 1;
        next = 3;
        if(!notmuch_messages_valid(subs))
            cont = 0;

        while(cont) {
            msg = notmuch_messages_get(subs);
            notmuch_messages_move_to_next(subs);
            if(!notmuch_messages_valid(subs)) {
                cont = 0;
                symbs[dec] = 6;
                next = 2;
            }
            print_message(msg, next, symbs, dec + 1, subject);
        }

        notmuch_messages_destroy(subs);
    }
    else {
        for(; notmuch_messages_valid(subs);
                notmuch_messages_move_to_next(subs)) {
            msg = notmuch_messages_get(subs);
            print_message(msg, new, symbs, dec, prevsubj);
        }
    }
}

static void print_thread(notmuch_thread_t* th)
{
    notmuch_messages_t* msgs;
    notmuch_message_t* msg;
    int symbs[MAX_DEPTH];

    for(msgs = notmuch_thread_get_toplevel_messages(th);
            notmuch_messages_valid(msgs);
            notmuch_messages_move_to_next(msgs)) {
        msg = notmuch_messages_get(msgs);
        print_message(msg, 4, symbs, 0, "");
    }
    notmuch_messages_destroy(msgs);
}

static void set_options()
{
    option_t opts[] = {
        {"maildir", 0, 1},
        {"format",  0, 1},
        {"mid",     0, 0},
        {"matched", 0, 0},
        {"hlmatch", 0, 0},
        {"ascii",   0, 0},
        {"colors",  0, 0},
        {NULL,      0, 0}
    };
    opts_set(opts);
}

int main(int argc, char *argv[])
{
    const char* maildir;
    const char* str;
    char query_str[QUERY_LENGTH];
    size_t length;
    notmuch_database_t* db;
    notmuch_status_t status;
    notmuch_query_t* query;
    notmuch_threads_t* threads;
    notmuch_thread_t* thread;

    /* Handling command line options. */
    set_options();
    opts_parse(argc, argv);

    /* Setting the symbols. */
    if(opts_as_bool("ascii"))
        symbols = ascii_symbols;
    else
        symbols = utf8_symbols;

    /* Setting the colors. */
    if(!opts_as_bool("colors")) {
        for(length = 0; length < COLOR_NB; ++length)
            colors[length] = "";
    }

    /* Getting the query. */
    query_str[0] = '\0';
    length = QUERY_LENGTH;
    while((str = opts_next())) {
        strncat(query_str, str, length);
        length -= strlen(str);
        strncat(query_str, " ", length);
        --length;
    }

    /* Getting the maildir. */
    maildir = opts_as_string("maildir");
    if(!maildir) {
        maildir = getenv("MAILDIR");
        if(!maildir)
            die("$MAILDIR is not set.\n");
    }

    /* Opening the database. */
    status = notmuch_database_open(maildir,
            NOTMUCH_DATABASE_MODE_READ_ONLY, &db);
    if(status != NOTMUCH_STATUS_SUCCESS) {
        die("Couldn't open notmuch database in \"%s\" : %s\n",
                maildir, notmuch_status_to_string(status));
    }

    /* Querying the database. */
    query = notmuch_query_create(db, query_str);
    if(!query)
        die("Couldn't create the query for \"%s\".\n", query);
    notmuch_query_set_sort(query, NOTMUCH_SORT_OLDEST_FIRST);
    notmuch_query_set_omit_excluded(query, NOTMUCH_EXCLUDE_ALL);
    notmuch_query_add_tag_exclude(query, "spam");
    notmuch_query_add_tag_exclude(query, "deleted");

    /* Printing threads. */
    for(threads = notmuch_query_search_threads(query);
            notmuch_threads_valid(threads);
            notmuch_threads_move_to_next(threads))
    {
        thread = notmuch_threads_get(threads);
        print_thread(thread);
        notmuch_thread_destroy(thread);
    }

    /* Clearing memory. */
    opts_close();
    notmuch_query_destroy(query);
    notmuch_database_close(db);

    return 0;
}

