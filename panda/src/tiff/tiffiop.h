/* $Header$ */

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

#ifndef _TIFFIOP_
#define _TIFFIOP_
/*
 * ``Library-private'' definitions.
 */
#include <pandabase.h>

#include "tiffconf.h"
#include "tiffcomp.h"
#include "tiffio.h"

/*
 * Internal format of a TIFF directory entry.
 */
typedef struct {
#define FIELD_SETLONGS  2
        /* bit vector of fields that are set */
        u_long  td_fieldsset[FIELD_SETLONGS];

        uint32  td_imagewidth, td_imagelength, td_imagedepth;
        uint32  td_tilewidth, td_tilelength, td_tiledepth;
        uint16  td_subfiletype;
        uint16  td_bitspersample;
        uint16  td_sampleformat;
        uint16  td_compression;
        uint16  td_photometric;
        uint16  td_threshholding;
        uint16  td_fillorder;
        uint16  td_orientation;
        uint16  td_samplesperpixel;
        uint16  td_predictor;
        uint32  td_rowsperstrip;
        u_long  td_minsamplevalue, td_maxsamplevalue;   /* XXX */
        float   td_xresolution, td_yresolution;
        uint16  td_resolutionunit;
        uint16  td_planarconfig;
        float   td_xposition, td_yposition;
        uint32  td_group3options;
        uint32  td_group4options;
        uint16  td_pagenumber[2];
        uint16  td_cleanfaxdata;
        uint16  td_badfaxrun;
        uint32  td_badfaxlines;
        uint16  *td_colormap[3];
        uint16  td_halftonehints[2];
        uint16  td_extrasamples, *td_sampleinfo;
        char    *td_documentname;
        char    *td_artist;
        char    *td_datetime;
        char    *td_hostcomputer;
        char    *td_imagedescription;
        char    *td_make;
        char    *td_model;
        char    *td_software;
        char    *td_pagename;
        tstrip_t td_stripsperimage;
        tstrip_t td_nstrips;            /* size of offset & bytecount arrays */
        uint32  *td_stripoffset;
        uint32  *td_stripbytecount;
#ifdef YCBCR_SUPPORT
        float   *td_ycbcrcoeffs;
        uint16  td_ycbcrsubsampling[2];
        uint16  td_ycbcrpositioning;
#endif
#ifdef JPEG_SUPPORT
        uint16  td_jpegproc;
        uint16  td_jpegrestartinterval;
        u_char  **td_qtab;
        u_char  **td_dctab;
        u_char  **td_actab;
#endif
#ifdef COLORIMETRY_SUPPORT
        float   *td_whitepoint;
        float   *td_primarychromas;
        float   *td_refblackwhite;
        uint16  *td_transferfunction[3];
#endif
#ifdef CMYK_SUPPORT
        uint16  td_inkset;
        uint16  td_dotrange[2];
        char    *td_inknames;
        char    *td_targetprinter;
#endif
} TIFFDirectory;

/*
 * Field flags used to indicate fields that have
 * been set in a directory, and to reference fields
 * when manipulating a directory.
 */
