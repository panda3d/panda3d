/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypePNM.cxx
 * @author drose
 * @date 1998-04-04
 */

#include "pnmFileTypePNM.h"

#ifdef HAVE_PNM

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

using std::istream;
using std::ostream;
using std::string;

static const char * const extensions_PNM[] = {
  "pbm", "pgm", "ppm", "pnm"
};
static const int num_extensions_PNM = sizeof(extensions_PNM) / sizeof(const char *);

TypeHandle PNMFileTypePNM::_type_handle;

// Some macros lifted from the original Netpbm sources.

#define PBM_MAGIC1 'P'
#define PBM_MAGIC2 '1'
#define RPBM_MAGIC2 '4'
#define PBM_FORMAT (PBM_MAGIC1 * 256 + PBM_MAGIC2)
#define RPBM_FORMAT (PBM_MAGIC1 * 256 + RPBM_MAGIC2)
#define PBM_TYPE PBM_FORMAT

#define PBM_FORMAT_TYPE(f)                                      \
  ((f) == PBM_FORMAT || (f) == RPBM_FORMAT ? PBM_TYPE : -1)

#define PGM_OVERALLMAXVAL 65535

#define PGM_MAGIC1 'P'
#define PGM_MAGIC2 '2'
#define RPGM_MAGIC2 '5'
#define PGM_FORMAT (PGM_MAGIC1 * 256 + PGM_MAGIC2)
#define RPGM_FORMAT (PGM_MAGIC1 * 256 + RPGM_MAGIC2)
#define PGM_TYPE PGM_FORMAT

#define PGM_FORMAT_TYPE(f) ((f) == PGM_FORMAT || (f) == RPGM_FORMAT ? PGM_TYPE : PBM_FORMAT_TYPE(f))

#define PPM_OVERALLMAXVAL PGM_OVERALLMAXVAL
#define PPM_MAXMAXVAL PGM_MAXMAXVAL

#define PPM_MAGIC1 'P'
#define PPM_MAGIC2 '3'
#define RPPM_MAGIC2 '6'
#define PPM_FORMAT (PPM_MAGIC1 * 256 + PPM_MAGIC2)
#define RPPM_FORMAT (PPM_MAGIC1 * 256 + RPPM_MAGIC2)
#define PPM_TYPE PPM_FORMAT

#define PPM_FORMAT_TYPE(f)                                              \
  ((f) == PPM_FORMAT || (f) == RPPM_FORMAT ? PPM_TYPE : PGM_FORMAT_TYPE(f))

#define PNM_OVERALLMAXVAL PPM_OVERALLMAXVAL
#define PNM_GET1(x) PPM_GETB(x)

#define PNM_FORMAT_TYPE(f) PPM_FORMAT_TYPE(f)

typedef unsigned char bit;
#define PBM_WHITE 0
#define PBM_BLACK 1

#define pbm_allocarray(cols, rows)                      \
  ((bit**) pm_allocarray(cols, rows, sizeof(bit)))
#define pbm_allocrow(cols) ((bit*) pm_allocrow(cols, sizeof(bit)))
#define pbm_freearray(bits, rows) pm_freearray((char**) bits, rows)
#define pbm_freerow(bitrow) pm_freerow((char*) bitrow)
#define pbm_packed_bytes(cols) (((cols)+7)/8)
#define pbm_allocrow_packed(cols)                               \
  ((unsigned char *) pm_allocrow(pbm_packed_bytes(cols),        \
                                 sizeof(unsigned char)))
#define pbm_freerow_packed(packed_bits)         \
  pm_freerow((char *) packed_bits)
#define pbm_allocarray_packed(cols, rows) ((unsigned char **)           \
                                           pm_allocarray(pbm_packed_bytes(cols), rows, sizeof(unsigned char)))
#define pbm_freearray_packed(packed_bits, rows) \
  pm_freearray((char **) packed_bits, rows)

#define pgm_allocarray( cols, rows ) ((gray**) pm_allocarray( cols, rows, sizeof(gray) ))
#define pgm_allocrow( cols ) ((gray*) pm_allocrow( cols, sizeof(gray) ))
#define pgm_freearray( grays, rows ) pm_freearray( (char**) grays, rows )
#define pgm_freerow( grayrow ) pm_freerow( (char*) grayrow )

