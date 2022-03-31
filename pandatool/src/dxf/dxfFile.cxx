/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfFile.cxx
 * @author drose
 * @date 2004-05-04
 */

#include "dxfFile.h"
#include "string_utils.h"
#include "virtualFileSystem.h"

using std::istream;
using std::ostream;
using std::string;

DXFFile::Color DXFFile::_colors[DXF_num_colors] = {
  { 1, 1, 1 },        // Color 0 is not used.
  { 1, 0, 0 },        // Color 1 = Red
  { 1, 1, 0 },        // Color 2 = Yellow
  { 0, 1, 0 },        // Color 3 = Green
  { 0, 1, 1 },        // Color 4 = Cyan
  { 0, 0, 1 },        // Color 5 = Blue
  { 1, 0, 1 },        // Color 6 = Magenta
  { 1, 1, 1 },        // Color 7 = Black/White
  { 0.3, 0.3, 0.3 },  // Color 8 = Gray
  { 0.7, 0.7, 0.7 },  // Color 9 = Gray
  { 1, 0, 0 },        // Remaining colors are from the fancy palette.
  { 1, 0.5, 0.5 },
  { 0.65, 0, 0 },
  { 0.65, 0.325, 0.325 },
  { 0.5, 0, 0 },
  { 0.5, 0.25, 0.25 },
  { 0.3, 0, 0 },
  { 0.3, 0.15, 0.15 },
  { 0.15, 0, 0 },
  { 0.15, 0.075, 0.075 },
  { 1, 0.25, 0 },
  { 1, 0.625, 0.5 },
  { 0.65, 0.1625, 0 },
  { 0.65, 0.4063, 0.325 },
  { 0.5, 0.125, 0 },
  { 0.5, 0.3125, 0.25 },
  { 0.3, 0.075, 0 },
  { 0.3, 0.1875, 0.15 },
  { 0.15, 0.0375, 0 },
  { 0.15, 0.0938, 0.075 },
  { 1, 0.5, 0 },
  { 1, 0.75, 0.5 },
  { 0.65, 0.325, 0 },
  { 0.65, 0.4875, 0.325 },
  { 0.5, 0.25, 0 },
  { 0.5, 0.375, 0.25 },
  { 0.3, 0.15, 0 },
  { 0.3, 0.225, 0.15 },
  { 0.15, 0.075, 0 },
  { 0.15, 0.1125, 0.075 },
  { 1, 0.75, 0 },
  { 1, 0.875, 0.5 },
  { 0.65, 0.4875, 0 },
  { 0.65, 0.5688, 0.325 },
  { 0.5, 0.375, 0 },
  { 0.5, 0.4375, 0.25 },
  { 0.3, 0.225, 0 },
  { 0.3, 0.2625, 0.15 },
  { 0.15, 0.1125, 0 },
  { 0.15, 0.1313, 0.075 },
  { 1, 1, 0 },
  { 1, 1, 0.5 },
  { 0.65, 0.65, 0 },
  { 0.65, 0.65, 0.325 },
  { 0.5, 0.5, 0 },
  { 0.5, 0.5, 0.25 },
  { 0.3, 0.3, 0 },
  { 0.3, 0.3, 0.15 },
  { 0.15, 0.15, 0 },
  { 0.15, 0.15, 0.075 },
  { 0.75, 1, 0 },
  { 0.875, 1, 0.5 },
  { 0.4875, 0.65, 0 },
  { 0.5688, 0.65, 0.325 },
  { 0.375, 0.5, 0 },
  { 0.4375, 0.5, 0.25 },
  { 0.225, 0.3, 0 },
  { 0.2625, 0.3, 0.15 },
  { 0.1125, 0.15, 0 },
  { 0.1313, 0.15, 0.075 },
  { 0.5, 1, 0 },
  { 0.75, 1, 0.5 },
  { 0.325, 0.65, 0 },
  { 0.4875, 0.65, 0.325 },
  { 0.25, 0.5, 0 },
  { 0.375, 0.5, 0.25 },
  { 0.15, 0.3, 0 },
  { 0.225, 0.3, 0.15 },
  { 0.075, 0.15, 0 },
  { 0.1125, 0.15, 0.075 },
  { 0.25, 1, 0 },
  { 0.625, 1, 0.5 },
  { 0.1625, 0.65, 0 },
  { 0.4063, 0.65, 0.325 },
  { 0.125, 0.5, 0 },
  { 0.3125, 0.5, 0.25 },
  { 0.075, 0.3, 0 },
  { 0.1875, 0.3, 0.15 },
  { 0.0375, 0.15, 0 },
  { 0.0938, 0.15, 0.075 },
  { 0, 1, 0 },
  { 0.5, 1, 0.5 },
  { 0, 0.65, 0 },
  { 0.325, 0.65, 0.325 },
  { 0, 0.5, 0 },
  { 0.25, 0.5, 0.25 },
  { 0, 0.3, 0 },
  { 0.15, 0.3, 0.15 },
  { 0, 0.15, 0 },
  { 0.075, 0.15, 0.075 },
  { 0, 1, 0.25 },
  { 0.5, 1, 0.625 },
  { 0, 0.65, 0.1625 },
  { 0.325, 0.65, 0.4063 },
  { 0, 0.5, 0.125 },
  { 0.25, 0.5, 0.3125 },
  { 0, 0.3, 0.075 },
  { 0.15, 0.3, 0.1875 },
  { 0, 0.15, 0.0375 },
  { 0.075, 0.15, 0.0938 },
  { 0, 1, 0.5 },
  { 0.5, 1, 0.75 },
  { 0, 0.65, 0.325 },
  { 0.325, 0.65, 0.4875 },
  { 0, 0.5, 0.25 },
  { 0.25, 0.5, 0.375 },
  { 0, 0.3, 0.15 },
  { 0.15, 0.3, 0.225 },
  { 0, 0.15, 0.075 },
  { 0.075, 0.15, 0.1125 },
  { 0, 1, 0.75 },
  { 0.5, 1, 0.875 },
  { 0, 0.65, 0.4875 },
  { 0.325, 0.65, 0.5688 },
  { 0, 0.5, 0.375 },
  { 0.25, 0.5, 0.4375 },
  { 0, 0.3, 0.225 },
  { 0.15, 0.3, 0.2625 },
  { 0, 0.15, 0.1125 },
  { 0.075, 0.15, 0.1313 },
  { 0, 1, 1 },
  { 0.5, 1, 1 },
  { 0, 0.65, 0.65 },
  { 0.325, 0.65, 0.65 },
  { 0, 0.5, 0.5 },
  { 0.25, 0.5, 0.5 },
  { 0, 0.3, 0.3 },
  { 0.15, 0.3, 0.3 },
  { 0, 0.15, 0.15 },
  { 0.075, 0.15, 0.15 },
  { 0, 0.75, 1 },
  { 0.5, 0.875, 1 },
  { 0, 0.4875, 0.65 },
  { 0.325, 0.5688, 0.65 },
  { 0, 0.375, 0.5 },
  { 0.25, 0.4375, 0.5 },
  { 0, 0.225, 0.3 },
  { 0.15, 0.2625, 0.3 },
  { 0, 0.1125, 0.15 },
  { 0.075, 0.1313, 0.15 },
  { 0, 0.5, 1 },
  { 0.5, 0.75, 1 },
  { 0, 0.325, 0.65 },
  { 0.325, 0.4875, 0.65 },
  { 0, 0.25, 0.5 },
  { 0.25, 0.375, 0.5 },
  { 0, 0.15, 0.3 },
  { 0.15, 0.225, 0.3 },
  { 0, 0.075, 0.15 },
  { 0.075, 0.1125, 0.15 },
  { 0, 0.25, 1 },
  { 0.5, 0.625, 1 },
  { 0, 0.1625, 0.65 },
  { 0.325, 0.4063, 0.65 },
  { 0, 0.125, 0.5 },
  { 0.25, 0.3125, 0.5 },
  { 0, 0.075, 0.3 },
  { 0.15, 0.1875, 0.3 },
  { 0, 0.0375, 0.15 },
  { 0.075, 0.0938, 0.15 },
  { 0, 0, 1 },
  { 0.5, 0.5, 1 },
  { 0, 0, 0.65 },
  { 0.325, 0.325, 0.65 },
  { 0, 0, 0.5 },
  { 0.25, 0.25, 0.5 },
  { 0, 0, 0.3 },
  { 0.15, 0.15, 0.3 },
  { 0, 0, 0.15 },
  { 0.075, 0.075, 0.15 },
  { 0.25, 0, 1 },
  { 0.625, 0.5, 1 },
  { 0.1625, 0, 0.65 },
  { 0.4063, 0.325, 0.65 },
  { 0.125, 0, 0.5 },
  { 0.3125, 0.25, 0.5 },
  { 0.075, 0, 0.3 },
  { 0.1875, 0.15, 0.3 },
  { 0.0375, 0, 0.15 },
  { 0.0938, 0.075, 0.15 },
  { 0.5, 0, 1 },
  { 0.75, 0.5, 1 },
  { 0.325, 0, 0.65 },
  { 0.4875, 0.325, 0.65 },
  { 0.25, 0, 0.5 },
  { 0.375, 0.25, 0.5 },
  { 0.15, 0, 0.3 },
  { 0.225, 0.15, 0.3 },
  { 0.075, 0, 0.15 },
  { 0.1125, 0.075, 0.15 },
  { 0.75, 0, 1 },
  { 0.875, 0.5, 1 },
  { 0.4875, 0, 0.65 },
  { 0.5688, 0.325, 0.65 },
  { 0.375, 0, 0.5 },
  { 0.4375, 0.25, 0.5 },
  { 0.225, 0, 0.3 },
  { 0.2625, 0.15, 0.3 },
  { 0.1125, 0, 0.15 },
  { 0.1313, 0.075, 0.15 },
  { 1, 0, 1 },
  { 1, 0.5, 1 },
  { 0.65, 0, 0.65 },
  { 0.65, 0.325, 0.65 },
  { 0.5, 0, 0.5 },
  { 0.5, 0.25, 0.5 },
  { 0.3, 0, 0.3 },
  { 0.3, 0.15, 0.3 },
  { 0.15, 0, 0.15 },
  { 0.15, 0.075, 0.15 },
  { 1, 0, 0.75 },
  { 1, 0.5, 0.875 },
  { 0.65, 0, 0.4875 },
  { 0.65, 0.325, 0.5688 },
  { 0.5, 0, 0.375 },
  { 0.5, 0.25, 0.4375 },
  { 0.3, 0, 0.225 },
  { 0.3, 0.15, 0.2625 },
  { 0.15, 0, 0.1125 },
  { 0.15, 0.075, 0.1313 },
  { 1, 0, 0.5 },
  { 1, 0.5, 0.75 },
  { 0.65, 0, 0.325 },
  { 0.65, 0.325, 0.4875 },
  { 0.5, 0, 0.25 },
  { 0.5, 0.25, 0.375 },
  { 0.3, 0, 0.15 },
  { 0.3, 0.15, 0.225 },
  { 0.15, 0, 0.075 },
  { 0.15, 0.075, 0.1125 },
  { 1, 0, 0.25 },
  { 1, 0.5, 0.625 },
  { 0.65, 0, 0.1625 },
  { 0.65, 0.325, 0.4063 },
  { 0.5, 0, 0.125 },
  { 0.5, 0.25, 0.3125 },
  { 0.3, 0, 0.075 },
  { 0.3, 0.15, 0.1875 },
  { 0.15, 0, 0.0375 },
  { 0.15, 0.075, 0.0938 },
  { 0.33, 0.33, 0.33 },
  { 0.464, 0.464, 0.464 },
  { 0.598, 0.598, 0.598 },
  { 0.732, 0.732, 0.732 },
  { 0.866, 0.866, 0.866 },
  { 1, 1, 1 },
};


/**
 *
 */
DXFFile::
DXFFile() {
  _in = nullptr;
  _owns_in = false;
  _layer = nullptr;
  reset_entity();
  _color_index = -1;
}

/**
 *
 */
DXFFile::
~DXFFile() {
  if (_owns_in) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_in);
  }
}


/**
 * Opens the indicated filename and reads it as a DXF file.
 */
void DXFFile::
process(Filename filename) {
  filename.set_text();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *in = vfs->open_read_file(filename, true);
  if (in == nullptr) {
    return;
  }
  process(in, true);
}


/**
 * Reads the indicated stream as a DXF file.  If owns_in is true, then the
 * istream will be deleted via vfs->close_read_file() when the DXFFile object
 * destructs.
 */
void DXFFile::
process(istream *in, bool owns_in) {
  if (_owns_in) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_in);
  }
  _in = in;
  _owns_in = owns_in;
  _state = ST_top;

  begin_file();
  while (_state != ST_done && _state != ST_error) {
    if (get_group()) {
      switch (_state) {
      case ST_top:
        state_top();
        break;

      case ST_section:
        state_section();
        break;

      case ST_entity:
        state_entity();
        break;

      case ST_verts:
        state_verts();
        break;

      default:
        break;
      }
    }
  }
}



/**
 * A hook for user code, if desired.  This function is called whenever
 * processing begins on the DXF file.
 */