/* multi-entry fields */
#define FIELD_IMAGEDIMENSIONS           0
#define FIELD_TILEDIMENSIONS            1
#define FIELD_CELLDIMENSIONS            2               /* XXX */
#define FIELD_RESOLUTION                3
#define FIELD_POSITION                  4
/* single-entry fields */
#define FIELD_SUBFILETYPE               5
#define FIELD_BITSPERSAMPLE             6
#define FIELD_COMPRESSION               7
#define FIELD_PHOTOMETRIC               8
#define FIELD_THRESHHOLDING             9
#define FIELD_FILLORDER                 10
#define FIELD_DOCUMENTNAME              11
#define FIELD_IMAGEDESCRIPTION          12
#define FIELD_MAKE                      13
#define FIELD_MODEL                     14
#define FIELD_ORIENTATION               15
#define FIELD_SAMPLESPERPIXEL           16
#define FIELD_ROWSPERSTRIP              17
#define FIELD_MINSAMPLEVALUE            18
#define FIELD_MAXSAMPLEVALUE            19
#define FIELD_PLANARCONFIG              20
#define FIELD_PAGENAME                  21
#define FIELD_GROUP3OPTIONS             22
#define FIELD_GROUP4OPTIONS             23
#define FIELD_RESOLUTIONUNIT            24
#define FIELD_PAGENUMBER                25
#define FIELD_STRIPBYTECOUNTS           26
#define FIELD_STRIPOFFSETS              27
#define FIELD_COLORMAP                  28
#define FIELD_PREDICTOR                 29
#define FIELD_ARTIST                    30
#define FIELD_DATETIME                  31
#define FIELD_HOSTCOMPUTER              32
#define FIELD_SOFTWARE                  33
#define FIELD_EXTRASAMPLES              34
#define FIELD_BADFAXLINES               35
#define FIELD_CLEANFAXDATA              36
#define FIELD_BADFAXRUN                 37
#define FIELD_SAMPLEFORMAT              38
#define FIELD_SMINSAMPLEVALUE           39
#define FIELD_SMAXSAMPLEVALUE           40
#define FIELD_IMAGEDEPTH                41
#define FIELD_TILEDEPTH                 42
#define FIELD_HALFTONEHINTS             43
#ifdef YCBCR_SUPPORT
#define FIELD_YCBCRCOEFFICIENTS         44
#define FIELD_YCBCRSUBSAMPLING          45
#define FIELD_YCBCRPOSITIONING          46
#endif
#ifdef JPEG_SUPPORT
#define FIELD_JPEGPROC                  47
#define FIELD_JPEGRESTARTINTERVAL       48
#define FIELD_JPEGQTABLES               49
#define FIELD_JPEGDCTABLES              50
#define FIELD_JPEGACTABLES              51
#endif
#ifdef COLORIMETRY_SUPPORT
#define FIELD_REFBLACKWHITE             52
#define FIELD_WHITEPOINT                53
#define FIELD_PRIMARYCHROMAS            54
#define FIELD_TRANSFERFUNCTION          55
#endif
#ifdef CMYK_SUPPORT
#define FIELD_INKSET                    56
#define FIELD_INKNAMES                  57
#define FIELD_DOTRANGE                  58
#define FIELD_TARGETPRINTER             59
#endif
#define FIELD_LAST                      59

#define TIFFExtractData(tif, type, v) \
    ((tif)->tif_header.tiff_magic == TIFF_BIGENDIAN ? \
        ((v) >> (tif)->tif_typeshift[type]) & (tif)->tif_typemask[type] : \
        (v) & (tif)->tif_typemask[type])
#define TIFFInsertData(tif, type, v) \
    ((tif)->tif_header.tiff_magic == TIFF_BIGENDIAN ? \
        ((v) & (tif)->tif_typemask[type]) << (tif)->tif_typeshift[type] : \
        (v) & (tif)->tif_typemask[type])

typedef struct {
        ttag_t  field_tag;              /* field's tag */
        short   field_readcount;        /* read count (-1 for unknown) */
        short   field_writecount;       /* write count (-1 for unknown) */
        TIFFDataType field_type;        /* type of associated data */
        u_short field_bit;              /* bit in fieldsset bit vector */
        u_short field_oktochange;       /* if true, can change while writing */
        char    *field_name;            /* ASCII name */
} TIFFFieldInfo;

#define FIELD_IGNORE    ((u_short)-1)   /* tags processed but ignored */

#define TIFF_ANY        TIFF_NOTYPE     /* for field descriptor searching */
#define TIFF_VARIABLE   -1              /* marker for variable length tags */
#define TIFF_SPP        -2              /* marker for SamplesPerPixel tags */

extern  const TIFFFieldInfo tiffFieldInfo[];/* table of field descriptors */
extern  const int tiffDataWidth[];      /* table of tag datatype widths */

#define BITn(n)                         (((u_long)1L)<<((n)&0x1f))
#define BITFIELDn(tif, n)               ((tif)->tif_dir.td_fieldsset[(n)/32])
#define TIFFFieldSet(tif, field)        (BITFIELDn(tif, field) & BITn(field))
#define TIFFSetFieldBit(tif, field)     (BITFIELDn(tif, field) |= BITn(field))
#define TIFFClrFieldBit(tif, field)     (BITFIELDn(tif, field) &= ~BITn(field))

#define FieldSet(fields, f)             (fields[(f)/32] & BITn(f))
#define ResetFieldBit(fields, f)        (fields[(f)/32] &= ~BITn(f))

/*
 * Typedefs for ``method pointers'' used internally.
 */
typedef unsigned char* tidata_t;        /* internal image data type */

typedef void (*TIFFVoidMethod)(TIFF*);
typedef int (*TIFFBoolMethod)(TIFF*);
typedef int (*TIFFCodeMethod)(TIFF*, tidata_t, tsize_t, tsample_t);
typedef int (*TIFFSeekMethod)(TIFF*, uint32);
typedef void (*TIFFPostMethod)(TIFF*, tidata_t, tsize_t);

