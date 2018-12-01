/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileMaterial.cxx
 * @author drose
 * @date 2001-06-19
 */

#include "xFileMaterial.h"
#include "xFileToEggConverter.h"
#include "xFileDataNode.h"
#include "eggMaterial.h"
#include "eggTexture.h"
#include "eggPrimitive.h"
#include "datagram.h"
#include "config_xfile.h"

#include <string.h>  // for strcmp, strdup

/**
 *
 */
XFileMaterial::
XFileMaterial() {
  _face_color.set(1.0, 1.0, 1.0, 1.0);
  _specular_color.set(0.0, 0.0, 0.0);
  _emissive_color.set(0.0, 0.0, 0.0);
  _power = 64.0;

  _has_material = false;
  _has_texture = false;
}

/**
 *
 */
XFileMaterial::
~XFileMaterial() {
}

/**
 * Sets the structure up from the indicated egg data.
 */
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
      const LColor &spec = egg_mat->get_spec();
      _specular_color.set(spec[0], spec[1], spec[2]);
    }
    if (egg_mat->has_emit()) {
      const LColor &emit = egg_mat->get_emit();
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

/**
 * Applies the properties in the material to the indicated egg primitive.
 */
void XFileMaterial::
apply_to_egg(EggPrimitive *egg_prim, XFileToEggConverter *converter) {
  // Is there a texture?
  if (_has_texture) {
    Filename texture = converter->convert_model_path(_texture);
    EggTexture temp("", texture);
    EggTexture *egg_tex = converter->create_unique_texture(temp);
    egg_prim->set_texture(egg_tex);
  }

  // Are there any fancy lighting effects?
  bool got_spec = (_specular_color != LRGBColor::zero());
  bool got_emit = (_emissive_color != LRGBColor::zero());
  if (got_spec || got_emit) {
    EggMaterial temp("");
    temp.set_diff(_face_color);
    if (got_spec) {
      temp.set_shininess(_power);
      temp.set_spec(LColor(_specular_color[0], _specular_color[1],
                           _specular_color[2], 1.0));
    }
    if (got_emit) {
      temp.set_emit(LColor(_emissive_color[0], _emissive_color[1],
                           _emissive_color[2], 1.0));
    }
    EggMaterial *egg_mat = converter->create_unique_material(temp);
    egg_prim->set_material(egg_mat);
  }

  // Also apply the face color.
  egg_prim->set_color(_face_color);
}

/**
 *
 */
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

/**
 * Returns true if this material represents something meaningful, or false if
 * the default material is sufficient.
 */
bool XFileMaterial::
has_material() const {
  return _has_material;
}

/**
 * Returns true if this material includes a texture map, false otherwise.
 */
bool XFileMaterial::
has_texture() const {
  return _has_texture;
}

/**
 * Creates a Material object for the material list.
 */
XFileDataNode *XFileMaterial::
make_x_material(XFileNode *x_meshMaterials, const std::string &suffix) {
  XFileDataNode *x_material =
    x_meshMaterials->add_Material("material" + suffix,
                                  _face_color, _power,
                                  _specular_color, _emissive_color);

  if (has_texture()) {
    x_material->add_TextureFilename("texture" + suffix, _texture);
  }

  return x_material;
}

/**
 * Fills the structure based on the raw data from the X file's Material
 * object.
 */
bool XFileMaterial::
fill_material(XFileDataNode *obj) {
  _face_color = LCAST(PN_stdfloat, (*obj)["faceColor"].vec4());
  _power = (*obj)["power"].d();
  _specular_color = LCAST(PN_stdfloat, (*obj)["specularColor"].vec3());
  _emissive_color = LCAST(PN_stdfloat, (*obj)["emissiveColor"].vec3());
  _has_material = true;

  // Walk through the children of the material.  If there are any, there
  // should be only one, and it should be just a Texture.
  int num_objects = obj->get_num_objects();
  for (int i = 0; i < num_objects; i++) {
    XFileDataNode *child = obj->get_object(i);
    if (child->is_standard_object("TextureFilename")) {
      _texture = Filename::from_os_specific((*child)["filename"].s());
      _has_texture = true;

    } else {
      if (xfile_cat.is_debug()) {
        xfile_cat.debug()
          << "Ignoring material object of unknown type: "
          << child->get_template_name() << "\n";
      }
    }
  }

  return true;
}
