/* acconfig.h
   This file is in the public domain.

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* Define if the C++ compiler uses namespaces.  */
#undef HAVE_NAMESPACE

/* Define if the C++ iostream library supports ios::binary.  */
#undef HAVE_IOS_BINARY

/* Define if we're compiling for Cygwin. */
#undef PLATFORM_CYGWIN

/* Define if we're compiling using Windows Microsoft Visual C++. */
#undef WIN32_VC

/* The platform ppremake is compiled for.  This primarily controls the
   initial setting of the $[PLATFORM] variable. */
#define PLATFORM ""


/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */


