/* libpbm1.c - pbm utility library part 1
**
** Copyright (C) 1988 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#include <pandabase.h>

#include "pbm.h"
#include "version.h"
#include "compile.h"
#include "libpbm.h"
#if defined(__STDC__) || defined(WIN32_VC)
#include <stdarg.h>
#else /*__STDC__*/
#include <varargs.h>
#endif /*__STDC__*/

/* Assume we have strerror() available. */
#define A_STRERROR


/* Variable-sized arrays. */

char*
pm_allocrow( cols, size )
    int cols;
    int size;
    {
    register char* itrow;

    itrow = (char*) malloc( cols * size );
    if ( itrow == (char*) 0 )
        pm_error( "out of memory allocating a row" );
    return itrow;
    }

void
pm_freerow( itrow )
    char* itrow;
    {
    free( itrow );
    }


#ifndef A_FRAGARRAY
char**
pm_allocarray( cols, rows, size )
    int cols, rows;
    int size;
    {
    char** its;
    int i;

    its = (char**) malloc( rows * sizeof(char*) );
    if ( its == (char**) 0 )
        pm_error( "out of memory allocating an array" );
    its[0] = (char*) malloc( rows * cols * size );
    if ( its[0] == (char*) 0 )
        pm_error( "out of memory allocating an array" );
    for ( i = 1; i < rows; ++i )
        its[i] = &(its[0][i * cols * size]);
    return its;
    }

void
pm_freearray( its, rows )
    char** its;
    int rows;
    {
    free( its[0] );
    free( its );
    }
#else /* A_FRAGARRAY */
char**
pm_allocarray( cols, rows, size )
    int cols, rows;
    int size;
    {
    char** its;
    int i;
    its = (char**) malloc( (rows + 1) * sizeof(char*) );
    if ( its == (char**) 0 )
        pm_error( "out of memory allocating an array" );
    its[rows] = its[0] = (char*) malloc( rows * cols * size );
    if ( its[0] != (char*) 0 )
        for ( i = 1; i < rows; ++i )
            its[i] = &(its[0][i * cols * size]);
    else
        for( i = 0; i < rows; ++i )
            its[i] = pm_allocrow( cols, size );
    return its;
    }
void
pm_freearray( its, rows )
    char** its;
    int rows;
    {
    int i;
    if( its[rows] != (char*) 0 )
        free( its[rows] );
    else
        for( i = 0; i < rows; ++i )
            pm_freerow( its[i] );
    free( its );
    }
#endif /* A_FRAGARRAY */


/* Case-insensitive keyword matcher. */

int
pm_keymatch( str, keyword, minchars )
    char* str;
    char* keyword;
    int minchars;
    {
    register int len;

    len = strlen( str );
    if ( len < minchars )
        return 0;
    while ( --len >= 0 )
        {
        register char c1, c2;

        c1 = *str++;
        c2 = *keyword++;
        if ( c2 == '\0' )
            return 0;
        if ( isupper( c1 ) )
            c1 = tolower( c1 );
        if ( isupper( c2 ) )
            c1 = tolower( c2 );
        if ( c1 != c2 )
            return 0;
        }
    return 1;
    }


/* Log base two hacks. */

int
pm_maxvaltobits( maxval )
    int maxval;
    {
    if ( maxval <= 1 )
        return 1;
    else if ( maxval <= 3 )
        return 2;
    else if ( maxval <= 7 )
        return 3;
    else if ( maxval <= 15 )
        return 4;
    else if ( maxval <= 31 )
        return 5;
    else if ( maxval <= 63 )
        return 6;
    else if ( maxval <= 127 )
        return 7;
    else if ( maxval <= 255 )
        return 8;
    else if ( maxval <= 511 )
        return 9;
    else if ( maxval <= 1023 )
        return 10;
    else if ( maxval <= 2047 )
        return 11;
    else if ( maxval <= 4095 )
        return 12;
    else if ( maxval <= 8191 )
        return 13;
    else if ( maxval <= 16383 )
        return 14;
    else if ( maxval <= 32767 )
        return 15;
    else if ( (long) maxval <= 65535L )
        return 16;
    else
        pm_error( "maxval of %d is too large!", maxval );
    return 0;  /* just to make compiler happy; can't get here. */
    }

int
pm_bitstomaxval( bits )
    int bits;
    {
    return ( 1 << bits ) - 1;
    }


/* Initialization. */

static char* progname;
static int showmessages;

