/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeStbImage.cxx
 * @author rdb
 * @date 2016-03-31
 */

#include "pnmFileTypeStbImage.h"

#ifdef HAVE_STB_IMAGE

#include "config_pnmimagetypes.h"
#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

// We use the public domain stb_image library for loading images.  Define the
// stb_image implementation.  We only use it in this unit.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

// Disable the stb_image implementation of these formats if we already support
// it through different loaders.
#ifndef HAVE_JPEG
#define STBI_ONLY_JPEG
#endif
#ifndef HAVE_PNG
#define STBI_ONLY_PNG
#endif
#ifndef HAVE_BMP
#define STBI_ONLY_BMP
#endif
#ifndef HAVE_TGA
#define STBI_ONLY_TGA
#endif
#ifndef HAVE_SOFTIMAGE_PIC
#define STBI_ONLY_PIC
#endif
#ifndef HAVE_PNM
#define STBI_ONLY_PNM
#endif

// These are always enabled because we don't support these via other means.
#define STBI_ONLY_PSD
#define STBI_ONLY_HDR
#define STBI_ONLY_GIF

#ifndef NDEBUG
// Get friendlier error messages in development builds.
#define STBI_FAILURE_USERMSG
#endif

// We read via callbacks, so no need for stbi_load_from_file.
#define STBI_NO_STDIO

#include "stb_image.h"

static const char *const stb_extensions[] = {
  // Expose the extensions that we don't already expose through other loaders.
#ifndef HAVE_JPEG
  "jpg", "jpeg",
#endif
#ifndef HAVE_PNG
  "png",
#endif
#ifndef HAVE_BMP
  "bmp",
#endif
#ifndef HAVE_TGA
  "tga",
#endif
#ifndef HAVE_SOFTIMAGE_PIC
  "pic",
#endif
#ifndef HAVE_PNM
  "ppm", "pgm",
#endif

  // We don't have other loaders for these, so add them unconditionally.
  "psd",
  "hdr",
  "gif",
};
static const int num_stb_extensions = sizeof(stb_extensions) / sizeof(const char *);

// Callbacks to allow stb_image to read from VFS.
static int cb_read(void *user, char *data, int size) {
  istream *in = (istream *)user;
  nassertr(in != NULL, 0);

  in->read(data, size);

  if (in->eof()) {
    // Gracefully handle EOF.
    in->clear();
  }

  return (int)in->gcount();
}

static void cb_skip(void *user, int n) {
  istream *in = (istream *)user;
  nassertv(in != NULL);

  in->seekg(n, ios::cur);

  if (in->fail()) {
    in->clear();

    // Implement skip by just reading and discarding the result.
    static const int size = 4096;
    char data[4096];
    while (n > 4096) {
      in->read(data, 4096);
      n -= 4096;
    }
    if (n > 0) {
      in->read(data, n);
    }
  }
}

static int cb_eof(void *user) {
  istream *in = (istream *)user;
  nassertr(in != NULL, 1);

  return in->eof();
}

static stbi_io_callbacks io_callbacks = {cb_read, cb_skip, cb_eof};

/**
 * This is defined in the .cxx file so we have access to stbi_context.
 */
class StbImageReader : public PNMReader {
public:
  StbImageReader(PNMFileType *type, istream *file, bool owns_file, string magic_number);

  virtual bool is_floating_point();
  virtual bool read_pfm(PfmFile &pfm);
  virtual int read_data(xel *array, xelval *alpha);

private:
  bool _is_float;
  stbi__context _context;
};

TypeHandle PNMFileTypeStbImage::_type_handle;

/**
 *
 */
