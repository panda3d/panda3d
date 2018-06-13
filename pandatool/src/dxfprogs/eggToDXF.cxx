/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToDXF.cxx
 * @author drose
 * @date 2004-05-04
 */

#include "eggToDXF.h"
#include "eggPolygon.h"
#include "dcast.h"

/**
 *
 */
EggToDXF::
EggToDXF() :
  EggToSomething("DXF", ".dxf", true, false)
{
  set_binary_output(true);
  set_program_brief("convert .egg files to AutoCAD .dxf files");
  set_program_description
    ("This program converts files from egg format to AutoCAD DXF format.  "
     "Since DXF does not support nested hierarchies, vertex normals, or any "
     "fancy stuff you are probably used to, there is some information lost "
     "in the conversion");

  add_option
    ("p", "", 0,
     "Use POLYLINE to represent polygons instead of the default, 3DFACE.",
     &EggToDXF::dispatch_none, &_use_polyline);

  _coordinate_system = CS_zup_right;
  _got_coordinate_system = true;
}

/**
 *
 */
void EggToDXF::
run() {
  get_layers(_data);
  if (_layers.empty()) {
    nout << "Egg file contains no polygons.  Output file not written.\n";
    exit(1);
  }

  // uniquify_names("layer", _layers.begin(), _layers.end());

  std::ostream &out = get_output();

  // Autodesk says we don't need the header, but some DXF-reading programs
  // might get confused if it's missing.  We'll write an empty header.
  out << "0\nSECTION\n"
      << "2\nHEADER\n"
      << "0\nENDSEC\n";

  write_tables(out);
  write_entities(out);
  out << "0\nEOF\n";   // Mark end of file.

  if (!out) {
    nout << "An error occurred while writing.\n";
    exit(1);
  }
}

/**
 * Traverses the hierarchy, looking for groups that contain polygons.  Any
 * such groups are deemed to be layers, and are added to the layers set.
 */
void EggToDXF::
get_layers(EggGroupNode *group) {
  bool has_polys = false;

  EggToDXFLayer layer(this, group);

  EggGroupNode::iterator ci;
  for (ci = group->begin(); ci != group->end(); ++ci) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon *poly = DCAST(EggPolygon, child);
      has_polys = true;

      layer.add_color(poly->get_color());

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      get_layers(DCAST(EggGroupNode, child));
    }
  }

  if (has_polys) {
    layer.choose_overall_color();
    _layers.push_back(layer);
  }
}


/**
 * Writes out the "layers", e.g.  groups.  This is just the layers definition
 * in the tables section at the beginning of the file; the actual geometry
 * gets written later, in write_entities().
 */
void EggToDXF::
write_tables(std::ostream &out) {
  out << "0\nSECTION\n"
      << "2\nTABLES\n"  // Begin TABLES section.
      << "0\nTABLE\n"
      << "2\nLAYER\n"   // Define LAYERS.
      << "70\n" << _layers.size() << "\n";

  EggToDXFLayers::iterator li;
  for (li = _layers.begin(); li != _layers.end(); ++li) {
    (*li).write_layer(out);
  }

  out << "0\nENDTAB\n"    // End LAYERS definition.
      << "0\nENDSEC\n";   // End TABLES section.
}

/**
 * Writes out the "entities", e.g.  polygons, defined for all layers.
 */
void EggToDXF::
write_entities(std::ostream &out) {
  out << "0\nSECTION\n"
      << "2\nENTITIES\n";  // Begin ENTITIES section.

  EggToDXFLayers::iterator li;
  for (li = _layers.begin(); li != _layers.end(); ++li) {
    (*li).write_entities(out);
  }

  out << "0\nENDSEC\n";   // End ENTITIES section.
}



int main(int argc, char *argv[]) {
  EggToDXF prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
