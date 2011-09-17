// Filename: pnmImageHeader.cxx
// Created by:  drose (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pnmImageHeader.h"
#include "pnmFileTypeRegistry.h"
#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"
#include "config_pnmimage.h"
#include "virtualFileSystem.h"
#include "zStream.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::read_header
//       Access: Published
//  Description: Opens up the image file and tries to read its header
//               information to determine its size, number of
//               channels, etc.  If successful, updates the header
//               information and returns true; otherwise, returns
//               false.
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
read_header(const Filename &filename, PNMFileType *type,
            bool report_unknown_type) {
  PNMReader *reader = make_reader(filename, type, report_unknown_type);
  if (reader != (PNMReader *)NULL) {
    (*this) = (*reader);
    delete reader;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::read_header
//       Access: Published
//  Description: Reads the image header information only from the
//               indicated stream.
//
//               The filename is advisory only, and may be used
//               to suggest a type if it has a known extension.
//
//               If type is non-NULL, it is a suggestion for the type
//               of file it is (and a non-NULL type will override any
//               magic number test or filename extension lookup).
//
//               Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
read_header(istream &data, const string &filename, PNMFileType *type,
            bool report_unknown_type) {
  PNMReader *reader = PNMImageHeader::make_reader
    (&data, false, filename, string(), type, report_unknown_type);
  if (reader != (PNMReader *)NULL) {
    (*this) = (*reader);
    delete reader;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_reader
//       Access: Published
//  Description: Returns a newly-allocated PNMReader of the suitable
//               type for reading from the indicated image filename,
//               or NULL if the filename cannot be read for some
//               reason.  The filename "-" always stands for standard
//               input.  If type is specified, it is a suggestion for
//               the file type to use.
//
//               The PNMReader should be deleted when it is no longer
//               needed.
////////////////////////////////////////////////////////////////////
PNMReader *PNMImageHeader::
make_reader(const Filename &filename, PNMFileType *type,
            bool report_unknown_type) const {
  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
      << "Reading image from " << filename << "\n";
  }
  bool owns_file = false;
  istream *file = (istream *)NULL;

  if (filename == "-") {
    owns_file = false;
    file = &cin;

    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
        << "(reading standard input)\n";
    }
  } else {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    owns_file = true;
    file = vfs->open_read_file(filename, true);
  }

  if (file == (istream *)NULL) {
    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
        << "Unable to open file.\n";
    }
    return NULL;
  }

  return make_reader(file, owns_file, filename, string(), type, 
                     report_unknown_type);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_reader
//       Access: Published
//  Description: Returns a newly-allocated PNMReader of the suitable
//               type for reading from the already-opened image file,
//               or NULL if the file cannot be read for some reason.
//
//               owns_file should be set true if the PNMReader is to
//               be considered the owner of the stream pointer (in
//               which case the stream will be deleted on completion,
//               whether successful or not), or false if it should not
//               delete it.
//
//               The filename parameter is optional here, since the
//               file has already been opened; it is only used to
//               examine the extension and attempt to guess the file
//               type.
//
//               If magic_number is nonempty, it is assumed to
//               represent the first few bytes that have already been
//               read from the file.  Some file types may have
//               difficulty if this is more than two bytes.
//
//               If type is non-NULL, it is a suggestion for the file
//               type to use.
//
//               The PNMReader should be deleted when it is no longer
//               needed.
////////////////////////////////////////////////////////////////////
PNMReader *PNMImageHeader::
make_reader(istream *file, bool owns_file, const Filename &filename,
            string magic_number, PNMFileType *type,
            bool report_unknown_type) const {
  if (type == (PNMFileType *)NULL) {
    if (!read_magic_number(file, magic_number, 2)) {
      // No magic number.  No image.
      if (pnmimage_cat.is_debug()) {
        pnmimage_cat.debug()
          << "Image file appears to be empty.\n";
      }
      if (owns_file) {
        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

        // We're assuming here that the file was opened via VFS.  That
        // may not necessarily be the case, but we don't make that
        // distinction.  However, at the moment at least, that
        // distinction doesn't matter, since vfs->close_read_file()
        // just deletes the file pointer anyway.
        vfs->close_read_file(file);
      }
      return NULL;
    }

    type = PNMFileTypeRegistry::get_global_ptr()->
      get_type_from_magic_number(magic_number);

    if (pnmimage_cat.is_debug()) {
      if (type != (PNMFileType *)NULL) {
        pnmimage_cat.debug()
          << "By magic number, image file appears to be type "
          << type->get_name() << ".\n";
      } else {
        pnmimage_cat.debug()
          << "Unable to determine image file type from magic number.\n";
      }
    }
  }

  if (type == (PNMFileType *)NULL && !filename.empty()) {
    // We still don't know the type; attempt to guess it from the
    // filename extension.
    type = PNMFileTypeRegistry::get_global_ptr()->get_type_from_extension(filename);

    if (pnmimage_cat.is_debug()) {
      if (type != (PNMFileType *)NULL) {
        pnmimage_cat.debug()
          << "From its extension, image file is probably type "
          << type->get_name() << ".\n";
      } else {
        pnmimage_cat.debug()
          << "Unable to guess image file type from its extension.\n";
      }
    }
  }

  if (type == (PNMFileType *)NULL) {
    // No?  How about the default type associated with this image header.
    type = _type;

    if (pnmimage_cat.is_debug() && type != (PNMFileType *)NULL) {
      pnmimage_cat.debug()
        << "Assuming image file type is " << type->get_name() << ".\n";
    }
  }

  if (type == (PNMFileType *)NULL) {
    // We can't figure out what type the file is; give up.
    if (report_unknown_type && pnmimage_cat.is_error()) {
      pnmimage_cat.error()
        << "Cannot determine type of image file " << filename << ".\n"
        << "Currently supported image types:\n";
      PNMFileTypeRegistry::get_global_ptr()->
        write(pnmimage_cat.error(false), 2);
    }
    if (owns_file) {
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

      // We're assuming here that the file was opened via VFS.  That
      // may not necessarily be the case, but we don't make that
      // distinction.  However, at the moment at least, that
      // distinction doesn't matter, since vfs->close_read_file()
      // just deletes the file pointer anyway.
      vfs->close_read_file(file);
    }
    return NULL;
  }

  PNMReader *reader = type->make_reader(file, owns_file, magic_number);
  if (reader == NULL && owns_file) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(file);
  }

  if (!reader->is_valid()) {
    delete reader;
    reader = NULL;
  }

  return reader;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_writer
//       Access: Published
//  Description: Returns a newly-allocated PNMWriter of the suitable
//               type for writing an image to the indicated filename,
//               or NULL if the filename cannot be written for some
//               reason.  The filename "-" always stands for standard
//               output.  If type is specified, it is a suggestion for
//               the file type to use.
//
//               The PNMWriter should be deleted when it is no longer
//               needed.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMImageHeader::
make_writer(const Filename &filename, PNMFileType *type) const {
  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
      << "Writing image to " << filename << "\n";
  }
  bool owns_file = false;
  ostream *file = (ostream *)NULL;

  if (filename == "-") {
    owns_file = false;
    file = &cout;

    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
        << "(writing to standard output)\n";
    }

  } else {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    Filename actual_name = Filename::binary_filename(filename);
    file = vfs->open_write_file(actual_name, true, true);
    if (file != NULL) {
      owns_file = true;
    }
  }

  if (file == (ostream *)NULL) {
    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
        << "Unable to write to file.\n";
    }
    return NULL;
  }

  return make_writer(file, owns_file, filename, type);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_writer
