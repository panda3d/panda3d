/* pbmplus.h - header file for PBM, PGM, PPM, and PNM
**
** Copyright (C) 1988, 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#ifndef _PBMPLUS_H_
#define _PBMPLUS_H_

#include <pandabase.h>

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#ifdef VMS
#include <perror.h>
#endif

#if defined(USG) || defined(SVR4) || defined(VMS)
#define SYSV
#endif
#if ! ( defined(BSD) || defined(SYSV) || defined(MSDOS) || defined(AMIGA) )
/* CONFIGURE: If your system is >= 4.2BSD, set the BSD option; if you're a
** System V site, set the SYSV option; if you're IBM-compatible, set MSDOS;
** and if you run on an Amiga, set AMIGA. If your compiler is ANSI C, you're
** probably better off setting SYSV - all it affects is string handling.
*/
/*#define BSD*/
#define SYSV
/* #define MSDOS */
/* #define AMIGA */
#endif

/* CONFIGURE: If you have an X11-style rgb color names file, define its
** path here.  This is used by PPM to parse color names into rgb values.
** If you don't have such a file, comment this out and use the alternative
** hex and decimal forms to specify colors (see ppm/pgmtoppm.1 for details).
*/
#ifndef RGB_DB
#define RGB_DB "/usr/lib/X11/rgb.txt"
/*#define RGB_DB "/usr/openwin/lib/rgb.txt"*/
#ifdef VMS
#define RGB_DB "PBMplus_Dir:RGB.TXT"
#endif
#endif

/* CONFIGURE: If you want to enable writing "raw" files, set this option.
** "Raw" files are smaller, and much faster to read and write, but you
** must have a filesystem that allows all 256 ASCII characters to be read
** and written.  You will no longer be able to mail P?M files without
** using uuencode or the equivalent, or running the files through pnmnoraw.
** Note that reading "raw" files works whether writing is enabled or not.
*/
#define PBMPLUS_RAWBITS

/* CONFIGURE: PGM can store gray values as either bytes or shorts.  For most
** applications, bytes will be big enough, and the memory savings can be
** substantial.  However, if you need more than 8 bits of grayscale resolution,
** then define this symbol. Unless your computer is very slow or you are short
** of memory, there is no reason why you should not set this symbol.
*/
#define PGM_BIGGRAYS

/* CONFIGURE: Normally, PPM handles a pixel as a struct of three grays.
** If grays are stored in bytes, that's 24 bits per color pixel; if
** grays are stored as shorts, that's 48 bits per color pixel.  PPM
** can also be configured to pack the three grays into a single longword,
** 10 bits each, 30 bits per pixel.
**
** If you have configured PGM with the PGM_BIGGRAYS option, AND you don't
** need more than 10 bits for each color component, AND you care more about
** memory use than speed, then this option might be a win.  Under these
** circumstances it will make some of the programs use 1.5 times less space,
** but all of the programs will run about 1.4 times slower.
**
** If you are not using PGM_BIGGRAYS, then this option is useless -- it
** doesn't save any space, but it still slows things down.
*/
/* #define PPM_PACKCOLORS */

/* CONFIGURE: uncomment this to enable debugging checks. */
/* #define DEBUG */

#if ( defined(SYSV) || defined(AMIGA) )

#include <string.h>
#define random rand
#define srandom(s) srand(s)
#ifndef __SASC
#define index(s,c) strchr(s,c)
#define rindex(s,c) strrchr(s,c)
#ifndef _DCC    /* Amiga DICE Compiler */
#define bzero(dst,len) memset(dst,0,len)
#define bcopy(src,dst,len) memcpy(dst,src,len)
#define bcmp memcmp
#ifndef __cplusplus
extern void srand();
extern int rand();
#endif
#endif /* _DCC */
#endif /* __SASC */

#else /* SYSV or AMIGA */

#include <strings.h>
extern void srandom();
extern long random();

#endif /*SYSV or AMIGA*/

#if (defined(MSDOS) || defined(AMIGA) )
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#else
#ifndef __cplusplus
extern int atoi();
extern void exit();
extern long time();
extern int write();
#endif
#endif

/* CONFIGURE: On most BSD systems, malloc() gets declared in stdlib.h, on
** system V, it gets declared in malloc.h. On some systems, malloc.h
** doesn't declare these, so we have to do it here. On other systems,
** for example HP/UX, it declares them incompatibly.  And some systems,
** for example Dynix, don't have a malloc.h at all.  A sad situation.
** If you have compilation problems that point here, feel free to tweak
** or remove these declarations.
*/
#ifdef BSD
#include <stdlib.h>
#endif
#if (defined(SYSV) && !defined(VMS))
#include <malloc.h>
#endif
/* extern char* malloc(); */
/* extern char* realloc(); */
/* extern char* calloc(); */