void
pm_init( argcP, argv )
    int* argcP;
    char* argv[];
    {
    int argn, i;
#ifdef A_RGBENV
    char *rgbdef;
#endif /* A_RGBENV */

#ifndef VMS
    /* Extract program name. */
    progname = rindex( argv[0], '/');
#else
{   char **temp_argv = argv;
    int old_argc = *argcP;
    int i;
    getredirection( argcP, &temp_argv );
    if (*argcP > old_argc) {
        /* Number of command line arguments has increased */
        fprintf( stderr, "Sorry!! getredirection() for VMS has changed the argument list!!!\n");
        fprintf( stderr, "This is intolerable at the present time, so we must stop!!!\n");
        exit(1);
    }
    for (i=0; i<*argcP; i++)
        argv[i] = temp_argv[i];
    }
    if ( progname == NULL ) progname = rindex( argv[0], ']');
    if ( progname == NULL ) progname = rindex( argv[0], '>');
#endif
    if ( progname == NULL )
        progname = argv[0];
    else
        ++progname;

    /* Check for any global args. */
    showmessages = 1;
    for ( argn = 1; argn < *argcP; ++argn )
        {
        if ( pm_keymatch( argv[argn], "-quiet", 6 ) )
            {
            showmessages = 0;
            }
        else if ( pm_keymatch( argv[argn], "-version", 7 ) )
            {
            pm_message( "Version: %s", PBMPLUS_VERSION );
#if defined(COMPILE_TIME) && defined(COMPILED_BY)
            pm_message( "Compiled %s by user \"%s\"",
                       COMPILE_TIME, COMPILED_BY );
#endif
#ifdef BSD
            pm_message( "BSD defined" );
#endif /*BSD*/
#ifdef SYSV
#ifdef VMS
            pm_message( "VMS & SYSV defined" );
#else
            pm_message( "SYSV defined" );
#endif
#endif /*SYSV*/
#ifdef MSDOS
            pm_message( "MSDOS defined" );
#endif /*MSDOS*/
#ifdef AMIGA
            pm_message( "AMIGA defined" );
#endif /* AMIGA */
#ifdef PBMPLUS_RAWBITS
            pm_message( "PBMPLUS_RAWBITS defined" );
#endif /*PBMPLUS_RAWBITS*/
#ifdef PBMPLUS_BROKENPUTC1
            pm_message( "PBMPLUS_BROKENPUTC1 defined" );
#endif /*PBMPLUS_BROKENPUTC1*/
#ifdef PBMPLUS_BROKENPUTC2
            pm_message( "PBMPLUS_BROKENPUTC2 defined" );
#endif /*PBMPLUS_BROKENPUTC2*/
#ifdef PGM_BIGGRAYS
            pm_message( "PGM_BIGGRAYS defined" );
#endif /*PGM_BIGGRAYS*/
#ifdef PPM_PACKCOLORS
            pm_message( "PPM_PACKCOLORS defined" );
#endif /*PPM_PACKCOLORS*/
#ifdef DEBUG
            pm_message( "DEBUG defined" );
#endif /*DEBUG*/
#ifdef NEED_VFPRINTF1
            pm_message( "NEED_VFPRINTF1 defined" );
#endif /*NEED_VFPRINTF1*/
#ifdef NEED_VFPRINTF2
            pm_message( "NEED_VFPRINTF2 defined" );
#endif /*NEED_VFPRINTF2*/
#ifdef RGB_DB
#ifndef A_RGBENV
            pm_message( "RGB_DB=\"%s\"", RGB_DB );
#else /* A_RGBENV */
            if( rgbdef = getenv(RGB_DB) )
                pm_message( "RGB_DB= Env-var %s (set to \"%s\")", RGB_DB, rgbdef );
            else
                pm_message( "RGB_DB= Env-var %s (unset)", RGB_DB );
#endif /* A_RGBENV */
#endif /*RGB_DB*/
#ifdef LIBTIFF
            pm_message( "LIBTIFF defined" );
#endif /*LIBTIFF*/
            exit( 0 );
            }
        else
            continue;
        for ( i = argn + 1; i <= *argcP; ++i )
            argv[i - 1] = argv[i];
        --(*argcP);
        }
    }

void
pbm_init( argcP, argv )
    int* argcP;
    char* argv[];
    {
    pm_init( argcP, argv );
    }


/* Error handling. */

void
pm_usage( usage )
    char* usage;
    {
    fprintf( stderr, "usage:  %s %s\n", progname, usage );
    exit( 1 );
    }

void
pm_perror( reason )
    char* reason;
    {
#ifndef A_STRERROR
    extern char* sys_errlist[];
#endif /* A_STRERROR */
    extern int errno;
    char* e;

#ifdef A_STRERROR
    e = strerror(errno);
#else /* A_STRERROR */
    e = sys_errlist[errno];
#endif /* A_STRERROR */

    if ( reason != 0 && reason[0] != '\0' )
        pm_error( "%s - %s", reason, e );
    else
        pm_error( "%s", e );
    }

