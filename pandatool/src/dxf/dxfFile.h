/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfFile.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFFILE_H
#define DXFFILE_H

#include "pandatoolbase.h"

#include "dxfLayer.h"
#include "dxfLayerMap.h"
#include "dxfVertex.h"

#include "luse.h"
#include "filename.h"


static const int DXF_max_line = 256;
static const int DXF_num_colors = 256;

/**
 * A generic DXF-reading class.  This class can read a DXF file but doesn't
 * actually do anything with the data; it's intended to be inherited from and
 * the appropriate functions overridden (particularly DoneEntity()).
 */
class DXFFile : public MemoryBase {
public:
  DXFFile();
  virtual ~DXFFile();

  void process(Filename filename);
  void process(std::istream *in, bool owns_in);

  // These functions are called as the file is processed.  These are the main
  // hooks for redefining how the class should dispense its data.  As each
  // function is called, the state stored in the DXFFile class reflects the
  // data that was most recently read.

  virtual void begin_file();
  virtual void begin_section();
  virtual void done_vertex();
  virtual void done_entity();
  virtual void end_section();
  virtual void end_file();
  virtual void error();

  // new_layer() is called whenever the DXFFile class encounters a new Layer
  // definition, and must allocate a DXFLayer instance.  This function is
  // provided so that user code may force allocate of a specialized DXFLayer
  // instance instead.
  virtual DXFLayer *new_layer(const std::string &name) {
    return new DXFLayer(name);
  }

  enum State {
    ST_top,
    ST_section,
    ST_entity,
    ST_verts,
    ST_error,
    ST_done,
  };
  enum Section {
    SE_unknown,
    SE_header,
    SE_tables,
    SE_blocks,
    SE_entities,
    SE_objects,
  };
  enum Entity {
    EN_unknown,
    EN_3dface,
    EN_point,
    EN_insert,
    EN_vertex,
    EN_polyline,
  };
  enum PolylineFlags {
    PF_closed              = 0x01,
    PF_curve_fit           = 0x02,
    PF_spline_fit          = 0x04,
    PF_3d                  = 0x08,
    PF_3d_mesh             = 0x10,
    PF_closed_n            = 0x20,
    PF_polyface            = 0x40,
    PF_continuous_linetype = 0x80,
  };

  // This is a table of standard Autocad colors.  DXF files can store only a
  // limited range of colors; specifically, the 255 colors defined by Autocad.
  struct Color {
    double r, g, b;
  };
  static Color _colors[DXF_num_colors];

  // find_color() returns the index of the closest matching AutoCAD color to
  // the indicated r, g, b.
  static int find_color(double r, double g, double b);

  // get_color() returns the r,g,b of the current entity.  It is valid at the
  // time done_entity() is called.
  const Color &get_color() const;

  // Some entities are defined in world coordinates, in 3-d space; other
  // entities are inherently 2-d in nature and are defined in planar
  // coordinates and must be converted to 3-d space.  Call this function from
  // done_entity() to convert a 2-d entity to 3-d world coordinates.
  void ocs_2_wcs();

  // These members indicate the current state and describe properties of the
  // current thing being processed.  They are valid at done_entity(), and at
  // other times.
  int _flags;
  Section _section;
  Entity _entity;
  LPoint3d _p, _q, _r, _s;
  LVector3d _z;
  int _color_index;
  DXFLayer *_layer;

  // _verts is the list of vertices associated with the current entity.  It is
  // valid at the time done_entity() is called.
  DXFVertices _verts;

  // This is the set of layers encountered within the DXF file.
  DXFLayerMap _layers;

protected:
  State _state;
  bool _vertices_follow;
  LMatrix4d _ocs2wcs;

  std::istream *_in;
  bool _owns_in;

  int _code;
  std::string _string;

  void compute_ocs();

  bool get_group();
  void change_state(State new_state);
  void change_section(Section new_section);
  void change_layer(const std::string &layer_name);
  void change_entity(Entity new_entity);
  void reset_entity();

  void state_top();
  void state_section();
  void state_entity();
  void state_verts();
};

std::ostream &operator << (std::ostream &out, const DXFFile::State &state);
std::ostream &operator << (std::ostream &out, const DXFFile::Section &section);
std::ostream &operator << (std::ostream &out, const DXFFile::Entity &entity);

#endif
