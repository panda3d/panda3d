// Filename: pipelineCyclerDummyImpl.I
// Created by:  drose (31Jan06)
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


////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::Constructor (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerDummyImpl::
PipelineCyclerDummyImpl(CycleData *initial_data, Pipeline *pipeline) :
  _data(initial_data),
  _pipeline(pipeline),
  _read_count(0),
  _write_count(0)
{
  if (_pipeline == (Pipeline *)NULL) {
    _pipeline = Pipeline::get_render_pipeline();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::Copy Constructor (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerDummyImpl::
PipelineCyclerDummyImpl(const PipelineCyclerDummyImpl &copy) :
  _data(copy._data->make_copy()),
  _pipeline(copy._pipeline),
  _read_count(0),
  _write_count(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::Copy Assignment (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerDummyImpl::
operator = (const PipelineCyclerDummyImpl &copy) {
  nassertv(_read_count == 0 && _write_count == 0);
  _data = copy._data->make_copy();
  _pipeline = copy._pipeline;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::Destructor (sanity-check)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerDummyImpl::
~PipelineCyclerDummyImpl() {
  nassertv(_read_count == 0 && _write_count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::read (sanity-check)
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the current stage of the pipeline as seen by
//               this thread.  This pointer should eventually be
//               released by calling release_read().
//
//               There should be no outstanding write pointers on the
//               data when this function is called.
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerDummyImpl::
read() const {
  // This function isn't truly const, but it doesn't change the data
  // in any meaningful way, so we pretend it is.
  ((PipelineCyclerDummyImpl *)this)->_read_count++;

  // It's not an error to grab a read pointer while someone else holds
  // a read or a write pointer.
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::increment_read (sanity-check)
//       Access: Public
//  Description: Increments the count on a pointer previously
//               retrieved by read(); now the pointer will need to be
//               released twice.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerDummyImpl::
increment_read(const CycleData *pointer) const {
  // This function isn't truly const, but it doesn't change the data
  // in any meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerDummyImpl *)this)->_read_count++;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::release_read (sanity-check)
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               read().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerDummyImpl::
release_read(const CycleData *pointer) const {
  // This function isn't truly const, but it doesn't change the data
  // in any meaningful way, so we pretend it is.
  nassertv(pointer == _data);
  nassertv(_read_count > 0);
  ((PipelineCyclerDummyImpl *)this)->_read_count--;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::write (sanity-check)
//       Access: Public
//  Description: Returns a non-const CycleData pointer, filled with a
//               unique copy of the data for the current stage of the
//               pipeline as seen by this thread.  This pointer may
//               now be used to write to the data, and that copy of
//               the data will be propagated to all later stages of the
//               pipeline.  This pointer should eventually be released
//               by calling release_write().
//
//               There may only be one outstanding write pointer on a
//               given stage at a time, and if there is a write
//               pointer there may be no read pointers on the same
//               stage (but see elevate_read).
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerDummyImpl::
write() {
  _write_count++;

  // It's an error to grab a write pointer while someone else holds a
  // read pointer, because doing so may invalidate the read pointer.
  nassertr(_read_count == 0, _data);

  // It's not an error to do this while someone else holds a write
  // pointer, however.

  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::elevate_read (sanity-check)
//       Access: Public
//  Description: Elevates a currently-held read pointer into a write
//               pointer.  This may or may not change the value of the
//               pointer.  It is only valid to do this if this is the
//               only currently-outstanding read pointer on the
//               current stage.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerDummyImpl::
elevate_read(const CycleData *pointer) {
  release_read(pointer);
  return write();
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::release_write (sanity-check)
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerDummyImpl::
release_write(CycleData *pointer) {
  nassertv(pointer == _data);
  nassertv(_write_count > 0);
  _write_count--;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::get_num_stages (sanity-check)
//       Access: Public
//  Description: Returns the number of stages in the pipeline.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerDummyImpl::
get_num_stages() {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::is_stage_unique (sanity-check)
//       Access: Public
//  Description: Returns true if the nth stage is a different pointer
//               than the previous stage, or false if its pointer is
//               shared with the previous one.
////////////////////////////////////////////////////////////////////
INLINE bool PipelineCyclerDummyImpl::
is_stage_unique(int n) const {
  nassertr(n == 0, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::write_stage (sanity-check)
//       Access: Public
//  Description: Returns a pointer suitable for writing to the nth
//               stage of the pipeline.  This is for special
//               applications that need to update the entire pipeline
//               at once (for instance, to remove an invalid pointer).
//               This pointer should later be released with
//               release_write_stage().
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerDummyImpl::
write_stage(int n) {
  nassertr(n == 0, (CycleData *)NULL);
  _write_count++;
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::release_write_stage (sanity-check)
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write_stage().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerDummyImpl::
release_write_stage(int n, CycleData *pointer) {
  nassertv(n == 0 && pointer == _data);
  nassertv(_write_count > 0);
  _write_count--;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::cheat (sanity-check)
//       Access: Public
//  Description: Returns a pointer without counting it.  This is only
//               intended for use as the return value for certain
//               nassertr() functions, so the application can recover
//               after a failure to manage the read and write pointers
//               correctly.  You should never call this function
//               directly.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerDummyImpl::
cheat() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::get_read_count (sanity-check)
//       Access: Public
//  Description: Returns the number of handles currently outstanding
//               to read the current stage of the data.  This should
//               only be used for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerDummyImpl::
get_read_count() const {
  return _read_count;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerDummyImpl::get_write_count (sanity-check)
//       Access: Public
//  Description: Returns the number of handles currently outstanding
//               to read the current stage of the data.  This will
//               normally only be either 0 or 1.  This should only be
//               used for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerDummyImpl::
get_write_count() const {
  return _write_count;
}