#if defined(__STDC__) || defined(WIN32_VC)
void
pm_message( char* format, ... )
    {
    va_list args;

    va_start( args, format );
#else /*__STDC__*/
/*VARARGS1*/
void
pm_message( va_alist )
    va_dcl
    { /*}*/
    va_list args;
    char* format;

    va_start( args );
    format = va_arg( args, char* );
#endif /*__STDC__*/

    if ( showmessages )
        {
        fprintf( stderr, "%s: ", progname );
        (void) vfprintf( stderr, format, args );
        fputc( '\n', stderr );
        }
    va_end( args );
    }

#if defined(__STDC__) || defined(WIN32_VC)
void
pm_error( char* format, ... )
    {
    va_list args;

    va_start( args, format );
#else /*__STDC__*/
/*VARARGS1*/
void
pm_error( va_alist )
    va_dcl
    { /*}*/
    va_list args;
    char* format;

    va_start( args );
    format = va_arg( args, char* );
#endif /*__STDC__*/

    fprintf( stderr, "%s: ", progname );
    (void) vfprintf( stderr, format, args );
    fputc( '\n', stderr );
    va_end( args );
    exit( 1 );
    }

#ifdef NEED_VFPRINTF1

/* Micro-vfprintf, for systems that don't have vfprintf but do have _doprnt.
*/

int
vfprintf( stream, format, args )
    FILE* stream;
    char* format;
    va_list args;
    {
    return _doprnt( format, args, stream );
    }
#endif /*NEED_VFPRINTF1*/

#ifdef NEED_VFPRINTF2

/* Portable mini-vfprintf, for systems that don't have either vfprintf or
** _doprnt.  This depends only on fprintf.  If you don't have fprintf,
** you might consider getting a new stdio library.
*/

int
vfprintf( stream, format, args )
    FILE* stream;
    char* format;
    va_list args;
    {
    int n;
    char* ep;
    char fchar;
    char tformat[512];
    int do_long;
    int i;
    long l;
    unsigned u;
    unsigned long ul;
    char* s;
    double d;

    n = 0;
    while ( *format != '\0' )
        {
        if ( *format != '%' )
            { /* Not special, just write out the char. */
            (void) putc( *format, stream );
            ++n;
            ++format;
            }
        else
            {
            do_long = 0;
            ep = format + 1;

            /* Skip over all the field width and precision junk. */
            if ( *ep == '-' )
                ++ep;
            if ( *ep == '0' )
                ++ep;
            while ( isdigit( *ep ) )
                ++ep;
            if ( *ep == '.' )
                {
                ++ep;
                while ( isdigit( *ep ) )
                    ++ep;
                }
            if ( *ep == '#' )
                ++ep;
            if ( *ep == 'l' )
                {
                do_long = 1;
                ++ep;
                }

            /* Here's the field type.  Extract it, and copy this format
            ** specifier to a temp string so we can add an end-of-string.
            */
            fchar = *ep;
            (void) strncpy( tformat, format, ep - format + 1 );
            tformat[ep - format + 1] = '\0';

            /* Now do a one-argument fprintf with the format string we have
            ** isolated.
            */
            switch ( fchar )
                {
                case 'd':
                if ( do_long )
                    {
                    l = va_arg( args, long );
                    n += fprintf( stream, tformat, l );
                    }
                else
                    {
                    i = va_arg( args, int );
                    n += fprintf( stream, tformat, i );
                    }
                break;

                case 'o':
                case 'x':
                case 'X':
                case 'u':
                if ( do_long )
                    {
                    ul = va_arg( args, unsigned long );
                    n += fprintf( stream, tformat, ul );
                    }
                else
                    {
                    u = va_arg( args, unsigned );
                    n += fprintf( stream, tformat, u );
                    }
                break;

                case 'c':
                i = (char) va_arg( args, int );
                n += fprintf( stream, tformat, i );
                break;

                case 's':
                s = va_arg( args, char* );
                n += fprintf( stream, tformat, s );
                break;

                case 'e':
                case 'E':
                case 'f':
                case 'g':
                case 'G':
                d = va_arg( args, double );
                n += fprintf( stream, tformat, d );
                break;

                case '%':
                (void) putc( '%', stream );
                ++n;
                break;

                default:
                return -1;
                }

            /* Resume formatting on the next character. */
            format = ep + 1;
            }
        }
    return nc;
    }
#endif /*NEED_VFPRINTF2*/

#ifdef NEED_STRSTR
/* for systems which do not provide strstr */
char*
strstr(s1, s2)
    char *s1, *s2;
{
    int ls2 = strlen(s2);

    if (ls2 == 0)
        return (s1);
    while (strlen(s1) >= ls2) {
        if (strncmp(s1, s2, ls2) == 0)
            return (s1);
        s1++;
    }
    return (0);
}

#endif /*NEED_STRSTR*/


/* File open/close that handles "-" as stdin and checks errors. */

FILE*
pm_openr( name )
    char* name;
    {
    FILE* f;

    if ( strcmp( name, "-" ) == 0 )
        f = stdin;
    else
        {
#ifndef VMS
        f = fopen( name, "rb" );
#else
        f = fopen ( name, "r", "ctx=stm" );
#endif
        if ( f == NULL )
            {
            pm_perror( name );
            exit( 1 );
            }
        }
    return f;
    }

FILE*
pm_openw( name )
    char* name;
    {
    FILE* f;

#ifndef VMS
    f = fopen( name, "wb" );
#else
    f = fopen ( name, "w", "mbc=32", "mbf=2" );  /* set buffer factors */
#endif
    if ( f == NULL )
        {
        pm_perror( name );
        exit( 1 );
        }
    return f;
    }

void
pm_close( f )
    FILE* f;
    {
    fflush( f );
    if ( ferror( f ) )
        pm_message( "a file read or write error occurred at some point" );
    if ( f != stdin )
        if ( fclose( f ) != 0 )
            pm_perror( "fclose" );
    }

/* Endian I/O.
*/

int
pm_readbigshort( in, sP )
    FILE* in;
    short* sP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *sP = ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
        return -1;
    *sP |= c & 0xff;
    return 0;
    }

#if __STDC__
int
pm_writebigshort( FILE* out, short s )
#else /*__STDC__*/
int
pm_writebigshort( out, s )
    FILE* out;
    short s;
#endif /*__STDC__*/
    {
    (void) putc( ( s >> 8 ) & 0xff, out );
    (void) putc( s & 0xff, out );
    return 0;
    }

int
pm_readbiglong( in, lP )
    FILE* in;
    long* lP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *lP = ( c & 0xff ) << 24;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 16;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= c & 0xff;
    return 0;
    }

