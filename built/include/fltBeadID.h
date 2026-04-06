/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltBeadID.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTBEADID_H
#define FLTBEADID_H

#include "pandatoolbase.h"

#include "fltBead.h"

/**
 * A base class for any of a broad family of flt beads that include an ID.
 */
class FltBeadID : public FltBead {
public:
  FltBeadID(FltHeader *header);

  const std::string &get_id() const;
  void set_id(const std::string &id);

  virtual void output(std::ostream &out) const;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool extract_ancillary(FltRecordReader &reader);

  virtual bool build_record(FltRecordWriter &writer) const;
  virtual FltError write_ancillary(FltRecordWriter &writer) const;

private:
  std::string _id;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltBead::init_type();
    register_type(_type_handle, "FltBeadID",
                  FltBead::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
