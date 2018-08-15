/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeTIFF.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "pnmFileTypeTIFF.h"

#ifdef HAVE_TIFF

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"
#include "ppmcmap.h"
#include "pfmFile.h"

// Tiff will want to re-typedef these things.
#define int8 tiff_int8
#define uint8 tiff_uint8
#define int32 tiff_int32
#define uint32 tiff_uint32

using std::ios;
using std::istream;
using std::ostream;
using std::string;

extern "C" {
#include <tiff.h>
#include <tiffio.h>
}

static const char * const extensions_tiff[] = {
  "tiff", "tif"
};
static const int num_extensions_tiff = sizeof(extensions_tiff) / sizeof(const char *);

// These are configurable parameters to specify TIFF details on output.  See
// tiff.h or type man pnmtotiff for a better explanation of options.

// unsigned short tiff_compression = COMPRESSION_LZW;   lzw not supported
// anymore because of big bad Unisys
unsigned short tiff_compression = COMPRESSION_NONE;

/* One of:
   COMPRESSION_NONE
   COMPRESSION_CCITTRLE
   COMPRESSION_CCITTFAX3
   COMPRESSION_CCITTFAX4
   COMPRESSION_LZW
   COMPRESSION_JPEG
   COMPRESSION_NEXT
   COMPRESSION_CCITTRLEW
   COMPRESSION_PACKBITS
   COMPRESSION_THUNDERSCAN
   */

long tiff_g3options = 0;
/* One or more of:
   GROUP3OPT_2DENCODING
   GROUP3OPT_FILLBITS

   meaningful when tiff_compression == COMPRESSION_CCITTFAX3.
   */

unsigned short tiff_fillorder = FILLORDER_MSB2LSB;
/* One of:
   FILLORDER_MSB2LSB
   FILLORDER_LSB2MSB
   */

short tiff_predictor = 0;
/* 0, 1, or 2;  meaningful when tiff_compression == COMPRESSION_LZW. */


#ifndef PHOTOMETRIC_DEPTH
#define PHOTOMETRIC_DEPTH 32768
#endif

// Here's a number of functions to support the iostream interface via the TIFF
// library.
static tsize_t
istream_read(thandle_t fd, tdata_t buf, tsize_t size) {
  istream *in = (istream *)fd;
  in->read((char *)buf, size);
  Thread::consider_yield();
  return in->gcount();
}

static tsize_t
ostream_write(thandle_t fd, tdata_t buf, tsize_t size) {
  ostream *out = (ostream *)fd;
  out->write((char *)buf, size);
  Thread::consider_yield();
  return out->fail() ? (tsize_t)0 : size;
}

static tsize_t
ostream_dont_read(thandle_t, tdata_t, tsize_t) {
  // This no-op variant of istream_read() is passed in when we open the file
  // for writing only.  Shouldn't mix reads and writes.
  return 0;
}

static tsize_t
istream_dont_write(thandle_t, tdata_t, tsize_t) {
  // This no-op variant of ostream_write() is passed in when we open the file
  // for reading only.  Shouldn't mix reads and writes.
  return 0;
}

static toff_t
istream_seek(thandle_t fd, toff_t off, int whence) {
  istream *in = (istream *)fd;

  ios_seekdir dir;
  switch (whence) {
  case SEEK_SET:
    dir = ios::beg;
    break;

  case SEEK_END:
    dir = ios::end;
    break;

  case SEEK_CUR:
    dir = ios::cur;
    break;

  default:
    return in->tellg();
  }

  in->seekg(off, dir);

  if (pnmimage_tiff_cat->is_spam()) {
    pnmimage_tiff_cat->spam()
      << "istream_seek(" << (void *)in << ", " << off << ", "
      << whence << "), result = " << in->tellg() << "\n";
  }
  return in->tellg();
}

static toff_t
ostream_seek(thandle_t fd, toff_t off, int whence) {
  ostream *out = (ostream *)fd;

  ios_seekdir dir;
  switch (whence) {
  case SEEK_SET:
    dir = ios::beg;
    break;

  case SEEK_END:
    dir = ios::end;
    break;

  case SEEK_CUR:
    dir = ios::cur;
    break;

  default:
    return out->tellp();
  }

  out->seekp(off, dir);

  if (pnmimage_tiff_cat->is_spam()) {
    pnmimage_tiff_cat->spam()
      << "ostream_seek(" << (void *)out << ", " << off << ", "
      << whence << "), result = " << out->tellp() << "\n";
  }
  return out->tellp();
}

static int
iostream_dont_close(thandle_t) {
  // We don't actually close the file; we'll leave that to PNMReader.
  return true;
}

