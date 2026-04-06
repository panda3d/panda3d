/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltMaterial.h
 * @author drose
 * @date 2000-08-25
 */

#ifndef FLTMATERIAL_H
#define FLTMATERIAL_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

class DatagramIterator;

/**
 * Represents a single material in the material palette.
 */
class FltMaterial : public FltRecord {
public:
  FltMaterial(FltHeader *header);

  enum Flags {
    F_materials_used    = 0x80000000,
  };

  int _material_index;
  std::string _material_name;
  unsigned int _flags;
  LRGBColor _ambient;
  LRGBColor _diffuse;
  LRGBColor _specular;
  LRGBColor _emissive;
  PN_stdfloat _shininess;
  PN_stdfloat _alpha;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  bool extract_14_record(int index, DatagramIterator &di);
  bool build_14_record(Datagram &datagram);

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
    register_type(_type_handle, "FltMaterial",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltHeader;
};

#endif
