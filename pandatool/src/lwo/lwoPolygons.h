// Filename: lwoPolygons.h
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOPOLYGONS_H
#define LWOPOLYGONS_H

#include <pandatoolbase.h>

#include "lwoChunk.h"

#include <luse.h>
#include <vector_int.h>
#include <referenceCount.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LwoPolygons
// Description : An array of polygons that will be referenced by later
//               chunks.
////////////////////////////////////////////////////////////////////
class LwoPolygons : public LwoChunk {
public:
  enum PolygonFlags {
    PF_continuity_1    = 0x0400,
    PF_continuity_2    = 0x0800,
    PF_numverts_mask   = 0x03ff
  };

  class Polygon : public ReferenceCount {
  public:
    int _flags;
    vector_int _vertices;
  };

  int get_num_polygons() const;
  Polygon *get_polygon(int n) const;

  IffId _polygon_type;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  typedef vector< PT(Polygon) > Polygons;
  Polygons _polygons;
  
public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoChunk::init_type();
    register_type(_type_handle, "LwoPolygons",
		  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "lwoPolygons.I"

#endif

  
