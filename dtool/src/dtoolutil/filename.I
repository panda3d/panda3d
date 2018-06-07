/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filename.I
 * @author drose
 * @date 1999-01-18
 */

/**
 *
 */
INLINE Filename::
Filename(const std::string &filename) {
  _flags = 0;
  (*this) = filename;
}

/**
 *
 */
INLINE Filename::
Filename(const std::wstring &filename) {
  _flags = 0;
  (*this) = filename;
}

/**
 *
 */
INLINE Filename::
Filename(const char *filename) {
  _flags = 0;
  (*this) = filename;
}

/**
 *
 */
INLINE Filename::
Filename(const Filename &copy) :
  _filename(copy._filename.c_str()),
  _dirname_end(copy._dirname_end),
  _basename_start(copy._basename_start),
  _basename_end(copy._basename_end),
  _extension_start(copy._extension_start),
  _hash_start(copy._hash_start),
  _hash_end(copy._hash_end),
  _flags(copy._flags)
{
}

/**
 *
 */
INLINE Filename::
Filename(std::string &&filename) noexcept : _flags(0) {
  (*this) = std::move(filename);
}

/**
 *
 */
INLINE Filename::
Filename(Filename &&from) noexcept :
  _filename(std::move(from._filename)),
  _dirname_end(from._dirname_end),
  _basename_start(from._basename_start),
  _basename_end(from._basename_end),
  _extension_start(from._extension_start),
  _hash_start(from._hash_start),
  _hash_end(from._hash_end),
  _flags(from._flags)
{
}

/**
 * Creates an empty Filename.
 */
INLINE Filename::
Filename() :
  _dirname_end(0),
  _basename_start(0),
  _basename_end(std::string::npos),
  _extension_start(std::string::npos),
  _hash_start(std::string::npos),
  _hash_end(std::string::npos),
  _flags(0) {
}

/**
 *
 */
INLINE Filename Filename::
text_filename(const Filename &filename) {
  Filename result(filename);
  result.set_text();
  return result;
}

/**
 *
 */
INLINE Filename Filename::
text_filename(const std::string &filename) {
  Filename result(filename);
  result.set_text();
  return result;
}

/**
 *
 */
INLINE Filename Filename::
binary_filename(const Filename &filename) {
  Filename result(filename);
  result.set_binary();
  return result;
}

/**
 *
 */
INLINE Filename Filename::
binary_filename(const std::string &filename) {
  Filename result(filename);
  result.set_binary();
  return result;
}

/**
 *
 */
INLINE Filename Filename::
dso_filename(const std::string &filename) {
  Filename result(filename);
  result.set_type(T_dso);
  return result;
}

/**
 *
 */
INLINE Filename Filename::
executable_filename(const std::string &filename) {
  Filename result(filename);
  result.set_type(T_executable);
  return result;
}

/**
 * Constructs a filename that represents a sequence of numbered files.  See
 * set_pattern().
 */
INLINE Filename Filename::
pattern_filename(const std::string &filename) {
  Filename result(filename);
  result.set_pattern(true);
  return result;
}

/**
 *
 */
INLINE Filename &Filename::
operator = (const std::string &filename) {
  _filename = filename;

  locate_basename();
  locate_extension();
  locate_hash();
  return *this;
}

/**
 *
 */
INLINE Filename &Filename::
operator = (const std::wstring &filename) {
  TextEncoder encoder;
  encoder.set_encoding(get_filesystem_encoding());
  encoder.set_wtext(filename);
  return operator = (encoder.get_text());
}

/**
 *
 */
INLINE Filename &Filename::
operator = (const char *filename) {
  assert(filename != nullptr);
  return (*this) = std::string(filename);
}

/**
 *
 */
INLINE Filename &Filename::
operator = (const Filename &copy) {
  _filename = copy._filename;
  _dirname_end = copy._dirname_end;
  _basename_start = copy._basename_start;
  _basename_end = copy._basename_end;
  _extension_start = copy._extension_start;
  _hash_start = copy._hash_start;
  _hash_end = copy._hash_end;
  _flags = copy._flags;
  return *this;
}

/**
 *
 */
INLINE Filename &Filename::
operator = (std::string &&filename) noexcept {
  _filename = std::move(filename);

  locate_basename();
  locate_extension();
  locate_hash();
  return *this;
}

/**
 *
 */
INLINE Filename &Filename::
operator = (Filename &&from) noexcept {
  _filename = std::move(from._filename);
  _dirname_end = from._dirname_end;
  _basename_start = from._basename_start;
  _basename_end = from._basename_end;
  _extension_start = from._extension_start;
  _hash_start = from._hash_start;
  _hash_end = from._hash_end;
  _flags = from._flags;
  return *this;
}

/**
 *
 */
INLINE Filename::
operator const std::string & () const {
  return _filename;
}