void DXFFile::
begin_file() {
}


/**
 * A hook for user code, if desired.  This function is called whenever a new
 * section in the DXF file is encountered.
 */
void DXFFile::
begin_section() {
}


/**
 * A hook for user code, if desired.  This function is called whenever a
 * vertex is read from the DXF file.  This function has the default behavior
 * of adding the vertex to the _verts list, so that when done_entity() is
 * called later, it will have the complete list of vertices available to it.
 */
void DXFFile::
done_vertex() {
  DXFVertex v;
  v._p = _p;
  _verts.push_back(v);
}


/**
 * This is the primary hook for user code.  This function is called when an
 * entity is read from the DXF file.  This may be something like a polygon,
 * point, or a polygon mesh: any geometry.  It is up to the user code to
 * override this function and do something interesting with each piece of
 * geometry that is read.
 */
void DXFFile::
done_entity() {
}


/**
 * A hook for user code, if desired.  This function is called as each section
 * in the DXF file is finished.
 */
void DXFFile::
end_section() {
}


/**
 * A hook for user code, if desired.  This function is called when the DXF
 * processing is complete.
 */
void DXFFile::
end_file() {
}


/**
 * A hook for user code, if desired.  This function is called when some
 * unexpected error occurs while reading the DXF file.
 */
void DXFFile::
error() {
  nout << "Error!\n";
}