#define ppm_allocarray( cols, rows ) ((pixel**) pm_allocarray( cols, rows, sizeof(pixel) ))
#define ppm_allocrow( cols ) ((pixel*) pm_allocrow( cols, sizeof(pixel) ))
#define ppm_freearray( pixels, rows ) pm_freearray( (char**) pixels, rows )
#define ppm_freerow( pixelrow ) pm_freerow( (char*) pixelrow )

static const bool pm_plain_output = false;

// Some functions lifted from Netpbm and adapted to use C++ iostreams.


char**
pm_allocarray(int const cols, int const rows, int const size )  {
  /*----------------------------------------------------------------------------
    Allocate an array of 'rows' rows of 'cols' columns each, with each
    element 'size' bytes.

    We use a special format where we tack on an extra element to the row
    index to indicate the format of the array.

    We have two ways of allocating the space: fragmented and
    unfragmented.  In both, the row index (plus the extra element) is
    in one block of memory.  In the fragmented format, each row is
    also in an independent memory block, and the extra row pointer is
    NULL.  In the unfragmented format, all the rows are in a single
    block of memory called the row heap and the extra row pointer is
    the address of that block.

    We use unfragmented format if possible, but if the allocation of the
    row heap fails, we fall back to fragmented.
    -----------------------------------------------------------------------------*/
  char** rowIndex;
  char * rowheap;

  rowIndex = (char **)PANDA_MALLOC_ARRAY((rows + 1) * sizeof(char *));
  if ( rowIndex == nullptr )
    pm_error("out of memory allocating row index (%u rows) for an array",
             rows);
  rowheap = (char *)PANDA_MALLOC_ARRAY( rows * cols * size );
  if ( rowheap == nullptr ) {
    /* We couldn't get the whole heap in one block, so try fragmented
       format.
    */
    int row;

    rowIndex[rows] = nullptr;   /* Declare it fragmented format */

    for (row = 0; row < rows; ++row) {
      rowIndex[row] = pm_allocrow(cols, size);
      if (rowIndex[row] == nullptr)
        pm_error("out of memory allocating Row %u "
                 "(%u columns, %u bytes per tuple) "
                 "of an array", row, cols, size);
    }
  } else {
    /* It's unfragmented format */
    int row;
    rowIndex[rows] = rowheap;  /* Declare it unfragmented format */

    for (row = 0; row < rows; ++row)
      rowIndex[row] = &(rowheap[row * cols * size]);
  }
  return rowIndex;
}

void
pm_freearray(char ** const rowIndex,
             int     const rows) {

  void * const rowheap = rowIndex[rows];

  if (rowheap != nullptr) {
    PANDA_FREE_ARRAY(rowheap);
  } else {
    int row;
    for (row = 0; row < rows; ++row) {
      pm_freerow(rowIndex[row]);
    }
  }
  PANDA_FREE_ARRAY(rowIndex);
}

static unsigned int
pm_getuint(istream * const ifP) {
  /*----------------------------------------------------------------------------
    Read an unsigned integer in ASCII decimal from the file stream
    represented by 'ifP' and return its value.

    If there is nothing at the current position in the file stream that
    can be interpreted as an unsigned integer, issue an error message
    to stderr and abort the program.

    If the number at the current position in the file stream is too
    great to be represented by an 'int' (Yes, I said 'int', not
    'unsigned int'), issue an error message to stderr and abort the
    program.
    -----------------------------------------------------------------------------*/
  int ch;
  unsigned int i;

  // skip whitespace
  do {
    ch = ifP->get();

    if (ch == '#') {
      // Skip a comment
      do {
        ch = ifP->get();
      } while (ch != EOF && ch != '\n');
    }
  } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

  if (ch < '0' || ch > '9')
    pm_error("junk in file where an unsigned integer should be");

  i = 0;
  do {
    unsigned int const digitVal = ch - '0';

    if (i > INT_MAX/10 - digitVal)
      pm_error("ASCII decimal integer in file is "
               "too large to be processed.  ");
    i = i * 10 + digitVal;
    ch = ifP->get();
  } while (ch >= '0' && ch <= '9');

  return i;
}

static void
ppm_readppminitrest(istream *   const file,
                    int *    const colsP,
                    int *    const rowsP,
                    pixval * const maxvalP) {
  unsigned int maxval;

  /* Read size. */
  *colsP = (int)pm_getuint(file);
  *rowsP = (int)pm_getuint(file);

  /* Read maxval. */
  maxval = pm_getuint(file);
  if (maxval > PPM_OVERALLMAXVAL)
    pm_error("maxval of input image (%u) is too large.  "
             "The maximum allowed by the PPM is %u.",
             maxval, PPM_OVERALLMAXVAL);
  if (maxval == 0)
    pm_error("maxval of input image is zero.");

  *maxvalP = maxval;
}