struct tiff {
        char*           tif_name;       /* name of open file */
        short           tif_fd;         /* open file descriptor */
        short           tif_mode;       /* open mode (O_*) */
        char            tif_fillorder;  /* natural bit fill order for machine */
        char            tif_options;    /* compression-specific options */
        short           tif_flags;
#define TIFF_DIRTYHEADER        0x1     /* header must be written on close */
#define TIFF_DIRTYDIRECT        0x2     /* current directory must be written */
#define TIFF_BUFFERSETUP        0x4     /* data buffers setup */
#define TIFF_BEENWRITING        0x8     /* written 1+ scanlines to file */
#define TIFF_SWAB               0x10    /* byte swap file information */
#define TIFF_NOBITREV           0x20    /* inhibit bit reversal logic */
#define TIFF_MYBUFFER           0x40    /* my raw data buffer; free on close */
#define TIFF_ISTILED            0x80    /* file is tile, not strip- based */
#define TIFF_MAPPED             0x100   /* file is mapped into memory */
#define TIFF_POSTENCODE         0x200   /* need call to postencode routine */
        toff_t          tif_diroff;     /* file offset of current directory */
        toff_t          tif_nextdiroff; /* file offset of following directory */
        TIFFDirectory   tif_dir;        /* internal rep of current directory */
        TIFFHeader      tif_header;     /* file's header block */
        const int*      tif_typeshift;  /* data type shift counts */
        const long*     tif_typemask;   /* data type masks */
        uint32          tif_row;        /* current scanline */
        tdir_t          tif_curdir;     /* current directory (index) */
        tstrip_t        tif_curstrip;   /* current strip for read/write */
        toff_t          tif_curoff;     /* current offset for read/write */
        toff_t          tif_dataoff;    /* current offset for writing dir */
/* tiling support */
        uint32          tif_col;        /* current column (offset by row too) */
        ttile_t         tif_curtile;    /* current tile for read/write */
        tsize_t         tif_tilesize;   /* # of bytes in a tile */
/* compression scheme hooks */
        TIFFBoolMethod  tif_predecode;  /* pre row/strip/tile decoding */
        TIFFBoolMethod  tif_preencode;  /* pre row/strip/tile encoding */
        TIFFBoolMethod  tif_postencode; /* post row/strip/tile encoding */
        TIFFCodeMethod  tif_decoderow;  /* scanline decoding routine */
        TIFFCodeMethod  tif_encoderow;  /* scanline encoding routine */
        TIFFCodeMethod  tif_decodestrip;/* strip decoding routine */
        TIFFCodeMethod  tif_encodestrip;/* strip encoding routine */
        TIFFCodeMethod  tif_decodetile; /* tile decoding routine */
        TIFFCodeMethod  tif_encodetile; /* tile encoding routine */
        TIFFVoidMethod  tif_close;      /* cleanup-on-close routine */
        TIFFSeekMethod  tif_seek;       /* position within a strip routine */
        TIFFVoidMethod  tif_cleanup;    /* routine called to cleanup state */
        tidata_t        tif_data;       /* compression scheme private data */
/* input/output buffering */
        tsize_t         tif_scanlinesize;/* # of bytes in a scanline */
        tsize_t         tif_scanlineskew;/* scanline skew for reading strips */
        tidata_t        tif_rawdata;    /* raw data buffer */
        tsize_t         tif_rawdatasize;/* # of bytes in raw data buffer */
        tidata_t        tif_rawcp;      /* current spot in raw buffer */
        tsize_t         tif_rawcc;      /* bytes unread from raw buffer */
/* memory-mapped file support */
        tidata_t        tif_base;       /* base of mapped file */
        toff_t          tif_size;       /* size of mapped file region (bytes) */
        TIFFMapFileProc tif_mapproc;    /* map file method */
        TIFFUnmapFileProc tif_unmapproc;/* unmap file method */
/* input/output callback methods */
        thandle_t       tif_clientdata; /* callback parameter */
        TIFFReadWriteProc tif_readproc; /* read method */
        TIFFReadWriteProc tif_writeproc;/* write method */
        TIFFSeekProc    tif_seekproc;   /* lseek method */
        TIFFCloseProc   tif_closeproc;  /* close method */
        TIFFSizeProc    tif_sizeproc;   /* filesize method */
/* post-decoding support */
        TIFFPostMethod  tif_postdecode;/* post decoding routine */
};

#define isTiled(tif)    (((tif)->tif_flags & TIFF_ISTILED) != 0)
#define isMapped(tif)   (((tif)->tif_flags & TIFF_MAPPED) != 0)
#define TIFFReadFile(tif, buf, size) \
        ((*(tif)->tif_readproc)((tif)->tif_clientdata,buf,size))
#define TIFFWriteFile(tif, buf, size) \
        ((*(tif)->tif_writeproc)((tif)->tif_clientdata,buf,size))
