// Filename: colladaPrimitive.cxx
// Created by:  rdb (23May11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "colladaPrimitive.h"
#include "geomLines.h"
#include "geomLinestrips.h"
#include "geomTriangles.h"
#include "geomTrifans.h"
#include "geomTristrips.h"

// Collada DOM includes.  No other includes beyond this point.
#include "pre_collada_include.h"
#include <dom/domLines.h>
#include <dom/domLinestrips.h>
#include <dom/domPolygons.h>
#include <dom/domPolylist.h>
#include <dom/domTriangles.h>
#include <dom/domTrifans.h>
#include <dom/domTristrips.h>

#if PANDA_COLLADA_VERSION < 15
#define domInput_local_offsetRef domInputLocalOffsetRef
#endif

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::Constructor
//  Description: Why do I even bother documenting the simplest of
//               constructors?  A private one at that.
////////////////////////////////////////////////////////////////////
ColladaPrimitive::
ColladaPrimitive(GeomPrimitive *prim, daeTArray<domInput_local_offsetRef> &inputs)
  : _stride (1), _gprim (prim) {

  PT(GeomVertexArrayFormat) aformat = new GeomVertexArrayFormat;

  // Add the inputs one by one.
  for (size_t in = 0; in < inputs.getCount(); ++in) {
    PT(ColladaInput) input = ColladaInput::from_dom(*inputs[in]);
    add_input(input);

    input->make_vertex_columns(aformat);
  }

  // Create the vertex data.
  PT(GeomVertexFormat) format = new GeomVertexFormat();
  format->add_array(aformat);
  _vdata = new GeomVertexData("", GeomVertexFormat::register_format(format), GeomEnums::UH_static);
  _geom = new Geom(_vdata);
  _geom->add_primitive(_gprim);
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domLines &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  ColladaPrimitive *new_prim =
    new ColladaPrimitive(new GeomLines(GeomEnums::UH_static),
                         prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  domPRef p = prim.getP();
  if (p != NULL) {
    new_prim->load_primitive(*p);
  }

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domLinestrips &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  ColladaPrimitive *new_prim =
    new ColladaPrimitive(new GeomLinestrips(GeomEnums::UH_static),
                         prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  new_prim->load_primitives(prim.getP_array());

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domPolygons &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  // We use trifans to represent polygons, seems to be easiest.
  // I tried using tristrips instead, but for some reason,
  // this resulted in a few flipped polygons.  Weird.
  ColladaPrimitive *new_prim =
    new ColladaPrimitive(new GeomTrifans(GeomEnums::UH_static),
                         prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  new_prim->load_primitives(prim.getP_array());

  if (prim.getPh_array().getCount() > 0) {
    collada_cat.error()
      << "Polygons with holes are not supported!\n";
  }

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domPolylist &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  // We use trifans to represent polygons, seems to be easiest.
  // I tried using tristrips instead, but for some reason,
  // this resulted in a few flipped polygons.  Weird.
  PT(GeomPrimitive) gprim = new GeomTrifans(GeomEnums::UH_static);

  ColladaPrimitive *new_prim =
    new ColladaPrimitive(gprim, prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  domPRef p = prim.getP();
  domPolylist::domVcountRef vcounts = prim.getVcount();
  if (p == NULL || vcounts == NULL) {
    return new_prim;
  }

  new_prim->write_data(new_prim->_vdata, 0, *p);

  daeTArray<domUint> &values = vcounts->getValue();
  for (size_t i = 0; i < values.getCount(); ++i) {
    unsigned int vcount = values[i];
    gprim->add_next_vertices(vcount);
    gprim->close_primitive();
  }

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domTriangles &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  ColladaPrimitive *new_prim =
    new ColladaPrimitive(new GeomTriangles(GeomEnums::UH_static),
                         prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  domPRef p = prim.getP();
  if (p != NULL) {
    new_prim->load_primitive(*p);
  }

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domTrifans &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  ColladaPrimitive *new_prim =
    new ColladaPrimitive(new GeomTrifans(GeomEnums::UH_static),
                         prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  new_prim->load_primitives(prim.getP_array());

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::from_dom
//  Description: Returns the ColladaPrimitive object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaPrimitive *ColladaPrimitive::
from_dom(domTristrips &prim) {
  // If we already loaded it before, use that.
  if (prim.getUserData() != NULL) {
    return (ColladaPrimitive *) prim.getUserData();
  }

  ColladaPrimitive *new_prim =
    new ColladaPrimitive(new GeomTristrips(GeomEnums::UH_static),
                         prim.getInput_array());
  new_prim->_material = prim.getMaterial();

  prim.setUserData(new_prim);

  new_prim->load_primitives(prim.getP_array());

  return new_prim;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::write_data
//  Description: Writes the vertex data to the GeomVertexData.
//               Returns the number of rows written.
////////////////////////////////////////////////////////////////////
unsigned int ColladaPrimitive::
write_data(GeomVertexData *vdata, int start_row, domP &p) {
  unsigned int num_vertices = p.getValue().getCount() / _stride;

  Inputs::iterator it;
  for (it = _inputs.begin(); it != _inputs.end(); ++it) {
    (*it)->write_data(vdata, start_row, p, _stride);
  }

  return num_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::load_primitive
//  Description: Adds the given indices to the primitive, and
//               writes the relevant data to the geom.
////////////////////////////////////////////////////////////////////
void ColladaPrimitive::
load_primitive(domP &p) {
  _gprim->add_next_vertices(write_data(_vdata, 0, p));
  _gprim->close_primitive();
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::load_primitives
//  Description: Adds the given indices to the primitive, and
//               writes the relevant data to the geom.
////////////////////////////////////////////////////////////////////
void ColladaPrimitive::
load_primitives(domP_Array &p_array) {
  int start_row = 0;

  for (size_t i = 0; i < p_array.getCount(); ++i) {
    unsigned int num_vertices = write_data(_vdata, start_row, *p_array[i]);
    _gprim->add_next_vertices(num_vertices);
    _gprim->close_primitive();
    start_row += num_vertices;
  }
}

