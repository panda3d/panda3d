// Filename: fltExternalReference.h
// Created by:  drose (30Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTEXTERNALREFERENCE_H
#define FLTEXTERNALREFERENCE_H

#include <pandatoolbase.h>

#include "fltBead.h"

#include <filename.h>

////////////////////////////////////////////////////////////////////
// 	 Class : FltExternalReference
// Description : An external reference to another flt file (possibly
//               to a specific bead within the flt file).
////////////////////////////////////////////////////////////////////
class FltExternalReference : public FltBead {
public:
  FltExternalReference(FltHeader *header);

  virtual void output(ostream &out) const;

  enum Flags {
    F_color_palette_override      = 0x80000000,
    F_material_palette_override   = 0x40000000,
    F_texture_palette_override    = 0x20000000,
    F_line_style_palette_override = 0x10000000,
    F_sound_palette_override      = 0x08000000,
    F_light_palette_override      = 0x04000000
  };

  Filename _filename;
  string _bead_id;
  int _flags;

  Filename get_ref_filename() const;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

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
    register_type(_type_handle, "FltExternalReference",
		  FltBead::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


