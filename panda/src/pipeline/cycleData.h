/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cycleData.h
 * @author drose
 * @date 2002-02-21
 */

#ifndef CYCLEDATA_H
#define CYCLEDATA_H

#include "pandabase.h"
#include "typeHandle.h"
#include "nodeReferenceCount.h"

class BamWriter;
class BamReader;
class TypedWritable;
class Datagram;
class DatagramIterator;

/**
 * A single page of data maintained by a PipelineCycler.  Normally you should
 * inherit from this class to define the data structures that are important to
 * protect between stages of a pipeline.  See PipelineCycler.
 */
#ifdef DO_PIPELINING

// If we are compiling in pipelining support, we maintain a pointer to a
// CycleData object in each containing class, instead of the object itself.
// Thus, it should be a ReferenceCount object.  Furthermore, since we want to
// make a distinction between references within the cycler, and references
// outside the cycler (e.g.  GeomPipelineReader), we make it a
// NodeReferenceCount.
class EXPCL_PANDA_PIPELINE CycleData : public NodeReferenceCount

#else  // !DO_PIPELINING

// If we are *not* compiling in pipelining support, the CycleData object is
// stored directly within its containing classes, and hence should not be a
// ReferenceCount object.
class EXPCL_PANDA_PIPELINE CycleData : public MemoryBase

#endif  // DO_PIPELINING
{
public:
  INLINE CycleData() = default;
  INLINE CycleData(CycleData &&from) = default;
  INLINE CycleData(const CycleData &copy) = default;
  virtual ~CycleData();

  CycleData &operator = (CycleData &&from) = default;
  CycleData &operator = (const CycleData &copy) = default;

  virtual CycleData *make_copy() const=0;

  virtual void write_datagram(BamWriter *, Datagram &) const;
  virtual void write_datagram(BamWriter *, Datagram &, void *extra_data) const;
  virtual int complete_pointers(TypedWritable **p_list, BamReader *manager);
  virtual void fillin(DatagramIterator &scan, BamReader *manager);
  virtual void fillin(DatagramIterator &scan, BamReader *manager,
                      void *extra_data);

  virtual TypeHandle get_parent_type() const;
  virtual void output(std::ostream &out) const;

#ifdef DO_PIPELINING
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    NodeReferenceCount::init_type();
    register_type(_type_handle, "CycleData",
                  NodeReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
#endif
};

INLINE std::ostream &
operator << (std::ostream &out, const CycleData &cd) {
  cd.output(out);
  return out;
}

#include "cycleData.I"

#endif
