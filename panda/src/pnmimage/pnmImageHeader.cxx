// Filename: pnmImageHeader.cxx
// Created by:  drose (14Jun00)
// 
////////////////////////////////////////////////////////////////////
  
#include "pnmImageHeader.h"
#include "pnmFileTypeRegistry.h"
#include "pnmFileType.h"
#include "pnmReader.h"
#include "config_pnmimage.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::read_header
//       Access: Public, Static
//  Description: Opens up the image file and tries to read its header
//               information to determine its size, number of
//               channels, etc.  If successful, updates the header
//               information and returns true; otherwise, returns
//               false.
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
read_header(const Filename &filename, PNMFileType *type) {
  PNMReader *reader = make_reader(filename, type);
  if (reader != (PNMReader *)NULL) {
    (*this) = (*reader);
    _type = reader->get_type();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_reader
//       Access: Public, Static
//  Description: Returns a PNMReader of the suitable type for reading
//               from the indicated image filename, or NULL if the
//               filename cannot be read for some reason.  The
//               filename "-" always stands for standard input.  If
//               type is specified, it is a suggestion for the file
//               type to use.
////////////////////////////////////////////////////////////////////
PNMReader *PNMImageHeader::
make_reader(const Filename &filename, PNMFileType *type) const {
  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
      << "Reading image from " << filename << "\n";
  }
  Filename actual_name = Filename::binary_filename(filename);
  bool owns_file;
  FILE *file;

  if (filename == "-") {
    owns_file = false;
    file = stdin;

    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
	<< "(reading standard input)\n";
    }

  } else {
    if (!actual_name.exists()) {
      if (pnmimage_cat.is_debug()) {
	pnmimage_cat.debug()
	  << "File " << filename << " does not exist.\n";
      }
      // The file doesn't exist.  Don't pass it through libpnm, which
      // will abort.
      return NULL;
    } 
    owns_file = true;
    file = pm_openr((char *)actual_name.to_os_specific().c_str());
  }

  if (file == (FILE *)NULL) {
    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
	<< "Unable to open file.\n";
    }
    return NULL;
  }

  return make_reader(file, owns_file, filename, string(), type);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_reader
//       Access: Public, Static
//  Description: Returns a PNMReader of the suitable type for reading
//               from the already-opened image file, or NULL if the
//               file cannot be read for some reason.  
//
//               owns_file should be set true if the PNMReader is to
//               be considered the owner of the FILE stream (in which
//               case the stream will be closed on completion, whether
//               successful or not), or false if it should not close
//               it.
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
////////////////////////////////////////////////////////////////////
PNMReader *PNMImageHeader::
make_reader(FILE *file, bool owns_file, const Filename &filename,
	    string magic_number, PNMFileType *type) const {
  if (type == (PNMFileType *)NULL) {
    if (!read_magic_number(file, magic_number, 2)) {
      // No magic number.  No image.
      if (pnmimage_cat.is_debug()) {
	pnmimage_cat.debug()
	  << "Image file appears to be empty.\n";
      }
      if (owns_file) {
	pm_close(file);
      }
      return NULL;
    }

    type = PNMFileTypeRegistry::get_ptr()->
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
    type = PNMFileTypeRegistry::get_ptr()->get_type_from_extension(filename);

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
    if (pnmimage_cat.is_error()) {
      pnmimage_cat.error()
	<< "Cannot determine type of image file " << filename << ".\n"
	<< "Currently supported image types:\n";
      PNMFileTypeRegistry::get_ptr()->
	write_types(pnmimage_cat.error(false), 2);
    }
    if (owns_file) {
      pm_close(file);
    }
    return NULL;
  }

  PNMReader *reader = type->make_reader(file, owns_file, magic_number);
  if (reader == NULL && owns_file) {
    pm_close(file);
  }

  if (!reader->is_valid()) {
    delete reader;
    reader = NULL;
  }

  return reader;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::make_writer
//       Access: Public, Static
//  Description: Returns a PNMWriter of the suitable type for writing
//               an image to the indicated filename, or NULL if the
//               filename cannot be written for some reason.  The
//               filename "-" always stands for standard output.  If
//               type is specified, it is a suggestion for the file
//               type to use.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMImageHeader::
make_writer(const Filename &filename, PNMFileType *type) const {
  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
      << "Writing image to " << filename << "\n";
  }
  Filename actual_name = Filename::binary_filename(filename);
  bool owns_file;
  FILE *file;

  if (filename == "-") {
    owns_file = false;
    file = stdout;

    if (pnmimage_cat.is_debug()) {
      pnmimage_cat.debug()
	<< "(writing to standard output)\n";
    }

  } else {
    owns_file = true;
    file = pm_openw((char *)actual_name.to_os_specific().c_str());
  }

  if (file == (FILE *)NULL) {
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
//       Access: Public, Static
//  Description: Returns a PNMWriter of the suitable type for writing
//               to the already-opened image file, or NULL if the
//               file cannot be written for some reason.  
//
//               owns_file should be set true if the PNMWriter is to
//               be considered the owner of the FILE stream (in which
//               case the stream will be closed on completion, whether
//               successful or not), or false if it should not close
//               it.
//
//               The filename parameter is optional here, since the
//               file has already been opened; it is only used to
//               examine the extension and attempt to guess the
//               intended file type.
//
//               If type is non-NULL, it is a suggestion for the file
//               type to use.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMImageHeader::
make_writer(FILE *file, bool owns_file, const Filename &filename,
	    PNMFileType *type) const {
  if (type == (PNMFileType *)NULL && !filename.empty()) {
    // We don't know the type; attempt to guess it from the filename
    // extension.
    type = PNMFileTypeRegistry::get_ptr()->get_type_from_extension(filename);

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
      pm_close(file);
    }
    return NULL;
  }

  PNMWriter *writer = type->make_writer(file, owns_file);
  if (writer == NULL && owns_file) {
    pm_close(file);
  }

  return writer;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::read_magic_number
//       Access: Public, Static
//  Description: Ensures that the first n bytes of the file are read
//               into magic_number.  If magic_number is initially
//               nonempty, assumes these represent the first few bytes
//               already extracted.  Returns true if successful, false
//               if an end of file or error occurred before num_bytes
//               could be read.
////////////////////////////////////////////////////////////////////
bool PNMImageHeader::
read_magic_number(FILE *file, string &magic_number, int num_bytes) {
  while (magic_number.size() < num_bytes) {
    int ch = getc(file);
    if (ch == EOF) {
      return false;
    }
    magic_number += (char)ch;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImageHeader::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PNMImageHeader::
output(ostream &out) const {
  out << "image: " << _x_size << " by " << _y_size << " pixels, " 
      << _num_channels << " channels, " << _maxval << " maxval.";
}
