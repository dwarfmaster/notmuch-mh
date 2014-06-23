#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <notmuch.h>

#define QUERY_LENGTH 256

static void die(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static void print_thread(notmuch_thread_t* th)
{
    printf("thread:%s (%i)\n", notmuch_thread_get_thread_id(th),
            notmuch_thread_get_total_messages(th));
    /* TODO */
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

