// Filename: multifile.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "multifile.h"

#include "config_express.h"
#include "streamWriter.h"
#include "streamReader.h"
#include "datagram.h"
#include "zStream.h"
#include "encryptStream.h"

#include <algorithm>

// This sequence of bytes begins each Multifile to identify it as a
// Multifile.
const char Multifile::_header[] = "pmf\0\n\r";
const size_t Multifile::_header_size = 6;

// These numbers identify the version of the Multifile.  Generally, a
// change in the major version is intolerable; while a Multifile with
// an older minor version may still be read.
const int Multifile::_current_major_ver = 1;
const int Multifile::_current_minor_ver = 0;

// To confirm that the supplied password matches, we write the
// Mutifile magic header at the beginning of the encrypted stream.
// I suppose this does compromise the encryption security a tiny
// bit by making it easy for crackers to validate that a
// particular password guess matches or doesn't match, but the
// encryption algorithm doesn't depend on this being difficult
// anyway.
const char Multifile::_encrypt_header[] = "crypty";
const size_t Multifile::_encrypt_header_size = 6;



//
// A Multifile consists of the following elements:
//
// (1) A header.  This is always the first n bytes of the Multifile,
// and contains a magic number to identify the file, as well as
// version numbers and any file-specific parameters.
//
//   char[6]    The string Multifile::_header, a magic number.
//   int16      The file's major version number
//   int16      The file's minor version number
//   uint32     Scale factor.  This scales all address references within
//              the file.  Normally 1, this may be set larger to
//              support Multifiles larger than 4GB.

//
// (2) Zero or more index entries, one for each subfile within the
// Multifile.  These entries are of variable length.  The first one of
// these immediately follows the header, and the first word of each
// index entry contains the address of the next index entry.  A zero
// "next" address marks the end of the chain.  These may appear at any
// point within the Multifile; they do not necessarily appear in
// sequential order at the beginning of the file (although they will
// after the file has been "packed").
//
//   uint32     The address of the next entry.
//   uint32     The address of this subfile's data record.
//   uint32     The length in bytes of this subfile's data record.
//   uint16     The Subfile::_flags member.
//  [uint32]    The original, uncompressed and unencrypted length of the
//               subfile, if it is compressed or encrypted.  This field
//               is only present if one or both of the SF_compressed
//               or SF_encrypted bits are set in _flags.
//   uint16     The length in bytes of the subfile's name.
//   char[n]    The subfile's name.
//
// (3) Zero or more data entries, one for each subfile.  These may
// appear at any point within the Multifile; they do not necessarily
// follow each index entry, nor are they necessarily all grouped
// together at the end (although they will be all grouped together at
// the end after the file has been "packed").  These are just blocks
// of literal data.
//

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Multifile::
Multifile() {
  _read = (istream *)NULL;
  _write = (ostream *)NULL;
  _next_index = 0;
  _last_index = 0;
  _needs_repack = false;
  _scale_factor = 1;
  _new_scale_factor = 1;
  _encryption_flag = false;
  _file_major_ver = 0;
  _file_minor_ver = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Multifile::
~Multifile() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Copy Constructor
//       Access: Private
//  Description: Don't try to copy Multifiles.
////////////////////////////////////////////////////////////////////
Multifile::
Multifile(const Multifile &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Copy Assignment Operator
//       Access: Private
//  Description: Don't try to copy Multifiles.
////////////////////////////////////////////////////////////////////
void Multifile::
operator = (const Multifile &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_read
//       Access: Published
//  Description: Opens the named Multifile on disk for reading.  The
//               Multifile index is read in, and the list of subfiles
//               becomes available; individual subfiles may then be
//               extracted or read, but the list of subfiles may not
//               be modified.
//
//               Also see the version of open_read() which accepts an
//               istream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Multifile::
open_read(const Filename &multifile_name) {
  close();
  Filename fname = multifile_name;
  fname.set_binary();
  if (!fname.open_read(_read_file)) {
    return false;
  }
  _read = &_read_file;
  _multifile_name = multifile_name;
  return read_index();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_write
//       Access: Published
//  Description: Opens the named Multifile on disk for writing.  If
//               there already exists a file by that name, it is
//               truncated.  The Multifile is then prepared for
//               accepting a brand new set of subfiles, which will be
//               written to the indicated filename.  Individual
//               subfiles may not be extracted or read.
//
//               Also see the version of open_write() which accepts an
//               ostream.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Multifile::
open_write(const Filename &multifile_name) {
  close();
  Filename fname = multifile_name;
  fname.set_binary();
  if (!fname.open_write(_write_file, true)) {
    return false;
  }
  _write = &_write_file;
  _multifile_name = multifile_name;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_read_write
//       Access: Published
//  Description: Opens the named Multifile on disk for reading and
//               writing.  If there already exists a file by that
//               name, its index is read.  Subfiles may be added or
//               removed, and the resulting changes will be written to
//               the named file.
//
//               Also see the version of open_read_write() which
//               accepts an iostream.  Returns true on success, false
//               on failure.
////////////////////////////////////////////////////////////////////
bool Multifile::
open_read_write(const Filename &multifile_name) {
  close();
  Filename fname = multifile_name;
  fname.set_binary();
  bool exists = fname.exists();
  if (!fname.open_read_write(_read_write_file)) {
    return false;
  }
  _read = &_read_write_file;
  _write = &_read_write_file;
  _multifile_name = multifile_name;

  if (exists) {
    return read_index();
  } else {
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::close
//       Access: Published
//  Description: Closes the Multifile if it is open.  All changes are
//               flushed to disk, and the file becomes invalid for
//               further operations until the next call to open().
////////////////////////////////////////////////////////////////////
void Multifile::
close() {
  if (_new_scale_factor != _scale_factor) {
    // If we have changed the scale factor recently, we need to force
    // a repack.
    repack();
  } else {
    flush();
  }

  _read = (istream *)NULL;
  _write = (ostream *)NULL;
  _next_index = 0;
  _last_index = 0;
  _needs_repack = false;
  _scale_factor = 1;
  _new_scale_factor = 1;
  _file_major_ver = 0;
  _file_minor_ver = 0;

  _read_file.close();
  _write_file.close();
  _read_write_file.close();
  _multifile_name = Filename();

  clear_subfiles();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::set_scale_factor
//       Access: Published
//  Description: Changes the internal scale factor for this Multifile.
//
//               This is normally 1, but it may be set to any
//               arbitrary value (greater than zero) to support
//               Multifile archives that exceed 4GB, if necessary.
//               (Individual subfiles may still not exceed 4GB.)
//
//               All addresses within the file are rounded up to the
//               next multiple of _scale_factor, and zeros are written
//               to the file to fill the resulting gaps.  Then the
//               address is divided by _scale_factor and written out
//               as a 32-bit integer.  Thus, setting a scale factor of
//               2 supports up to 8GB files, 3 supports 12GB files,
//               etc.
//
//               Calling this function on an already-existing
//               Multifile will have no immediate effect until a
//               future call to repack() or close() (or until the
//               Multifile is destructed).
////////////////////////////////////////////////////////////////////
void Multifile::
set_scale_factor(size_t scale_factor) {
  nassertv(is_write_valid());
  nassertv(scale_factor != (size_t)0);

  if (_next_index == (streampos)0) {
    // If it's a brand new Multifile, we can go ahead and set it
    // immediately.
    _scale_factor = scale_factor;
  } else {
    // Otherwise, we'd better have read access so we can repack it
    // later.
    nassertv(is_read_valid());
  }

  // Setting the _new_scale_factor different from the _scale_factor
  // will force a repack operation on close.
  _new_scale_factor = scale_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::add_subfile
//       Access: Published
//  Description: Adds a file on disk as a subfile to the Multifile.
//               The file named by filename will be read and added to
//               the Multifile at the next call to flush().  If there
//               already exists a subfile with the indicated name, it
//               is replaced without examining its contents (but see
//               also update_subfile).
//
//               Returns the subfile name on success (it might have
//               been modified slightly), or empty string on failure.
////////////////////////////////////////////////////////////////////
string Multifile::
add_subfile(const string &subfile_name, const Filename &filename,
            int compression_level) {
  nassertr(is_write_valid(), string());

  if (!filename.exists()) {
    return string();
  }
  string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    Subfile *subfile = new Subfile;
    subfile->_name = name;
    subfile->_source_filename = filename;
    subfile->_source_filename.set_binary();
    
    add_new_subfile(subfile, compression_level);
  }

  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::update_subfile
//       Access: Published
//  Description: Adds a file on disk to the subfile.  If a subfile
//               already exists with the same name, its contents are
//               compared to the disk file, and it is replaced only if
//               it is different; otherwise, the multifile is left
//               unchanged.
////////////////////////////////////////////////////////////////////
string Multifile::
update_subfile(const string &subfile_name, const Filename &filename,
               int compression_level) {
  nassertr(is_write_valid(), string());

  if (!filename.exists()) {
    return string();
  }
  string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    int index = find_subfile(name);
    if (index >= 0) {
      // The subfile already exists; compare it to the source file.
      if (compare_subfile(index, filename)) {
        // The files are identical; do nothing.
        return name;
      }
    }

    // The subfile does not already exist or it is different from the
    // source file.  Add the new source file.
    Subfile *subfile = new Subfile;
    subfile->_name = name;
    subfile->_source_filename = filename;
    subfile->_source_filename.set_binary();

    add_new_subfile(subfile, compression_level);
  }

  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::flush
//       Access: Published
//  Description: Writes all contents of the Multifile to disk.  Until
//               flush() is called, add_subfile() and remove_subfile()
//               do not actually do anything to disk.  At this point,
//               all of the recently-added subfiles are read and their
//               contents are added to the end of the Multifile, and
//               the recently-removed subfiles are marked gone from
//               the Multifile.
//
//               This may result in a suboptimal index.  To guarantee
//               that the index is written at the beginning of the
//               file, call repack() instead of flush().
//
//               It is not necessary to call flush() explicitly unless
//               you are concerned about reading the recently-added
//               subfiles immediately.
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Multifile::
flush() {
  if (!is_write_valid()) {
    return false;
  }

  if (_next_index == (streampos)0) {
    // If we don't have an index yet, we don't have a header.  Write
    // the header.
    if (!write_header()) {
      return false;
    }
  }

  nassertr(_write != (ostream *)NULL, false);

  // First, mark out all of the removed subfiles.
  PendingSubfiles::iterator pi;
  for (pi = _removed_subfiles.begin(); pi != _removed_subfiles.end(); ++pi) {
    Subfile *subfile = (*pi);
    subfile->rewrite_index_flags(*_write);
    delete subfile;
  }
  _removed_subfiles.clear();

  bool wrote_ok = true;

  if (!_new_subfiles.empty()) {
    // Add a few more files to the end.  We always add subfiles at the
    // end of the multifile, so go there first.
    if (_last_index != (streampos)0) {
      _write->seekp(0, ios::end);
      if (_write->fail()) {
        express_cat.info()
          << "Unable to seek Multifile " << _multifile_name << ".\n";
        return false;
      }
      _next_index = _write->tellp();
      _next_index = pad_to_streampos(_next_index);

      // And update the forward link from the last_index to point to
      // this new index location.
      _write->seekp(_last_index);
      StreamWriter writer(_write);
      writer.add_uint32(streampos_to_word(_next_index));
    }

    _write->seekp(_next_index);
    nassertr(_next_index == _write->tellp(), false);
    
    // Ok, here we are at the end of the file.  Write out the
    // recently-added subfiles here.  First, count up the index size.
    for (pi = _new_subfiles.begin(); pi != _new_subfiles.end(); ++pi) {
      Subfile *subfile = (*pi);
      _last_index = _next_index;
      _next_index = subfile->write_index(*_write, _next_index, this);
      nassertr(_next_index == _write->tellp(), false);
      _next_index = pad_to_streampos(_next_index);
      nassertr(_next_index == _write->tellp(), false);
    }
    
    // Now we're at the end of the index.  Write a 0 here to mark the
    // end.
    StreamWriter writer(_write);
    writer.add_uint32(0);
    _next_index += 4;
    nassertr(_next_index == _write->tellp(), false);
    _next_index = pad_to_streampos(_next_index);

    // All right, now write out each subfile's data.
    for (pi = _new_subfiles.begin(); pi != _new_subfiles.end(); ++pi) {
      Subfile *subfile = (*pi);
      _next_index = subfile->write_data(*_write, _read, _next_index,
                                        this);
      nassertr(_next_index == _write->tellp(), false);
      _next_index = pad_to_streampos(_next_index);
      if (subfile->is_data_invalid()) {
        wrote_ok = false;
      }
      nassertr(_next_index == _write->tellp(), false);
    }
    
    // Now go back and fill in the proper addresses for the data start.
    // We didn't do it in the first pass, because we don't really want
    // to keep all those pile handles open, and so we didn't have to
    // determine each pile's length ahead of time.
    for (pi = _new_subfiles.begin(); pi != _new_subfiles.end(); ++pi) {
      Subfile *subfile = (*pi);
      subfile->rewrite_index_data_start(*_write, this);
    }

    _new_subfiles.clear();
  }

  _write->flush();
  if (!wrote_ok || _write->fail()) {
    express_cat.info()
      << "Unable to update Multifile " << _multifile_name << ".\n";
    close();
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::repack
//       Access: Published
//  Description: Forces a complete rewrite of the Multifile and all of
//               its contents, so that its index will appear at the
//               beginning of the file with all of the subfiles listed
//               in alphabetical order.  This is considered optimal
//               for reading, and is the standard configuration; but
//               it is not essential to do this.
//
//               It is only valid to call this if the Multifile was
//               opened using open_read_write() and an explicit
//               filename, rather than an iostream.  Also, we must
//               have write permission to the directory containing the
//               Multifile.
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Multifile::
repack() {
  if (_next_index == (streampos)0) {
    // If the Multifile hasn't yet been written, this is really just a
    // flush operation.
    return flush();
  }

  nassertr(is_write_valid() && is_read_valid(), false);
  nassertr(!_multifile_name.empty(), false);

  // First, we open a temporary filename to copy the Multifile to.
  Filename dirname = _multifile_name.get_dirname();
  if (dirname.empty()) {
    dirname = ".";
  }
  Filename temp_filename = Filename::temporary(dirname, "mftemp");
  temp_filename.set_binary();
  ofstream temp;
  if (!temp_filename.open_write(temp)) {
    express_cat.info()
      << "Unable to open temporary file " << temp_filename << "\n";
    return false;
  }

  // Now we scrub our internal structures so it looks like we're a
  // brand new Multifile.
  PendingSubfiles::iterator pi;
  for (pi = _removed_subfiles.begin(); pi != _removed_subfiles.end(); ++pi) {
    Subfile *subfile = (*pi);
    delete subfile;
  }
  _removed_subfiles.clear();
  _new_subfiles.clear();
  copy(_subfiles.begin(), _subfiles.end(), back_inserter(_new_subfiles));
  _next_index = 0;
  _last_index = 0;
  _scale_factor = _new_scale_factor;

  // And we write our contents to our new temporary file.
  _write = &temp;
  if (!flush()) {
    temp.close();
    temp_filename.unlink();
    return false;
  }

  // Now close everything, and move the temporary file back over our
  // original file.
  Filename orig_name = _multifile_name;
  temp.close();
  close();
  orig_name.unlink();
  if (!temp_filename.rename_to(orig_name)) {
    express_cat.info()
      << "Unable to rename temporary file " << temp_filename << " to "
      << orig_name << ".\n";
    return false;
  }

  if (!open_read_write(orig_name)) {
    express_cat.info()
      << "Unable to read newly repacked " << _multifile_name 
      << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_num_subfiles
//       Access: Published
//  Description: Returns the number of subfiles within the Multifile.
//               The subfiles may be accessed in alphabetical order by
//               iterating through [0 .. get_num_subfiles()).
////////////////////////////////////////////////////////////////////
int Multifile::
get_num_subfiles() const {
  return _subfiles.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::find_subfile
//       Access: Published
//  Description: Returns the index of the subfile with the indicated
//               name, or -1 if the named subfile is not within the
//               Multifile.
////////////////////////////////////////////////////////////////////
int Multifile::
find_subfile(const string &subfile_name) const {
  Subfile find_subfile;
  find_subfile._name = standardize_subfile_name(subfile_name);
  Subfiles::const_iterator fi;
  fi = _subfiles.find(&find_subfile);
  if (fi == _subfiles.end()) {
    // Not present.
    return -1;
  }
  return (fi - _subfiles.begin());
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::has_directory
//       Access: Published
//  Description: Returns true if the indicated subfile name is the
//               directory prefix to one or more files within the
//               Multifile.  That is, the Multifile contains at least
//               one file named "subfile_name/...".
////////////////////////////////////////////////////////////////////
bool Multifile::
has_directory(const string &subfile_name) const {
  string prefix = subfile_name;
  if (!prefix.empty()) {
    prefix += '/';
  }
  Subfile find_subfile;
  find_subfile._name = prefix;
  Subfiles::const_iterator fi;
  fi = _subfiles.upper_bound(&find_subfile);
  if (fi == _subfiles.end()) {
    // Not present.
    return false;
  }

  // At least one subfile exists whose name sorts after prefix.  If it
  // contains prefix as the initial substring, then we have a match.
  Subfile *subfile = (*fi);
  return (subfile->_name.length() > prefix.length() &&
          subfile->_name.substr(0, prefix.length()) == prefix);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::scan_directory
//       Access: Published
//  Description: Considers subfile_name to be the name of a
//               subdirectory within the Multifile, but not a file
//               itself; fills the given vector up with the sorted list
//               of subdirectories or files within the named
//               directory.
//
//               Note that directories do not exist explicitly within
//               a Multifile; this just checks for the existence of
//               files with the given initial prefix.
//
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool Multifile::
scan_directory(vector_string &contents, const string &subfile_name) const {
  string prefix = subfile_name;
  if (!prefix.empty()) {
    prefix += '/';
  }
  Subfile find_subfile;
  find_subfile._name = prefix;
  Subfiles::const_iterator fi;
  fi = _subfiles.upper_bound(&find_subfile);

  string previous = "";
  while (fi != _subfiles.end()) {
    Subfile *subfile = (*fi);
    if (!(subfile->_name.length() > prefix.length() &&
          subfile->_name.substr(0, prefix.length()) == prefix)) {
      // We've reached the end of the list of subfiles beneath the
      // indicated direcotry prefix.
      return true;
    }

    size_t slash = subfile->_name.find('/', prefix.length());
    string basename = subfile->_name.substr(prefix.length(), slash);
    if (basename != previous) {
      contents.push_back(basename);
      previous = basename;
    }
    ++fi;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::remove_subfile
//       Access: Published
//  Description: Removes the nth subfile from the Multifile.  This
//               will cause all subsequent index numbers to decrease
//               by one.  The file will not actually be removed from
//               the disk until the next call to flush().
//
//               Note that this does not actually remove the data from
//               the indicated subfile; it simply removes it from the
//               index.  The Multifile will not be reduced in size
//               after this operation, until the next call to
//               repack().
////////////////////////////////////////////////////////////////////
void Multifile::
remove_subfile(int index) {
  nassertv(is_write_valid());
  nassertv(index >= 0 && index < (int)_subfiles.size());
  Subfile *subfile = _subfiles[index];
  subfile->_flags |= SF_deleted;
  _removed_subfiles.push_back(subfile);
  _subfiles.erase(_subfiles.begin() + index);

  _needs_repack = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_subfile_name
//       Access: Published
//  Description: Returns the name of the nth subfile.
////////////////////////////////////////////////////////////////////
const string &Multifile::
get_subfile_name(int index) const {
#ifndef NDEBUG
  static string empty_string;
  nassertr(index >= 0 && index < (int)_subfiles.size(), empty_string);
#endif
  return _subfiles[index]->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_subfile_length
//       Access: Published
//  Description: Returns the uncompressed data length of the nth
//               subfile.  This might return 0 if the subfile has
//               recently been added and flush() has not yet been
//               called.
////////////////////////////////////////////////////////////////////
size_t Multifile::
get_subfile_length(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_uncompressed_length;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::is_subfile_compressed
//       Access: Published
//  Description: Returns true if the indicated subfile has been
//               compressed when stored within the archive, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool Multifile::
is_subfile_compressed(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return (_subfiles[index]->_flags & SF_compressed) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::is_subfile_encrypted
//       Access: Published
//  Description: Returns true if the indicated subfile has been
//               encrypted when stored within the archive, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool Multifile::
is_subfile_encrypted(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return (_subfiles[index]->_flags & SF_encrypted) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_subfile_internal_length
//       Access: Published
//  Description: Returns the number of bytes the indicated subfile
//               consumes within the archive.  For compressed
//               subfiles, this will generally be smaller than
//               get_subfile_length(); for encrypted (but
//               noncompressed) subfiles, it may be slightly
//               different, for noncompressed and nonencrypted
//               subfiles, it will be equal.
////////////////////////////////////////////////////////////////////
size_t Multifile::
get_subfile_internal_length(int index) const {
  nassertr(index >= 0 && index < (int)_subfiles.size(), 0);
  return _subfiles[index]->_data_length;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_read_subfile
//       Access: Published
//  Description: Returns an istream that may be used to read the
//               indicated subfile.  You may seek() within this
//               istream to your heart's content; even though it will
//               be a reference to the already-opened fstream of the
//               Multifile itself, byte 0 appears to be the beginning
//               of the subfile and EOF appears to be the end of the
//               subfile.
//
//               The returned istream will have been allocated via
//               new; you should delete it when you are finished
//               reading the subfile.
//
//               Any future calls to repack() or close() (or the
//               Multifile destructor) will invalidate all currently
//               open subfile pointers.
//
//               The return value will be NULL if the stream cannot be
//               opened for some reason.
////////////////////////////////////////////////////////////////////
istream *Multifile::
open_read_subfile(int index) {
  nassertr(is_read_valid(), NULL);
  nassertr(index >= 0 && index < (int)_subfiles.size(), NULL);
  Subfile *subfile = _subfiles[index];

  if (subfile->_source != (istream *)NULL ||
      !subfile->_source_filename.empty()) {
    // The subfile has not yet been copied into the physical
    // Multifile.  Force a flush operation to incorporate it.
    flush();

    // That shouldn't change the subfile index or delete the subfile
    // pointer.
    nassertr(subfile == _subfiles[index], NULL);
  }

  // Return an ISubStream object that references into the open
  // Multifile istream.
  nassertr(subfile->_data_start != (streampos)0, NULL);
  istream *stream = 
    new ISubStream(_read, subfile->_data_start,
                   subfile->_data_start + (streampos)subfile->_data_length); 
  
  if ((subfile->_flags & SF_encrypted) != 0) {
#ifndef HAVE_SSL
    express_cat.error()
      << "OpenSSL not compiled in; cannot read encrypted multifiles.\n";
    delete stream;
    return NULL;
#else  // HAVE_SSL
    // The subfile is encrypted.  So actually, return an
    // IDecryptStream that wraps around the ISubStream.
    IDecryptStream *wrapper = 
      new IDecryptStream(stream, true, _encryption_password);
    stream = wrapper;

    // Validate the password by confirming that the encryption header
    // matches.
    char this_header[_encrypt_header_size];
    stream->read(this_header, _encrypt_header_size);
    if (stream->fail() || stream->gcount() != (unsigned)_encrypt_header_size ||
        memcmp(this_header, _encrypt_header, _encrypt_header_size) != 0) {
      express_cat.error()
        << "Unable to decrypt subfile " << subfile->_name << ".\n";
      delete stream;
      return NULL;
    }
#endif  // HAVE_SSL
  }

  if ((subfile->_flags & SF_compressed) != 0) {
#ifndef HAVE_ZLIB
    express_cat.error()
      << "zlib not compiled in; cannot read compressed multifiles.\n";
    delete stream;
    return NULL;
#else  // HAVE_ZLIB
    // Oops, the subfile is compressed.  So actually, return an
    // IDecompressStream that wraps around the ISubStream.
    IDecompressStream *wrapper = new IDecompressStream(stream, true);
    stream = wrapper;
#endif  // HAVE_ZLIB
  }

  if (stream->fail()) {
    // Hmm, some inexplicable problem.
    delete stream;
    return NULL;
  }

  return stream;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::extract_subfile
//       Access: Published
//  Description: Extracts the nth subfile into a file with the given
//               name.
////////////////////////////////////////////////////////////////////
bool Multifile::
extract_subfile(int index, const Filename &filename) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  
  Filename fname = filename;
  fname.set_binary();
  fname.make_dir();
  ofstream out;
  if (!fname.open_write(out, true)) {
    express_cat.info()
      << "Unable to write to file " << filename << "\n";
    return false;
  }

  return extract_subfile_to(index, out);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::compare_subfile
//       Access: Published
//  Description: Performs a byte-for-byte comparison of the indicated
//               file on disk with the nth subfile.  Returns true if
//               the files are equivalent, or false if they are
//               different (or the file is missing).
////////////////////////////////////////////////////////////////////
bool Multifile::
compare_subfile(int index, const Filename &filename) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  if (!filename.exists()) {
    express_cat.info()
      << "File is missing: " << filename << "\n";
    return false;
  }

  istream *in1 = open_read_subfile(index);
  if (in1 == (istream *)NULL) {
    return false;
  }

  ifstream in2;
  Filename bin_filename = Filename::binary_filename(filename);
  if (!bin_filename.open_read(in2)) {
    express_cat.info()
      << "Cannot read " << filename << "\n";
    return false;
  }

  int byte1 = in1->get();
  int byte2 = in2.get();
  while (!in1->fail() && !in1->eof() &&
         !in2.fail() && !in2.eof()) {
    if (byte1 != byte2) {
      delete in1;
      return false;
    }
    byte1 = in1->get();
    byte2 = in2.get();
  }

  bool failed = (in1->fail() && !in1->eof()) || (in2.fail() && !in2.eof());
  delete in1;

  nassertr(!failed, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void Multifile::
output(ostream &out) const {
  out << "Multifile " << _multifile_name << ", " << get_num_subfiles()
      << " subfiles.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::ls
//       Access: Published
//  Description: Shows a list of all subfiles within the Multifile.
////////////////////////////////////////////////////////////////////
void Multifile::
ls(ostream &out) const {
  int num_subfiles = get_num_subfiles();
  for (int i = 0; i < num_subfiles; i++) {
    string subfile_name = get_subfile_name(i);
    out << subfile_name << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::read_subfile
//       Access: Public
//  Description: Fills a string with the entire contents of
//               the indicated subfile.
////////////////////////////////////////////////////////////////////
bool Multifile::
read_subfile(int index, string &result) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);
  result = string();

  istream *in = open_read_subfile(index);
  if (in == (istream *)NULL) {
    return false;
  }

  int byte = in->get();
  while (!in->eof() && !in->fail()) {
    result += (char)byte;
    byte = in->get();
  }
  bool failed = in->fail();
  delete in;
  nassertr(!failed, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_read
//       Access: Public
//  Description: Opens an anonymous Multifile for reading using an
//               istream.  There must be seek functionality via
//               seekg() and tellg() on the istream.
//
//               This version of open_read() does not close the
//               istream when Multifile.close() is called.
////////////////////////////////////////////////////////////////////
bool Multifile::
open_read(istream *multifile_stream) {
  close();
  _read = multifile_stream;
  return read_index();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_write
//       Access: Public
//  Description: Opens an anonymous Multifile for writing using an
//               ostream.  There must be seek functionality via
//               seekp() and tellp() on the pstream.
//
//               This version of open_write() does not close the
//               ostream when Multifile.close() is called.
////////////////////////////////////////////////////////////////////
bool Multifile::
open_write(ostream *multifile_stream) {
  close();
  _write = multifile_stream;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::open_read_write
//       Access: Public
//  Description: Opens an anonymous Multifile for reading and writing
//               using an iostream.  There must be seek functionality
//               via seekg()/seekp() and tellg()/tellp() on the
//               iostream.
//
//               This version of open_read_write() does not close the
//               iostream when Multifile.close() is called.
////////////////////////////////////////////////////////////////////
bool Multifile::
open_read_write(iostream *multifile_stream) {
  close();
  _read = multifile_stream;
  _write = multifile_stream;

  // Check whether the read stream is empty.
  _read->seekg(0, ios::end);
  if (_read->tellg() == (streampos)0) {
    // The read stream is empty, which is always valid.
    return true;
  }

  // The read stream is not empty, so we'd better have a valid
  // Multifile.
  _read->seekg(0);
  return read_index();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::add_subfile
//       Access: Public
//  Description: Adds a file from a stream as a subfile to the Multifile.
//               The indicated istream will be read and its contents
//               added to the Multifile at the next call to flush().
//
//               Returns the subfile name on success (it might have
//               been modified slightly), or empty string on failure.
////////////////////////////////////////////////////////////////////
string Multifile::
add_subfile(const string &subfile_name, istream *subfile_data,
            int compression_level) {
  nassertr(is_write_valid(), string());

  string name = standardize_subfile_name(subfile_name);
  if (!name.empty()) {
    Subfile *subfile = new Subfile;
    subfile->_name = name;
    subfile->_source = subfile_data;
    add_new_subfile(subfile, compression_level);
  }

  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::extract_subfile_to
//       Access: Public
//  Description: Extracts the nth subfile to the indicated ostream.
////////////////////////////////////////////////////////////////////
bool Multifile::
extract_subfile_to(int index, ostream &out) {
  nassertr(is_read_valid(), false);
  nassertr(index >= 0 && index < (int)_subfiles.size(), false);

  istream *in = open_read_subfile(index);
  if (in == (istream *)NULL) {
    return false;
  }

  int byte = in->get();
  while (!in->fail() && !in->eof()) {
    out.put(byte);
    byte = in->get();
  }

  bool failed = (in->fail() && !in->eof());
  delete in;
  nassertr(!failed, false);

  return (!out.fail());
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::pad_to_streampos
//       Access: Private
//  Description: Assumes the _write pointer is at the indicated fpos,
//               rounds the fpos up to the next legitimate address
//               (using normalize_streampos()), and writes enough
//               zeroes to the stream to fill the gap.  Returns the
//               new fpos.
////////////////////////////////////////////////////////////////////
streampos Multifile::
pad_to_streampos(streampos fpos) {
  nassertr(_write != (ostream *)NULL, fpos);
  nassertr(_write->tellp() == fpos, fpos);
  streampos new_fpos = normalize_streampos(fpos);
  while (fpos < new_fpos) {
    _write->put(0);
    fpos += 1; // VC++ doesn't define streampos++ (!)
  }
  nassertr(_write->tellp() == fpos, fpos);
  return fpos;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::add_new_subfile
//       Access: Private
//  Description: Adds a newly-allocated Subfile pointer to the
//               Multifile.
////////////////////////////////////////////////////////////////////
void Multifile::
add_new_subfile(Subfile *subfile, int compression_level) {
  if (compression_level != 0) {
#ifndef HAVE_ZLIB
    express_cat.warning()
      << "zlib not compiled in; cannot generated compressed multifiles.\n";
    compression_level = 0;
#else  // HAVE_ZLIB
    subfile->_flags |= SF_compressed;
    subfile->_compression_level = compression_level;
#endif  // HAVE_ZLIB
  }

#ifdef HAVE_SSL
  if (_encryption_flag) {
    subfile->_flags |= SF_encrypted;
  }
#endif  // HAVE_SSL

  if (_next_index != (streampos)0) {
    // If we're adding a Subfile to an already-existing Multifile, we
    // will eventually need to repack the file.
    _needs_repack = true;
  }

  pair<Subfiles::iterator, bool> insert_result = _subfiles.insert(subfile);
  if (!insert_result.second) {
    // Hmm, unable to insert.  There must already be a subfile by that
    // name.  Remove the old one.
    Subfile *old_subfile = (*insert_result.first);
    old_subfile->_flags |= SF_deleted;
    _removed_subfiles.push_back(old_subfile);
    (*insert_result.first) = subfile;
  }

  _new_subfiles.push_back(subfile);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::standardize_subfile_name
//       Access: Private
//  Description: Returns the standard form of the subfile name.
////////////////////////////////////////////////////////////////////
string Multifile::
standardize_subfile_name(const string &subfile_name) const {
  Filename name = subfile_name;
  name.standardize();
  if (name.empty() || name == "/") {
    // Invalid empty name.
    return string();
  }

  if (name[0] == '/') {
    return name.get_fullpath().substr(1);
  } else {
    return name.get_fullpath();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::clear_subfiles
//       Access: Private
//  Description: Removes the set of subfiles from the tables and frees
//               their associated memory.
////////////////////////////////////////////////////////////////////
void Multifile::
clear_subfiles() {
  PendingSubfiles::iterator pi;
  for (pi = _removed_subfiles.begin(); pi != _removed_subfiles.end(); ++pi) {
    Subfile *subfile = (*pi);
    subfile->rewrite_index_flags(*_write);
    delete subfile;
  }
  _removed_subfiles.clear();

  // We don't have to delete the ones in _new_subfiles, because these
  // also appear in _subfiles.
  _new_subfiles.clear();

  Subfiles::iterator fi;
  for (fi = _subfiles.begin(); fi != _subfiles.end(); ++fi) {
    Subfile *subfile = (*fi);
    delete subfile;
  }
  _subfiles.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::read_index
//       Access: Private
//  Description: Reads the Multifile header and index.  Returns true
//               if successful, false if the Multifile is not valid.
////////////////////////////////////////////////////////////////////
bool Multifile::
read_index() {
  nassertr(_read != (istream *)NULL, false);

  char this_header[_header_size];
  _read->read(this_header, _header_size);
  if (_read->fail() || _read->gcount() != (unsigned)_header_size) {
    express_cat.info()
      << "Unable to read Multifile header " << _multifile_name << ".\n";
    close();
    return false;
  }
  if (memcmp(this_header, _header, _header_size) != 0) {
    express_cat.info()
      << _multifile_name << " is not a Multifile.\n";
    return false;
  }

  // Now get the version numbers out.
  StreamReader reader(_read, false);
  _file_major_ver = reader.get_int16();
  _file_minor_ver = reader.get_int16();
  _scale_factor = reader.get_uint32();
  _new_scale_factor = _scale_factor;

  if (_read->eof() || _read->fail()) {
    express_cat.info()
      << _multifile_name << " header is truncated.\n";
    return false;
  }

  if (_file_major_ver != _current_major_ver ||
      (_file_major_ver == _current_major_ver && 
       _file_minor_ver > _current_minor_ver)) {
    express_cat.info()
      << _multifile_name << " has version " << _file_major_ver << "."
      << _file_minor_ver << ", expecting version " 
      << _current_major_ver << "." << _current_minor_ver << ".\n";
    return false;
  }


  // Now read the index out.
  _next_index = _read->tellg();
  _next_index = normalize_streampos(_next_index);  
  _read->seekg(_next_index);
  _last_index = 0;
  streampos index_forward;

  Subfile *subfile = new Subfile;
  index_forward = subfile->read_index(*_read, _next_index, this);
  while (index_forward != (streampos)0) {
    _last_index = _next_index;
    if (subfile->is_deleted()) {
      // Ignore deleted Subfiles in the index.
      _needs_repack = true;
      delete subfile;
    } else {
      _subfiles.push_back(subfile);
    }
    if (index_forward != normalize_streampos(_read->tellg())) {
      // If the index entries don't follow exactly sequentially, the
      // file ought to be repacked.
      _needs_repack = true;
    }
    _read->seekg(index_forward);
    _next_index = index_forward;
    subfile = new Subfile;
    index_forward = subfile->read_index(*_read, _next_index, this);
  }
  if (subfile->is_index_invalid()) {
    express_cat.info()
      << "Error reading index for " << _multifile_name << ".\n";
    close();
    delete subfile;
    return false;
  }

  size_t before_size = _subfiles.size();
  _subfiles.sort();
  size_t after_size = _subfiles.size();

  // If these don't match, the same filename appeared twice in the
  // index, which shouldn't be possible.
  nassertr(before_size == after_size, true);

  delete subfile;
  return true;
}  

////////////////////////////////////////////////////////////////////
//     Function: Multifile::write_header
//       Access: Private
//  Description: Writes just the header part of the Multifile, not the
//               index.
////////////////////////////////////////////////////////////////////
bool Multifile::
write_header() {
  nassertr(_write != (ostream *)NULL, false);
  nassertr(_write->tellp() == (streampos)0, false);
  _write->write(_header, _header_size);
  StreamWriter writer(_write);
  writer.add_int16(_current_major_ver);
  writer.add_int16(_current_minor_ver);
  writer.add_uint32(_scale_factor);

  _next_index = _write->tellp();
  _next_index = pad_to_streampos(_next_index);
  _last_index = 0;

  if (_write->fail()) {
    express_cat.info()
      << "Unable to write header for " << _multifile_name << ".\n";
    close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::read_index
//       Access: Public
//  Description: Reads the index record for the Subfile from the
//               indicated istream.  Assumes the istream has already
//               been positioned to the indicated stream position,
//               fpos, the start of the index record.  Returns the
//               position within the file of the next index record.
////////////////////////////////////////////////////////////////////
streampos Multifile::Subfile::
read_index(istream &read, streampos fpos, Multifile *multifile) {
  nassertr(read.tellg() == fpos, fpos);

  // First, get the next stream position.  We do this separately,
  // because if it is zero, we don't get anything else.
  StreamReader reader(read);

  streampos next_index = multifile->word_to_streampos(reader.get_uint32());
  if (read.eof() || read.fail()) {
    _flags |= SF_index_invalid;
    return 0;
  }

  if (next_index == (streampos)0) {
    return 0;
  }

  // Now get the rest of the index.

  _index_start = fpos;
  
  _data_start = multifile->word_to_streampos(reader.get_uint32());
  _data_length = reader.get_uint32();
  _flags = reader.get_uint16();
  if ((_flags & (SF_compressed | SF_encrypted)) != 0) {
    _uncompressed_length = reader.get_uint32();
  } else {
    _uncompressed_length = _data_length;
  }
  size_t name_length = reader.get_uint16();
  if (read.eof() || read.fail()) {
    _flags |= SF_index_invalid;
    return 0;
  }

  // And finally, get the rest of the name.
  char *name_buffer = new char[name_length];
  nassertr(name_buffer != (char *)NULL, next_index);
  for (size_t ni = 0; ni < name_length; ni++) {
    name_buffer[ni] = read.get() ^ 0xff;
  }
  _name = string(name_buffer, name_length);
  delete[] name_buffer;

  if (read.eof() || read.fail()) {
    _flags |= SF_index_invalid;
    return 0;
  }

  return next_index;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::write_index
//       Access: Public
//  Description: Writes the index record for the Subfile to the
//               indicated ostream.  Assumes the istream has already
//               been positioned to the indicated stream position,
//               fpos, the start of the index record, and that this is
//               the effective end of the file.  Returns the position
//               within the file of the next index record.
//
//               The _index_start member is updated by this operation.
////////////////////////////////////////////////////////////////////
streampos Multifile::Subfile::
write_index(ostream &write, streampos fpos, Multifile *multifile) {
  nassertr(write.tellp() == fpos, fpos);

  _index_start = fpos;

  // This will be the contents of this particular index record.  We
  // build it up first since it will be variable length.
  Datagram dg;
  dg.add_uint32(multifile->streampos_to_word(_data_start));
  dg.add_uint32(_data_length);
  dg.add_uint16(_flags);
  if ((_flags & (SF_compressed | SF_encrypted)) != 0) {
    dg.add_uint32(_uncompressed_length);
  }
  dg.add_uint16(_name.length());

  // For no real good reason, we'll invert all the bits in the name.
  // The only reason we do this is to make it inconvenient for a
  // casual browser of the Multifile to discover the names of the
  // files stored within it.  Naturally, this isn't real obfuscation
  // or security.
  string::iterator ni;
  for (ni = _name.begin(); ni != _name.end(); ++ni) {
    dg.add_int8((*ni) ^ 0xff);
  }

  size_t this_index_size = 4 + dg.get_length();

  // Plus, we will write out the next index address first.
  streampos next_index = fpos + (streampos)this_index_size;
  Datagram idg;
  idg.add_uint32(multifile->streampos_to_word(next_index));

  write.write((const char *)idg.get_data(), idg.get_length());
  write.write((const char *)dg.get_data(), dg.get_length());

  return next_index;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::write_index
//       Access: Public
//  Description: Writes the data record for the Subfile to the
//               indicated ostream: the actual contents of the
//               Subfile.  Assumes the istream has already been
//               positioned to the indicated stream position, fpos,
//               the start of the data record, and that this is the
//               effective end of the file.  Returns the position
//               within the file of the next data record.
//
//               The _data_start, _data_length, and
//               _uncompressed_length members are updated by this
//               operation.
//
//               If the "read" pointer is non-NULL, it is the readable
//               istream of a Multifile in which the Subfile might
//               already be packed.  This is used for reading the
//               contents of the Subfile during a repack() operation.
////////////////////////////////////////////////////////////////////
streampos Multifile::Subfile::
write_data(ostream &write, istream *read, streampos fpos,
           Multifile *multifile) {
  nassertr(write.tellp() == fpos, fpos);

  istream *source = _source;
  ifstream source_file;
  if (source == (istream *)NULL && !_source_filename.empty()) {
    // If we have a filename, open it up and read that.
    if (!_source_filename.open_read(source_file)) {
      // Unable to open the source file.
      express_cat.info()
        << "Unable to read " << _source_filename << ".\n";
      _flags |= SF_data_invalid;
      _data_length = 0;
      _uncompressed_length = 0;
    } else {
      source = &source_file;
    }
  }

  if (source == (istream *)NULL) {
    // We don't have any source data.  Perhaps we're reading from an
    // already-packed Subfile (e.g. during repack()).
    if (read == (istream *)NULL) {
      // No, we're just screwed.
      express_cat.info()
        << "No source for subfile " << _name << ".\n";
      _flags |= SF_data_invalid;
    } else {
      // Read the data from the original Multifile.
      read->seekg(_data_start);
      for (size_t p = 0; p < _data_length; p++) {
        int byte = read->get();
        if (read->eof() || read->fail()) {
          // Unexpected EOF or other failure on the source file.
          express_cat.info()
            << "Unexpected EOF for subfile " << _name << ".\n";
          _flags |= SF_data_invalid;
          break;
        }
        write.put(byte);
      }
    }
  } else {
    // We do have source data.  Copy it in, and also measure its
    // length.
    ostream *putter = &write;
    bool delete_putter = false;

#ifndef HAVE_SSL
    // Without OpenSSL, we can't support encryption.  The flag had
    // better not be set.
    nassertr((_flags & SF_encrypted) == 0, fpos);
#else  // HAVE_ZLIB
    if ((_flags & SF_encrypted) != 0) {
      // Write it encrypted.
      OEncryptStream *encrypt = new OEncryptStream;
      encrypt->set_iteration_count(multifile_encryption_iteration_count);
      encrypt->open(putter, delete_putter, multifile->_encryption_password);

      putter = encrypt;
      delete_putter = true;

      // Also write the encrypt_header to the beginning of the
      // encrypted stream, so we can validate the password on
      // decryption.
      putter->write(_encrypt_header, _encrypt_header_size);
    }
#endif  // HAVE_ZLIB

#ifndef HAVE_ZLIB
    // Without ZLIB, we can't support compression.  The flag had
    // better not be set.
    nassertr((_flags & SF_compressed) == 0, fpos);
#else  // HAVE_ZLIB
    if ((_flags & SF_compressed) != 0) {
      // Write it compressed.
      putter = new OCompressStream(putter, delete_putter, _compression_level);
      delete_putter = true;
    }
#endif  // HAVE_ZLIB

    streampos write_start = fpos;
    _uncompressed_length = 0;

    int byte = source->get();
    while (!source->eof() && !source->fail()) {
      _uncompressed_length++;
      putter->put(byte);
      byte = source->get();
    }

    if (delete_putter) {
      delete putter;
    }

    streampos write_end = write.tellp();
    _data_length = (size_t)(write_end - write_start);
  }

  // We can't set _data_start until down here, after we have read the
  // Subfile.  (In case we are running during repack()).
  _data_start = fpos;

  _source = (istream *)NULL;
  _source_filename = Filename();
  source_file.close();

  return fpos + (streampos)_data_length;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::rewrite_index_data_start
//       Access: Public
//  Description: Seeks within the indicate fstream back to the index
//               record and rewrites just the _data_start and
//               _data_length part of the index record.
////////////////////////////////////////////////////////////////////
void Multifile::Subfile::
rewrite_index_data_start(ostream &write, Multifile *multifile) {
  nassertv(_index_start != (streampos)0);

  static const size_t data_start_offset = 4;
  size_t data_start_pos = _index_start + (streampos)data_start_offset;
  write.seekp(data_start_pos);
  nassertv(!write.fail());

  StreamWriter writer(write);
  writer.add_uint32(multifile->streampos_to_word(_data_start));
  writer.add_uint32(_data_length);
  writer.add_uint16(_flags);
  if ((_flags & (SF_compressed | SF_encrypted)) != 0) {
    writer.add_uint32(_uncompressed_length);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::rewrite_index_flags
//       Access: Public
//  Description: Seeks within the indicate fstream back to the index
//               record and rewrites just the _flags part of the
//               index record.
////////////////////////////////////////////////////////////////////
void Multifile::Subfile::
rewrite_index_flags(ostream &write) {
  // If the subfile has never even been recorded to disk, we don't
  // need to do anything at all in this function.
  if (_index_start != (streampos)0) {
    static const size_t flags_offset = 4 + 4 + 4;
    size_t flags_pos = _index_start + (streampos)flags_offset;
    write.seekp(flags_pos);
    nassertv(!write.fail());
    
    StreamWriter writer(write);
    writer.add_uint16(_flags);
  }
}