static toff_t
istream_size(thandle_t fd) {
  istream *in = (istream *)fd;
  in->seekg(0, ios::end);
  return in->tellg();
}

static toff_t
ostream_size(thandle_t fd) {
  ostream *out = (ostream *)fd;
  out->seekp(0, ios::end);
  return out->tellp();
}

static int
iostream_map(thandle_t, tdata_t*, toff_t*) {
  return (0);
}

static void
iostream_unmap(thandle_t, tdata_t, toff_t) {
}

bool PNMFileTypeTIFF::_installed_error_handlers = false;
TypeHandle PNMFileTypeTIFF::_type_handle;

/**
 *
 */
PNMFileTypeTIFF::
PNMFileTypeTIFF() {
  // This constructor may run at static init time, so we use the ->
  // dereferencing convention on the notify category.
  if (pnmimage_tiff_cat->is_debug()) {
    pnmimage_tiff_cat->debug()
      << TIFFGetVersion() << "\n";
  }
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeTIFF::
get_name() const {
  return "TIFF";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypeTIFF::
get_num_extensions() const {
  return num_extensions_tiff;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeTIFF::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_tiff, string());
  return extensions_tiff[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypeTIFF::
get_suggested_extension() const {
  return "tiff";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypeTIFF::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypeTIFF::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);
  int mn =
    ((unsigned char)magic_number[0] << 8) |
    ((unsigned char)magic_number[1]);
  return (mn == TIFF_BIGENDIAN || mn == TIFF_LITTLEENDIAN);
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeTIFF::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  install_error_handlers();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeTIFF::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  install_error_handlers();
  return new Writer(this, file, owns_file);
}

/**
 *
 */
PNMFileTypeTIFF::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  bool grayscale = false;
  int numcolors;
  int i;
  unsigned short* redcolormap;
  unsigned short* greencolormap;
  unsigned short* bluecolormap;

  // Hope we can putback() more than one character.
  for (string::reverse_iterator mi = magic_number.rbegin();
       mi != magic_number.rend();
       mi++) {
    _file->putback(*mi);
  }
  if (_file->fail()) {
    pnmimage_tiff_cat.error()
      << "Unable to put back magic number.\n";
    _is_valid = false;
  }

  if (_is_valid) {
    tif = TIFFClientOpen("TIFF file", "r",
                         (thandle_t) _file,
                         istream_read, istream_dont_write,
                         (TIFFSeekProc)istream_seek,
                         iostream_dont_close, istream_size,
                         iostream_map, iostream_unmap);

    if ( tif == nullptr ) {
      _is_valid = false;
    }
  }

  if (_is_valid) {
    if ( ! TIFFGetField( tif, TIFFTAG_SAMPLEFORMAT, &sample_format ) )
      sample_format = SAMPLEFORMAT_UINT;
    if ( ! TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &bps ) )
      bps = 1;
    if ( ! TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &spp ) )
      spp = 1;

    if ( ! TIFFGetField( tif, TIFFTAG_PHOTOMETRIC, &photomet ) ) {
      pnmimage_tiff_cat.error()
        << "Error getting photometric from TIFF file.\n";
      _is_valid = false;

    } else if (sample_format == SAMPLEFORMAT_IEEEFP) {
      // Floating-point TIFF.  We only accept 32-bit floats.
      if (bps != 32) {
        pnmimage_tiff_cat.error()
          << "Can only read 32-bit float TIFF files, not " << bps << "-bit.\n";
        _is_valid = false;
      }

    } else if (sample_format != SAMPLEFORMAT_UINT) {
      // Can't understand other kinds of sample formats.
      pnmimage_tiff_cat.error()
        << "Can't understand sample format\n";
      _is_valid = false;
    }
  }

  if (_is_valid) {
    unsigned short num_extra_samples;
    unsigned short *extra_samples = nullptr;

    if (!TIFFGetField(tif, TIFFTAG_EXTRASAMPLES, &num_extra_samples,
                      &extra_samples)) {
      num_extra_samples = 0;
    }
    _num_channels = spp - num_extra_samples;
    unassoc_alpha_sample = 0;
    assoc_alpha_sample = 0;

    if (_num_channels == 1 || _num_channels == 3) {
      // Look for an alpha channel in one of the extra samples, if any.
      bool got_alpha = false;
      for (unsigned short s = 0; s < num_extra_samples && !got_alpha; s++) {
        if (extra_samples[s] == EXTRASAMPLE_UNASSALPHA) {
          unassoc_alpha_sample = s + _num_channels;
          _num_channels++;
          got_alpha = true;

        } else if (extra_samples[s] == EXTRASAMPLE_ASSOCALPHA) {
          assoc_alpha_sample = s + _num_channels;
          _num_channels++;
          got_alpha = true;
        }
      }

      // Unfortunately, Photoshop seems to write EXTRASAMPLE_UNSPECIFIED into
      // the EXTRASAMPLES field for its alpha channels.  If we have exactly
      // one extra channel and it's an UNSPECIFIED channel, assume it's meant
      // to be alpha.
      if (!got_alpha && num_extra_samples == 1 &&
          extra_samples[0] == EXTRASAMPLE_UNSPECIFIED) {
        unassoc_alpha_sample = _num_channels;
        _num_channels++;
      }

    } else if ((_num_channels == 2 || _num_channels == 4) && num_extra_samples == 0) {
      // If we have a 2- or 4-channel image but the extra samples are not
      // declared, assume it was written out by a broken TIFF implementation
      // and that the extra channel is really meant to be alpha.
      unassoc_alpha_sample = _num_channels - 1;
      if (pnmimage_tiff_cat.is_debug()) {
        pnmimage_tiff_cat.debug()
          << "Assuming last channel of " << spp
          << "-color image is meant to represent alpha.\n";
      }

    } else {
      pnmimage_tiff_cat.error()
        << "Cannot handle " << spp << "-color image (with "
        << num_extra_samples << " extra channels).\n";
      _is_valid = false;
    }
  }

  if (_is_valid) {
    (void) TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &_x_size );
    (void) TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &_y_size );

    if (pnmimage_tiff_cat.is_debug()) {
      pnmimage_tiff_cat.debug()
        << "Reading TIFF image: " << _x_size << " x " << _y_size << "\n"
        << bps << " bits/sample, " << spp << " samples/pixel\n";
    }

    _maxval = ( 1 << bps ) - 1;
    if ( _maxval == 1 && _num_channels == 1 ) {
        if (pnmimage_tiff_cat.is_debug()) {
          pnmimage_tiff_cat.debug(false)
            << "monochrome\n";
        }
        grayscale = true;
    } else {
      switch ( photomet ) {
      case PHOTOMETRIC_MINISBLACK:
        if (pnmimage_tiff_cat.is_debug()) {
          pnmimage_tiff_cat.debug(false)
            << _maxval + 1 << " graylevels (min is black)\n";
        }
        grayscale = true;
        break;

      case PHOTOMETRIC_MINISWHITE:
        if (pnmimage_tiff_cat.is_debug()) {
          pnmimage_tiff_cat.debug(false)
            << _maxval + 1 << " graylevels (min is white)\n";
        }
        grayscale = true;
        break;

      case PHOTOMETRIC_PALETTE:
        if (pnmimage_tiff_cat.is_debug()) {
          pnmimage_tiff_cat.debug(false)
            << " colormapped\n";
        }
        if ( ! TIFFGetField( tif, TIFFTAG_COLORMAP, &redcolormap, &greencolormap, &bluecolormap ) ) {
          pnmimage_tiff_cat.error()
            << "Error getting colormap from TIFF file.\n";
          _is_valid = false;
        } else {
          numcolors = _maxval + 1;
          if ( numcolors > TIFF_COLORMAP_MAXCOLORS ) {
            pnmimage_tiff_cat.error()
              << "Cannot read TIFF file with " << numcolors
              << " in colormap; max supported is " << TIFF_COLORMAP_MAXCOLORS << "\n";
            _is_valid = false;
          } else {
            _maxval = PNM_MAXMAXVAL;
            grayscale = false;
            for ( i = 0; i < numcolors; ++i ) {
              xelval r, g, b;
              r = (xelval)(_maxval * (double)(redcolormap[i] / 65535.0));
              g = (xelval)(_maxval * (double)(greencolormap[i] / 65535.0));
              b = (xelval)(_maxval * (double)(bluecolormap[i] / 65535.0));
              PPM_ASSIGN( colormap[i], r, g, b );
            }
          }
        }
        break;

      case PHOTOMETRIC_RGB:
        if (pnmimage_tiff_cat.is_debug()) {
          pnmimage_tiff_cat.debug(false)
            << "truecolor\n";
        }
        grayscale = false;
        break;

      case PHOTOMETRIC_MASK:
        pnmimage_tiff_cat.error()
          << "Don't know how to handle TIFF image with PHOTOMETRIC_MASK.\n";
        _is_valid = false;
        break;

      case PHOTOMETRIC_DEPTH:
        pnmimage_tiff_cat.error()
          << "Don't know how to handle TIFF image with PHOTOMETRIC_DEPTH.\n";
        _is_valid = false;
        break;

      default:
        pnmimage_tiff_cat.error()
          << "Unknown photometric " << photomet << " in TIFF image.\n";
        _is_valid = false;
        break;
      }
    }
  }

  if (_is_valid ) {
    if ( _maxval > PNM_MAXMAXVAL ) {
      pnmimage_tiff_cat.error()
        << "Cannot read TIFF file with maxval of " << _maxval << "\n";
      _is_valid = false;
    }
  }

  if (_is_valid) {
    if (grayscale && !is_grayscale()) {
      _num_channels = (has_alpha()) ? 2 : 1;
    } else if (!grayscale && is_grayscale()) {
      _num_channels = (has_alpha()) ? 4 : 3;
    }

    current_row = 0;
  }
}

/**
 *
 */
PNMFileTypeTIFF::Reader::
~Reader() {
  if (tif != nullptr) {
    TIFFClose(tif);
  }
}

/**
 * Returns true if this PNMFileType represents a floating-point image type,
 * false if it is a normal, integer type.  If this returns true, read_pfm() is
 * implemented instead of read_data().
 */
bool PNMFileTypeTIFF::Reader::
is_floating_point() {
  return (sample_format == SAMPLEFORMAT_IEEEFP);
}

/**
 * Reads floating-point data directly into the indicated PfmFile.  Returns
 * true on success, false on failure.
 */
bool PNMFileTypeTIFF::Reader::
read_pfm(PfmFile &pfm) {
  if (!is_valid()) {
    return false;
  }

  int x_size = get_x_size();
  int y_size = get_y_size();
  int num_channels = get_num_channels();

  pfm.clear(x_size, y_size, num_channels);
  vector_float table;
  pfm.swap_table(table);

  for (int yi = 0; yi < y_size; ++yi) {
    float *row = &table[(yi * x_size) * _num_channels];

    if (TIFFReadScanline(tif, row, yi, 0 ) < 0 ) {
      pnmimage_tiff_cat.error()
        << "failed a scanline read on row " << yi << "\n";
      pfm.swap_table(table);
      return false;
    }
  }

  pfm.swap_table(table);
  return true;
}

/**
 * Returns true if this particular PNMReader supports a streaming interface to
 * reading the data: that is, it is capable of returning the data one row at a
 * time, via repeated calls to read_row().  Returns false if the only way to
 * read from this file is all at once, via read_data().
 */
bool PNMFileTypeTIFF::Reader::
supports_read_row() const {
  return true;
}

/**
 * If supports_read_row(), above, returns true, this function may be called
 * repeatedly to read the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully read, false if there is
 * an error or end of file.
 */
bool PNMFileTypeTIFF::Reader::
read_row(xel *row_data, xelval *alpha_data, int x_size, int) {
  if (!is_valid()) {
    return false;
  }

  size_t scanline_size = (size_t)TIFFScanlineSize(tif);
  unsigned char *buf = (unsigned char*) alloca(scanline_size);

  int col;
  xelval gray, sample;
  xelval r, g, b;

  if ( TIFFReadScanline( tif, buf, current_row, 0 ) < 0 ) {
    pnmimage_tiff_cat.error()
      << "Bad data read on line " << current_row << " of TIFF image.\n";
    return false;
  }

  unsigned char *buf_ptr = buf;
  unsigned s;
  int bits_left = 8;

  // Get a pointer to a function that extracts the next bps-bit sample from
  // the bitarray.  There are a handful of different functions, which are
  // optimized for different values of bps.
  xelval (PNMFileTypeTIFF::Reader::*next_sample)(unsigned char *&buf_ptr, int &bits_left) const;

  if (bps < 8) {
    next_sample = &PNMFileTypeTIFF::Reader::next_sample_lt_8;

  } else if (bps == 8) {
    next_sample = &PNMFileTypeTIFF::Reader::next_sample_8;

  } else if (bps == 16) {
    next_sample = &PNMFileTypeTIFF::Reader::next_sample_16;

  } else if (bps == 32) {
    // Actually, it's not likely that a 32-bit sample will fit within a
    // xelval.  Deal with this when we come to it.
    next_sample = &PNMFileTypeTIFF::Reader::next_sample_32;

  } else {
    next_sample = &PNMFileTypeTIFF::Reader::next_sample_general;
  }

  switch ( photomet ) {
  case PHOTOMETRIC_MINISBLACK:
    for ( col = 0; col < x_size; ++col )
      {
        sample = (this->*next_sample)(buf_ptr, bits_left);
        gray = sample;

        for (s = 1; s < spp; s++) {
          sample = (this->*next_sample)(buf_ptr, bits_left);
          if (s == unassoc_alpha_sample) {
            alpha_data[col] = sample;

          } else if (s == assoc_alpha_sample) {
            alpha_data[col] = sample;
            if (sample != 0) {
              gray = (xelval)((float)gray * _maxval / (float)sample);
            }
          }
        }
        PPM_PUTB(row_data[col], gray);
      }
    break;

  case PHOTOMETRIC_MINISWHITE:
    for ( col = 0; col < x_size; ++col )
      {
        sample = (this->*next_sample)(buf_ptr, bits_left);
        gray = _maxval - sample;
        for (s = 1; s < spp; s++) {
          sample = (this->*next_sample)(buf_ptr, bits_left);
          sample = _maxval - sample;

          if (s == unassoc_alpha_sample) {
            alpha_data[col] = sample;

          } else if (s == assoc_alpha_sample) {
            alpha_data[col] = sample;
            if (sample != 0) {
              gray = (xelval)((float)gray * _maxval / (float)sample);
            }
          }
        }

        PPM_PUTB(row_data[col], gray);
      }
    break;

  case PHOTOMETRIC_PALETTE:
    for ( col = 0; col < x_size; ++col )
      {
        sample = (this->*next_sample)(buf_ptr, bits_left);
        row_data[col] = colormap[sample];

        for (s = 1; s < spp; s++) {
          sample = (this->*next_sample)(buf_ptr, bits_left);
          if (s == unassoc_alpha_sample) {
            alpha_data[col] = sample;

          } else if (s == assoc_alpha_sample) {
            alpha_data[col] = sample;
            if (sample != 0) {
              r = PPM_GETR(row_data[col]);
              g = PPM_GETG(row_data[col]);
              b = PPM_GETB(row_data[col]);
              r = (xelval)((float)r * _maxval / (float)sample);
              g = (xelval)((float)g * _maxval / (float)sample);
              b = (xelval)((float)b * _maxval / (float)sample);
              PPM_ASSIGN(row_data[col], r, g, b);
            }
          }
        }
      }
    break;

  case PHOTOMETRIC_RGB:
    for ( col = 0; col < x_size; ++col ) {
      sample = (this->*next_sample)(buf_ptr, bits_left);
      r = sample;
      sample = (this->*next_sample)(buf_ptr, bits_left);
      g = sample;
      sample = (this->*next_sample)(buf_ptr, bits_left);
      b = sample;

      for (s = 3; s < spp; s++) {
        sample = (this->*next_sample)(buf_ptr, bits_left);
        if (s == unassoc_alpha_sample) {
          alpha_data[col] = sample;

        } else if (s == assoc_alpha_sample) {
          alpha_data[col] = sample;
          if (sample != 0) {
            r = (xelval)((float)r * _maxval / (float)sample);
            g = (xelval)((float)g * _maxval / (float)sample);
            b = (xelval)((float)b * _maxval / (float)sample);
          }
        }
      }

      PPM_ASSIGN(row_data[col], r, g, b);
    }
    break;

  default:
    pnmimage_tiff_cat.error()
      << "Internal error: unsupported photometric " << photomet << "\n";
    return false;
  }

  nassertr(buf_ptr <= buf + scanline_size, false);

  current_row++;
  return true;
}

/**
 * Returns the next color sample from the row, when it is known that bps < 8.
 */
xelval PNMFileTypeTIFF::Reader::
next_sample_lt_8(unsigned char *&buf_ptr, int &bits_left) const {
  if (bits_left == 0) {
    ++buf_ptr;
    bits_left = 8;
  }

  bits_left -= bps;
  return (*buf_ptr >> bits_left) & _maxval;
}

/**
 * Returns the next color sample from the row, when it is known that bps == 8.
 */
xelval PNMFileTypeTIFF::Reader::
next_sample_8(unsigned char *&buf_ptr, int &bits_left) const {
  return *buf_ptr++;
}

/**
 * Returns the next color sample from the row, when it is known that bps ==
 * 16.
 */
xelval PNMFileTypeTIFF::Reader::
next_sample_16(unsigned char *&buf_ptr, int &bits_left) const {
  // The TIFF library has already byte-swapped the values if necessary.  Thus,
  // we only need to treat it as an array of shorts.
  unsigned short result = *(unsigned short *)buf_ptr;
  buf_ptr += 2;
  return result;
}

/**
 * Returns the next color sample from the row, when it is known that bps ==
 * 32.
 */
xelval PNMFileTypeTIFF::Reader::
next_sample_32(unsigned char *&buf_ptr, int &bits_left) const {
  // The TIFF library has already byte-swapped the values if necessary.  Thus,
  // we only need to treat it as an array of longs.
  unsigned long result = *(unsigned long *)buf_ptr;
  buf_ptr += 2;
  return (xelval)result;
}

/**
 * Returns the next color sample from the row, in general.  This unpacks an
 * arbitrary string of bits from the sequence.
 */
xelval PNMFileTypeTIFF::Reader::
next_sample_general(unsigned char *&buf_ptr, int &bits_left) const {
  unsigned int result = 0;
  int bits_needed = bps;

  while (bits_needed > 0) {
    nassertr(bits_left >= 0, 0);
    if (bits_left == 0) {
      ++buf_ptr;
      bits_left = 8;
    }

    if (bits_needed <= bits_left) {
      bits_left -= bits_needed;
      unsigned int mask = (1 << bits_needed) - 1;
      result |= ((*buf_ptr) >> bits_left) & mask;
      bits_needed = 0;

    } else {
      bits_needed -= bits_left;
      unsigned int mask = (1 << bits_left) - 1;
      result |= ((*buf_ptr) & mask) << bits_needed;
      bits_left = 0;
    }
  }

  return result;
}


/**
 *
 */
PNMFileTypeTIFF::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

/**
 * Returns true if this PNMFileType can accept a floating-point image type,
 * false if it can only accept a normal, integer type.  If this returns true,
 * write_pfm() is implemented.
 */
bool PNMFileTypeTIFF::Writer::
supports_floating_point() {
  return true;
}

/**
 * Returns true if this PNMFileType can accept an integer image type, false if
 * it can only accept a floating-point type.  If this returns true,
 * write_data() or write_row() is implemented.
 */
bool PNMFileTypeTIFF::Writer::
supports_integer() {
  return true;
}

/**
 * Writes floating-point data from the indicated PfmFile.  Returns true on
 * success, false on failure.
 */
bool PNMFileTypeTIFF::Writer::
write_pfm(const PfmFile &pfm) {
  struct tiff *tif;

  // Open output file.
  tif = TIFFClientOpen("TIFF file", "w",
                       (thandle_t) _file,
                       ostream_dont_read, ostream_write,
                       (TIFFSeekProc)ostream_seek,
                       iostream_dont_close, ostream_size,
                       iostream_map, iostream_unmap);
  if (tif == nullptr) {
    return false;
  }

  int x_size = pfm.get_x_size();
  int y_size = pfm.get_y_size();
  int num_channels = pfm.get_num_channels();

  int photometric = 0;
  if (num_channels >= 3) {
    photometric = PHOTOMETRIC_RGB;
  }

  // Set TIFF parameters.
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, x_size);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, y_size);
  TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "Generated via pnmimage.\n" );
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, num_channels);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  const vector_float &table = pfm.get_table();
  for (int yi = 0; yi < y_size; ++yi) {
    const float *row = &table[(yi * x_size) * _num_channels];

    if (TIFFWriteScanline(tif, (tdata_t)row, yi, 0 ) < 0) {
      pnmimage_tiff_cat.error()
        << "failed a scanline write on row " << yi << "\n";
      return false;
    }
  }

  TIFFFlushData(tif);
  TIFFClose(tif);

  return true;
}

/**
 * Writes out an entire image all at once, including the header, based on the
 * image data stored in the given _x_size * _y_size array and alpha pointers.
 * (If the image type has no alpha channel, alpha is ignored.) Returns the
 * number of rows correctly written.
 *
 * It is the user's responsibility to fill in the header data via calls to
 * set_x_size(), set_num_channels(), etc., or copy_header_from(), before
 * calling write_data().
 *
 * It is important to delete the PNMWriter class after successfully writing
 * the data.  Failing to do this may result in some data not getting flushed!
 *
 * Derived classes need not override this if they instead provide
 * supports_streaming() and write_row(), below.
 */
