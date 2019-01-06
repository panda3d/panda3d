/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToDXFLayer.cxx
 * @author drose
 * @date 2004-05-04
 */

#include "eggToDXFLayer.h"
#include "eggToDXF.h"
#include "dxfFile.h"
#include "eggGroup.h"
#include "eggGroupNode.h"
#include "eggPolygon.h"
#include "dcast.h"

using std::ostream;

/**
 *
 */
EggToDXFLayer::
EggToDXFLayer(EggToDXF *egg2dxf, EggGroupNode *group) :
  _egg2dxf(egg2dxf), _group(group)
{
  _layer_color = -1;
}

/**
 *
 */
EggToDXFLayer::
EggToDXFLayer(const EggToDXFLayer &copy) :
  _egg2dxf(copy._egg2dxf),
  _group(copy._group),
  _layer_color(copy._layer_color)
{
  // The copy constructor doesn't bother with the ColorCounts.
}

/**
 *
 */
void EggToDXFLayer::
operator = (const EggToDXFLayer &copy) {
  _egg2dxf = copy._egg2dxf;
  _group = copy._group;
  _layer_color = copy._layer_color;

  // The copy constructor doesn't bother with the ColorCounts.
}

/**
 * Records that one polygon is defined using the indicated color.  This will
 * get accumulated; the color used by the majority of polygons will become the
 * layer color.
 */
void EggToDXFLayer::
add_color(const LColor &color) {
  int autocad_color = get_autocad_color(color);

  ColorCounts::iterator cci;
  cci = _color_counts.find(autocad_color);
  if (cci == _color_counts.end()) {
    // The first time a particular color was used.  Count it once.
    _color_counts[autocad_color] = 1;
  } else {
    // This color has been used before.  Count it again.
    (*cci).second++;
  }
}


/**
 * After all polygons have been accounted for, chooses the polygon color that
 * occurred most often as the layer color.
 */
void EggToDXFLayer::
choose_overall_color() {
  int max_count = 0;

  ColorCounts::iterator cci;
  for (cci = _color_counts.begin(); cci != _color_counts.end(); ++cci) {
    int count = (*cci).second;
    if (count > max_count) {
      _layer_color = (*cci).first;
      max_count = count;
    }
  }
}


/**
 * Writes the layer definition into the table at the beginning of the DXF
 * file.  This does not write the actual geometry; that gets done later by
 * write_entities().
 */
void EggToDXFLayer::
write_layer(ostream &out) {
  out << "0\nLAYER\n"
      << "2\n" << _group->get_name() << "\n"
      << "70\n0\n"
      << "62\n" << _layer_color << "\n"
      << "6\nCONTINUOUS\n";
}

/**
 * Writes a polygon as a POLYLINE entity.
 */
void EggToDXFLayer::
write_polyline(EggPolygon *poly, ostream &out) {
  out << "0\nPOLYLINE\n"
      << "8\n" << _group->get_name() << "\n"
      << "66\n1\n"
      << "70\n1\n"
      << "62\n" << get_autocad_color(poly->get_color()) << "\n";

  // Since DXF uses a clockwise ordering convention, we must reverse the order
  // in which we write out the vertices.
  EggPolygon::reverse_iterator vi;
  for (vi = poly->rbegin(); vi != poly->rend(); ++vi) {
    EggVertex *vtx = (*vi);
    LVecBase3d pos = vtx->get_pos3() * _group->get_vertex_frame();
    out << "0\nVERTEX\n"
        << "10\n" << pos[0] << "\n"
        << "20\n" << pos[1] << "\n"
        << "30\n" << pos[2] << "\n";
  }
  out << "0\nSEQEND\n";
}

/**
 * Writes a polygon as a 3DFACE entity.
 */
void EggToDXFLayer::
write_3d_face(EggPolygon *poly, ostream &out) {
  if (poly->size() > 4) {
    // If we have a big polygon, we have to triangulate it, since 3DFaces can
    // only be tris and quads.
    PT(EggGroup) group = new EggGroup;
    poly->triangulate_into(group, true);

    EggGroupNode::iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      EggNode *child = (*ci);
      if (child->is_of_type(EggPolygon::get_class_type())) {
        write_3d_face(DCAST(EggPolygon, child), out);
      }
    }

  } else if (poly->size() > 2) {
    // Otherwise, if we have a tri or a quad, just write it out.
    out << "0\n3DFACE\n"
        << "8\n" << _group->get_name() << "\n";

    // Since DXF uses a clockwise ordering convention, we must reverse the
    // order in which we write out the vertices.
    int i;
    EggPolygon::reverse_iterator vi;
    for (i = 0, vi = poly->rbegin(); vi != poly->rend(); ++i, ++vi) {
      EggVertex *vtx = (*vi);
      LVecBase3d pos = vtx->get_pos3() * _group->get_vertex_frame();
      out << 10 + i << "\n" << pos[0] << "\n"
          << 20 + i << "\n" << pos[1] << "\n"
          << 30 + i << "\n" << pos[2] << "\n";
      if (i == 2 && poly->size() == 3) {
        // A special case for triangles: repeat the last vertex.
        out << 11 + i << "\n" << pos[0] << "\n"
            << 21 + i << "\n" << pos[1] << "\n"
            << 31 + i << "\n" << pos[2] << "\n";
      }
    }
  }
}


/**
 * Writes out the "entities", e.g.  polygons, defined for the current layer.
 */
void EggToDXFLayer::
write_entities(ostream &out) {
  EggGroupNode::iterator ci;
  for (ci = _group->begin(); ci != _group->end(); ++ci) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon *poly = DCAST(EggPolygon, child);
      if (_egg2dxf->_use_polyline) {
        write_polyline(poly, out);
      } else {
        write_3d_face(poly, out);
      }
    }
  }
}

/**
 * Returns the AutoCAD color index that most closely matches the indicated
 * EggColor.
 */
int EggToDXFLayer::
get_autocad_color(const LColor &color) {
  typedef pmap<LColor, int> ColorMap;
  static ColorMap _map;

  ColorMap::iterator cmi;
  cmi = _map.find(color);
  if (cmi != _map.end()) {
    return (*cmi).second;
  }

  int result = DXFFile::find_color(color[0], color[1], color[2]);
  _map[color] = result;
  return result;
}