int
pm_writebiglong( out, l )
    FILE* out;
    long l;
    {
    (void) putc( ( l >> 24 ) & 0xff, out );
    (void) putc( ( l >> 16 ) & 0xff, out );
    (void) putc( ( l >> 8 ) & 0xff, out );
    (void) putc( l & 0xff, out );
    return 0;
    }

int
pm_readlittleshort( in, sP )
    FILE* in;
    short* sP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *sP = c & 0xff;
    if ( (c = getc( in )) == EOF )
        return -1;
    *sP |= ( c & 0xff ) << 8;
    return 0;
    }

#if defined(__STDC__) || defined(WIN32_VC)
int
pm_writelittleshort( FILE* out, short s )
#else /*__STDC__*/
int
pm_writelittleshort( out, s )
    FILE* out;
    short s;
#endif /*__STDC__*/
    {
    (void) putc( s & 0xff, out );
    (void) putc( ( s >> 8 ) & 0xff, out );
    return 0;
    }

int
pm_readlittlelong( in, lP )
    FILE* in;
    long* lP;
    {
    int c;

    if ( (c = getc( in )) == EOF )
        return -1;
    *lP = c & 0xff;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 8;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 16;
    if ( (c = getc( in )) == EOF )
        return -1;
    *lP |= ( c & 0xff ) << 24;
    return 0;
    }

int
pm_writelittlelong( out, l )
    FILE* out;
    long l;
    {
    (void) putc( l & 0xff, out );
    (void) putc( ( l >> 8 ) & 0xff, out );
    (void) putc( ( l >> 16 ) & 0xff, out );
    (void) putc( ( l >> 24 ) & 0xff, out );
    return 0;
    }


/* Read a file of unknown size to a buffer. Return the number of bytes
   read. Allocate more memory as we need it. The calling routine has
   to free() the buffer.

   Oliver Trepte, oliver@fysik4.kth.se, 930613 */

#define PM_BUF_SIZE 16384      /* First try this size of the buffer, then
                                   double this until we reach PM_MAX_BUF_INC */
#define PM_MAX_BUF_INC 65536   /* Don't allocate more memory in larger blocks
                                   than this. */

char *pm_read_unknown_size( file, nread )
    FILE* file;
    long* nread;
{
    long nalloc;
    register int val;
    char* buf;

    *nread = 0;
    if ((buf=malloc(PM_BUF_SIZE)) == NULL)
        pm_error("Cannot allocate memory");
    nalloc = PM_BUF_SIZE;

    while(1) {
        if (*nread >= nalloc) { /* We need a larger buffer */
            if (nalloc > PM_MAX_BUF_INC)
                nalloc += PM_MAX_BUF_INC;
            else
                nalloc += nalloc;
            if ((buf=realloc(buf, nalloc)) == NULL)
                pm_error("Cannot allocate %d bytes of memory", nalloc);
        }

        if ((val = getc(file)) == EOF)
            return (buf);

        buf[(*nread)++] = val;
    }
}

/*****************************************************************************/

#ifdef VMS
/*
 * @(#)argproc.c 1.0 89/02/01           Mark Pizzolato (mark@infopiz.uucp)
 */

#ifndef lint
char argproc_version[] = "@(#)argproc.c VMS uucp Version infopiz-1.0";
#endif

#include "includes.h"           /* System include files, system dependent */


/*
 * getredirection() is intended to aid in porting C programs
 * to VMS (Vax-11 C) which does not support '>' and '<'
 * I/O redirection, along with a command line pipe mechanism
 * using the '|' AND background command execution '&'.
 * The piping mechanism will probably work with almost any 'filter' type
 * of program.  With suitable modification, it may useful for other
 * portability problems as well.
 *
 * Author:  Mark Pizzolato      mark@infopiz.UUCP
 */
struct list_item
    {
    struct list_item *next;
    char *value;
    };

