#ifndef lint
static char rcsid[] = "$Header$";
#endif

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992 Sam Leffler
 * Copyright (c) 1991, 1992 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library MSDOS-specific Routines.
 */
#include "tiffiop.h"
#if defined(__WATCOMC__) || defined(__BORLANDC__)
#include <io.h>         /* for open, close, etc. function prototypes */
#endif

static tsize_t 
_tiffReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
        return (read((int) fd, buf, size));
}

static tsize_t
_tiffWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
        return (write((int) fd, buf, size));
}

static toff_t
_tiffSeekProc(thandle_t fd, toff_t off, int whence)
{
        return (lseek((int) fd, (off_t) off, whence));
}

static int
_tiffCloseProc(thandle_t fd)
{
        return (close((int) fd));
}

#include <sys/stat.h>

static toff_t
_tiffSizeProc(thandle_t fd)
{
        struct stat sb;
        return (fstat((int) fd, &sb) < 0 ? 0 : sb.st_size);
}

static int
_tiffMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
        return (0);
}

static void
_tiffUnmapProc(thandle_t fd, tdata_t base, toff_t size)
{
}

/*
 * Open a TIFF file descriptor for read/writing.
 */
TIFF*
TIFFFdOpen(int fd, const char* name, const char* mode)
{
        TIFF* tif;

        tif = TIFFClientOpen(name, mode,
            (void*) fd,
            _tiffReadProc, _tiffWriteProc, _tiffSeekProc, _tiffCloseProc,
            _tiffSizeProc, _tiffMapProc, _tiffUnmapProc);
        if (tif)
                tif->tif_fd = fd;
        return (tif);
}

/*
 * Open a TIFF file for read/writing.
 */
TIFF*
TIFFOpen(const char* name, const char* mode)
{
        static const char module[] = "TIFFOpen";
        int m, fd;

        m = _TIFFgetMode(mode, module);
        if (m == -1)
                return ((TIFF*)0);
        fd = open(name, m|O_BINARY, 0666);
        if (fd < 0) {
                TIFFError(module, "%s: Cannot open", name);
                return ((TIFF*)0);
        }
        return (TIFFFdOpen(fd, name, mode));
}

#ifdef __GNUC__
extern  char *malloc();
extern  char *realloc();
#else
#include <malloc.h>
#endif

void *
_TIFFmalloc(size_t s)
{
        return (malloc(s));
}

void
_TIFFfree(void* p)
{
        free(p);
}

void *
_TIFFrealloc(void* p, size_t s)
{
        return (realloc(p, s));
}