/**
 *
 */
INLINE const char *Filename::
c_str() const {
  return _filename.c_str();
}

/**
 *
 */
INLINE bool Filename::
empty() const {
  return _filename.empty();
}

/**
 *
 */
INLINE size_t Filename::
length() const {
  return _filename.length();
}

/**
 *
 */
INLINE char Filename::
operator [] (size_t n) const {
  assert(n < _filename.length());
  return _filename[n];
}

/**
 *
 */
INLINE std::string Filename::
substr(size_t begin) const {
  return _filename.substr(begin);
}

/**
 *
 */
INLINE std::string Filename::
substr(size_t begin, size_t end) const {
  return _filename.substr(begin, end);
}

/**
 * Appends the other filename onto the end of this one.  This does not
 * introduce an intervening slash, but see the Filename constructor that takes
 * two parameters.
 */
INLINE void Filename::
operator += (const std::string &other) {
  _filename += other;
  locate_basename();
  locate_extension();
  locate_hash();
}

/**
 * Returns a new Filename representing the concatenation of the two filenames.
 */
INLINE Filename Filename::
operator + (const std::string &other) const {
  Filename a(*this);
  a += other;
  return a;
}

/**
 * Returns a new Filename that is composed of the other filename added to the
 * end of this filename, with an intervening slash added if necessary.
 */
INLINE Filename Filename::
operator / (const Filename &other) const {
  return Filename(*this, other);
}

/**
 * Returns the entire filename: directory, basename, extension.  This is the
 * same thing returned by the string typecast operator.
 */
INLINE std::string Filename::
get_fullpath() const {
  return _filename;
}

/**
 * Returns the entire filename as a wide-character string.
 */
INLINE std::wstring Filename::
get_fullpath_w() const {
  TextEncoder encoder;
  encoder.set_encoding(get_filesystem_encoding());
  encoder.set_text(get_fullpath());
  return encoder.get_wtext();
}

/**
 * Returns the directory part of the filename.  This is everything in the
 * filename up to, but not including the rightmost slash.
 */
INLINE std::string Filename::
get_dirname() const {
  return _filename.substr(0, _dirname_end);
}

/**
 * Returns the basename part of the filename.  This is everything in the
 * filename after the rightmost slash, including any extensions.
 */
INLINE std::string Filename::
get_basename() const {
  return _filename.substr(_basename_start);
}


/**
 * Returns the full filename--directory and basename parts--except for the
 * extension.
 */
INLINE std::string Filename::
get_fullpath_wo_extension() const {
  return _filename.substr(0, _basename_end);
}


/**
 * Returns the basename part of the filename, without the file extension.
 */
INLINE std::string Filename::
get_basename_wo_extension() const {
  if (_basename_end == std::string::npos) {
    return _filename.substr(_basename_start);
  } else {
    return _filename.substr(_basename_start, _basename_end - _basename_start);
  }
}


/**
 * Returns the file extension.  This is everything after the rightmost dot, if
 * there is one, or the empty string if there is not.
 */
INLINE std::string Filename::
get_extension() const {
  if (_extension_start == std::string::npos) {
    return std::string();
  } else {
    return _filename.substr(_extension_start);
  }
}

/**
 * Indicates that the filename represents a binary file.  This is primarily
 * relevant to the read_file() and write_file() methods, so they can set the
 * appropriate flags to the OS.
 */
INLINE void Filename::
set_binary() {
  _flags = (_flags & ~F_text) | F_binary;
}

/**
 * Indicates that the filename represents a text file.  This is primarily
 * relevant to the read_file() and write_file() methods, so they can set the
 * appropriate flags to the OS.
 */
INLINE void Filename::
set_text() {
  _flags = (_flags & ~F_binary) | F_text;
}

/**
 * Returns true if the Filename has been indicated to represent a binary file
 * via a previous call to set_binary().  It is possible that neither
 * is_binary() nor is_text() will be true, if neither set_binary() nor
 * set_text() was ever called.
 */
INLINE bool Filename::
is_binary() const {
  return ((_flags & F_binary) != 0);
}

/**
 * Returns true either is_binary() or is_text() is true; that is, that the
 * filename has been specified as either binary or text.  If this is false,
 * the filename has not been specified.
 */
INLINE bool Filename::
is_binary_or_text() const {
  return ((_flags & (F_binary | F_text)) != 0);
}

/**
 * Returns true if the Filename has been indicated to represent a text file
 * via a previous call to set_text().  It is possible that neither is_binary()
 * nor is_text() will be true, if neither set_binary() nor set_text() was ever
 * called.
 */
INLINE bool Filename::
is_text() const {
  return ((_flags & F_text) != 0);
}