#define TIFFSeekFile(tif, off, whence) \
        ((*(tif)->tif_seekproc)((tif)->tif_clientdata,off,whence))
#define TIFFCloseFile(tif) \
        ((*(tif)->tif_closeproc)((tif)->tif_clientdata))
#define TIFFGetFileSize(tif) \
        ((*(tif)->tif_sizeproc)((tif)->tif_clientdata))
#define TIFFMapFileContents(tif, paddr, psize) \
        ((*(tif)->tif_mapproc)((tif)->tif_clientdata,paddr,psize))
#define TIFFUnmapFileContents(tif, addr, size) \
        ((*(tif)->tif_unmapproc)((tif)->tif_clientdata,addr,size))

/*
 * Default Read/Seek/Write definitions.
 */
#ifndef ReadOK
#define ReadOK(tif, buf, size) \
        (TIFFReadFile(tif, (tdata_t) buf, (tsize_t) size) == size)
#endif
#ifndef SeekOK
#define SeekOK(tif, off) \
        (TIFFSeekFile(tif, (toff_t) off, L_SET) == (toff_t) off)
#endif
#ifndef WriteOK
#define WriteOK(tif, buf, size) \
        (TIFFWriteFile(tif, (tdata_t) buf, (tsize_t) size) == (tsize_t) size)
#endif

/* generic option bit names */
#define TIFF_OPT0       0x1
#define TIFF_OPT1       0x2
#define TIFF_OPT2       0x4
#define TIFF_OPT3       0x8
#define TIFF_OPT4       0x10
#define TIFF_OPT5       0x20
#define TIFF_OPT6       0x40
#define TIFF_OPT7       0x80

/* NB: the u_long casts are to silence certain ANSI-C compilers */
#ifdef howmany
#undef howmany
#endif
#define howmany(x, y)   ((((u_long)(x))+(((u_long)(y))-1))/((u_long)(y)))
#ifdef roundup
#undef roundup
#endif
#define roundup(x, y)   (howmany(x,y)*((u_long)(y)))

#if defined(__cplusplus)
extern "C" {
#endif
extern  void* _TIFFmalloc(size_t);
extern  void* _TIFFrealloc(void*, size_t);
extern  void _TIFFfree(void*);
extern  int _TIFFgetMode(const char* mode, const char* module);
extern  const TIFFFieldInfo *TIFFFindFieldInfo(ttag_t, TIFFDataType);
extern  const TIFFFieldInfo *TIFFFieldWithTag(ttag_t);
extern  void _TIFFgetfield(TIFFDirectory*, ttag_t, ...);
extern  int TIFFNoRowEncode(TIFF*, tidata_t, tsize_t, tsample_t);
extern  int TIFFNoStripEncode(TIFF*, tidata_t, tsize_t, tsample_t);
extern  int TIFFNoTileEncode(TIFF*, tidata_t, tsize_t, tsample_t);
extern  int TIFFNoRowDecode(TIFF*, tidata_t, tsize_t, tsample_t);
extern  int TIFFNoStripDecode(TIFF*, tidata_t, tsize_t, tsample_t);
extern  int TIFFNoTileDecode(TIFF*, tidata_t, tsize_t, tsample_t);
extern  void TIFFNoPostDecode(TIFF*, tidata_t, tsize_t);
extern  void TIFFSwab16BitData(TIFF*, tidata_t, tsize_t);
extern  void TIFFSwab32BitData(TIFF*, tidata_t, tsize_t);
extern  int TIFFFlushData1(TIFF*);
extern  void TIFFFreeDirectory(TIFF*);
extern  int TIFFDefaultDirectory(TIFF*);
extern  int TIFFSetCompressionScheme(TIFF *, int);

extern  int TIFFInitDumpMode(TIFF*);
#ifdef PACKBITS_SUPPORT
extern  int TIFFInitPackBits(TIFF*);
#endif
#ifdef CCITT_SUPPORT
extern  int TIFFInitCCITTRLE(TIFF*), TIFFInitCCITTRLEW(TIFF*);
extern  int TIFFInitCCITTFax3(TIFF*), TIFFInitCCITTFax4(TIFF*);
#endif
#ifdef THUNDER_SUPPORT
extern  int TIFFInitThunderScan(TIFF*);
#endif
#ifdef NEXT_SUPPORT
extern  int TIFFInitNeXT(TIFF*);
#endif
#ifdef LZW_SUPPORT
extern  int TIFFInitLZW(TIFF*);
#endif
#ifdef JPEG_SUPPORT
extern  int TIFFInitJPEG(TIFF*);
#endif
#if defined(__cplusplus)
}
#endif
#endif /* _TIFFIOP_ */