/* CONFIGURE: Some systems don't have vfprintf(), which we need for the
** error-reporting routines.  If you compile and get a link error about
** this routine, uncomment the first define, which gives you a vfprintf
** that uses the theoretically non-portable but fairly common routine
** _doprnt().  If you then get a link error about _doprnt, or
** message-printing doesn't look like it's working, try the second
** define instead.
*/
/* #define NEED_VFPRINTF1 */
/* #define NEED_VFPRINTF2 */

/* CONFIGURE: Some systems don't have strstr(), which some routines need.
** If you compile and get a link error about this routine, uncomment the
** define, which gives you a strstr.
*/
/* #define NEED_STRSTR */

/* CONFIGURE: If you don't want a fixed path to an X11 color name file
** compiled into PBMPlus, set this option.  Now RGB_DB (see Makefile)
** defines the name of an environment-variable that holds the complete
** path and name of this file.
*/
/* #define A_RGBENV */
#ifdef A_RGBENV
#define RGB_DB "RGBDEF"    /* name of env-var */
#endif /* A_RGBENV */

/* CONFIGURE: Set this option if your compiler uses strerror(errno)
** instead of sys_errlist[errno] for error messages.
*/
/* #define A_STRERROR */

/* CONFIGURE: On small systems without VM it is possible that there is
** enough memory for a large array, but it is fragmented.  So the usual
** malloc( all-in-one-big-chunk ) fails.  With this option, if the first
** method fails, pm_allocarray() tries to allocate the array row by row.
*/
/* #define A_FRAGARRAY */

/*
** Some special things for the Amiga.
*/

/* End of configurable definitions. */


#ifdef AMIGA
#include <clib/exec_protos.h>
#define getpid(x)   ((long)FindTask(NULL))
#endif /* AMIGA */

/*
#undef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#undef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#undef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#undef odd
#define odd(n) ((n) & 1)
*/

/* Definitions to make PBMPLUS work with either ANSI C or C Classic. */

#if __STDC__
#define ARGS(alist) alist
#else /*__STDC__*/
#define ARGS(alist) ()
/*#define const*/
#endif /*__STDC__*/


/* Initialization. */

EXPCL_PANDA void pm_init( int* argcP, char* argv[] );

/* Variable-sized arrays definitions. */

char** pm_allocarray ARGS(( int cols, int rows, int size ));
EXPCL_PANDA char* pm_allocrow( int cols, int size );
void pm_freearray ARGS(( char** its, int rows ));
EXPCL_PANDA void pm_freerow( char* itrow );


/* Case-insensitive keyword matcher. */

int pm_keymatch ARGS(( char* str, char* keyword, int minchars ));


/* Log base two hacks. */

EXPCL_PANDA int pm_maxvaltobits( int maxval );
EXPCL_PANDA int pm_bitstomaxval( int bits );


/* Error handling definitions. */

EXPCL_PANDA void pm_message( char*, ... );
EXPCL_PANDA void pm_error( char*, ... );			/* doesn't return */
void pm_perror ARGS(( char* reason ));			/* doesn't return */
void pm_usage ARGS(( char* usage ));			/* doesn't return */


/* File open/close that handles "-" as stdin and checks errors. */

EXPCL_PANDA FILE* pm_openr( char* name );
EXPCL_PANDA FILE* pm_openw( char* name );
EXPCL_PANDA void pm_close( FILE* f );


/* Endian I/O. */

EXPCL_PANDA int pm_readbigshort( FILE* in, short* sP );
EXPCL_PANDA int pm_writebigshort( FILE* out, short s );
EXPCL_PANDA int pm_readbiglong( FILE* in, long* lP );
EXPCL_PANDA int pm_writebiglong( FILE* out, long l );
EXPCL_PANDA int pm_readlittleshort( FILE* in, short* sP );
EXPCL_PANDA int pm_writelittleshort( FILE* out, short s );
EXPCL_PANDA int pm_readlittlelong( FILE* in, long* lP );
EXPCL_PANDA int pm_writelittlelong( FILE* out, long l );


/* Compatibility stuff */

#ifdef NEED_STRSTR
char *strstr ARGS((char *s1, char *s2));
#endif

#if defined(NEED_VFPRINTF1) || defined(NEED_VFPRINTF2)
int vfprintf ARGS(( FILE* stream, char* format, va_list args ));
#endif /*NEED_VFPRINTF*/


#endif /*_PBMPLUS_H_*/
