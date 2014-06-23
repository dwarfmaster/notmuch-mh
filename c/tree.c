#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <notmuch.h>

#define QUERY_LENGTH 256
#define MAX_DEPTH    256
#define COLUMN_WIDTH   2

static const char* symbols[] = {
    "|",  /* Vertical bar.   */
    "-",  /* Horizontal bar. */
    "\\", /* Angle.          */
    "|",  /* New branch.     */
    "+",  /* New sub-branch. */
    ">",  /* Arrow.          */
};

static void die(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static void print_line(notmuch_message_t* msg)
{
    const char* subject = notmuch_message_get_header(msg, "Subject");
    const char* from    = notmuch_message_get_header(msg, "From");

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

    printf("\x1b[36m\x1b[1m%s \x1b[0m\x1b[32m[%s] \x1b[31m[%s]\x1b[0m\n", subject, from, tags);
}

static void print_message(notmuch_message_t* msg, int new,
        int symbs[MAX_DEPTH], size_t dec)
{
    size_t i, j;
    notmuch_messages_t* subs;
    subs = notmuch_message_get_replies(msg);

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

    if(subs)
        printf("%s", symbols[4]);
    else
        printf("%s", symbols[1]);

    printf("%s", symbols[5]);
    print_line(msg);

    /* TODO Handle last. */
    symbs[dec] = 0;
    for(;   notmuch_messages_valid(subs);
            notmuch_messages_move_to_next(subs)) {
        msg = notmuch_messages_get(subs);
        print_message(msg, 4, symbs, dec + 1);
    }

    notmuch_messages_destroy(subs);
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
        print_message(msg, 4, symbs, 0);
    }
    notmuch_messages_destroy(msgs);
}

int main(int argc, char *argv[])
{
    const char* maildir;
    char query_str[QUERY_LENGTH];
    size_t length;
    int i;
    notmuch_database_t* db;
    notmuch_status_t status;
    notmuch_query_t* query;
    notmuch_threads_t* threads;
    notmuch_thread_t* thread;

    /* Getting the query. */
    if(argc <= 1)
        die("No query string.\n");
    query_str[0] = '\0';
    length = QUERY_LENGTH;
    for(i = 1; i < argc; ++i) {
        strncat(query_str, argv[i], length);
        length -= strlen(argv[i]);
        strncat(query_str, " ", length);
        --length;
    }

    /* Getting the maildir. */
    maildir = getenv("MAILDIR");
    if(!maildir)
        die("$MAILDIR is not set.\n");

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
    notmuch_query_destroy(query);
    notmuch_database_close(db);

    return 0;
}