static void
pgm_readpgminitrest(istream * const file,
                    int *  const colsP,
                    int *  const rowsP,
                    gray * const maxvalP) {

  gray maxval;

  /* Read size. */
  *colsP = (int)pm_getuint(file);
  *rowsP = (int)pm_getuint(file);

  /* Read maxval. */
  maxval = pm_getuint(file);
  if (maxval > PGM_OVERALLMAXVAL)
    pm_error("maxval of input image (%u) is too large.  "
             "The maximum allowed by PGM is %u.",
             maxval, PGM_OVERALLMAXVAL);
  if (maxval == 0)
    pm_error("maxval of input image is zero.");

  *maxvalP = maxval;
}

static void
pbm_readpbminitrest(istream* file,
                    int* colsP,
                    int* rowsP) {
  /* Read size. */
  *colsP = (int)pm_getuint( file );
  *rowsP = (int)pm_getuint( file );

  /* *colsP and *rowsP really should be unsigned int, but they come
     from the time before unsigned ints (or at least from a person
     trained in that tradition), so they are int.  We could simply
     consider negative numbers to mean values > INT_MAX/2 and much
     code would just automatically work.  But some code would fail
     miserably.  So we consider values that won't fit in an int to
     be unprocessable.
  */
  if (*colsP < 0)
    pm_error("Number of columns in header is too large.");
  if (*rowsP < 0)
    pm_error("Number of columns in header is too large.");
}

static unsigned char
pm_getrawbyte(istream * const file) {
  int iby;

  iby = file->get();
  if (iby == EOF)
    pm_error("EOF / read error reading a one-byte sample");
  return (unsigned char) iby;
}

