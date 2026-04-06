/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltBead.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTBEAD_H
#define FLTBEAD_H

#include "pandatoolbase.h"

#include "fltRecord.h"
#include "fltTransformRecord.h"

#include "luse.h"

/**
 * A base class for any of a broad family of flt records that represent
 * particular beads in the hierarchy.  These are things like group beads and
 * object beads, as opposed to things like push and pop or comment records.
 */
class FltBead : public FltRecord {
public:
  FltBead(FltHeader *header);

  bool has_transform() const;
  const LMatrix4d &get_transform() const;
  void set_transform(const LMatrix4d &mat);
  void clear_transform();

  int get_num_transform_steps() const;
  FltTransformRecord *get_transform_step(int n);
  const FltTransformRecord *get_transform_step(int n) const;
  void add_transform_step(FltTransformRecord *record);

  int get_replicate_count() const;
  void set_replicate_count(int count);

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool extract_ancillary(FltRecordReader &reader);

  virtual bool build_record(FltRecordWriter &writer) const;
  virtual FltError write_ancillary(FltRecordWriter &writer) const;

private:
  bool extract_transform_matrix(FltRecordReader &reader);
  bool extract_replicate_count(FltRecordReader &reader);

  FltError write_transform(FltRecordWriter &writer) const;
  FltError write_replicate_count(FltRecordWriter &writer) const;

private:
  bool _has_transform;
  LMatrix4d _transform;

  typedef pvector<PT(FltTransformRecord)> Transforms;
  Transforms _transform_steps;

  int _replicate_count;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltBead",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