/**
 * Returns the index of the closest matching AutoCAD color to the indicated r,
 * g, b.
 */
int DXFFile::
find_color(double r, double g, double b) {
  double best_diff = 4.0;   // 4 is greater than our expected max, 3.
  int best_index = 7;

  for (int i = 0; i < 255; i++) {
    double diff = ((r - _colors[i].r) * (r - _colors[i].r) +
                   (g - _colors[i].g) * (g - _colors[i].g) +
                   (b - _colors[i].b) * (b - _colors[i].b));
    if (diff < best_diff) {
      best_diff = diff;
      best_index = i;
    }
  }

  return best_index;
}

/**
 * This is a convenience function to return the r,g,b color of the current
 * entity (at the time of done_entity()).  It's based on the _color_index
 * value that was read from the DXF file.
 */
const DXFFile::Color &DXFFile::
get_color() const {
  if (_color_index >= 0 && _color_index <= 255) {
    return _colors[_color_index];
  }
  return _colors[0];
}


/**
 * Assuming the current entity is a planar-based entity, for instance, a 2-d
 * polygon (as opposed to a 3-d polygon), this converts the coordinates from
 * the funny planar coordinate system to the world coordinates.  It converts
 * the _p value of the entity, as well as all vertices in the _verts list.
 */
void DXFFile::
ocs_2_wcs() {
  compute_ocs();

  // Convert the entity's position.
  _p = _p * _ocs2wcs;

  // Maybe we have these coordinates too.
  _q = _q * _ocs2wcs;
  _r = _r * _ocs2wcs;
  _s = _s * _ocs2wcs;

  // If there are any vertices, convert them too.
  DXFVertices::iterator vi;
  for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
    (*vi)._p = (*vi)._p * _ocs2wcs;
  }
}


/**
 * Computes the matrix used to convert from the planar coordinate system to
 * world coordinates.
 */
void DXFFile::
compute_ocs() {
  // A 2-d entity's vertices might be defined in an "Object Coordinate System"
  // which has a funny definition.  Its Z axis is defined by _z, and its X and
  // Y axes are inferred from that.  The origin is the same as the world
  // coordinate system's origin.

  // The Z axis is _z.  Determine the x and y axes.
  LVector3d x, y;

  if (fabs(_z[0]) < 1.0/64.0 && fabs(_z[1]) < 1.0/64.0) {
    x = cross(LVector3d(0.0, 1.0, 0.0), _z);
  } else {
    x = cross(LVector3d(0.0, 0.0, 1.0), _z);
  }
  x.normalize();
  y = cross(x, _z);
  y.normalize();

  // Now build a rotate matrix from these vectors.
  LMatrix4d
    ocs( x[0],  x[1],  x[2],    0,
         y[0],  y[1],  y[2],    0,
        _z[0], _z[1], _z[2],    0,
            0,     0,     0,    1);

  _ocs2wcs.invert_from(ocs);
}


