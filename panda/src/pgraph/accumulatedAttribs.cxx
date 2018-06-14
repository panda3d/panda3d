/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file accumulatedAttribs.cxx
 * @author drose
 * @date 2003-01-30
 */

#include "accumulatedAttribs.h"
#include "sceneGraphReducer.h"
#include "pandaNode.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "texMatrixAttrib.h"
#include "textureAttrib.h"
#include "clipPlaneAttrib.h"
#include "cullFaceAttrib.h"
#include "config_pgraph.h"


/**
 *
 */
AccumulatedAttribs::
AccumulatedAttribs() {
  _transform = TransformState::make_identity();
  _color_override = 0;
  _color_scale_override = 0;
  _tex_matrix_override = 0;
  _texture_override = 0;
  _clip_plane_override = 0;
  _cull_face_override = 0;
  _other = RenderState::make_empty();
}

/**
 *
 */
AccumulatedAttribs::
AccumulatedAttribs(const AccumulatedAttribs &copy) :
  _transform(copy._transform),
  _color(copy._color),
  _color_override(copy._color_override),
  _color_scale(copy._color_scale),
  _color_scale_override(copy._color_scale_override),
  _tex_matrix(copy._tex_matrix),
  _tex_matrix_override(copy._tex_matrix_override),
  _texture(copy._texture),
  _texture_override(copy._texture_override),
  _clip_plane(copy._clip_plane),
  _clip_plane_override(copy._clip_plane_override),
  _cull_face(copy._cull_face),
  _cull_face_override(copy._cull_face_override),
  _other(copy._other)
{
}

/**
 *
 */
void AccumulatedAttribs::
operator = (const AccumulatedAttribs &copy) {
  _transform = copy._transform;
  _color = copy._color;
  _color_override = copy._color_override;
  _color_scale = copy._color_scale;
  _color_scale_override = copy._color_scale_override;
  _tex_matrix = copy._tex_matrix;
  _tex_matrix_override = copy._tex_matrix_override;
  _texture = copy._texture;
  _texture_override = copy._texture_override;
  _clip_plane = copy._clip_plane;
  _clip_plane_override = copy._clip_plane_override;
  _cull_face = copy._cull_face;
  _cull_face_override = copy._cull_face_override;
  _other = copy._other;
}

/**
 *
 */
void AccumulatedAttribs::
write(std::ostream &out, int attrib_types, int indent_level) const {
  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    _transform->write(out, indent_level);
  }
  if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
    if (_color == nullptr) {
      indent(out, indent_level) << "no color\n";
    } else {
      _color->write(out, indent_level);
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
    if (_color_scale == nullptr) {
      indent(out, indent_level) << "no color scale\n";
    } else {
      _color_scale->write(out, indent_level);
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_tex_matrix) != 0) {
    if (_tex_matrix == nullptr) {
      indent(out, indent_level) << "no tex matrix\n";
    } else {
      _tex_matrix->write(out, indent_level);
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_clip_plane) != 0) {
    if (_clip_plane == nullptr) {
      indent(out, indent_level) << "no clip plane\n";
    } else {
      _clip_plane->write(out, indent_level);
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_cull_face) != 0) {
    if (_cull_face == nullptr) {
      indent(out, indent_level) << "no cull face\n";
    } else {
      _cull_face->write(out, indent_level);
    }
  }
  if ((attrib_types & SceneGraphReducer::TT_other) != 0) {
    _other->write(out, indent_level);
  }
}

/**
 * Collects the state and transform from the indicated node and adds it to the
 * accumulator, removing it from the node.
 */
void AccumulatedAttribs::
collect(PandaNode *node, int attrib_types) {
  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    // Collect the node's transform.
    nassertv(_transform != nullptr);
    _transform = _transform->compose(node->get_transform());
    node->set_transform(TransformState::make_identity());
    node->set_prev_transform(TransformState::make_identity());
  }

  CPT(RenderState) new_state = collect(node->get_state(), attrib_types);
  node->set_state(new_state);
}

/**
 * Collects the state and transform from the indicated node and adds it to the
 * accumulator, removing it from the state (and returning a new state).
 */