int PNMFileTypeTIFF::Writer::
write_data(xel *array, xelval *alpha) {
  colorhist_vector chv = nullptr;
  colorhash_table cht;
  unsigned short
    red[TIFF_COLORMAP_MAXCOLORS],
    grn[TIFF_COLORMAP_MAXCOLORS],
    blu[TIFF_COLORMAP_MAXCOLORS];
  int row, colors, i;
  int col;
  int grayscale = false;
  struct tiff * tif;
  short photometric = 0;
  short samplesperpixel = 0;
  unsigned short extra_samples[1] = { EXTRASAMPLE_UNASSALPHA };
  short bitspersample = 0;
  int bytesperrow = 0;
  unsigned char* buf;
  unsigned char* tP;

  switch ( get_color_type() ) {
  case CT_color:
    // This call is a bit of fakery to convert our proper 2-d array of xels to
    // an indirect 2-d array of pixels.  We make it look like a single row of
    // _x_size * _y_size pixels.

    // We can't actually write palettes bigger than 256 colors, regardless of
    // the number of colors we can read.
    chv = ppm_computecolorhist( (pixel **)&array, _x_size * _y_size, 1,
                                256, &colors );
    if ( chv == nullptr ) {
      pnmimage_tiff_cat.debug()
        << colors << " colors found; too many for a palette.\n"
        << "Writing a 24-bit RGB file.\n";
      grayscale = false;
    } else {
      pnmimage_tiff_cat.debug()
        << colors << " colors found; writing an 8-bit palette file.\n";
      grayscale = true;
      for ( i = 0; i < colors; ++i ) {
        xelval r, g, b;

        r = PPM_GETR( chv[i].color );
        g = PPM_GETG( chv[i].color );
        b = PPM_GETB( chv[i].color );
        if ( r != g || g != b ) {
          grayscale = false;
          break;
        }
      }
    }
    break;

  case CT_two_channel:  // We don't yet support two-channel output for TIFF's.
  case CT_four_channel:
    chv = nullptr;
    grayscale = false;
    break;

  case CT_grayscale:
    chv = nullptr;
    grayscale = true;
    break;

  default:
    break;
  }

  /* Open output file. */
  tif = TIFFClientOpen("TIFF file", "w",
                       (thandle_t) _file,
                       ostream_dont_read, ostream_write,
                       (TIFFSeekProc)ostream_seek,
                       iostream_dont_close, ostream_size,
                       iostream_map, iostream_unmap);
  if ( tif == nullptr ) {
    return 0;
  }

  /* Figure out TIFF parameters. */
  switch ( get_color_type() ) {
  case CT_color:
  case CT_four_channel:
    if ( chv == nullptr ) {
      samplesperpixel = _num_channels;
      bitspersample = 8;
      photometric = PHOTOMETRIC_RGB;
      bytesperrow = _x_size * samplesperpixel;
    } else if ( grayscale ) {
      samplesperpixel = 1;
      bitspersample = std::min(8, pm_maxvaltobits(_maxval));
      photometric = PHOTOMETRIC_MINISBLACK;
      i = 8 / bitspersample;
      bytesperrow = ( _x_size + i - 1 ) / i;
    } else {
      samplesperpixel = 1;
      bitspersample = std::min(8, pm_maxvaltobits(_maxval));
      photometric = PHOTOMETRIC_PALETTE;
      bytesperrow = _x_size;
    }
    break;

  case CT_grayscale:
  case CT_two_channel:
    samplesperpixel = _num_channels;
    bitspersample = pm_maxvaltobits( _maxval );
    photometric = PHOTOMETRIC_MINISBLACK;
    i = 8 / bitspersample;
    bytesperrow = ( _x_size + i - 1 ) / i;
    break;

  default:
    break;
  }

  buf = (unsigned char*) malloc( bytesperrow );
  if ( buf == nullptr ) {
    pnmimage_tiff_cat.error()
      << "Can't allocate memory for row buffer\n";
    return 0;
  }

  /* Set TIFF parameters. */
  TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, _x_size );
  TIFFSetField( tif, TIFFTAG_IMAGELENGTH, _y_size );
  TIFFSetField( tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT );
  TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, bitspersample );
  TIFFSetField( tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( tif, TIFFTAG_COMPRESSION, tiff_compression );
  if ( tiff_compression == COMPRESSION_CCITTFAX3 && tiff_g3options != 0 )
    TIFFSetField( tif, TIFFTAG_GROUP3OPTIONS, tiff_g3options );
  if ( tiff_compression == COMPRESSION_LZW && tiff_predictor != 0 )
    TIFFSetField( tif, TIFFTAG_PREDICTOR, tiff_predictor );
  TIFFSetField( tif, TIFFTAG_PHOTOMETRIC, photometric );
  TIFFSetField( tif, TIFFTAG_FILLORDER, tiff_fillorder );
  // TIFFSetField( tif, TIFFTAG_DOCUMENTNAME, "TIFF Image File");
  TIFFSetField( tif, TIFFTAG_IMAGEDESCRIPTION,
                "Generated via pnmimage.\n" );
  TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel );
  if (has_alpha()) {
    TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, 1, extra_samples);
  }
  TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );

  if ( chv == nullptr ) {
    cht = nullptr;
  } else {
    /* Make TIFF colormap. */
    for ( i = 0; i < colors; ++i ) {
      red[i] = (unsigned short) (PPM_GETR( chv[i].color ) * 65535L / _maxval);
      grn[i] = (unsigned short) (PPM_GETG( chv[i].color ) * 65535L / _maxval);
      blu[i] = (unsigned short) (PPM_GETB( chv[i].color ) * 65535L / _maxval);
    }
    TIFFSetField( tif, TIFFTAG_COLORMAP, red, grn, blu );

    /* Convert color vector to color hash table, for fast lookup. */
    cht = ppm_colorhisttocolorhash( chv, colors );
    ppm_freecolorhist( chv );
  }

  /* Now write the TIFF data. */
  for ( row = 0; row < _y_size; ++row ) {
    xel *row_data = array + row*_x_size;
    xelval *alpha_data = alpha + row*_x_size;

    if ( !is_grayscale() && ! grayscale ) {
      if ( cht == nullptr ) {
        tP = buf;
        for ( col = 0; col < _x_size; ++col ) {
          *tP++ = (unsigned char)(255 * PPM_GETR(row_data[col]) / _maxval);
          *tP++ = (unsigned char)(255 * PPM_GETG(row_data[col]) / _maxval);
          *tP++ = (unsigned char)(255 * PPM_GETB(row_data[col]) / _maxval);
          if (samplesperpixel==4) {
            *tP++ = (unsigned char)(255 * alpha_data[col] / _maxval);
          }
        }
      } else {
        tP = buf;
        for ( col = 0; col < _x_size; ++col ) {
          int s;

          s = ppm_lookupcolor( cht, (pixel *)(&row_data[col]) );
          if ( s == -1 ) {
            pnmimage_tiff_cat.error()
              << "Internal error: color not found?!?  row=" << row
              << " col=" << col << "\n";
            return 0;
          }
          *tP++ = (unsigned char) s;
          if (samplesperpixel==2) {
            *tP++ = (unsigned char)(255 * alpha_data[col] / _maxval);
          }
        }
      }
    } else {
      xelval bigger_maxval;
      int bitshift;
      unsigned char byte;
      xelval s;

      bigger_maxval = pm_bitstomaxval( bitspersample );
      bitshift = 8 - bitspersample;
      byte = 0;
      tP = buf;
      for ( col = 0; col < _x_size; ++col ) {
        s = PPM_GETB(row_data[col]);
        if ( _maxval != bigger_maxval )
          s = (xelval)((long) s * bigger_maxval / _maxval);
        byte |= s << bitshift;
        bitshift -= bitspersample;
        if ( bitshift < 0 ) {
          *tP++ = byte;
          bitshift = 8 - bitspersample;
          byte = 0;
        }
      }
      if ( bitshift != 8 - bitspersample )
        *tP++ = byte;
    }

    if ( TIFFWriteScanline( tif, buf, row, 0 ) < 0 ) {
      pnmimage_tiff_cat.error()
        << "failed a scanline write on row " << row << "\n";
      return row;
    }
  }
  TIFFFlushData( tif );
  TIFFClose( tif );

  return _y_size;
}

