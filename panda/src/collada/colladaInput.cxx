// Filename: colladaInput.cxx
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

#include "colladaInput.h"
#include "string_utils.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexWriter.h"

// Collada DOM includes.  No other includes beyond this point.
#include "pre_collada_include.h"
#include <dom/domAccessor.h>
#include <dom/domP.h>
#include <dom/domSource.h>
#include <dom/domVertices.h>

#if PANDA_COLLADA_VERSION >= 15
#include <dom/domInput_local_offset.h>
#include <dom/domInput_local.h>
#else
#include <dom/domInputLocalOffset.h>
#include <dom/domInputLocal.h>
#define domList_of_floats domListOfFloats
#define domList_of_uints domListOfUInts
#endif

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::Constructor
//  Description: Pretty obvious what this does.
////////////////////////////////////////////////////////////////////
ColladaInput::
ColladaInput(const string &semantic) :
  _column_name (NULL),
  _semantic (semantic),
  _offset (0),
  _have_set (false),
  _set (0) {

  if (semantic == "POSITION") {
    _column_name = InternalName::get_vertex();
    _column_contents = GeomEnums::C_point;
  } else if (semantic == "COLOR") {
    _column_name = InternalName::get_color();
    _column_contents = GeomEnums::C_color;
  } else if (semantic == "NORMAL") {
    _column_name = InternalName::get_normal();
    _column_contents = GeomEnums::C_vector;
  } else if (semantic == "TEXCOORD") {
    _column_name = InternalName::get_texcoord();
    _column_contents = GeomEnums::C_texcoord;
  } else if (semantic == "TEXBINORMAL") {
    _column_name = InternalName::get_binormal();
    _column_contents = GeomEnums::C_vector;
  } else if (semantic == "TEXTANGENT") {
    _column_name = InternalName::get_tangent();
    _column_contents = GeomEnums::C_vector;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::Constructor
//  Description: Pretty obvious what this does.
////////////////////////////////////////////////////////////////////
ColladaInput::
ColladaInput(const string &semantic, unsigned int set) :
  _column_name (NULL),
  _semantic (semantic),
  _offset (0),
  _have_set (true),
  _set (set) {

  ostringstream setstr;
  setstr << _set;

  if (semantic == "POSITION") {
    _column_name = InternalName::get_vertex();
    _column_contents = GeomEnums::C_point;
  } else if (semantic == "COLOR") {
    _column_name = InternalName::get_color();
    _column_contents = GeomEnums::C_color;
  } else if (semantic == "NORMAL") {
    _column_name = InternalName::get_normal();
    _column_contents = GeomEnums::C_vector;
  } else if (semantic == "TEXCOORD") {
    _column_name = InternalName::get_texcoord_name(setstr.str());
    _column_contents = GeomEnums::C_texcoord;
  } else if (semantic == "TEXBINORMAL") {
    _column_name = InternalName::get_binormal_name(setstr.str());
    _column_contents = GeomEnums::C_vector;
  } else if (semantic == "TEXTANGENT") {
    _column_name = InternalName::get_tangent_name(setstr.str());
    _column_contents = GeomEnums::C_vector;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::from_dom
//  Description: Returns the ColladaInput object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaInput *ColladaInput::
from_dom(domInput_local_offset &input) {
  // If we already loaded it before, use that.
  if (input.getUserData() != NULL) {
    return (ColladaInput *) input.getUserData();
  }

  ColladaInput *new_input = new ColladaInput(input.getSemantic(), input.getSet());
  new_input->_offset = input.getOffset();

  // If this has the VERTEX semantic, it points to a <vertices> element.
  if (new_input->is_vertex_source()) {
    domVertices *verts = daeSafeCast<domVertices> (input.getSource().getElement());
    nassertr(verts != NULL, NULL);
    daeTArray<domInput_localRef> &inputs = verts->getInput_array();

    // Iterate over the <input> elements in <vertices>.
    for (size_t i = 0; i < inputs.getCount(); ++i) {
      PT(ColladaInput) vtx_input = ColladaInput::from_dom(*inputs[i]);
      new_input->_vertex_inputs.push_back(vtx_input);
    }
  } else {
    domSource *source = daeSafeCast<domSource> (input.getSource().getElement());
    nassertr(source != NULL, NULL);
    new_input->read_data(*source);
  }

  return new_input;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::from_dom
//  Description: Returns the ColladaInput object that represents
//               the provided DOM input element.
////////////////////////////////////////////////////////////////////
ColladaInput *ColladaInput::
from_dom(domInput_local &input) {
  // If we already loaded it before, use that.
  if (input.getUserData() != NULL) {
    return (ColladaInput *) input.getUserData();
  }

  ColladaInput *new_input = new ColladaInput(input.getSemantic());
  new_input->_offset = 0;

  nassertr (!new_input->is_vertex_source(), NULL);

  domSource *source = daeSafeCast<domSource> (input.getSource().getElement());
  nassertr(source != NULL, NULL);
  new_input->read_data(*source);

  return new_input;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::make_vertex_columns
//  Description: Takes a semantic and source URI, and adds a new
//               column to the format.  If this is a vertex source,
//               adds all of the inputs from the corresponding
//               <vertices> element.  Returns the number of
//               columns added to the format.
////////////////////////////////////////////////////////////////////
int ColladaInput::
make_vertex_columns(GeomVertexArrayFormat *format) const {

  if (is_vertex_source()) {
    int counter = 0;
    Inputs::const_iterator it;
    for (it = _vertex_inputs.begin(); it != _vertex_inputs.end(); ++it) {
      counter += (*it)->make_vertex_columns(format);
    }
    return counter;
  }

  nassertr(_column_name != NULL, 0);

  format->add_column(_column_name, _num_bound_params, GeomEnums::NT_stdfloat, _column_contents);
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::read_data
//  Description: Reads the data from the source and fills in _data.
////////////////////////////////////////////////////////////////////
bool ColladaInput::
read_data(domSource &source) {
  _data.clear();

  // Get this, get that
  domFloat_array* float_array = source.getFloat_array();
  if (float_array == NULL) {
    return false;
  }

  domList_of_floats &floats = float_array->getValue();
  domAccessor &accessor = *source.getTechnique_common()->getAccessor();
  domParam_Array &params = accessor.getParam_array();

  // Count the number of params that have a name attribute.
  _num_bound_params = 0;
  for (size_t p = 0; p < params.getCount(); ++p) {
    if (params[p]->getName()) {
      ++_num_bound_params;
    }
  }

  _data.reserve(accessor.getCount());

  domUint pos = accessor.getOffset();
  for (domUint a = 0; a < accessor.getCount(); ++a) {
    domUint c = 0;
    // Yes, the last component defaults to 1 to work around a
    // perspective divide that Panda3D does internally for points.
    LVecBase4f v (0, 0, 0, 1);
    for (domUint p = 0; p < params.getCount(); ++p) {
      if (params[c]->getName()) {
        v[c++] = floats[pos + p];
      }
    }
    _data.push_back(v);
    pos += accessor.getStride();
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::write_data
//  Description: Writes data to the indicated GeomVertexData using
//               the given indices.
////////////////////////////////////////////////////////////////////
void ColladaInput::
write_data(GeomVertexData *vdata, int start_row, domP &p, unsigned int stride) const {
  if (is_vertex_source()) {
    Inputs::const_iterator it;
    for (it = _vertex_inputs.begin(); it != _vertex_inputs.end(); ++it) {
      (*it)->write_data(vdata, start_row, p, stride, _offset);
    }

  } else {
    write_data(vdata, start_row, p, stride, _offset);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaInput::write_data
//  Description: Called internally by the other write_data.
////////////////////////////////////////////////////////////////////
void ColladaInput::
write_data(GeomVertexData *vdata, int start_row, domP &p, unsigned int stride, unsigned int offset) const {
  nassertv(_column_name != NULL);
  GeomVertexWriter writer (vdata, _column_name);
  writer.set_row_unsafe(start_row);

  domList_of_uints &indices = p.getValue();

  // Allocate space for all the rows we're going to write.
  int min_length = start_row + indices.getCount() / stride;
  if (vdata->get_num_rows() < min_length) {
    vdata->unclean_set_num_rows(start_row);
  }

  for (size_t i = 0; i < indices.getCount(); i += stride) {
    size_t index = indices[i + offset];
    writer.add_data4f(_data[index]);
  }
}