CPT(RenderState) AccumulatedAttribs::
collect(const RenderState *state, int attrib_types) {
  CPT(RenderState) new_state = state;

  if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
    const RenderAttrib *node_attrib =
      new_state->get_attrib(ColorAttrib::get_class_slot());
    if (node_attrib != nullptr) {
      int color_override = new_state->get_override(ColorAttrib::get_class_slot());
      if (color_override >= _color_override ||
          _color == nullptr) {
        // The node has a color attribute; apply it.
        if (_color == nullptr) {
          _color = node_attrib;
        } else {
          _color = _color->compose(node_attrib);
        }
        _color_override = color_override;
      }
      new_state = new_state->remove_attrib(ColorAttrib::get_class_slot());
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
    const RenderAttrib *node_attrib =
      new_state->get_attrib(ColorScaleAttrib::get_class_slot());
    if (node_attrib != nullptr) {
      int color_scale_override = new_state->get_override(ColorScaleAttrib::get_class_slot());
      if (color_scale_override >= _color_scale_override ||
          _color_scale == nullptr) {
        if (_color_scale == nullptr) {
          _color_scale = node_attrib;
        } else {
          _color_scale = _color_scale->compose(node_attrib);
        }
        _color_scale_override = color_scale_override;
      }
      new_state = new_state->remove_attrib(ColorScaleAttrib::get_class_slot());
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_tex_matrix) != 0) {
    const RenderAttrib *node_attrib =
      new_state->get_attrib(TexMatrixAttrib::get_class_slot());
    if (node_attrib != nullptr) {
      int tex_matrix_override = new_state->get_override(TexMatrixAttrib::get_class_slot());
      if (tex_matrix_override >= _tex_matrix_override ||
          _tex_matrix == nullptr) {
        if (_tex_matrix == nullptr) {
          _tex_matrix = node_attrib;
        } else {
          _tex_matrix = _tex_matrix->compose(node_attrib);
        }
        _tex_matrix_override = tex_matrix_override;
      }
      new_state = new_state->remove_attrib(TexMatrixAttrib::get_class_slot());
    }

    // We also need to accumulate the texture state if we are accumulating
    // texture matrix.
    const RenderAttrib *tex_attrib =
      new_state->get_attrib(TextureAttrib::get_class_slot());
    if (tex_attrib != nullptr) {
      int texture_override = new_state->get_override(TextureAttrib::get_class_slot());
      if (texture_override >= _texture_override ||
          _texture == nullptr) {
        if (_texture == nullptr) {
          _texture = tex_attrib;
        } else {
          _texture = _texture->compose(tex_attrib);
        }
        _texture_override = texture_override;
      }

      // However, we don't remove the texture state from the node.  We're just
      // accumulating it so we can tell which texture coordinates are safe to
      // flatten.
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_clip_plane) != 0) {
    const RenderAttrib *node_attrib =
      new_state->get_attrib(ClipPlaneAttrib::get_class_slot());
    if (node_attrib != nullptr) {
      int clip_plane_override = new_state->get_override(ClipPlaneAttrib::get_class_slot());
      if (clip_plane_override >= _clip_plane_override ||
          _clip_plane == nullptr) {
        if (_clip_plane == nullptr) {
          _clip_plane = node_attrib;
        } else {
          _clip_plane = _clip_plane->compose(node_attrib);
        }
        _clip_plane_override = clip_plane_override;
      }
      new_state = new_state->remove_attrib(ClipPlaneAttrib::get_class_slot());
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_cull_face) != 0) {
    const RenderAttrib *node_attrib =
      new_state->get_attrib(CullFaceAttrib::get_class_slot());
    if (node_attrib != nullptr) {
      int cull_face_override = new_state->get_override(CullFaceAttrib::get_class_slot());
      if (cull_face_override >= _cull_face_override ||
          _cull_face == nullptr) {
        if (_cull_face == nullptr) {
          _cull_face = node_attrib;
        } else {
          _cull_face = _cull_face->compose(node_attrib);
        }
        _cull_face_override = cull_face_override;
      }
      new_state = new_state->remove_attrib(CullFaceAttrib::get_class_slot());
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_other) != 0) {
    // Collect everything else.
    nassertr(_other != nullptr, new_state);
    _other = _other->compose(new_state);
    new_state = RenderState::make_empty();
  }

  return new_state;
}

/**
 * Stores the indicated attributes in the node's transform and state
 * information; does not attempt to apply the properties to the vertices.
 * Clears the attributes from the accumulator for future traversals.
 */
void AccumulatedAttribs::
apply_to_node(PandaNode *node, int attrib_types) {
  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    node->set_transform(_transform->compose(node->get_transform())->get_unique());
    node->reset_prev_transform();
    _transform = TransformState::make_identity();
  }

  if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
    if (_color != nullptr) {
      const RenderAttrib *node_attrib =
        node->get_attrib(ColorAttrib::get_class_slot());
      if (node_attrib != nullptr) {
        node->set_attrib(_color->compose(node_attrib)->get_unique());
      } else {
        node->set_attrib(_color->get_unique());
      }
      _color = nullptr;
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
    if (_color_scale != nullptr) {
      const RenderAttrib *node_attrib =
        node->get_attrib(ColorScaleAttrib::get_class_slot());
      if (node_attrib != nullptr) {
        node->set_attrib(_color_scale->compose(node_attrib)->get_unique());
      } else {
        node->set_attrib(_color_scale->get_unique());
      }
      _color_scale = nullptr;
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_tex_matrix) != 0) {
    if (_tex_matrix != nullptr) {
      const RenderAttrib *node_attrib =
        node->get_attrib(TexMatrixAttrib::get_class_slot());
      if (node_attrib != nullptr) {
        node->set_attrib(_tex_matrix->compose(node_attrib)->get_unique());
      } else {
        node->set_attrib(_tex_matrix->get_unique());
      }
      _tex_matrix = nullptr;
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_clip_plane) != 0) {
    if (_clip_plane != nullptr) {
      const RenderAttrib *node_attrib =
        node->get_attrib(ClipPlaneAttrib::get_class_slot());
      if (node_attrib != nullptr) {
        node->set_attrib(_clip_plane->compose(node_attrib)->get_unique());
      } else {
        node->set_attrib(_clip_plane->get_unique());
      }
      _clip_plane = nullptr;
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_cull_face) != 0) {
    if (_cull_face != nullptr) {
      const RenderAttrib *node_attrib =
        node->get_attrib(CullFaceAttrib::get_class_slot());
      if (node_attrib != nullptr) {
        node->set_attrib(_cull_face->compose(node_attrib)->get_unique());
      } else {
        node->set_attrib(_cull_face->get_unique());
      }
      _cull_face = nullptr;
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_other) != 0) {
    node->set_state(_other->compose(node->get_state())->get_unique());
    _other = RenderState::make_empty();
  }
}
