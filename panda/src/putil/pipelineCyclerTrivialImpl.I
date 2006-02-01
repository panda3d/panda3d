// Filename: pipelineCyclerTrivialImpl.I
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
//     Function: PipelineCyclerTrivialImpl::Constructor (trivial)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrivialImpl::
PipelineCyclerTrivialImpl(CycleData *initial_data, Pipeline *) {
  // In the trivial implementation, a derived class (the
  // PipelineCycler template class) stores the CycleData object
  // directly within itself, and since we have no data members or
  // virtual functions, we get away with assuming the pointer is the
  // same as the 'this' pointer.

  // If this turns out not to be true on a particular platform, we
  // will have to store the pointer in this class, for a little bit of
  // extra overhead.
#ifdef SIMPLE_STRUCT_POINTERS
  nassertv(initial_data == (CycleData *)this);
#else
  _data = initial_data;
#endif  // SIMPLE_STRUCT_POINTERS
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::Copy Constructor (trivial)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrivialImpl::
PipelineCyclerTrivialImpl(const PipelineCyclerTrivialImpl &) {
  // The copy constructor for the PipelineCyclerTrivialImpl case
  // doesn't work.  Don't try to use it.  The PipelineCycler template
  // class is #ifdeffed appropriately to call the normal constructor
  // instead.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::Copy Assignment (trivial)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrivialImpl::
operator = (const PipelineCyclerTrivialImpl &) {
  // The copy assignment operator for the PipelineCyclerTrivialImpl
  // case doesn't work.  Don't try to use it.  The PipelineCycler
  // template class is #ifdeffed appropriately not to call this
  // method.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::Destructor (trivial)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerTrivialImpl::
~PipelineCyclerTrivialImpl() {
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::read (trivial)
//       Access: Public
//  Description: Returns a const CycleData pointer, filled with the
//               data for the current stage of the pipeline as seen by
//               this thread.  This pointer should eventually be
//               released by calling release_read().
////////////////////////////////////////////////////////////////////
INLINE const CycleData *PipelineCyclerTrivialImpl::
read() const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (const CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::increment_read (trivial)
//       Access: Public
//  Description: Increments the count on a pointer previously
//               retrieved by read(); now the pointer will need to be
//               released twice.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrivialImpl::
increment_read(const CycleData *) const {
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::release_read (trivial)
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               read().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrivialImpl::
release_read(const CycleData *) const {
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::write (trivial)
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
INLINE CycleData *PipelineCyclerTrivialImpl::
write() {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::elevate_read (trivial)
//       Access: Public
//  Description: Elevates a currently-held read pointer into a write
//               pointer.  This may or may not change the value of the
//               pointer.  It is only valid to do this if this is the
//               only currently-outstanding read pointer on the
//               current stage.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrivialImpl::
elevate_read(const CycleData *) {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::release_write (trivial)
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrivialImpl::
release_write(CycleData *) {
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::get_num_stages (trivial)
//       Access: Public
//  Description: Returns the number of stages in the pipeline.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerTrivialImpl::
get_num_stages() {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::is_stage_unique (trivial)
//       Access: Public
//  Description: Returns true if the nth stage is a different pointer
//               than the previous stage, or false if its pointer is
//               shared with the previous one.
////////////////////////////////////////////////////////////////////
INLINE bool PipelineCyclerTrivialImpl::
is_stage_unique(int n) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::write_stage (trivial)
//       Access: Public
//  Description: Returns a pointer suitable for writing to the nth
//               stage of the pipeline.  This is for special
//               applications that need to update the entire pipeline
//               at once (for instance, to remove an invalid pointer).
//               This pointer should later be released with
//               release_write_stage().
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrivialImpl::
write_stage(int) {
  return (CycleData *)this;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::release_write_stage (trivial)
//       Access: Public
//  Description: Releases a pointer previously obtained via a call to
//               write_stage().
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerTrivialImpl::
release_write_stage(int, CycleData *) {
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::cheat (trivial)
//       Access: Public
//  Description: Returns a pointer without counting it.  This is only
//               intended for use as the return value for certain
//               nassertr() functions, so the application can recover
//               after a failure to manage the read and write pointers
//               correctly.  You should never call this function
//               directly.
////////////////////////////////////////////////////////////////////
INLINE CycleData *PipelineCyclerTrivialImpl::
cheat() const {
#ifdef SIMPLE_STRUCT_POINTERS
  return (CycleData *)this;
#else
  return _data;
#endif  // SIMPLE_STRUCT_POINTERS
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::get_read_count (trivial)
//       Access: Public
//  Description: Returns the number of handles currently outstanding
//               to read the current stage of the data.  This should
//               only be used for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerTrivialImpl::
get_read_count() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerTrivialImpl::get_write_count (trivial)
//       Access: Public
//  Description: Returns the number of handles currently outstanding
//               to read the current stage of the data.  This will
//               normally only be either 0 or 1.  This should only be
//               used for debugging purposes.
////////////////////////////////////////////////////////////////////
INLINE int PipelineCyclerTrivialImpl::
get_write_count() const {
  return 0;
}

