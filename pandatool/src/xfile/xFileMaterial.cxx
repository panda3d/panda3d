// Filename: xFileMaterial.cxx
// Created by:  drose (19Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "xFileMaterial.h"
#include "xFileToEggConverter.h"

#include "eggMaterial.h"
#include "eggTexture.h"
#include "eggPrimitive.h"
#include "datagram.h"

#include <string.h>  // for strcmp, strdup

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMaterial::
XFileMaterial() {
  _face_color.set(1.0, 1.0, 1.0, 1.0);
  _specular_color.set(0.0, 0.0, 0.0);
  _emissive_color.set(0.0, 0.0, 0.0);
  _power = 64.0;

  _has_material = false;
  _has_texture = false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMaterial::
~XFileMaterial() {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::set_from_egg
//       Access: Public
//  Description: Sets the structure up from the indicated egg data.
////////////////////////////////////////////////////////////////////
void XFileMaterial::
set_from_egg(EggPrimitive *egg_prim) {
  // First, determine the face color.
  if (egg_prim->has_color()) {
    _face_color = egg_prim->get_color();
    _has_material = true;
  }

  // Do we have an actual EggMaterial, to control lighting effects?
  if (egg_prim->has_material()) {
    _has_material = true;
    EggMaterial *egg_mat = egg_prim->get_material();
    if (egg_mat->has_diff()) {
      _face_color = egg_mat->get_diff();
    }
    if (egg_mat->has_spec()) {
      const Colorf &spec = egg_mat->get_spec();
      _specular_color.set(spec[0], spec[1], spec[2]);
    }
    if (egg_mat->has_emit()) {
      const Colorf &emit = egg_mat->get_emit();
      _emissive_color.set(emit[0], emit[1], emit[2]);
    }
    if (egg_mat->has_shininess()) {
      _power = egg_mat->get_shininess();
    }
  }

  // Finally, if we also have a texture, record that.
  if (egg_prim->has_texture()) {
    _has_material = true;
    _has_texture = true;
    EggTexture *egg_tex = egg_prim->get_texture();
    _texture = egg_tex->get_filename();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::apply_to_egg
//       Access: Public
//  Description: Applies the properties in the material to the
//               indicated egg primitive.
////////////////////////////////////////////////////////////////////
void XFileMaterial::
apply_to_egg(EggPrimitive *egg_prim, XFileToEggConverter *converter) {
  // Is there a texture?
  if (_has_texture) {
    Filename texture = converter->convert_texture_path(_texture);
    EggTexture temp("", texture);
    EggTexture *egg_tex = converter->create_unique_texture(temp);
    egg_prim->set_texture(egg_tex);
  }

  // Are there any fancy lighting effects?
  bool got_spec = (_specular_color != RGBColorf::zero());
  bool got_emit = (_emissive_color != RGBColorf::zero());
  if (got_spec || got_emit) {
    EggMaterial temp("");
    temp.set_diff(_face_color);
    if (got_spec) {
      temp.set_shininess(_power);
      temp.set_spec(Colorf(_specular_color[0], _specular_color[1],
                           _specular_color[2], 1.0));
    }
    if (got_emit) {
      temp.set_emit(Colorf(_emissive_color[0], _emissive_color[1],
                           _emissive_color[2], 1.0));
    }
    EggMaterial *egg_mat = converter->create_unique_material(temp);
    egg_prim->set_material(egg_mat);
  }

  // Also apply the face color.
  egg_prim->set_color(_face_color);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int XFileMaterial::
compare_to(const XFileMaterial &other) const {
  int ct;
  ct = _face_color.compare_to(other._face_color);
  if (ct == 0) {
    ct = (_power == other._power) ? 0 : ((_power < other._power) ? -1 : 1);
  }
  if (ct == 0) {
    ct = _specular_color.compare_to(other._specular_color);
  }
  if (ct == 0) {
    ct = _emissive_color.compare_to(other._emissive_color);
  }
  if (ct == 0) {
    ct = strcmp(_texture.c_str(), other._texture.c_str());
  }
  return ct;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::has_material
//       Access: Public
//  Description: Returns true if this material represents something
//               meaningful, or false if the default material is
//               sufficient.
////////////////////////////////////////////////////////////////////
bool XFileMaterial::
has_material() const {
  return _has_material;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::has_texture
//       Access: Public
//  Description: Returns true if this material includes a texture map,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMaterial::
has_texture() const {
  return _has_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::make_material_data
//       Access: Public
//  Description: Fills the datagram with the raw data for the DX
//               Material template.
////////////////////////////////////////////////////////////////////
void XFileMaterial::
make_material_data(Datagram &raw_data) {
  raw_data.clear();
  raw_data.add_float32(_face_color[0]);
  raw_data.add_float32(_face_color[1]);
  raw_data.add_float32(_face_color[2]);
  raw_data.add_float32(_face_color[3]);
  raw_data.add_float32(_power);
  raw_data.add_float32(_specular_color[0]);
  raw_data.add_float32(_specular_color[1]);
  raw_data.add_float32(_specular_color[2]);
  raw_data.add_float32(_emissive_color[0]);
  raw_data.add_float32(_emissive_color[1]);
  raw_data.add_float32(_emissive_color[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::make_texture_data
//       Access: Public
//  Description: Fills the datagram with the raw data for the DX
//               TextureFilename template.
////////////////////////////////////////////////////////////////////
void XFileMaterial::
make_texture_data(Datagram &raw_data) {
  raw_data.clear();

  // Convert the filename to an appropriate form for the X file.
  string os_filename = _texture.to_os_specific();
  // Now we have to double up the backslashes.
  string filename;
  for (string::const_iterator pi = os_filename.begin();
       pi != os_filename.end();
       ++pi) {
    if ((*pi) == '\\') {
      filename += '\\';
      filename += '\\';
    } else {
      filename += (*pi);
    }
  }

  // Get a char * pointer to the texture filename, to pass into the
  // Microsoft DX file interface.  Unfortunately, we can't delete this
  // again, since it needs to live longer than the life of the
  // XFileMaterial object itself, so this becomes a memory leak
  // (unless the DX file interface frees it, but the documentation is
  // far from clear).  Too bad.
  char *filename_str = strdup(filename.c_str());

  // The Microsoft convention is to stuff a pointer into a four-byte
  // field.  Not terribly portable, but that's the interface.
  raw_data.add_int32((int)filename_str);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::read_material_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               Material template.
////////////////////////////////////////////////////////////////////
bool XFileMaterial::
read_material_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  _face_color[0] = di.get_float32();
  _face_color[1] = di.get_float32();
  _face_color[2] = di.get_float32();
  _face_color[3] = di.get_float32();
  _power = di.get_float32();
  _specular_color[0] = di.get_float32();
  _specular_color[1] = di.get_float32();
  _specular_color[2] = di.get_float32();
  _emissive_color[0] = di.get_float32();
  _emissive_color[1] = di.get_float32();
  _emissive_color[2] = di.get_float32();
  _has_material = true;

  if (di.get_remaining_size() != 0) {
    nout << "Ignoring " << di.get_remaining_size()
         << " trailing MeshMaterial.\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMaterial::read_texture_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               TextureFilename template.
////////////////////////////////////////////////////////////////////
bool XFileMaterial::
read_texture_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  // The Microsoft convention is to stuff a pointer into a four-byte
  // field.  Not terribly portable, but that's the interface.
  const char *ptr = (const char *)di.get_int32();
  _texture = Filename::from_os_specific(ptr);
  _has_texture = true;

  if (di.get_remaining_size() != 0) {
    nout << "Ignoring " << di.get_remaining_size()
         << " trailing MeshMaterial.\n";
  }
  return true;
}