/**
 * Sets the type of the file represented by the filename.  This is useful for
 * to_os_specific(), resolve_filename(), test_existence(), and all such real-
 * world access functions.  It helps the Filename know how to map the internal
 * filename to the OS-specific filename (for instance, maybe executables
 * should have an .exe extension).
 */
INLINE void Filename::
set_type(Filename::Type type) {
  _flags = (_flags & ~F_type) | type;
  switch (type) {
  case T_dso:
  case T_executable:
    set_binary();

  case T_general:
    break;
  }
}

/**
 * Returns the type of the file represented by the filename, as previously set
 * by set_type().
 */
INLINE Filename::Type Filename::
get_type() const {
  return (Type)(_flags & (int)F_type);
}

/**
 * Sets the flag indicating whether this is a filename pattern.  When this is
 * true, the filename is understood to be a placeholder for a numbered
 * sequence of filename, such as an image sequence.  In this case, a sequence
 * of one or more hash characters ("#") should appear in the filename string;
 * these characters will be filled in with the corresponding number (or more)
 * of digits representing the sequence number.  Sequence numbers always begin
 * counting at 0.
 *
 * When this is true, methods like has_hash() and get_hash_to_end() and
 * get_filename_index() may be called.  Methods like is_exists() will
 * implicitly test for existance of filename sequence 0.
 */
INLINE void Filename::
set_pattern(bool pattern) {
  if (pattern != get_pattern()) {
    if (pattern) {
      _flags |= F_pattern;
    } else {
      _flags &= ~F_pattern;
    }
    locate_hash();
  }
}

/**
 * Returns the flag indicating whether this is a filename pattern.  See
 * set_pattern().
 */
INLINE bool Filename::
get_pattern() const {
  return (_flags & F_pattern) != 0;
}

/**
 * Returns true if the filename is indicated to be a filename pattern (that
 * is, set_pattern(true) was called), and the filename pattern did include a
 * sequence of hash marks, or false if it was not a filename pattern or did
 * not include hash marks.  If this is true, then get_filename_index() will
 * return a different filename each time.
 */
INLINE bool Filename::
has_hash() const {
  return (_hash_start != _hash_end);
}

/**
 * Returns the part of the filename beginning at the hash sequence (if any),
 * and continuing to the end of the filename.
 */
INLINE std::string Filename::
get_hash_to_end() const {
  return _filename.substr(_hash_start);
}

/**
 * Returns true if the filename is local, e.g.  does not begin with a slash,
 * or false if the filename is fully specified from the root.
 */
INLINE bool Filename::
is_local() const {
  return _filename.empty() || _filename[0] != '/';
}

/**
 * Returns true if the filename is fully qualified, e.g.  begins with a slash.
 * This is almost, but not quite, the same thing as !is_local().  It's not
 * exactly the same because a special case is made for filenames that begin
 * with a single dot followed by a slash--these are considered to be fully
 * qualified (they are explicitly relative to the current directory, and do
 * not refer to a filename on a search path somewhere).
 */
INLINE bool Filename::
is_fully_qualified() const {
  return
    (_filename.size() > 2 && _filename[0] == '.' && _filename[1] == '/') ||
    (!_filename.empty() && _filename[0] == '/');
}

/**
 *
 */
INLINE bool Filename::
operator == (const std::string &other) const {
  return (*(std::string *)this) == other;
}

/**
 *
 */
INLINE bool Filename::
operator != (const std::string &other) const {
  return (*(std::string *)this) != other;
}

/**
 *
 */
INLINE bool Filename::
operator < (const std::string &other) const {
  return (*(std::string *)this) < other;
}

/**
 *
 */
INLINE int Filename::
compare_to(const Filename &other) const {
  return strcmp(_filename.c_str(), other._filename.c_str());
}


/**
 * Returns true if the Filename is valid (not empty), or false if it is an
 * empty string.
 *
 * This implements the Python equivalent to operator bool.  Defining an actual
 * operator bool method for C++ use would work too, but it seems to cause too
 * many ambiguities for the C++ compiler, so we use this Python-only approach
 * instead.
 */
INLINE bool Filename::
__nonzero__() const {
  return !_filename.empty();
}

/**
 *
 */
INLINE void Filename::
output(std::ostream &out) const {
  out << _filename;
}

/**
 * Specifies the default encoding to be used for all subsequent Filenames.
 * This is used to represent wide-character (Unicode) filenames internally.
 * On non-Windows-based systems, the encoded filename is also passed to the
 * underlying operating system.
 */
INLINE void Filename::
set_filesystem_encoding(TextEncoder::Encoding encoding) {
  _filesystem_encoding = encoding;
}

/**
 * Specifies the default encoding to be used for all subsequent Filenames
 * objects.  See set_filesystem_encoding().
 */
INLINE TextEncoder::Encoding Filename::
get_filesystem_encoding() {
  return _filesystem_encoding;
}