/**
 * Reads the next code, string pair from the DXF file.  This is the basic unit
 * of data in a DXF file.
 */
bool DXFFile::
get_group() {
  istream &in = *_in;
  do {
    in >> _code;
    if (!in) {
      change_state(ST_error);
      return false;
    }

    // Now skip past exactly one newline character and any number of other
    // whitespace characters.
    while (in && in.peek() != '\n') {
      in.get();
    }
    in.get();
    while (in && isspace(in.peek()) && in.peek() != '\n') {
      in.get();
    }

    std::getline(in, _string);
    _string = trim_right(_string);

    if (!in) {
      change_state(ST_error);
      return false;
    }

    // If we just read a comment, go back and get another one.
  } while (_code == 999);

  return true;
}


/**
 * Called as new nodes are read to update the internal state correctly.
 */
void DXFFile::
change_state(State new_state) {
  if (_state == ST_verts) {
    done_vertex();
    _p.set(0.0, 0.0, 0.0);
    _q.set(0.0, 0.0, 0.0);
    _r.set(0.0, 0.0, 0.0);
    _s.set(0.0, 0.0, 0.0);
  }
  if ((_state == ST_entity || _state == ST_verts) &&
      new_state != ST_verts) {
    // We finish an entity when we read a new entity, or when we've read the
    // last vertex (if we were scanning the vertices after an entity).
    done_entity();
    reset_entity();
  }
  switch (new_state) {
  case ST_top:
    end_section();
    break;

  case ST_done:
    end_file();
    break;

  default:
    break;
  }
  _state = new_state;
}


/**
 *
 */
void DXFFile::
change_section(Section new_section) {
  change_state(ST_section);
  _section = new_section;
  begin_section();
}


/**
 * Given a newly read layer name, sets the _layer pointer to point to the
 * associate layer.  If the layer name has not been encountered before,
 * creates a new layer definition.
 */
void DXFFile::
change_layer(const string &layer_name) {
  if (_layer == nullptr || _layer->get_name() != layer_name) {
    _layer = _layers.get_layer(layer_name, this);
  }
}


/**
 *
 */
void DXFFile::
change_entity(Entity new_entity) {
  if (new_entity == EN_vertex && _vertices_follow) {
    // If we read a new vertex and we're still scanning the vertices that
    // follow an entity, keep scanning it--we haven't finished the entity yet.
    change_state(ST_verts);

  } else {
    // Otherwise, begin a new entity.
    change_state(ST_entity);
    _entity = new_entity;
  }
}


/**
 * Resets the current entity to its initial, default state prior to reading a
 * new entity.
 */
void DXFFile::
reset_entity() {
  _p.set(0.0, 0.0, 0.0);
  _q.set(0.0, 0.0, 0.0);
  _r.set(0.0, 0.0, 0.0);
  _s.set(0.0, 0.0, 0.0);
  _z.set(0.0, 0.0, 1.0);
  _vertices_follow = false;
  // _color_index = -1;

  _verts.erase(_verts.begin(), _verts.end());
}


/**
 * Does the DXF processing when we are at the top of the file, outside of any
 * section.
 */
void DXFFile::
state_top() {
  if (_code != 0) {
    nout << "Group code 0 not found at top level; found code " << _code
         << " instead.\n";
    change_state(ST_error);
  } else {
    if (_string == "SECTION") {
      if (get_group()) {
        if (_code != 2) {
          nout << "Group code 0 not immediately followed by code 2; found code "
               << _code << " instead.\n";
        } else {
          if (_string == "HEADER") {
            change_section(SE_header);
          } else if (_string == "TABLES") {
            change_section(SE_tables);
          } else if (_string == "BLOCKS") {
            change_section(SE_blocks);
          } else if (_string == "ENTITIES") {
            change_section(SE_entities);
          } else if (_string == "OBJECTS") {
            change_section(SE_objects);
          } else {
            change_section(SE_unknown);
          }
        }
      }
    } else if (_string == "EOF") {
      change_state(ST_done);
    } else {
      nout << "Unexpected section at top level: '" << _string << "'\n";
      change_state(ST_error);
    }
  }
}



/**
 * Does the DXF processing when we are within some section.
 */