int
getredirection(ac, av)
int             *ac;
char            ***av;
/*
 * Process vms redirection arg's.  Exit if any error is seen.
 * If getredirection() processes an argument, it is erased
 * from the vector.  getredirection() returns a new argc and argv value.
 * In the event that a background command is requested (by a trailing "&"),
 * this routine creates a background subprocess, and simply exits the program.
 *
 * Warning: do not try to simplify the code for vms.  The code
 * presupposes that getredirection() is called before any data is
 * read from stdin or written to stdout.
 *
 * Normal usage is as follows:
 *
 *      main(argc, argv)
 *      int             argc;
 *      char            *argv[];
 *      {
 *              getredirection(&argc, &argv);
 *      }
 */
{
    int                 argc = *ac;     /* Argument Count         */
    char                **argv = *av;   /* Argument Vector        */
    char                *ap;            /* Argument pointer       */
    int                 j;              /* argv[] index           */
    extern int          errno;          /* Last vms i/o error     */
    int                 item_count = 0; /* Count of Items in List */
    int                 new_file;       /* flag, true if '>' used */
    struct list_item    *list_head = 0; /* First Item in List       */
    struct list_item    *list_tail;     /* Last Item in List        */
    char                *in = NULL;     /* Input File Name          */
    char                *out = NULL;    /* Output File Name         */
    char                *outmode = "w"; /* Mode to Open Output File */
    int                 cmargc = 0;     /* Piped Command Arg Count  */
    char                **cmargv = NULL;/* Piped Command Arg Vector */
    stat_t              statbuf;        /* fstat buffer             */

    /*
     * First handle the case where the last thing on the line ends with
     * a '&'.  This indicates the desire for the command to be run in a
     * subprocess, so we satisfy that desire.
     */
    ap = argv[argc-1];
    if (0 == strcmp("&", ap))
        exit(background_process(--argc, argv));
    if ('&' == ap[strlen(ap)-1])
        {
        ap[strlen(ap)-1] = '\0';
        exit(background_process(argc, argv));
        }
    /*
     * Now we handle the general redirection cases that involve '>', '>>',
     * '<', and pipes '|'.
     */
    for (new_file = 0, j = 0; j < argc; ++j)
        {
        if (0 == strcmp("<", argv[j]))
            {
            if (j+1 >= argc)
                {
                errno = EINVAL;
                perror("No input file");
                exit(EXIT_ERR);
                }
            in = argv[++j];
            continue;
            }
        if ('<' == *(ap = argv[j]))
            {
            in = 1 + ap;
            continue;
            }
        if (0 == strcmp(">", ap))
            {
            if (j+1 >= argc)
                {
                errno = EINVAL;
                perror("No output file");
                exit(EXIT_ERR);
                }
            out = argv[++j];
            new_file = 1;
            continue;
            }
        if ('>' == *ap)
            {
            if ('>' == ap[1])
                {
                outmode = "a";
                if ('\0' == ap[2])
                    out = argv[++j];
                else
                    out = 2 + ap;
                }
            else
                { out = 1 + ap;  new_file = 1; }
            continue;
            }
        if (0 == strcmp("|", argv[j]))
            {
            if (j+1 >= argc)
                {
                errno = EPIPE;
                perror("No command to Pipe to");
                exit(EXIT_ERR);
                }
            cmargc = argc-(j+1);
            cmargv = &argv[j+1];
            argc = j;
            outmode = "wb";     /* pipes are binary mode devices */
            continue;
            }
        if ('|' == *(ap = argv[j]))
            {
            ++argv[j];
            cmargc = argc-j;
            cmargv = &argv[j];
            argc = j;
            outmode = "wb";     /* pipes are binary mode devices */
            continue;
            }
        expand_wild_cards(ap, &list_head, &list_tail, &item_count);
        }
    /*
     * Allocate and fill in the new argument vector, Some Unix's terminate
     * the list with an extra null pointer.
     */
    argv = *av = calloc(item_count+1, sizeof(char *));
    for (j = 0; j < item_count; ++j, list_head = list_head->next)
        argv[j] = list_head->value;
    *ac = item_count;
    if (cmargv != NULL)
        {
        char subcmd[1024];
        static char *pipe_and_fork();

        if (out != NULL)
            {
            errno = EINVAL;
            perror("Invalid '|' and '>' specified");
            exit(EXIT_ERR);
            }
        strcpy(subcmd, cmargv[0]);
        for (j = 1; j < cmargc; ++j)
            {
            strcat(subcmd, " \"");
            strcat(subcmd, cmargv[j]);
            strcat(subcmd, "\"");
            }
        out = pipe_and_fork(subcmd);
        outmode = "wb";
        }

    /* Check for input from a pipe (mailbox) */

    if(fstat(0, &statbuf) == 0){
        if(strncmp(statbuf.st_dev, "MB", 2) == 0 || 
            strncmp(statbuf.st_dev, "_MB", 3) == 0){

            /* Input from a pipe, reopen it in binary mode to disable   */
            /* carriage control processing.                             */

            if (in != NULL){
                errno = EINVAL;
                perror("Invalid '|' and '<' specified");
                exit(EXIT_ERR);
                }
            freopen(statbuf.st_dev, "rb", stdin);
            }
        }
    else {
        perror("fstat failed");
        exit(EXIT_ERR);
        }

#ifdef __ALPHA
        /*, "mbc=32", "mbf=2"))) blows up on the ALPHA cbm 11/08/92 */
    if ((in != NULL) && (NULL == freopen(in, "r", stdin)))
        {
#else
    if ((in != NULL) && (NULL == freopen(in, "r", stdin, "mbc=32", "mbf=2")))
        {
#endif
        perror(in);             /* Can't find file              */
        exit(EXIT_ERR);         /* Is a fatal error             */
        }
#ifdef __ALPHA
    if ((out != NULL) && (NULL == freopen(out, outmode, stdout)))
        {
#else
    if ((out != NULL) && (NULL == freopen(out, outmode, stdout, "mbc=32", "mbf=2")))
        {
#endif
        perror(ap);             /* Error, can't write or append */
        exit(EXIT_ERR);         /* Is a fatal error             */
        }

     if ( new_file ) {
        /*
         * We are making an explicit output file, fstat the file and
         * declare exit handler to be able change the file to fixed length
         * records if necessary. 
         */
        char fname[256];
        static int outfile_rundown(), check_outfile_filetype();
        static stat_t ofile_stat;
        static struct exit_control_block {
            struct exit_control_block *flink;
            int (*exit_routine)();
            int arg_count;
            int *status_address;        /* arg 1 */
            stat_t *stat;               /* arg 2 */
            int exit_status;
            int skew[128];
        } exit_block = 
            { 0, outfile_rundown, 2, &exit_block.exit_status, &ofile_stat, 0 };

        if ( fstat ( fileno(stdout), &ofile_stat ) == 0 )
             sys$dclexh ( &exit_block );
        else fprintf(stderr,"Error fstating stdout - %s\n",
                strerror(errno,vaxc$errno) );

        if ( fgetname ( stdout, fname, 0 ) ) check_outfile_filetype ( fname );
     }
#ifdef DEBUG
    fprintf(stderr, "Arglist:\n");
    for (j = 0; j < *ac;  ++j)
        fprintf(stderr, "argv[%d] = '%s'\n", j, argv[j]);
#endif
}

static int binary_outfile = 0;
void set_outfile_binary() { binary_outfile = 1; return; }

/*
 * Check if output file should be set to binary (fixed 512) on exit based
 * upon the filetype.
 */
static int check_outfile_filetype ( name )
    char *name;
{
    char *binary_filetypes, *ext, *p, *t;
    binary_filetypes = (char *) getenv ( "PBM_BINARY_FILETYPES" );
    if ( binary_filetypes == NULL ) return 0;

    ext = strchr ( name, '.' );  if ( ext == NULL ) return 0;
    ext++;
    t = strrchr ( name, '.' );   if ( t != NULL ) *t = '\0';

    for ( p = binary_filetypes; *p != '\0'; p++ ) {
        for ( t = p;
              (*p != '\0' ) && (strchr ( ",     ", *p ) == NULL); 
             p++ ) *p = toupper(*p);
        *p = '\0';

        if ( strcmp ( t, ext ) == 0 ) {
            binary_outfile = 1;
            break;
        }
    }
    return binary_outfile;
}


/*
 * Exit handler to set output file to binary on image exit.
 */
static int outfile_rundown ( reason, statbuf )
    int *reason;
    stat_t *statbuf;
{
    int code, channel, status, LIB$GETDVI(), sys$assign(), sys$qiow();
    long int fib_desc[2], devchar;
    short int iosb[4];
    $DESCRIPTOR(device, statbuf->st_dev);
    struct fibdef fib;          /* File information block (XQP) */
    struct atrdef atr[2];
    struct fat {
      unsigned char rtype, ratattrib;
      unsigned short int rsize, hiblk[2], efblk[2], ffbyte, maxrec, defext, gbc;
      unsigned short int reserved[4], versions;
    } rat;

    if ( !binary_outfile ) return 1;    /* leave file alone */
    /*
     * Assign channel to device listed in stat block and test if it is
     * a directory structured device, returning if not.
     */
    device.dsc$w_length = strlen ( statbuf->st_dev );
    status = sys$assign ( &device, &channel, 0, 0 );
    if ((status & 1) == 0) { fprintf(stderr, 
        "assign error to %s (%d)\n", device.dsc$a_pointer, status);
                return status; }
    code = DVI$_DEVCHAR;
    status = LIB$GETDVI ( &code, &channel, 0, &devchar );
    if ((status & 1) == 0) { fprintf(stderr, "getdvi error: %d\n", status);
                return status; }
    if ( (devchar & DEV$M_DIR) == 0 ) return 1;
    /*
     * Build File Information Block and Atrribute block.
     */
#ifdef __ALPHA
    fib.fib$w_fid[0] = statbuf->st_ino[0];
    fib.fib$w_fid[1] = statbuf->st_ino[1];
    fib.fib$w_fid[2] = statbuf->st_ino[2];
#else
    fib.fib$r_fid_overlay.fib$w_fid[0] = statbuf->st_ino[0];
    fib.fib$r_fid_overlay.fib$w_fid[1] = statbuf->st_ino[1];
    fib.fib$r_fid_overlay.fib$w_fid[2] = statbuf->st_ino[2];
#endif

    atr[0].atr$w_size = sizeof(rat);
    atr[0].atr$w_type = ATR$C_RECATTR;
    atr[0].atr$l_addr = &rat;
    atr[1].atr$w_size = 0; atr[1].atr$w_type = 0;
    /*
     * Perform access function with read_attributes sub-function.
     */
    freopen ( "SYS$OUTPUT:", "a", stdout );
    fib_desc[0] = 10; fib_desc[1] = (long int) &fib;
#ifdef __ALPHA
    fib.fib$l_acctl = FIB$M_WRITE;
#else
    fib.fib$r_acctl_overlay.fib$l_acctl = FIB$M_WRITE;
#endif
    status = sys$qiow ( 0, channel, IO$_ACCESS|IO$M_ACCESS,
                 &iosb, 0, 0, &fib_desc, 0, 0, 0, &atr, 0 );
    /*
     * If status successful, update record byte and perform a MODIFY.
     */
    if ( (status&1) == 1 ) status = iosb[0];
    if ( (status&1) == 1 ) {
      rat.rtype = 1;            /* fixed length records */
      rat.rsize = 512;          /* 512 byte block recrods */
      rat.ratattrib = 0;        /* Record attributes: none */

     status = sys$qiow
        ( 0, channel, IO$_MODIFY, &iosb, 0, 0, &fib_desc, 0, 0, 0, &atr, 0 );
       if ( (status&1) == 1 ) status = iosb[0];
   }
   sys$dassgn ( channel );
   if ( (status & 1) == 0 ) fprintf ( stderr,
        "Failed to convert output file to binary format, status: %d\n", status);
   return status;
}


static add_item(head, tail, value, count)
struct list_item **head;
struct list_item **tail;
char *value;
int *count;
{
    if (*head == 0)
        {
        if (NULL == (*head = calloc(1, sizeof(**head))))
            {
            errno = ENOMEM;
            perror("");
            exit(EXIT_ERR);
            }
        *tail = *head;
        }
    else
        if (NULL == ((*tail)->next = calloc(1, sizeof(**head))))
            {
            errno = ENOMEM;
            perror("");
            exit(EXIT_ERR);
            }
        else
            *tail = (*tail)->next;
    (*tail)->value = value;
    ++(*count);
}

static expand_wild_cards(item, head, tail, count)
char *item;
struct ltem_list **head;
struct ltem_list **tail;
int *count;
{
int expcount = 0;
int context = 0;
int status;
int status_value;
int had_version;
$DESCRIPTOR(filespec, item);
$DESCRIPTOR(defaultspec, "SYS$DISK:[]*.*;");
$DESCRIPTOR(resultspec, "");

    if (strcspn(item, "*%") == strlen(item))
        {
        add_item(head, tail, item, count);
        return;
        }
    resultspec.dsc$b_dtype = DSC$K_DTYPE_T;
    resultspec.dsc$b_class = DSC$K_CLASS_D;
    resultspec.dsc$a_pointer = NULL;
    filespec.dsc$w_length = strlen(item);
    /*
     * Only return version specs, if the caller specified a version
     */
    had_version = strchr(item, ';');
    while (1 == (1&lib$find_file(&filespec, &resultspec, &context,
                                 &defaultspec, 0, &status_value, &0)))
        {
        char *string;
        char *c;

        if (NULL == (string = calloc(1, resultspec.dsc$w_length+1)))
            {
            errno = ENOMEM;
            perror("");
            exit(EXIT_ERR);
            }
        strncpy(string, resultspec.dsc$a_pointer, resultspec.dsc$w_length);
        string[resultspec.dsc$w_length] = '\0';
        if (NULL == had_version)
            *((char *)strrchr(string, ';')) = '\0';
        /*
         * Be consistent with what the C RTL has already done to the rest of
         * the argv items and lowercase all of these names.
         */
        for (c = string; *c; ++c)
            if (isupper(*c))
                *c = tolower(*c);
        add_item(head, tail, string, count);
        ++expcount;
        }
    if (expcount == 0)
        add_item(head, tail, item, count);
    lib$sfree1_dd(&resultspec);
    lib$find_file_end(&context);
}

static int child_st[2]; /* Event Flag set when child process completes  */

static short child_chan;/* I/O Channel for Pipe Mailbox                 */

static exit_handler(status)
int *status;
{
short iosb[4];

    if (0 == child_st[0])
        {
#ifdef DEBUG
        fprintf(stderr, "Waiting for Child Process to Finnish . . .\n");
#endif
        fflush(stdout);     /* Have to flush pipe for binary data to    */
                            /* terminate properly -- <tp@mccall.com>    */
#ifdef DEBUG
        fprintf(stderr, "    stdout flushed. . .\n");
#endif
        sys$qio(0, child_chan, IO$_WRITEOF, iosb, 0, 0, 0, 0, 0, 0, 0, 0);
#ifdef DEBUG
        fprintf(stderr, "    Pipe terminated. . .\n");
#endif
        fclose(stdout);
#ifdef DEBUG
        fprintf(stderr, "    stdout closed. . .\n");
#endif
        sys$synch(0, child_st);
        sys$dassgn(child_chan);
        }
#ifdef DEBUG
        fprintf(stderr, "    sync done. . .\n");
#endif
}

#include <syidef>               /* System Information Definitions       */

static sig_child(chan)
int chan;
{
#ifdef DEBUG
    fprintf(stderr, "Child Completion AST, st: %x\n", child_st[0] );
#endif
    if (child_st[0] == 0)
        { child_st[0] = 1; }
    sys$setef ( 0 );
}

static struct exit_control_block
    {
    struct exit_control_block *flink;
    int (*exit_routine)();
    int arg_count;
    int *status_address;
    int exit_status;
    } exit_block =
    {
    0,
    exit_handler,
    1,
    &exit_block.exit_status,
    0
    };

static char *pipe_and_fork(cmd)
char *cmd;
{
    $DESCRIPTOR(cmddsc, cmd);
    static char mbxname[64], ef = 0;
    $DESCRIPTOR(mbxdsc, mbxname);
    short iosb[4];
    int status;
    int pid;
    struct
        {
        short dna_buflen;
        short dna_itmcod;
        char *dna_buffer;
        short *dna_retlen;
        int listend;
        } itmlst =
        {
        sizeof(mbxname),
        DVI$_DEVNAM,
        mbxname,
        &mbxdsc.dsc$w_length,
        0
        };
    int mbxsize;
    struct
        {
        short mbf_buflen;
        short mbf_itmcod;
        int *mbf_maxbuf;
        short *mbf_retlen;
        int listend;
        } syiitmlst =
        {
        sizeof(mbxsize),
        SYI$_MAXBUF,
        &mbxsize,
        0,
        0
        };

    cmddsc.dsc$w_length = strlen(cmd);
    /*
     * Get the SYSGEN parameter MAXBUF, and the smaller of it and 2048 as
     * the size of the 'pipe' mailbox.
     */
    if (1 == (1&(vaxc$errno = sys$getsyiw(0, 0, 0, &syiitmlst, iosb, 0, 0, 0))))
        vaxc$errno = iosb[0];
    if (0 == (1&vaxc$errno))
        {
        errno = EVMSERR;
        perror("Can't get SYSGEN parameter value for MAXBUF");
        exit(EXIT_ERR);
        }
    if (mbxsize > 2048)
        mbxsize = 2048;
    if (0 == (1&(vaxc$errno = sys$crembx(0, &child_chan, mbxsize, mbxsize, 0, 0, 0))))
        {
        errno = EVMSERR;
        perror("Can't create pipe mailbox");
        exit(EXIT_ERR);
        }
    if (1 == (1&(vaxc$errno = sys$getdviw(0, child_chan, 0, &itmlst, iosb,
                                          0, 0, 0))))
        vaxc$errno = iosb[0];
    if (0 == (1&vaxc$errno))
        {
        errno = EVMSERR;
        perror("Can't get pipe mailbox device name");
        exit(EXIT_ERR);
        }
    mbxname[mbxdsc.dsc$w_length] = '\0';
#ifdef DEBUG
    fprintf(stderr, "Pipe Mailbox Name = '%s'\n", mbxname);
#endif
    if (0 == (1&(vaxc$errno = lib$spawn(&cmddsc, &mbxdsc, 0, &1,
                                        0, &pid, child_st, &ef, sig_child,
                                        &child_chan))))
        {
        errno = EVMSERR;
        perror("Can't spawn subprocess");
        exit(EXIT_ERR);
        }
#ifdef DEBUG
    fprintf(stderr, "Subprocess's Pid = %08X\n", pid);
#endif
    sys$dclexh(&exit_block);
    return(mbxname);
}

background_process(argc, argv)
int argc;
char **argv;
{
char command[2048] = "$";
$DESCRIPTOR(value, command);
$DESCRIPTOR(cmd, "BACKGROUND$COMMAND");
$DESCRIPTOR(null, "NLA0:");
int pid;

    strcat(command, argv[0]);
    while (--argc)
        {
        strcat(command, " \"");
        strcat(command, *(++argv));
        strcat(command, "\"");
        }
    value.dsc$w_length = strlen(command);
    if (0 == (1&(vaxc$errno = lib$set_symbol(&cmd, &value))))
        {
        errno = EVMSERR;
        perror("Can't create symbol for subprocess command");
        exit(EXIT_ERR);
        }
    if (0 == (1&(vaxc$errno = lib$spawn(&cmd, &null, 0, &17, 0, &pid))))
        {
        errno = EVMSERR;
        perror("Can't spawn subprocess");
        exit(EXIT_ERR);
        }
#ifdef DEBUG
    fprintf(stderr, "%s\n", command);
#endif
    fprintf(stderr, "%08X\n", pid);
    return(EXIT_OK);
}

/* got this off net.sources */

#ifdef  VMS
#define index   strchr
#endif  /*VMS*/

/*
 * get option letter from argument vector
 */
int     opterr = 1,             /* useless, never set or used */
        optind = 1,             /* index into parent argv vector */
        optopt;                 /* character checked for validity */
char    *optarg;                /* argument associated with option */

#define BADCH   (int)'?'
#define EMSG    ""
#define tell(s) fputs(progname,stderr);fputs(s,stderr); \
                fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);

getopt(nargc,nargv,ostr)
int     nargc;
char    **nargv,
        *ostr;
{
        static char     *place = EMSG;  /* option letter processing */
        register char   *oli;           /* option letter list index */
        char    *index();
        char *progname;

        if(!*place) {                   /* update scanning pointer */
                if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
                if (*place == '-') {    /* found "--" */
                        ++optind;
                        return(EOF);
                }
        }                               /* option letter okay? */
        if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
                if(!*place) ++optind;
#ifdef VMS
                progname = strrchr(nargv[0],']');
#else
                progname = rindex(nargv[0],'/');
#endif
                if (!progname) progname = nargv[0]; else progname++;
                tell(": illegal option -- ");
        }
        if (*++oli != ':') {            /* don't need argument */
                optarg = NULL;
                if (!*place) ++optind;
        }
        else {                          /* need an argument */
                if (*place) optarg = place;     /* no white space */
                else if (nargc <= ++optind) {   /* no arg */
                        place = EMSG;
#ifdef VMS
                        progname = strrchr(nargv[0],']');
#else
                        progname = rindex(nargv[0],'/');
#endif
                        if (!progname) progname = nargv[0]; else progname++;
                        tell(": option requires an argument -- ");
                }
                else optarg = nargv[optind];    /* white space */
                place = EMSG;
                ++optind;
        }
        return(optopt);                 /* dump back option letter */
}
#endif /* VMS */
