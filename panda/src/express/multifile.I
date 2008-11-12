// Filename: multifile.I
// Created by:  mike (09Jan97)
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


////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_multifile_name
//       Access: Published
//  Description: Returns the filename of the Multifile, if it is
//               available.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Multifile::
get_multifile_name() const {
  return _multifile_name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::set_multifile_name
//       Access: Published
//  Description: Replaces the filename of the Multifile.  This is
//               primarily used for documentation purposes only;
//               changing this name does not open the indicated file.
//               See open_read() or open_write() for that.
////////////////////////////////////////////////////////////////////
INLINE void Multifile::
set_multifile_name(const Filename &multifile_name) {
  _multifile_name = multifile_name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::is_read_valid
//       Access: Published
//  Description: Returns true if the Multifile has been opened for
//               read mode and there have been no errors, and
//               individual Subfile contents may be extracted.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
is_read_valid() const {
  return (_read != (IStreamWrapper *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::is_write_valid
//       Access: Published
//  Description: Returns true if the Multifile has been opened for
//               write mode and there have been no errors, and
//               Subfiles may be added or removed from the Multifile.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
is_write_valid() const {
  return (_write != (ostream *)NULL && !_write->fail());
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::needs_repack
//       Access: Published
//  Description: Returns true if the Multifile index is suboptimal and
//               should be repacked.  Call repack() to achieve this.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
needs_repack() const {
  return _needs_repack || (_scale_factor != _new_scale_factor);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_timestamp
//       Access: Published
//  Description: Returns the modification timestamp of the overall
//               Multifile.  This indicates the most recent date at
//               which subfiles were added or removed from the
//               Multifile.  Note that it is logically possible for an
//               individual subfile to have a more recent timestamp
//               than the overall timestamp.
////////////////////////////////////////////////////////////////////
INLINE time_t Multifile::
get_timestamp() const {
  return _timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::set_record_timestamp
//       Access: Published
//  Description: Sets the flag indicating whether timestamps should be
//               recorded within the Multifile or not.  The default is
//               true, indicating the Multifile will record timestamps
//               for the overall file and also for each subfile.  
//
//               If this is false, the Multifile will not record
//               timestamps internally.  In this case, the return
//               value from get_timestamp() or get_subfile_timestamp()
//               will be estimations.
//
//               You may want to set this false to minimize the
//               bitwise difference between independently-generated
//               Multifiles.
////////////////////////////////////////////////////////////////////
INLINE void Multifile::
set_record_timestamp(bool flag) {
  _record_timestamp = flag;
  _timestamp_dirty = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_record_timestamp
//       Access: Published
//  Description: Returns the flag indicating whether timestamps
//               should be recorded within the Multifile or not.  See
//               set_record_timestamp().
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
get_record_timestamp() const {
  return _record_timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_scale_factor
//       Access: Published
//  Description: Returns the internal scale factor for this Multifile.
//               See set_scale_factor().
////////////////////////////////////////////////////////////////////
INLINE size_t Multifile::
get_scale_factor() const {
  return _new_scale_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::set_encryption_flag
//       Access: Published
//  Description: Sets the flag indicating whether subsequently-added
//               subfiles should be encrypted before writing them to
//               the multifile.  If true, subfiles will be encrypted;
//               if false (the default), they will be written without
//               encryption.
//
//               When true, subfiles will be encrypted with the
//               password specified by set_encryption_password().  It
//               is possible to apply a different password to
//               different files, but you must call flush() or
//               repack() before changing these properties, and the
//               resulting file can't be mounted via VFS.
////////////////////////////////////////////////////////////////////
INLINE void Multifile::
set_encryption_flag(bool flag) {
#ifndef HAVE_OPENSSL
  if (flag) {
    express_cat.warning()
      << "OpenSSL not compiled in; cannot generated encrypted multifiles.\n";
    flag = false;
  }
#endif  // HAVE_OPENSSL
  _encryption_flag = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_encryption_flag
//       Access: Published
//  Description: Returns the flag indicating whether
//               subsequently-added subfiles should be encrypted
//               before writing them to the multifile.  See
//               set_encryption_flag().
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
get_encryption_flag() const {
  return _encryption_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::set_encryption_password
//       Access: Published
//  Description: Specifies the password that will be used to encrypt
//               subfiles subsequently added to the multifile, if the
//               encryption flag is also set true (see
//               set_encryption_flag()).
////////////////////////////////////////////////////////////////////
INLINE void Multifile::
set_encryption_password(const string &password) {
  _encryption_password = password;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_encryption_password
//       Access: Published
//  Description: Returns the password that will be used to encrypt
//               subfiles subsequently added to the multifile.  See
//               set_encryption_password().
////////////////////////////////////////////////////////////////////
INLINE const string &Multifile::
get_encryption_password() const {
  return _encryption_password;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::read_subfile
//       Access: Published
//  Description: Returns a string that contains the entire contents of
//               the indicated subfile.
////////////////////////////////////////////////////////////////////
INLINE string Multifile::
read_subfile(int index) {
  string result;
  read_subfile(index, result);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_magic_number
//       Access: Published, Static
//  Description: Returns a string with the first n bytes written to a
//               Multifile, to identify it as a Multifile.
////////////////////////////////////////////////////////////////////
INLINE string Multifile::
get_magic_number() {
  return string(_header, _header_size);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::word_to_streampos
//       Access: Private
//  Description: Converts a size_t address read from the file to
//               a streampos byte address within the file.
////////////////////////////////////////////////////////////////////
INLINE streampos Multifile::
word_to_streampos(size_t word) const {
  return (streampos)word * (streampos)_scale_factor;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::streampos_to_word
//       Access: Private
//  Description: Converts a streampos byte address within the file to
//               a size_t value suitable for writing to the file.
////////////////////////////////////////////////////////////////////
INLINE size_t Multifile::
streampos_to_word(streampos fpos) const {
  return (size_t)((fpos + (streampos)_scale_factor - (streampos)1) / (streampos)_scale_factor); 
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::normalize_streampos
//       Access: Private
//  Description: Rounds the streampos byte address up to the next
//               multiple of _scale_factor.  Only multiples of
//               _scale_factor may be written to the file.
////////////////////////////////////////////////////////////////////
INLINE streampos Multifile::
normalize_streampos(streampos fpos) const {
  return word_to_streampos(streampos_to_word(fpos));
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Multifile::Subfile::
Subfile() {
  _index_start = 0;
  _data_start = 0;
  _data_length = 0;
  _timestamp = 0;
  _source = (istream *)NULL;
  _flags = 0;
  _compression_level = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::Subfile::
operator < (const Multifile::Subfile &other) const {
  return _name < other._name;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::is_deleted
//       Access: Public
//  Description: Returns true if the Subfile indicates it has been
//               deleted (removed from the index), false otherwise.
//               This should never be true of Subfiles that currently
//               appear in either the _subfiles or _new_subfiles
//               lists.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::Subfile::
is_deleted() const {
  return (_flags & SF_deleted) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::is_index_invalid
//       Access: Public
//  Description: Returns true if there was some problem reading the
//               index record for this Subfile from the Multifile.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::Subfile::
is_index_invalid() const {
  return (_flags & SF_index_invalid) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Subfile::is_data_invalid
//       Access: Public
//  Description: Returns true if there was some problem reading the
//               data contents of this Subfile, particularly when
//               copying into the Multifile.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::Subfile::
is_data_invalid() const {
  return (_flags & SF_data_invalid) != 0;
}