void DXFFile::
state_section() {
  string tail;

  switch (_code) {
  case 0:
    if (_string == "ENDSEC") {
      change_state(ST_top);
    } else {
      if (_section == SE_entities) {
        if (_string == "3DFACE") {
          change_entity(EN_3dface);
        } else if (_string == "POINT") {
          change_entity(EN_point);
        } else if (_string == "INSERT") {
          change_entity(EN_insert);
        } else if (_string == "VERTEX") {
          change_entity(EN_vertex);
        } else if (_string == "POLYLINE") {
          change_entity(EN_polyline);
        } else {
          change_entity(EN_unknown);
        }
      }
    }
    break;

  case 8:
    change_layer(_string);
    break;

  case 62:  // Color.
    _color_index = string_to_int(_string, tail);
    break;

  default:
    break;
  }
}


/**
 * Does the DXF processing when we are reading an entity.
 */
void DXFFile::
state_entity() {
  string tail;

  switch (_code) {
  case 0:
    state_section();
    break;

  case 8:
    change_layer(_string);
    break;

  case 10:
    _p[0] = string_to_double(_string, tail);
    break;

  case 11:
    _q[0] = string_to_double(_string, tail);
    break;

  case 12:
    _r[0] = string_to_double(_string, tail);
    break;

  case 13:
    _s[0] = string_to_double(_string, tail);
    break;

  case 20:
    _p[1] = string_to_double(_string, tail);
    break;

  case 21:
    _q[1] = string_to_double(_string, tail);
    break;

  case 22:
    _r[1] = string_to_double(_string, tail);
    break;

  case 23:
    _s[1] = string_to_double(_string, tail);
    break;

  case 30:
    _p[2] = string_to_double(_string, tail);
    break;

  case 31:
    _q[2] = string_to_double(_string, tail);
    break;

  case 32:
    _r[2] = string_to_double(_string, tail);
    break;

  case 33:
    _s[2] = string_to_double(_string, tail);
    break;

  case 62:  // Color.
    _color_index = string_to_int(_string, tail);
    break;

  case 66:  // Vertices-follow.
    _vertices_follow = (string_to_int(_string, tail) != 0);
    break;

  case 70:  // Polyline flags.
    _flags = string_to_int(_string, tail);
    break;

  case 210:
    _z[0] = string_to_double(_string, tail);
    break;

  case 220:
    _z[1] = string_to_double(_string, tail);
    break;

  case 230:
    _z[2] = string_to_double(_string, tail);
    break;

  default:
    break;
  }
}


/**
 * Does the DXF processing when we are reading the list of vertices that might
 * follow an entity.
 */
void DXFFile::
state_verts() {
  string tail;

  switch (_code) {
  case 0:
    state_section();
    break;

  case 8:
    change_layer(_string);
    break;

  case 10:
    _p[0] = string_to_double(_string, tail);
    break;

  case 20:
    _p[1] = string_to_double(_string, tail);
    break;

  case 30:
    _p[2] = string_to_double(_string, tail);
    break;

  default:
    break;
  }
}


ostream &operator << (ostream &out, const DXFFile::State &state) {
  switch (state) {
  case DXFFile::ST_top:
    return out << "ST_top";
  case DXFFile::ST_section:
    return out << "ST_section";
  case DXFFile::ST_entity:
    return out << "ST_entity";
  case DXFFile::ST_verts:
    return out << "ST_verts";
  case DXFFile::ST_error:
    return out << "ST_error";
  case DXFFile::ST_done:
    return out << "ST_done";
  }
  return out << "Unknown state";
}

ostream &operator << (ostream &out, const DXFFile::Section &section) {
  switch (section) {
  case DXFFile::SE_unknown:
    return out << "SE_unknown";
  case DXFFile::SE_header:
    return out << "SE_header";
  case DXFFile::SE_tables:
    return out << "SE_tables";
  case DXFFile::SE_blocks:
    return out << "SE_blocks";
  case DXFFile::SE_entities:
    return out << "SE_entities";
  case DXFFile::SE_objects:
    return out << "SE_objects";
  }
  return out << "Unknown section";
}

ostream &operator << (ostream &out, const DXFFile::Entity &entity) {
  switch (entity) {
  case DXFFile::EN_unknown:
    return out << "EN_unknown";
  case DXFFile::EN_3dface:
    return out << "EN_3dface";
  case DXFFile::EN_point:
    return out << "EN_point";
  case DXFFile::EN_insert:
    return out << "EN_insert";
  case DXFFile::EN_vertex:
    return out << "EN_vertex";
  case DXFFile::EN_polyline:
    return out << "EN_polyline";
  }
  return out << "Unknown entity";
}
