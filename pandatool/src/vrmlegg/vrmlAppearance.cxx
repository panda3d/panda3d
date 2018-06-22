/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlAppearance.cxx
 * @author drose
 * @date 1999-06-24
 */

#include "vrmlAppearance.h"
#include "vrmlNode.h"
#include "deg_2_rad.h"

VRMLAppearance::
VRMLAppearance(const VrmlNode *appearance) {
  _has_material = false;
  _transparency = 0.0;
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _has_tex_transform = false;

  if (appearance != nullptr) {
    const VrmlNode *material = appearance->get_value("material")._sfnode._p;
    if (material != nullptr) {
      _has_material = true;
      const double *c = material->get_value("diffuseColor")._sfvec;
      _transparency = material->get_value("transparency")._sffloat;
      _color.set(c[0], c[1], c[2], 1.0 - _transparency);
    }

    const VrmlNode *tex_transform = appearance->get_value("textureTransform")._sfnode._p;
    if (tex_transform != nullptr) {
      if (strcmp(tex_transform->_type->getName(), "TextureTransform") == 0) {
        _has_tex_transform = true;
        const double *c = tex_transform->get_value("center")._sfvec;
        _tex_center.set(c[0], c[1]);
        _tex_rotation = tex_transform->get_value("rotation")._sffloat;
        const double *s = tex_transform->get_value("scale")._sfvec;
        _tex_scale.set(s[0], s[1]);
        const double *t = tex_transform->get_value("translation")._sfvec;
        _tex_translation.set(t[0], t[1]);
      }
    }

    const VrmlNode *texture = appearance->get_value("texture")._sfnode._p;
    if (texture != nullptr) {
      if (strcmp(texture->_type->getName(), "ImageTexture") == 0) {
        MFArray *url = texture->get_value("url")._mf;
        if (!url->empty()) {
          const char *filename = (*url->begin())._sfstring;
          _tex = new EggTexture("tref", filename);

          if (_has_tex_transform) {
            _tex->add_translate2d(-_tex_center);
            _tex->add_scale2d(_tex_scale);
            _tex->add_rotate2d(rad_2_deg(_tex_rotation));
            _tex->add_translate2d(_tex_center);
            _tex->add_translate2d(_tex_translation);
          }
        }
      }
    }
  }
}