//       Access: Published
//  Description: Returns a newly-allocated PNMWriter of the suitable
//               type for writing to the already-opened image file, or
//               NULL if the file cannot be written for some reason.
//
//               owns_file should be set true if the PNMWriter is to
//               be considered the owner of the stream pointer (in
//               which case the stream will be deleted on completion,
//               whether successful or not), or false if it should not
//               delete it.
//
//               The filename parameter is optional here, since the
//               file has already been opened; it is only used to
//               examine the extension and attempt to guess the
//               intended file type.
//
//               If type is non-NULL, it is a suggestion for the file
//               type to use.
//
//               The PNMWriter should be deleted when it is no longer
//               needed.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMImageHeader::
make_writer(ostream *file, bool owns_file, const Filename &filename,
            PNMFileType *type) const {
  if (type == (PNMFileType *)NULL && !filename.empty()) {
    // We don't know the type; attempt to guess it from the filename
    // extension.
    type = PNMFileTypeRegistry::get_global_ptr()->get_type_from_extension(filename);

    if (pnmimage_cat.is_debug()) {
      if (type != (PNMFileType *)NULL) {
        pnmimage_cat.debug()
          << "From its extension, image file is intended to be type "
          << type->get_name() << ".\n";
      } else {
        pnmimage_cat.debug()
          << "Unable to guess image file type from its extension.\n";
      }
    }
  }

  if (type == (PNMFileType *)NULL) {
    // No?  How about the default type associated with this image header.
    type = _type;

    if (pnmimage_cat.is_debug() && type != (PNMFileType *)NULL) {
      pnmimage_cat.debug()
        << "Assuming image file type is " << type->get_name() << ".\n";
    }
  }

  if (type == (PNMFileType *)NULL) {
    // We can't figure out what type the file is; give up.
    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
        << "Cannot determine type of image file " << filename << ".\n";
    }
    if (owns_file) {
      delete file;
    }
    return NULL;
  }

  PNMWriter *writer = type->make_writer(file, owns_file);
  if (writer == NULL && owns_file) {
    delete file;
  }

  if (!writer->is_valid()) {
    delete writer;
    writer = NULL;
  }

  return writer;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::read_magic_number
