// Filename: pipelineCyclerTrivialImpl.h
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

#ifndef PIPELINECYCLERTRIVIALIMPL_H
#define PIPELINECYCLERTRIVIALIMPL_H

#include "pandabase.h"

#ifndef DO_PIPELINING

class CycleData;
class Pipeline;

////////////////////////////////////////////////////////////////////
//       Class : PipelineCyclerTrivialImpl
// Description : This is the trivial, non-threaded implementation of
//               PipelineCyclerBase.  It is only compiled when
//               DO_PIPELINING is not defined (which usually implies
//               that threading is not available).
//
//               This implementation is designed to do as little as
//               possible, and to compile to nothing, or almost
//               nothing.  It doesn't actually support pipelining in
//               any way.  It doesn't even perform any sanity checks
//               to speak of.  It's designed for a strictly
//               single-threaded application, and its purpose is to be
//               as low-overhead as possible.
//
//               We define this as a struct instead of a class to
//               emphasize the importance of byte placement within the
//               object, so that the inherited struct's data is likely
//               to be placed by the compiler at the "this" pointer.
////////////////////////////////////////////////////////////////////
struct EXPCL_PANDA PipelineCyclerTrivialImpl {
public:
  INLINE PipelineCyclerTrivialImpl(CycleData *initial_data, Pipeline *pipeline = NULL);
private:
  INLINE PipelineCyclerTrivialImpl(const PipelineCyclerTrivialImpl &copy);
  INLINE void operator = (const PipelineCyclerTrivialImpl &copy);
public:
  INLINE ~PipelineCyclerTrivialImpl();

  INLINE void lock();
  INLINE void release();

  INLINE const CycleData *read() const;
  INLINE void increment_read(const CycleData *pointer) const;
  INLINE void release_read(const CycleData *pointer) const;

  INLINE CycleData *write();
  INLINE CycleData *elevate_read(const CycleData *pointer);
  INLINE CycleData *elevate_read_upstream(const CycleData *pointer, bool force_to_0);
  INLINE void increment_write(CycleData *pointer) const;
  INLINE void release_write(CycleData *pointer);

  INLINE int get_num_stages();
  INLINE const CycleData *read_stage(int n) const;
  INLINE void release_read_stage(int n, const CycleData *pointer) const;
  INLINE CycleData *write_upstream(bool force_to_0);
  INLINE CycleData *write_stage(int n);
  INLINE CycleData *elevate_read_stage(int n, const CycleData *pointer);
  INLINE void release_write_stage(int n, CycleData *pointer);

  INLINE TypeHandle get_parent_type() const;

  INLINE CycleData *cheat() const;
  INLINE int get_read_count() const;
  INLINE int get_write_count() const;

  // In a trivial implementation, we only need to store the CycleData
  // pointer.  Actually, we don't even need to do that, if we're lucky
  // and the compiler doesn't do anything funny with the struct
  // layout.
  #ifndef SIMPLE_STRUCT_POINTERS
  CycleData *_data;
  #endif  // SIMPLE_STRUCT_POINTERS
};

#include "pipelineCyclerTrivialImpl.I"

#endif  // !DO_PIPELINING

#endif