/**
 * Installs our personal error and warning message handlers if they have not
 * already been installed.  These methods are used to route the Tiff error
 * messages through notify, so we can turn some of them off.
 */
void PNMFileTypeTIFF::
install_error_handlers() {
  if (!_installed_error_handlers) {
    TIFFSetWarningHandler(tiff_warning);
    TIFFSetErrorHandler(tiff_error);
    _installed_error_handlers = true;
  }
}

/**
 * This is our own warning handler.  It is called by the tiff library to issue
 * a warning message.
 */
void PNMFileTypeTIFF::
tiff_warning(const char *, const char *format, va_list ap) {
  static const int buffer_size = 1024;
  char buffer[buffer_size];
#if defined(WIN32_VC) || defined(WIN64_VC)
  vsprintf(buffer, format, ap);
#else
  vsnprintf(buffer, buffer_size, format, ap);
#endif

  // We ignore the module.  It seems generally useless to us.
  pnmimage_tiff_cat.warning()
    << buffer << "\n";
}

/**
 * This is our own error handler.  It is called by the tiff library to issue a
 * error message.
 */
void PNMFileTypeTIFF::
tiff_error(const char *module, const char *format, va_list ap) {
  static const int buffer_size = 1024;
  char buffer[buffer_size];
#if defined(WIN32_VC) || defined(WIN64_VC)
  vsprintf(buffer, format, ap);
#else
  vsnprintf(buffer, buffer_size, format, ap);
#endif

  // We ignore the module.  It seems generally useless to us.
  pnmimage_tiff_cat.error()
    << buffer << "\n";
}


/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeTIFF::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeTIFF);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeTIFF::
make_PNMFileTypeTIFF(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_TIFF
