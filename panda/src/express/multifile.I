// Filename: multifile.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Multifile::is_read_valid
//       Access: Published
//  Description: Returns true if the Multifile has been opened for
//               read mode and there have been no errors, and
//               individual Subfile contents may be extracted.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
is_read_valid() const {
  return (_read != (istream *)NULL && !_read->fail() && 
          _open_subfile == (Subfile *)NULL);
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
  return (_write != (ostream *)NULL && !_write->fail() && 
          _open_subfile == (Subfile *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::needs_repack
//       Access: Published
//  Description: Returns true if the Multifile index is suboptimal and
//               should be repacked.  Call repack() to achieve this.
////////////////////////////////////////////////////////////////////
INLINE bool Multifile::
needs_repack() const {
  return _needs_repack;
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
//               Multifile forces an immediate repack() operation.
////////////////////////////////////////////////////////////////////
INLINE void Multifile::
set_scale_factor(size_t scale_factor) {
  nassertv(scale_factor != (size_t)0);
  if (_scale_factor != scale_factor) {
    _scale_factor = scale_factor;
    if (_next_index != (streampos)0) {
      repack();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::get_scale_factor
//       Access: Published
//  Description: Returns the internal scale factor for this Multifile.
//               See set_scale_factor().
////////////////////////////////////////////////////////////////////
INLINE size_t Multifile::
get_scale_factor() const {
  return _scale_factor;
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
Subfile(const string &name) :
  _name(name)
{
  _index_start = 0;
  _data_start = 0;
  _data_length = 0;
  _source = (istream *)NULL;
  _flags = 0;
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