PNMFileTypeStbImage::
PNMFileTypeStbImage() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeStbImage::
get_name() const {
  return "stb_image";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypeStbImage::
get_num_extensions() const {
  return num_stb_extensions;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeStbImage::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_stb_extensions, string());
  return stb_extensions[n];
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypeStbImage::
has_magic_number() const {
  return false;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypeStbImage::
matches_magic_number(const string &magic_number) const {
  return false;
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeStbImage::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new StbImageReader(this, file, owns_file, magic_number);
}

/**
 *
 */
StbImageReader::
StbImageReader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file),
  _is_float(false)
{
  // Hope we can putback() more than one character.
  for (string::reverse_iterator mi = magic_number.rbegin();
       mi != magic_number.rend();
       mi++) {
    _file->putback(*mi);
  }
  if (_file->fail()) {
    pnmimage_cat.error()
      << "Unable to put back magic number.\n";
    _is_valid = false;
    return;
  }

  stbi__start_callbacks(&_context, &io_callbacks, (void *)file);

  if (strncmp(magic_number.c_str(), "#?", 2) == 0 &&
      stbi__hdr_info(&_context, &_x_size, &_y_size, &_num_channels)) {
    _is_valid = true;
    _is_float = true;
  } else if (stbi__info_main(&_context, &_x_size, &_y_size, &_num_channels)) {
    _is_valid = true;
  } else {
    _is_valid = false;
    pnmimage_cat.error()
      << "stb_info failure: " << stbi_failure_reason() << "\n";
  }

  _maxval = 255;
}

/**
 * Returns true if this PNMFileType represents a floating-point image type,
 * false if it is a normal, integer type.  If this returns true, read_pfm() is
 * implemented instead of read_data().
 */
bool StbImageReader::
is_floating_point() {
  return _is_float;
}

/**
 * Reads floating-point data directly into the indicated PfmFile.  Returns
 * true on success, false on failure.
 */
bool StbImageReader::
read_pfm(PfmFile &pfm) {
  if (!is_valid()) {
    return false;
  }

  // Reposition the file at the beginning.
  _file->seekg(0, ios::beg);
  if (_file->tellg() != 0) {
    pnmimage_cat.error()
      << "Could not reposition file pointer to the beginning.\n";
    return false;
  }

  stbi__start_callbacks(&_context, &io_callbacks, (void *)_file);

  nassertr(_num_channels == 3, false);

  // This next bit is copied and pasted from stbi__hdr_load so that we can
  // avoid making an unnecessary extra copy of the data.
  char buffer[STBI__HDR_BUFLEN];
  char *token;
  int valid = 0;
  int width, height;
  stbi_uc *scanline;
  int len;
  unsigned char count, value;
  int i, j, k, c1, c2, z;

  // Check identifier
  if (strcmp(stbi__hdr_gettoken(&_context, buffer), "#?RADIANCE") != 0) {
    pnmimage_cat.error()
      << "Missing #?RADIANCE header.\n";
    return false;
  }

  // Parse header
  for(;;) {
    token = stbi__hdr_gettoken(&_context, buffer);
    if (token[0] == 0) break;
    if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0) valid = 1;
  }

  if (!valid) {
    pnmimage_cat.error() << "Unsupported HDR format.\n";
    return false;
  }

  // Parse width and height
  // can't use sscanf() if we're not using stdio!
  token = stbi__hdr_gettoken(&_context, buffer);
  if (strncmp(token, "-Y ", 3)) {
    pnmimage_cat.error() << "Unsupported HDR data layout.\n";
    return false;
  }
  token += 3;
  height = (int) strtol(token, &token, 10);
  while (*token == ' ') ++token;
  if (strncmp(token, "+X ", 3)) {
    pnmimage_cat.error() << "Unsupported HDR data layout.\n";
    return false;
  }
  token += 3;
  width = (int) strtol(token, NULL, 10);

  // Read data
  pfm.clear(width, height, 3);
  vector_float table;
  pfm.swap_table(table);
  float *hdr_data = (float *)&table[0];

  // Load image data
  // image data is stored as some number of sca
  if (width < 8 || width >= 32768) {
    // Read flat data
    for (j = 0; j < height; ++j) {
      for (i = 0; i < width; ++i) {
        stbi_uc rgbe[4];
main_decode_loop:
        stbi__getn(&_context, rgbe, 4);
        stbi__hdr_convert(hdr_data + j * width * 3 + i * 3, rgbe, 3);
      }
    }
  } else {
    // Read RLE-encoded data
    scanline = NULL;

    for (j = 0; j < height; ++j) {
      c1 = stbi__get8(&_context);
      c2 = stbi__get8(&_context);
      len = stbi__get8(&_context);
      if (c1 != 2 || c2 != 2 || (len & 0x80)) {
        // not run-length encoded, so we have to actually use THIS data as a decoded
        // pixel (note this can't be a valid pixel--one of RGB must be >= 128)
        stbi_uc rgbe[4];
        rgbe[0] = (stbi_uc) c1;
        rgbe[1] = (stbi_uc) c2;
        rgbe[2] = (stbi_uc) len;
        rgbe[3] = (stbi_uc) stbi__get8(&_context);
        stbi__hdr_convert(hdr_data, rgbe, 3);
        i = 1;
        j = 0;
        STBI_FREE(scanline);
        goto main_decode_loop; // yes, this makes no sense
      }
      len <<= 8;
      len |= stbi__get8(&_context);
      if (len != width) {
        STBI_FREE(scanline);
        pnmimage_cat.error() << "Corrupt HDR: invalid decoded scanline length.\n";
        return false;
      }
      if (scanline == NULL) {
        scanline = (stbi_uc *) stbi__malloc(width * 4);
      }

      for (k = 0; k < 4; ++k) {
        i = 0;
        while (i < width) {
          count = stbi__get8(&_context);
          if (count > 128) {
            // Run
            value = stbi__get8(&_context);
            count -= 128;
            for (z = 0; z < count; ++z) {
              scanline[i++ * 4 + k] = value;
            }
          } else {
            // Dump
            for (z = 0; z < count; ++z) {
              scanline[i++ * 4 + k] = stbi__get8(&_context);
            }
          }
        }
      }
      for (i = 0; i < width; ++i) {
        stbi__hdr_convert(hdr_data+(j*width + i)*3, scanline + i*4, 3);
      }
    }
    STBI_FREE(scanline);
  }

  pfm.swap_table(table);
  return true;
}

/**
 * Reads in an entire image all at once, storing it in the pre-allocated
 * _x_size * _y_size array and alpha pointers.  (If the image type has no
 * alpha channel, alpha is ignored.)  Returns the number of rows correctly
 * read.
 *
 * Derived classes need not override this if they instead provide
 * supports_read_row() and read_row(), below.
 */
int StbImageReader::
read_data(xel *array, xelval *alpha) {
  // Reposition the file at the beginning.
  _file->seekg(0, ios::beg);
  if (_file->tellg() != 0) {
    pnmimage_cat.error()
      << "Could not reposition file pointer to the beginning.\n";
    return 0;
  }

  stbi__start_callbacks(&_context, &io_callbacks, (void *)_file);

  int cols = 0;
  int rows = 0;
  stbi_uc *data = stbi__load_main(&_context, &cols, &rows, NULL, _num_channels);

  if (data == NULL) {
    pnmimage_cat.error()
      << "stbi_load failure: " << stbi_failure_reason() << "\n";
    return 0;
  }

  nassertr(cols == _x_size, 0);

  size_t pixels = (size_t)_x_size * (size_t)rows;
  stbi_uc *ptr = data;
  switch (_num_channels) {
  case 1:
    for (size_t i = 0; i < pixels; ++i) {
      PPM_ASSIGN(array[i], ptr[i], ptr[i], ptr[i]);
    }
    break;

  case 2:
    for (size_t i = 0; i < pixels; ++i) {
      PPM_ASSIGN(array[i], ptr[0], ptr[0], ptr[0]);
      alpha[i] = ptr[1];
      ptr += 2;
    }
    break;

  case 3:
    for (size_t i = 0; i < pixels; ++i) {
      PPM_ASSIGN(array[i], ptr[0], ptr[1], ptr[2]);
      ptr += 3;
    }
    break;

  case 4:
    for (size_t i = 0; i < pixels; ++i) {
      PPM_ASSIGN(array[i], ptr[0], ptr[1], ptr[2]);
      alpha[i] = ptr[3];
      ptr += 4;
    }
    break;
  }

  stbi_image_free(data);
  return rows;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeStbImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeStbImage);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeStbImage::
make_PNMFileTypeStbImage(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_STB_IMAGE
