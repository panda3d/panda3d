/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file multifile.I
 * @author mike
 * @date 1997-01-09
 */

/**
 * Returns the filename of the Multifile, if it is available.
 */
INLINE const Filename &Multifile::
get_multifile_name() const {
  return _multifile_name;
}

/**
 * Replaces the filename of the Multifile.  This is primarily used for
 * documentation purposes only; changing this name does not open the indicated
 * file.  See open_read() or open_write() for that.
 */
INLINE void Multifile::
set_multifile_name(const Filename &multifile_name) {
  _multifile_name = multifile_name;
}

/**
 * Returns true if the Multifile has been opened for read mode and there have
 * been no errors, and individual Subfile contents may be extracted.
 */
INLINE bool Multifile::
is_read_valid() const {
  return (_read != nullptr);
}

/**
 * Returns true if the Multifile has been opened for write mode and there have
 * been no errors, and Subfiles may be added or removed from the Multifile.
 */
INLINE bool Multifile::
is_write_valid() const {
  return (_write != nullptr && !_write->fail());
}

/**
 * Returns true if the Multifile index is suboptimal and should be repacked.
 * Call repack() to achieve this.
 */
INLINE bool Multifile::
needs_repack() const {
  return _needs_repack || (_scale_factor != _new_scale_factor);
}

/**
 * Returns the modification timestamp of the overall Multifile.  This
 * indicates the most recent date at which subfiles were added or removed from
 * the Multifile.  Note that it is logically possible for an individual
 * subfile to have a more recent timestamp than the overall timestamp.
 */
INLINE time_t Multifile::
get_timestamp() const {
  return _timestamp;
}

/**
 * Sets the flag indicating whether timestamps should be recorded within the
 * Multifile or not.  The default is true, indicating the Multifile will
 * record timestamps for the overall file and also for each subfile.
 *
 * If this is false, the Multifile will not record timestamps internally.  In
 * this case, the return value from get_timestamp() or get_subfile_timestamp()
 * will be estimations.
 *
 * You may want to set this false to minimize the bitwise difference between
 * independently-generated Multifiles.
 */
INLINE void Multifile::
set_record_timestamp(bool flag) {
  _record_timestamp = flag;
  _timestamp_dirty = true;
}

/**
 * Returns the flag indicating whether timestamps should be recorded within
 * the Multifile or not.  See set_record_timestamp().
 */
INLINE bool Multifile::
get_record_timestamp() const {
  return _record_timestamp;
}

/**
 * Returns the internal scale factor for this Multifile.  See
 * set_scale_factor().
 */
INLINE size_t Multifile::
get_scale_factor() const {
  return _new_scale_factor;
}

/**
 * Sets the flag indicating whether subsequently-added subfiles should be
 * encrypted before writing them to the multifile.  If true, subfiles will be
 * encrypted; if false (the default), they will be written without encryption.
 *
 * When true, subfiles will be encrypted with the password specified by
 * set_encryption_password().  It is possible to apply a different password to
 * different files, but the resulting file can't be mounted via VFS.
 */
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

/**
 * Returns the flag indicating whether subsequently-added subfiles should be
 * encrypted before writing them to the multifile.  See set_encryption_flag().
 */
INLINE bool Multifile::
get_encryption_flag() const {
  return _encryption_flag;
}

/**
 * Specifies the password that will be used to encrypt subfiles subsequently
 * added to the multifile, if the encryption flag is also set true (see
 * set_encryption_flag()).
 *
 * It is possible to apply a different password to different files, but the
 * resulting file can't be mounted via VFS.  Changing this value may cause an
 * implicit call to flush().
 */
INLINE void Multifile::
set_encryption_password(const std::string &encryption_password) {
  if (_encryption_password != encryption_password) {
    if (!_new_subfiles.empty()) {
      flush();
    }
    _encryption_password = encryption_password;
  }
}

/**
 * Returns the password that will be used to encrypt subfiles subsequently
 * added to the multifile.  See set_encryption_password().
 */
INLINE const std::string &Multifile::
get_encryption_password() const {
  return _encryption_password;
}

/**
 * Specifies the encryption algorithm that should be used for future calls to
 * add_subfile().  The default is whatever is specified by the encryption-
 * algorithm config variable.  The complete set of available algorithms is
 * defined by the current version of OpenSSL.
 *
 * If an invalid algorithm is specified, there is no immediate error return
 * code, but flush() will fail and the file will be invalid.
 *
 * It is possible to apply a different encryption algorithm to different
 * files, and unlike the password, this does not interfere with mounting the
 * multifile via VFS.  Changing this value may cause an implicit call to
 * flush().
 */
INLINE void Multifile::
set_encryption_algorithm(const std::string &encryption_algorithm) {
  if (_encryption_algorithm != encryption_algorithm) {
    if (!_new_subfiles.empty()) {
      flush();
    }
    _encryption_algorithm = encryption_algorithm;
  }
}

/**
 * Returns the encryption algorithm that was specified by
 * set_encryption_algorithm().
 */
INLINE const std::string &Multifile::
get_encryption_algorithm() const {
  return _encryption_algorithm;
}

/**
 * Specifies the length of the key, in bits, that should be used to encrypt
 * the stream in future calls to add_subfile().  The default is whatever is
 * specified by the encryption-key-length config variable.
 *
 * If an invalid key_length for the chosen algorithm is specified, there is no
 * immediate error return code, but flush() will fail and the file will be
 * invalid.
 *
 * It is possible to apply a different key length to different files, and
 * unlike the password, this does not interfere with mounting the multifile
 * via VFS. Changing this value may cause an implicit call to flush().
 */
INLINE void Multifile::
set_encryption_key_length(int encryption_key_length) {
  if (_encryption_key_length != encryption_key_length) {
    if (!_new_subfiles.empty()) {
      flush();
    }
    _encryption_key_length = encryption_key_length;
  }
}

/**
 * Returns the encryption key length, in bits, that was specified by
 * set_encryption_key_length().
 */
INLINE int Multifile::
get_encryption_key_length() const {
  return _encryption_key_length;
}

/**
 * Specifies the number of times to repeatedly hash the key before writing it
 * to the stream in future calls to add_subfile().  Its purpose is to make it
 * computationally more expensive for an attacker to search the key space
 * exhaustively.  This should be a multiple of 1,000 and should not exceed
 * about 65 million; the value 0 indicates just one application of the hashing
 * algorithm.
 *
 * The default is whatever is specified by the multifile-encryption-iteration-
 * count config variable.
 *
 * It is possible to apply a different iteration count to different files, and
 * unlike the password, this does not interfere with mounting the multifile
 * via VFS.  Changing this value causes an implicit call to flush().
 */
INLINE void Multifile::
set_encryption_iteration_count(int encryption_iteration_count) {
  if (_encryption_iteration_count != encryption_iteration_count) {
    flush();
    _encryption_iteration_count = encryption_iteration_count;
  }
}

/**
 * Returns the value that was specified by set_encryption_iteration_count().
 */
INLINE int Multifile::
get_encryption_iteration_count() const {
  return _encryption_iteration_count;
}

/**
 * Removes the named subfile from the Multifile, if it exists; returns true if
 * successfully removed, or false if it did not exist in the first place.  The
 * file will not actually be removed from the disk until the next call to
 * flush().
 *
 * Note that this does not actually remove the data from the indicated
 * subfile; it simply removes it from the index.  The Multifile will not be
 * reduced in size after this operation, until the next call to repack().
 */
INLINE bool Multifile::
remove_subfile(const std::string &subfile_name) {
  int index = find_subfile(subfile_name);
  if (index >= 0) {
    remove_subfile(index);
    return true;
  }
  return false;
}

/**
 * Returns a vector_uchar that contains the entire contents of the indicated
 * subfile.
 */
INLINE vector_uchar Multifile::
read_subfile(int index) {
  vector_uchar result;
  read_subfile(index, result);
  return result;
}

/**
 * Returns a string with the first n bytes written to a Multifile, to identify
 * it as a Multifile.
 */
INLINE std::string Multifile::
get_magic_number() {
  return std::string(_header, _header_size);
}

/**
 * Returns the string that preceded the Multifile header on the file, if any.
 * See set_header_prefix().
 */
INLINE const std::string &Multifile::
get_header_prefix() const {
  return _header_prefix;
}

/**
 * Converts a size_t address read from the file to a streampos byte address
 * within the file.
 */
INLINE std::streampos Multifile::
word_to_streampos(size_t word) const {
  return (std::streampos)word * (std::streampos)_scale_factor;
}

/**
 * Converts a streampos byte address within the file to a size_t value
 * suitable for writing to the file.
 */
INLINE size_t Multifile::
streampos_to_word(std::streampos fpos) const {
  return (size_t)((fpos + (std::streampos)_scale_factor - (std::streampos)1) / (std::streampos)_scale_factor);
}

/**
 * Rounds the streampos byte address up to the next multiple of _scale_factor.
 * Only multiples of _scale_factor may be written to the file.
 */
INLINE std::streampos Multifile::
normalize_streampos(std::streampos fpos) const {
  return word_to_streampos(streampos_to_word(fpos));
}

/**
 * Converts a single nibble to a hex digit.
 */
INLINE char Multifile::
tohex(unsigned int nibble) {
  nibble &= 0xf;
  if (nibble < 10) {
    return nibble + '0';
  }
  return nibble - 10 + 'a';
}

/**
 *
 */
INLINE Multifile::Subfile::
Subfile() {
  _index_start = 0;
  _data_start = 0;
  _data_length = 0;
  _timestamp = 0;
  _source = nullptr;
  _flags = 0;
  _compression_level = 0;
#ifdef HAVE_OPENSSL
  _pkey = nullptr;
#endif
}

/**
 * Compares two Subfiles for proper sorting within the index.
 */
INLINE bool Multifile::Subfile::
operator < (const Multifile::Subfile &other) const {
  // This should only be called on normal subfiles, not on certificate files
  // or signature files.  (We don't attempt to sort these special signature
  // files.)
  nassertr(!is_cert_special() && !other.is_cert_special(), false);

  // Normal subfiles are simply sorted in alphabetical order by filename.
  return _name < other._name;
}

/**
 * Returns true if the Subfile indicates it has been deleted (removed from the
 * index), false otherwise.  This should never be true of Subfiles that
 * currently appear in either the _subfiles or _new_subfiles lists.
 */
INLINE bool Multifile::Subfile::
is_deleted() const {
  return (_flags & SF_deleted) != 0;
}

/**
 * Returns true if there was some problem reading the index record for this
 * Subfile from the Multifile.
 */
INLINE bool Multifile::Subfile::
is_index_invalid() const {
  return (_flags & SF_index_invalid) != 0;
}

/**
 * Returns true if there was some problem reading the data contents of this
 * Subfile, particularly when copying into the Multifile.
 */
INLINE bool Multifile::Subfile::
is_data_invalid() const {
  return (_flags & SF_data_invalid) != 0;
}

/**
 * Returns true if this Subfile represents a signature record, which is
 * treated specially; or false if it is an ordinary Subfile.
 */
INLINE bool Multifile::Subfile::
is_cert_special() const {
  return (_flags & SF_signature) != 0;
}

/**
 * Returns the byte position within the Multifile of the last byte that
 * contributes to this Subfile, either in the index record or in the subfile
 * data.
 */
INLINE std::streampos Multifile::Subfile::
get_last_byte_pos() const {
  return std::max(_index_start + (std::streampos)_index_length,
             _data_start + (std::streampos)_data_length) - (std::streampos)1;
}
