// Filename: fltMaterial.h
// Created by:  drose (25Aug00)
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

#ifndef FLTMATERIAL_H
#define FLTMATERIAL_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : FltMaterial
// Description : Represents a single material in the material palette.
////////////////////////////////////////////////////////////////////
class FltMaterial : public FltRecord {
public:
  FltMaterial(FltHeader *header);

  enum Flags {
    F_materials_used    = 0x80000000,
  };

  int _material_index;
  string _material_name;
  unsigned int _flags;
  RGBColorf _ambient;
  RGBColorf _diffuse;
  RGBColorf _specular;
  RGBColorf _emissive;
  float _shininess;
  float _alpha;

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


