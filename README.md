NotMuch-MH
=================

This is a collection of scripts and programs to use
alongside notmuch to handle mails.

Here is a list of scripts :
 - treeview.pl : pipe the output of a notmuch show
   with json output to this script to get a threaded
   list of mails. Accept -r (hide Re:), -d (hide
   duplicated subjects) and -s (number of spaces
   when identing) arguments.

It requires JSON and Getopt::Std perl modules.

