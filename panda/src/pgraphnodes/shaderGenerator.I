/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderGenerator.I
 * @author jyelon
 * @date 2007-12-15
 */

/**
 * Returns the alpha test mode subsumed by this shader.
 */
INLINE RenderAttrib::PandaCompareFunc ShaderGenerator::ShaderKey::
get_alpha_test_mode() const {
  return (RenderAttrib::PandaCompareFunc)
    ((_flags & ShaderKey::F_ALPHA_TEST_MASK) >> ShaderKey::F_ALPHA_TEST_SHIFT);
}

/**
 * Returns true if the flat or vertex color participates in the color
 * calculation, rather than being subsumed by the material colors.
 */
INLINE bool ShaderGenerator::ShaderKey::
needs_color() const {
  if ((_flags & (F_flat_color | F_vertex_color)) == 0) {
    return false;
  }
  if ((_flags & F_lighting) == 0) {
    return true;
  }
  return ((_flags & Material::F_ambient) == 0 &&
          (_flags & F_have_separate_ambient) != 0) ||
         (_flags & Material::F_diffuse) == 0 ||
         (_flags & F_calc_primary_alpha) != 0;
}

/**
 * Returns true if a specular contribution is calculated, which is the case
 * when lighting with either a specular material color or a gloss map.
 */
INLINE bool ShaderGenerator::ShaderKey::
has_specular() const {
  return (_flags & F_lighting) != 0 &&
         ((_flags & Material::F_specular) != 0 ||
          (_texture_flags & TF_map_gloss) != 0);
}

/**
 * Returns true if any texture stage generates its texture coordinates from
 * the point sprite coordinates.
 */
INLINE bool ShaderGenerator::ShaderKey::
needs_point_coord() const {
  for (const TextureInfo &tex : _textures) {
    if (tex._gen_mode == TexGenAttrib::M_point_sprite) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if any texture stage generates its texture coordinates from
 * the view-space reflection vector.
 */
INLINE bool ShaderGenerator::ShaderKey::
needs_eye_reflection() const {
  for (const TextureInfo &tex : _textures) {
    if (tex._gen_mode == TexGenAttrib::M_eye_sphere_map ||
        tex._gen_mode == TexGenAttrib::M_eye_cube_map ||
        tex._gen_mode == TexGenAttrib::M_world_cube_map) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if any texture stage requires the reflection vector to be
 * transformed into world space.
 */
INLINE bool ShaderGenerator::ShaderKey::
needs_view_to_world() const {
  for (const TextureInfo &tex : _textures) {
    if (tex._gen_mode == TexGenAttrib::M_world_cube_map) {
      return true;
    }
  }
  return false;
}

/**
 * Returns the index of the last texture stage with the given TF_map_* flag,
 * or -1 if there is none.
 */
INLINE int ShaderGenerator::ShaderKey::
find_map_index(int flag) const {
  int index = -1;
  for (size_t i = 0; i < _textures.size(); ++i) {
    if (_textures[i]._flags & flag) {
      index = (int)i;
    }
  }
  return index;
}

/**
 * Returns true if the clip-space vertex position is passed along to the
 * fragment shader, which needs it to apply fog.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_hpos() const {
  return _hpos_location >= 0;
}

/**
 * Returns true if the world-space vertex position is passed along to the
 * fragment shader, for clip planes or texture coordinate generation.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_world_position() const {
  return _world_position_location >= 0;
}

/**
 * Returns true if the world-space normal is passed along to the fragment
 * shader, for texture coordinate generation.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_world_normal() const {
  return _world_normal_location >= 0;
}

/**
 * Returns true if the view-space vertex position is passed along to the
 * fragment shader, for lighting or texture coordinate generation.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_eye_position() const {
  return _eye_position_location >= 0;
}

/**
 * Returns true if the view-space normal is passed along to the fragment
 * shader, for lighting or texture coordinate generation.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_eye_normal() const {
  return _eye_normal_location >= 0;
}

/**
 * Returns true if the view-space tangent and binormal are passed along to
 * the fragment shader, which needs them to apply normal maps.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_tangents() const {
  return _tangent_location >= 0;
}

/**
 * Returns true if the vertex color is passed along to the fragment shader.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_color() const {
  return _color_location >= 0;
}

/**
 * Returns true if the tangent-space eye vector is passed along to the
 * fragment shader, which needs it to apply parallax mapping.
 */
INLINE bool ShaderGenerator::ShaderPlan::
has_eyevec() const {
  return _eyevec_location >= 0;
}

/**
 * Returns true if the point sprite coordinates are calculated by the shaders
 * because the driver does not provide them, in which case the vertex shader
 * passes along the data needed to calculate them.
 */
INLINE bool ShaderGenerator::ShaderPlan::
calc_point_coord() const {
  return _pointcoord_location >= 0;
}
