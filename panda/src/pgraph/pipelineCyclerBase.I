// Filename: pipelineCyclerBase.I
// Created by:  drose (21Feb02)
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
//     Function: PipelineCyclerBase::read
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the current stage of the pipeline as seen by
//               this thread.  This pointer should eventually be
//               released by calling release_read().
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerBase::
read() const {
  // This function isn't truly const, but it doesn't change the data
  // in any meaningful way, so we pretend it is.
  ((PipelineCyclerBase *)this)->_read_count++;
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::increment_read
//       Access: Public
//  Description: Increments the count on a pointer previously
//               retrieved by read(); now the pointer will need to be
//               released twice.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerBase::
increment_read(const CycleData *pointer) const {
  // This function isn't truly const, but it doesn't change the data
  // in any meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerBase *)this)->_read_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::release_read
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               read().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerBase::
release_read(const CycleData *pointer) const {
  // This function isn't truly const, but it doesn't change the data
  // in any meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerBase *)this)->_read_count--;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::write
//       Access: Public
//  Description: Returns a non-const CycleData pointer, filled with a
//               unique copy of the data for the current stage of the
//               pipeline as seen by this thread.  This pointer may
//               now be used to write to the data, and that copy of
//               the data will be propagate to all later stages of the
//               pipeline.  This pointer should eventually be released
//               by calling release_write().
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerBase::
write() {
  _write_count++;
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::increment_write
//       Access: Public
//  Description: Increments the count on a pointer previously
//               retrieved by write(); now the pointer will need to be
//               released twice.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerBase::
increment_write(CycleData *pointer) {
  nassertv(pointer == _data);
  nassertv(_write_count > 0);
  _write_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerBase::release_write
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerBase::
release_write(CycleData *pointer) {
  nassertv(pointer == _data);
  nassertv(_write_count > 0);
  _write_count--;
}
