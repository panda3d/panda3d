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

/* The current version number. */
#define VERSION 0.0

/* Define if we have Python installed.  */
#undef HAVE_PYTHON

/* Define if we have NSPR installed.  */
#undef HAVE_NSPR

/* Define if we have zlib installed.  */
#undef HAVE_ZLIB

/* Define if we have OpenGL installed and want to build for GL.  */
#undef HAVE_GL

/* Define if we have GLU installed.  */
#undef HAVE_GLU

/* Define if we have GLX installed and want to build for GLX.  */
#undef HAVE_GLX

/* Define if we have Glut installed and want to build for Glut.  */
#undef HAVE_GLUT

/* Define if we have DirectX installed and want to build for DirectX.  */
#undef HAVE_DX

/* Define if we want to build the Renderman interface.  */
#undef HAVE_RIB

/* Define if we want to use mikmod for audio.  */
#undef HAVE_MIKMOD

/* Define if we have a gettimeofday() function. */
#undef HAVE_GETTIMEOFDAY

/* Define if gettimeofday() takes only one parameter. */
#undef GETTIMEOFDAY_ONE_PARAM


/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */


