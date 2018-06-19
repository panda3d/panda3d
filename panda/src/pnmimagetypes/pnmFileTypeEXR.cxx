/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeEXR.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "pnmFileTypeEXR.h"

#ifdef HAVE_OPENEXR

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"
#include "pfmFile.h"

#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfVersion.h>
#include <ImfIO.h>

#ifndef IMATH_NAMESPACE
#define IMATH_NAMESPACE Imath
#endif

using std::istream;
using std::ostream;
using std::string;

TypeHandle PNMFileTypeEXR::_type_handle;

static const char * const extensions_exr[] = {
  "exr"
};
static const int num_extensions_exr = sizeof(extensions_exr) / sizeof(const char *);

// A wrapper class to map OpenEXR's OStream class onto std::ostream.
class ImfStdOstream : public IMF::OStream {
public:
  ImfStdOstream(std::ostream &strm) : IMF::OStream("ostream"), _strm(strm) {}

  virtual void write(const char c[/*n*/], int n) {
    _strm.write(c, n);
  }

  virtual IMF::Int64 tellp() {
    return _strm.tellp();
  }

  virtual void seekp(IMF::Int64 pos) {
    _strm.seekp(pos);
  }

private:
  std::ostream &_strm;
};

// A wrapper class to map OpenEXR's IStream class onto std::istream.
class ImfStdIstream : public IMF::IStream {
public:
  ImfStdIstream(std::istream &strm, const std::string &magic_number) : IMF::IStream("istream"), _strm(strm) {
    // Start by putting back the magic number.
    for (std::string::const_reverse_iterator mi = magic_number.rbegin();
         mi != magic_number.rend();
         mi++) {
      _strm.putback(*mi);
    }
  }

  virtual bool isMemoryMapped () const {
    return false;
  }

  virtual bool read (char c[/*n*/], int n) {
    _strm.read(c, n);
    if (_strm.gcount() != n) {
      throw std::exception();
    }

    bool not_eof = !_strm.eof();
    return not_eof;
  }

  virtual IMF::Int64 tellg() {
    return _strm.tellg();
  }

  virtual void seekg(IMF::Int64 pos) {
    _strm.seekg(pos);
  }

  virtual void clear() {
    _strm.clear();
  }

private:
  std::istream &_strm;
};

PNMFileTypeEXR::
PNMFileTypeEXR() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeEXR::
get_name() const {
  return "OpenEXR";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypeEXR::
get_num_extensions() const {
  return num_extensions_exr;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeEXR::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_exr, string());
  return extensions_exr[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypeEXR::
get_suggested_extension() const {
  return "exr";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypeEXR::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypeEXR::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);

  if (magic_number.size() >= 4) {
    // If we have already read all four bytes, use the built-in
    // function to check them.
    return IMF::isImfMagic(magic_number.data());
  } else {
    // Otherwise, check only the first two bytes and call it good enough.
    return magic_number[0] == ((IMF::MAGIC >>  0) & 0x00ff) &&
      magic_number[1] == ((IMF::MAGIC >>  8) & 0x00ff);
  }
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeEXR::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeEXR::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}

/**
 *
 */
PNMFileTypeEXR::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file),
  _strm(new ImfStdIstream(*_file, magic_number)),
  _imf_file(*_strm)
{
  const IMF::Header &header = _imf_file.header();

  IMATH_NAMESPACE::Box2i dw = header.dataWindow();
  _x_size = dw.max.x - dw.min.x + 1;
  _y_size = dw.max.y - dw.min.y + 1;

  // Find the channels we care about, and ensure they're placed in the
  // correct order.
  _channel_names.clear();

  const IMF::ChannelList &channels = header.channels();

  // Note: including Y in this list allows us to handle grayscale or
  // grayscale/alpha images correctly, but also incorrectly detects
  // luminance/chroma images as grayscale only.  However, these kind
  // of images are a pain to handle anyway, so maybe that's OK.
  const char *possible_channel_names[] = { "R", "G", "B", "Y", "A", nullptr };
  for (const char **pni = possible_channel_names; *pni != nullptr; ++pni) {
    std::string name = *pni;
    IMF::ChannelList::ConstIterator ci = channels.find(name.c_str());
    if (ci != channels.end()) {
      // Found a match.
      if (name == "Y" && !_channel_names.empty()) {
        // Y is luminance or grayscale.  Ignore Y if there are
        // already any RGB channels.
      } else {
        _channel_names.push_back(name);
      }
    }
  }

  if (_channel_names.empty()) {
    // Didn't find any channel names that match R, G, B, A, so just
    // ask for RGB anyway and trust the OpenEXR library to do the
    // right thing.  Actually, it just fills them with black, but
    // whatever.
    _channel_names.push_back("R");
    _channel_names.push_back("G");
    _channel_names.push_back("B");
  }

  _num_channels = (int)_channel_names.size();
  if (_num_channels == 0 || _num_channels > 4) {
    _is_valid = false;
    return;
  }
  // We read all OpenEXR files to floating-point, even UINT type, so
  // _maxval doesn't matter.  But we set it anyway.
  _maxval = 65535;

  _is_valid = true;
}

/**
 *
 */
PNMFileTypeEXR::Reader::
~Reader() {
  delete _strm;
}

/**
 * Returns true if this PNMFileType represents a floating-point image type,
 * false if it is a normal, integer type.  If this returns true, read_pfm() is
 * implemented instead of read_data().
 */
bool PNMFileTypeEXR::Reader::
is_floating_point() {
  // We read everything to floating-point, since even the UINT type is
  // 32 bits, more fidelity than we can represent in our 16-bit
  // PNMImage.
  return true;
}

/**
 * Reads floating-point data directly into the indicated PfmFile.  Returns
 * true on success, false on failure.
 */
bool PNMFileTypeEXR::Reader::
read_pfm(PfmFile &pfm) {
  pfm.clear(_x_size, _y_size, _num_channels);
  vector_float table;
  pfm.swap_table(table);

  PN_float32 *table_data = table.data();
  size_t x_stride = sizeof(PN_float32) * pfm.get_num_channels();
  size_t y_stride = x_stride * pfm.get_x_size();
  nassertr(y_stride * pfm.get_y_size() <= table.size() * sizeof(PN_float32), false);

  const IMF::Header &header = _imf_file.header();
  IMATH_NAMESPACE::Box2i dw = header.dataWindow();

  IMF::FrameBuffer frameBuffer;
  for (int ci = 0; ci < pfm.get_num_channels(); ++ci) {
    char *base = (char *)(table_data - (dw.min.x + dw.min.y * pfm.get_x_size()) * pfm.get_num_channels() + ci);
    frameBuffer.insert(_channel_names[ci].c_str(),
                       IMF::Slice(IMF::FLOAT, base, x_stride, y_stride,
                                  1, 1, 0.0));
  }

  _imf_file.setFrameBuffer(frameBuffer);

  try {
    _imf_file.readPixels(dw.min.y, dw.max.y);
  } catch (const std::exception &exc) {
    pnmimage_exr_cat.error()
      << exc.what() << "\n";
    return false;
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
int PNMFileTypeEXR::Reader::
read_data(xel *array, xelval *alpha) {
  // This should never come here, since we always read to
  // floating-point data.
  nassertr(false, 0);
  return 0;
}

/**
 *
 */
PNMFileTypeEXR::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

/**
 * Returns true if this PNMFileType can accept a floating-point image type,
 * false if it can only accept a normal, integer type.  If this returns true,
 * write_pfm() is implemented.
 */
bool PNMFileTypeEXR::Writer::
supports_floating_point() {
  return true;
}

/**
 * Returns true if this PNMFileType can accept an integer image type, false if
 * it can only accept a floating-point type.  If this returns true,
 * write_data() or write_row() is implemented.
 */
bool PNMFileTypeEXR::Writer::
supports_integer() {
  return false;
}

/**
 * Writes floating-point data from the indicated PfmFile.  Returns true on
 * success, false on failure.
 */
bool PNMFileTypeEXR::Writer::
write_pfm(const PfmFile &pfm) {
  const vector_float &table = pfm.get_table();
  const PN_float32 *table_data = table.data();
  size_t x_stride = sizeof(PN_float32) * pfm.get_num_channels();
  size_t y_stride = x_stride * pfm.get_x_size();
  nassertr(y_stride * pfm.get_y_size() <= table.size() * sizeof(PN_float32), false);

  const char *channel_names_1[] = { "G" };
  const char *channel_names_2[] = { "G", "A" };
  const char *channel_names_3[] = { "R", "G", "B" };
  const char *channel_names_4[] = { "R", "G", "B", "A" };
  const char **channel_names = nullptr;

  switch (pfm.get_num_channels()) {
  case 1:
    channel_names = channel_names_1;
    break;

  case 2:
    channel_names = channel_names_2;
    break;

  case 3:
    channel_names = channel_names_3;
    break;

  case 4:
    channel_names = channel_names_4;
    break;

  default:
    return false;
  };

  IMF::Header header(pfm.get_x_size(), pfm.get_y_size());
  for (int ci = 0; ci < pfm.get_num_channels(); ++ci) {
    header.channels().insert(channel_names[ci], IMF::Channel(IMF::FLOAT));
  }

  IMF::FrameBuffer frameBuffer;
  for (int ci = 0; ci < pfm.get_num_channels(); ++ci) {
    const char *base = (const char *)(table_data + ci);
    frameBuffer.insert(channel_names[ci],
                       IMF::Slice(IMF::FLOAT, (char *)base, x_stride, y_stride));
  }

  ImfStdOstream strm(*_file);
  IMF::OutputFile file(strm, header);
  file.setFrameBuffer(frameBuffer);

  try {
    file.writePixels(pfm.get_y_size());
  } catch (const std::exception &exc) {
    pnmimage_exr_cat.error()
      << exc.what() << "\n";
    return false;
  }

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
int PNMFileTypeEXR::Writer::
write_data(xel *array, xelval *alpha) {
  // This should never come here, since we always write to
  // floating-point data.
  nassertr(false, 0);
  return 0;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeEXR::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeEXR);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeEXR::
make_PNMFileTypeEXR(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_OPENEXR