//       Access: Published, Static
//  Description: Ensures that the first n bytes of the file are read
//               into magic_number.  If magic_number is initially
//               nonempty, assumes these represent the first few bytes
//               already extracted.  Returns true if successful, false
//               if an end of file or error occurred before num_bytes
//               could be read.
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
read_magic_number(istream *file, string &magic_number, int num_bytes) {
  while ((int)magic_number.size() < num_bytes) {
    int ch = file->get();
    if (file->eof() || file->fail()) {
      return false;
    }
    magic_number += (char)ch;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PNMImageHeader::
output(ostream &out) const {
  out << "image: " << _x_size << " by " << _y_size << " pixels, "
      << _num_channels << " channels, " << _maxval << " maxval.";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::compute_histogram
//       Access: Protected
//  Description: Computes a histogram of the colors used in the
//               indicated rgb/grayscale array and/or alpha array.
//               This is most likely to be useful in a PNMWriter
//               class, but it is defined at this level in case it has
//               general utilty for PNMImages.
//
//               Also see PNMImage::make_histogram(), which is a
//               higher-level function.
//
//               The max_colors parameter, if greater than zero,
//               limits the maximum number of colors we are interested
//               in.  If we encounter more than this number of colors,
//               the function aborts before completion and returns
//               false; otherwise, it returns true.
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
compute_histogram(PNMImageHeader::HistMap &hist, 
                  xel *array, xelval *alpha, int max_colors) {
  int num_pixels = _x_size * _y_size;
  int pi;

  switch (get_color_type()) {
  case CT_invalid:
    return false;

  case CT_grayscale:
    for (pi = 0; pi < num_pixels; pi++) {
      record_color(hist, PixelSpec(PPM_GETB(array[pi])));
      if (max_colors > 0 && (int)hist.size() > max_colors) {
        return false;
      }
    }
    return true;

  case CT_two_channel:
    for (pi = 0; pi < num_pixels; pi++) {
      record_color(hist, PixelSpec(PPM_GETB(array[pi]), alpha[pi]));
      if (max_colors > 0 && (int)hist.size() > max_colors) {
        return false;
      }
    }
    return true;

  case CT_color:
    for (pi = 0; pi < num_pixels; pi++) {
      record_color(hist, PixelSpec(PPM_GETR(array[pi]), PPM_GETG(array[pi]), PPM_GETB(array[pi])));
      if (max_colors > 0 && (int)hist.size() > max_colors) {
        return false;
      }
    }
    return true;

  case CT_four_channel:
    for (pi = 0; pi < num_pixels; pi++) {
      record_color(hist, PixelSpec(PPM_GETR(array[pi]), PPM_GETG(array[pi]), PPM_GETB(array[pi]), alpha[pi]));
      if (max_colors > 0 && (int)hist.size() > max_colors) {
        return false;
      }
    }
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::compute_palette
//       Access: Protected
//  Description: Returns a linear list of all of the colors in the
//               image, similar to compute_histogram().
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
compute_palette(PNMImageHeader::Palette &palette, 
                xel *array, xelval *alpha, int max_colors) {
  HistMap hist;

  int num_pixels = _x_size * _y_size;

  // In case there are already entries in the palette, preserve them.
  Palette::const_iterator pi;
  for (pi = palette.begin(); pi != palette.end(); ++pi) {
    hist.insert(HistMap::value_type(*pi, num_pixels + 1));
  }

  if (!compute_histogram(hist, array, alpha, max_colors)) {
    return false;
  }

  // Now append the new entries discovered in the histogram onto the
  // end of the palette.
  palette.reserve(hist.size());
  HistMap::const_iterator hi;
  for (hi = hist.begin(); hi != hist.end(); ++hi) {
    if ((*hi).second <= num_pixels) {
      palette.push_back((*hi).first);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::PixelSpec::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PNMImageHeader::PixelSpec::
output(ostream &out) const {
  out << "(" << _red << ", " << _green << ", " << _blue << ", " << _alpha << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::Histogram::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PNMImageHeader::Histogram::
write(ostream &out) const {
  out << "Histogram: {\n";
  PixelCount::const_iterator pi;
  for (pi = _pixels.begin(); pi != _pixels.end(); ++pi) {
    out << "  " << (*pi)._pixel << ": " << (*pi)._count << ",\n";
  }
  out << "}\n";
}