static bit
getbit (istream * const file) {
  char ch;

  do {
    ch = file->get();
  } while ( ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' );

  if ( ch != '0' && ch != '1' )
    pm_error( "junk in file where bits should be" );

  return ( ch == '1' ) ? 1 : 0;
}

static void
pbm_readpbmrow( istream *file, bit *bitrow, int cols, int format ) {
  int col, bitshift;
  bit* bP;

  switch ( format )
    {
    case PBM_FORMAT:
      for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
        *bP = getbit( file );
      break;

    case RPBM_FORMAT: {
      unsigned char item;
      bitshift = -1;  item = 0;  /* item's value is meaningless here */
      for ( col = 0, bP = bitrow; col < cols; ++col, ++bP )
        {
          if ( bitshift == -1 )
            {
              item = pm_getrawbyte( file );
              bitshift = 7;
            }
          *bP = ( item >> bitshift ) & 1;
          --bitshift;
        }
    }
      break;

    default:
      pm_error( "can't happen" );
    }
}

static gray
pgm_getrawsample(istream * const file, gray const maxval) {

  if (maxval < 256) {
    /* The sample is just one byte.  Read it. */
    return(pm_getrawbyte(file));
  } else {
    /* The sample is two bytes.  Read both. */
    unsigned char byte_pair[2];
    size_t pairs_read;

    file->read((char *)byte_pair, 2);
    pairs_read = file->gcount();
    if (pairs_read == 0)
      pm_error("EOF /read error while reading a long sample");
    /* This could be a few instructions faster if exploited the internal
       format (i.e. endianness) of a pixval.  Then we might be able to
       skip the shifting and oring.
    */
    return((byte_pair[0]<<8) | byte_pair[1]);
  }
}

static void
pgm_readpgmrow(istream* const file, gray* const grayrow,
               int const cols, gray const maxval, int const format) {

  switch (format) {
  case PGM_FORMAT: {
    int col;
    for (col = 0; col < cols; ++col) {
      grayrow[col] = pm_getuint(file);
#ifdef DEBUG
      if (grayrow[col] > maxval)
        pm_error( "value out of bounds (%u > %u)",
                  grayrow[col], maxval );
#endif /*DEBUG*/
    }
  }
    break;

  case RPGM_FORMAT: {
    int col;
    for (col = 0; col < cols; ++col) {
      grayrow[col] = pgm_getrawsample( file, maxval );
#ifdef DEBUG
      if ( grayrow[col] > maxval )
        pm_error( "value out of bounds (%u > %u)",
                  grayrow[col], maxval );
#endif /*DEBUG*/
    }
  }
    break;

  case PBM_FORMAT:
  case RPBM_FORMAT:
    {
      bit * bitrow;
      int col;

      bitrow = pbm_allocrow(cols);
      pbm_readpbmrow( file, bitrow, cols, format );
      for (col = 0; col < cols; ++col)
        grayrow[col] = (bitrow[col] == PBM_WHITE ) ? maxval : 0;
      pbm_freerow(bitrow);
    }
    break;

  default:
    pm_error( "can't happen" );
  }
}

static void
ppm_readppmrow(istream*  const fileP,
               pixel* const pixelrow,
               int    const cols,
               pixval const maxval,
               int    const format) {

  switch (format) {
  case PPM_FORMAT: {
    int col;
    for (col = 0; col < cols; ++col) {
      pixval const r = pm_getuint(fileP);
      pixval const g = pm_getuint(fileP);
      pixval const b = pm_getuint(fileP);
      PPM_ASSIGN(pixelrow[col], r, g, b);
    }
  }
    break;

  case RPPM_FORMAT: {
    int col;
    for (col = 0; col < cols; ++col) {
      pixval const r = pgm_getrawsample(fileP, maxval);
      pixval const g = pgm_getrawsample(fileP, maxval);
      pixval const b = pgm_getrawsample(fileP, maxval);
      PPM_ASSIGN(pixelrow[col], r, g, b);
    }
  }
    break;

  case PGM_FORMAT:
  case RPGM_FORMAT: {
    gray * const grayrow = pgm_allocrow(cols);
    int col;

    pgm_readpgmrow(fileP, grayrow, cols, maxval, format);
    for (col = 0; col < cols; ++col) {
      pixval const g = grayrow[col];
      PPM_ASSIGN(pixelrow[col], g, g, g);
    }
    pgm_freerow(grayrow);
  }
    break;

  case PBM_FORMAT:
  case RPBM_FORMAT: {
    bit * const bitrow = pbm_allocrow(cols);
    int col;

    pbm_readpbmrow(fileP, bitrow, cols, format);
    for (col = 0; col < cols; ++col) {
      pixval const g = (bitrow[col] == PBM_WHITE) ? maxval : 0;
      PPM_ASSIGN(pixelrow[col], g, g, g);
    }
    pbm_freerow(bitrow);
  }
    break;

  default:
    pm_error("Invalid format code");
  }
}

static void
pnm_readpnmrow( istream* file, xel* xelrow, int cols, xelval maxval, int format ) {
  int col;
  xel* xP;
  gray* grayrow;
  gray* gP;
  bit* bitrow;
  bit* bP;

  switch ( PNM_FORMAT_TYPE(format) )
    {
    case PPM_TYPE:
      ppm_readppmrow( file, (pixel*) xelrow, cols, (pixval) maxval, format );
      break;

    case PGM_TYPE:
      grayrow = pgm_allocrow( cols );
      pgm_readpgmrow( file, grayrow, cols, (gray) maxval, format );
      for ( col = 0, xP = xelrow, gP = grayrow; col < cols; ++col, ++xP, ++gP )
        PNM_ASSIGN1( *xP, *gP );
      pgm_freerow( grayrow );
      break;

    case PBM_TYPE:
      bitrow = pbm_allocrow( cols );
      pbm_readpbmrow( file, bitrow, cols, format );
      for ( col = 0, xP = xelrow, bP = bitrow; col < cols; ++col, ++xP, ++bP )
        PNM_ASSIGN1( *xP, *bP == PBM_BLACK ? 0: maxval );
      pbm_freerow( bitrow );
      break;

    default:
      pm_error( "can't happen" );
    }
}

static void
pbm_writepbminit(ostream * const fileP,
                 int    const cols,
                 int    const rows,
                 int    const forceplain) {

  if (!forceplain && !pm_plain_output) {
    (*fileP)
      << (char)PBM_MAGIC1
      << (char)RPBM_MAGIC2
      << '\n'
      << cols << ' ' << rows << '\n';
  } else {
    (*fileP)
      << (char)PBM_MAGIC1
      << (char)PBM_MAGIC2
      << '\n'
      << cols << ' ' << rows << '\n';
  }
}

static void
pgm_writepgminit(ostream * const fileP,
                 int    const cols,
                 int    const rows,
                 gray   const maxval,
                 int    const forceplain) {

  bool const plainFormat = forceplain || pm_plain_output;

  if (maxval > PGM_OVERALLMAXVAL && !plainFormat)
    pm_error("too-large maxval passed to ppm_writepgminit(): %d.\n"
             "Maximum allowed by the PGM format is %d.",
             maxval, PGM_OVERALLMAXVAL);

  (*fileP)
    << (char)PGM_MAGIC1
    << (char)(plainFormat /*|| maxval >= 1<<16*/ ? PGM_MAGIC2 : RPGM_MAGIC2)
    << '\n'
    << cols << ' ' << rows << '\n' << maxval << '\n';
}

static void
ppm_writeppminit(ostream*  const fileP,
                 int    const cols,
                 int    const rows,
                 pixval const maxval,
                 int    const forceplain) {

  bool const plainFormat = forceplain || pm_plain_output;

  if (maxval > PPM_OVERALLMAXVAL && !plainFormat)
    pm_error("too-large maxval passed to ppm_writeppminit(): %d."
             "Maximum allowed by the PPM format is %d.",
             maxval, PPM_OVERALLMAXVAL);

  (*fileP)
    << (char)PPM_MAGIC1
    << (char)(plainFormat /*|| maxval >= 1<<16*/ ? PPM_MAGIC2 : RPPM_MAGIC2)
    << '\n'
    << cols << ' ' << rows << '\n' << maxval << '\n';
}

static void
pnm_writepnminit(ostream * const fileP,
                 int    const cols,
                 int    const rows,
                 xelval const maxval,
                 int    const format,
                 int    const forceplain) {

  bool const plainFormat = forceplain || pm_plain_output;

  switch (PNM_FORMAT_TYPE(format)) {
  case PPM_TYPE:
    ppm_writeppminit(fileP, cols, rows, (pixval) maxval, plainFormat);
    break;

  case PGM_TYPE:
    pgm_writepgminit(fileP, cols, rows, (gray) maxval, plainFormat);
    break;

  case PBM_TYPE:
    pbm_writepbminit(fileP, cols, rows, plainFormat);
    break;

  default:
    pm_error("invalid format argument received by pnm_writepnminit(): %d"
             "PNM_FORMAT_TYPE(format) must be %d, %d, or %d",
             format, PBM_TYPE, PGM_TYPE, PPM_TYPE);
  }
}

static void
packBitsGeneric(ostream *          const fileP,
                const bit *     const bitrow,
                unsigned char * const packedBits,
                int             const cols,
                int *           const nextColP) {
  /*----------------------------------------------------------------------------
    Pack the bits of bitrow[] into byts at 'packedBits'.  Going left to right,
    stop when there aren't enough bits left to fill a whole byte.  Return
    as *nextColP the number of the next column after the rightmost one we
    packed.

    Don't use any special CPU facilities to do the packing.
    -----------------------------------------------------------------------------*/
  int col;

#define iszero(x) ((x) == 0 ? 0 : 1)

  for (col = 0; col + 7 < cols; col += 8)
    packedBits[col/8] = (
                         iszero(bitrow[col+0]) << 7 |
                         iszero(bitrow[col+1]) << 6 |
                         iszero(bitrow[col+2]) << 5 |
                         iszero(bitrow[col+3]) << 4 |
                         iszero(bitrow[col+4]) << 3 |
                         iszero(bitrow[col+5]) << 2 |
                         iszero(bitrow[col+6]) << 1 |
                         iszero(bitrow[col+7]) << 0
                         );
  *nextColP = col;
}

static void
writePackedRawRow(ostream *                const fileP,
                  const unsigned char * const packed_bits,
                  int                   const cols) {

  fileP->write((const char *)packed_bits, pbm_packed_bytes(cols));
  if (fileP->fail()) {
    pm_error("I/O error writing packed row to raw PBM file.");
  }
}

static void
writePbmRowRaw(ostream *      const fileP,
               const bit * const bitrow,
               int         const cols) {

  int nextCol;

  unsigned char * const packedBits = pbm_allocrow_packed(cols);

  packBitsGeneric(fileP, bitrow, packedBits, cols, &nextCol);

  /* routine for partial byte at the end of packed_bits[]
     Prior to addition of the above enhancement,
     this method was used for the entire process
  */

  if (cols % 8 > 0) {
    int col;
    int bitshift;
    unsigned char item;

    bitshift = 7;  /* initial value */
    item = 0;      /* initial value */
    for (col = nextCol; col < cols; ++col, --bitshift )
      if (bitrow[col] !=0)
        item |= 1 << bitshift
          ;

    packedBits[col/8] = item;
  }

  writePackedRawRow(fileP, packedBits, cols);

  pbm_freerow_packed(packedBits);
}



static void
writePbmRowPlain(ostream * const fileP,
                 bit *  const bitrow,
                 int    const cols) {

  int col, charcount;

  charcount = 0;
  for (col = 0; col < cols; ++col) {
    if (charcount >= 70) {
      fileP->put('\n');
      charcount = 0;
    }
    fileP->put(bitrow[col] ? '1' : '0');
    ++charcount;
  }
  fileP->put('\n');
}

static void
pbm_writepbmrow(ostream * const fileP,
                bit *  const bitrow,
                int    const cols,
                int    const forceplain) {

  if (!forceplain && !pm_plain_output)
    writePbmRowRaw(fileP, bitrow, cols);
  else
    writePbmRowPlain(fileP, bitrow, cols);
}

static void
pgm_writerawsample(ostream *file, const gray val, const gray maxval) {

  if (maxval < 256) {
    /* Samples fit in one byte, so write just one byte */
    file->put(val);
    if (file->fail())
      pm_error("Error writing single byte sample to file");
  } else {
    /* Samples are too big for one byte, so write two */
    unsigned char outval[2];
    /* We could save a few instructions if we exploited the internal
       format of a gray, i.e. its endianness.  Then we might be able
       to skip the shifting and anding.
    */
    outval[0] = val >> 8;
    outval[1] = val & 0xFF;
    file->write((const char *)outval, 2);
    if (file->fail())
      pm_error("Error writing double byte sample to file");
  }
}

static void
pgm_writepgmrowraw(ostream *file, gray *grayrow, int cols, gray maxval ) {
  int col;

  for (col = 0; col < cols; ++col) {
#ifdef DEBUG
    if (grayrow[col] > maxval)
      pm_error( "value out of bounds (%u > %u)", grayrow[col], maxval);
#endif /*DEBUG*/
    pgm_writerawsample(file, grayrow[col], maxval);
  }
}

static void
putus(unsigned short const n,
      ostream *         const fileP) {

  if (n >= 10)
    putus(n / 10, fileP);
  fileP->put(n % 10 + '0');
}


static void
pgm_writepgmrowplain(ostream * const fileP,
                     gray * const grayrow,
                     int    const cols,
                     gray   const maxval) {

  int col, charcount;
  gray* gP;

  charcount = 0;
  for (col = 0, gP = grayrow; col < cols; ++col, ++gP) {
    if (charcount >= 65) {
      fileP->put('\n');
      charcount = 0;
    } else if (charcount > 0) {
      fileP->put(' ');
      ++charcount;
    }
#ifdef DEBUG
    if (*gP > maxval)
      pm_error("value out of bounds (%u > %u)", *gP, maxval);
#endif /*DEBUG*/
    putus((unsigned short)*gP, fileP);
    charcount += 3;
  }
  if (charcount > 0)
    fileP->put('\n');
}

static void
pgm_writepgmrow(ostream* const fileP,
                gray* const grayrow,
                int   const cols,
                gray  const maxval,
                int   const forceplain) {

  if (forceplain || pm_plain_output /*|| maxval >= 1<<16*/)
    pgm_writepgmrowplain(fileP, grayrow, cols, maxval);
  else
    pgm_writepgmrowraw(fileP, grayrow, cols, maxval);
}

static void
ppm_writeppmrowraw(ostream *file, pixel *pixelrow, int cols, pixval maxval ) {
  int col;
  pixval val;

  for ( col = 0; col < cols; ++col )
    {
      val = PPM_GETR( pixelrow[col] );
#ifdef DEBUG
      if ( val > maxval )
        pm_error( "r value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
      pgm_writerawsample( file, val, maxval );
      val = PPM_GETG( pixelrow[col] );
#ifdef DEBUG
      if ( val > maxval )
        pm_error( "g value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
      pgm_writerawsample( file, val, maxval );
      val = PPM_GETB( pixelrow[col] );
#ifdef DEBUG
      if ( val > maxval )
        pm_error( "b value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
      pgm_writerawsample( file, val, maxval );
    }
}

static void
ppm_writeppmrowplain(ostream *file, pixel *pixelrow, int cols, pixval maxval ) {
  int col, charcount;
  pixel* pP;
  pixval val;

  charcount = 0;
  for ( col = 0, pP = pixelrow; col < cols; ++col, ++pP )
    {
      if ( charcount >= 65 )
        {
          (void) file->put( '\n' );
          charcount = 0;
        }
      else if ( charcount > 0 )
        {
          (void) file->put( ' ' );
          (void) file->put( ' ' );
          charcount += 2;
        }
      val = PPM_GETR( *pP );
#ifdef DEBUG
      if ( val > maxval )
        pm_error( "r value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
      putus( val, file );
      (void) file->put( ' ' );
      val = PPM_GETG( *pP );
#ifdef DEBUG
      if ( val > maxval )
        pm_error( "g value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
      putus( val, file );
      (void) file->put( ' ' );
      val = PPM_GETB( *pP );
#ifdef DEBUG
      if ( val > maxval )
        pm_error( "b value out of bounds (%u > %u)", val, maxval );
#endif /*DEBUG*/
      putus( val, file );
      charcount += 11;
    }
  if ( charcount > 0 )
    (void) file->put( '\n' );
}

static void
ppm_writeppmrow(ostream *  const fileP,
                pixel * const pixelrow,
                int     const cols,
                pixval  const maxval,
                int     const forceplain) {

  if (forceplain || pm_plain_output /*|| maxval >= 1<<16*/)
    ppm_writeppmrowplain(fileP, pixelrow, cols, maxval);
  else
    ppm_writeppmrowraw(fileP, pixelrow, cols, maxval);
}

static void
pnm_writepnmrow(ostream * const fileP,
                xel *  const xelrow,
                int    const cols,
                xelval const maxval,
                int    const format,
                int    const forceplain) {

  bool const plainFormat = forceplain || pm_plain_output;

  switch (PNM_FORMAT_TYPE(format)) {
  case PPM_TYPE:
    ppm_writeppmrow(fileP, (pixel*) xelrow, cols, (pixval) maxval,
                    plainFormat);
    break;

  case PGM_TYPE: {
    gray* grayrow;
    int col;

    grayrow = pgm_allocrow(cols);

    for (col = 0; col < cols; ++col)
      grayrow[col] = PNM_GET1(xelrow[col]);

    pgm_writepgmrow(fileP, grayrow, cols, (gray) maxval, plainFormat);

    pgm_freerow( grayrow );
  }
    break;

  case PBM_TYPE: {
    bit* bitrow;
    int col;

    bitrow = pbm_allocrow(cols);

    for (col = 0; col < cols; ++col)
      bitrow[col] = PNM_GET1(xelrow[col]) == 0 ? PBM_BLACK : PBM_WHITE;

    pbm_writepbmrow(fileP, bitrow, cols, plainFormat);

    pbm_freerow(bitrow);
  }
    break;

  default:
    pm_error("invalid format argument received by pnm_writepnmrow(): %d"
             "PNM_FORMAT_TYPE(format) must be %d, %d, or %d",
             format, PBM_TYPE, PGM_TYPE, PPM_TYPE);
  }
}

/**
 *
 */
PNMFileTypePNM::
PNMFileTypePNM() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypePNM::
get_name() const {
  return "NetPBM-style PBM/PGM/PPM/PNM";
}

/**
 * Returns the number of different possible filename extensions_PNM associated
 * with this particular file type.
 */
int PNMFileTypePNM::
get_num_extensions() const {
  return num_extensions_PNM;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypePNM::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_PNM, string());
  return extensions_PNM[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypePNM::
get_suggested_extension() const {
  return "ppm";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypePNM::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypePNM::
matches_magic_number(const string &magic_number) const {
  return (magic_number.size() >= 2) &&
    magic_number[0] == 'P' &&
    (magic_number[1] >= '1' && magic_number[1] <= '6');
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypePNM::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypePNM::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


/**
 *
 */
PNMFileTypePNM::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  if (!read_magic_number(_file, magic_number, 2)) {
    // No magic number.  No image.
    if (pnmimage_pnm_cat.is_debug()) {
      pnmimage_pnm_cat.debug()
        << "PNM file appears to be empty.\n";
    }
    _is_valid = false;
    return;
  }

  _ftype =
    ((unsigned char)magic_number[0] << 8) |
    (unsigned char)magic_number[1];

  switch ( PNM_FORMAT_TYPE(_ftype) ) {
  case PPM_TYPE:
    ppm_readppminitrest( file, &_x_size, &_y_size, &_maxval );
    _num_channels = 3;
    break;

  case PGM_TYPE:
    pgm_readpgminitrest( file, &_x_size, &_y_size, &_maxval );
    _num_channels = 1;
    break;

  case PBM_TYPE:
    pbm_readpbminitrest( file, &_x_size, &_y_size );
    _num_channels = 1;
    _maxval = 1;
    break;

  default:
    _is_valid = false;
  }

  if (pnmimage_pnm_cat.is_debug()) {
    if (is_valid()) {
      pnmimage_pnm_cat.debug()
        << "Reading ";
      switch (PNM_FORMAT_TYPE(_ftype)) {
      case PPM_TYPE:
        pnmimage_pnm_cat.debug(false) << "PPM";
        break;
      case PGM_TYPE:
        pnmimage_pnm_cat.debug(false) << "PGM";
        break;
      case PBM_TYPE:
        pnmimage_pnm_cat.debug(false) << "PBM";
        break;
      }
      pnmimage_pnm_cat.debug(false)
        << " " << *this << "\n";
    } else {
      pnmimage_pnm_cat.debug()
        << "File is not a valid PNM image.\n";
    }
  }
}

/**
 * Returns true if this particular PNMReader supports a streaming interface to
 * reading the data: that is, it is capable of returning the data one row at a
 * time, via repeated calls to read_row().  Returns false if the only way to
 * read from this file is all at once, via read_data().
 */
bool PNMFileTypePNM::Reader::
supports_read_row() const {
  return true;
}

/**
 * If supports_read_row(), above, returns true, this function may be called
 * repeatedly to read the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully read, false if there is
 * an error or end of file.
 *
 * The x_size and y_size parameters are the value of _x_size and _y_size as
 * originally filled in by the constructor; it is the actual number of pixels
 * in the image.  (The _x_size and _y_size members may have been automatically
 * modified by the time this method is called if we are scaling on load, so
 * should not be used.)
 */
bool PNMFileTypePNM::Reader::
read_row(xel *array, xelval *, int x_size, int y_size) {
  if (!is_valid()) {
    return false;
  }
  pnm_readpnmrow(_file, array, x_size, _maxval, _ftype);
  return true;
}


/**
 *
 */
PNMFileTypePNM::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

/**
 * Returns true if this particular PNMWriter supports a streaming interface to
 * writing the data: that is, it is capable of writing the image one row at a
 * time, via repeated calls to write_row().  Returns false if the only way to
 * write from this file is all at once, via write_data().
 */
bool PNMFileTypePNM::Writer::
supports_write_row() const {
  return true;
}

/**
 * If supports_write_row(), above, returns true, this function may be called
 * to write out the image header in preparation to writing out the image data
 * one row at a time.  Returns true if the header is successfully written,
 * false if there is an error.
 *
 * It is the user's responsibility to fill in the header data via calls to
 * set_x_size(), set_num_channels(), etc., or copy_header_from(), before
 * calling write_header().
 */
bool PNMFileTypePNM::Writer::
write_header() {
  switch (get_color_type()) {
  case PNMImageHeader::CT_grayscale:
  case PNMImageHeader::CT_two_channel:
    if (_maxval == 1) {
      _pnm_format = PBM_TYPE;
    } else {
      _pnm_format = PGM_TYPE;
    }
    break;

  case PNMImageHeader::CT_color:
  case PNMImageHeader::CT_four_channel:
    _pnm_format = PPM_TYPE;
    break;

  default:
    break;
  }

  pnm_writepnminit(_file, _x_size, _y_size, _maxval, _pnm_format, 0);
  return true;
}

/**
 * If supports_write_row(), above, returns true, this function may be called
 * repeatedly to write the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully written, false if there
 * is an error.
 *
 * You must first call write_header() before writing the individual rows.  It
 * is also important to delete the PNMWriter class after successfully writing
 * the last row.  Failing to do this may result in some data not getting
 * flushed!
 */
bool PNMFileTypePNM::Writer::
write_row(xel *row_data, xelval *) {
  pnm_writepnmrow(_file, row_data, _x_size, _maxval, _pnm_format, 0);

  return true;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypePNM::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypePNM);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypePNM::
make_PNMFileTypePNM(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_PNM
