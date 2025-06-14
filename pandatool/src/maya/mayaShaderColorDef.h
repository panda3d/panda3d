/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaShaderColorDef.h
 * @author drose
 * @date 2003-04-12
 */

#ifndef MAYASHADERCOLORDEF_H
#define MAYASHADERCOLORDEF_H

#include "pandatoolbase.h"

#include "luse.h"
#include "lmatrix.h"
#include "pmap.h"
#include "pvector.h"

class MayaShader;
class MayaShaderColorDef;
typedef pvector<MayaShaderColorDef *> MayaShaderColorList;
typedef pmap<std::string, std::string> MayaFileToUVSetMap;

/**
 * This defines the various attributes that Maya may associate with the
 * "color" channel for a particular shader (as well as on the "transparency"
 * channel).
 */
class MayaShaderColorDef {
public:
  MayaShaderColorDef();
  MayaShaderColorDef (MayaShaderColorDef&);
  ~MayaShaderColorDef();

  std::string strip_prefix(std::string full_name);

  LMatrix3d compute_texture_matrix() const;
  bool has_projection() const;
  LTexCoordd project_uv(const LPoint3d &pos, const LPoint3d &ref_point) const;
  bool reset_maya_texture(const Filename &texture);

  void write(std::ostream &out) const;

  enum BlendType {
    BT_unspecified,
    BT_modulate,
    BT_decal,
    BT_blend,
    BT_replace,
    BT_add,
    BT_blend_color_scale,
    BT_modulate_glow,
    BT_modulate_gloss,
    BT_normal,
    BT_normal_height,
    BT_gloss,
    BT_glow,
    BT_height,
    BT_selector,
  };

  enum ProjectionType {
    PT_off,
    PT_planar,
    PT_spherical,
    PT_cylindrical,
    PT_ball,
    PT_cubic,
    PT_triplanar,
    PT_concentric,
    PT_perspective,
  };

  BlendType _blend_type;
  ProjectionType _projection_type;
  LMatrix4d _projection_matrix;
  double _u_angle;
  double _v_angle;

  Filename _texture_filename;
  std::string _texture_name;
  LColor _color_gain;

  LVector2 _coverage;
  LVector2 _translate_frame;
  double _rotate_frame;

  bool _mirror;
  bool _stagger;
  bool _wrap_u;
  bool _wrap_v;

  LVector2 _repeat_uv;
  LVector2 _offset;
  double _rotate_uv;

  bool _is_alpha;

  std::string _uvset_name;
  MayaShaderColorDef *_opposite;

  std::string get_panda_uvset_name();

private:
  MObject *_color_object;

private:
  static void find_textures_modern(const std::string &shadername, MayaShaderColorList &list, MPlug inplug, bool is_alpha);
  void find_textures_legacy(MayaShader *shader, MObject color, bool trans=false);

  void set_projection_type(const std::string &type);

  LPoint2d map_planar(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_spherical(const LPoint3d &pos, const LPoint3d &centroid) const;
  LPoint2d map_cylindrical(const LPoint3d &pos, const LPoint3d &centroid) const;

  // Define a pointer to one of the above member functions.
  LPoint2d (MayaShaderColorDef::*_map_uvs)(const LPoint3d &pos, const LPoint3d &centroid) const;

  friend class MayaShader;


/*
 * Legacy Fields - these fields are only used by the legacy codepath.  These
 * fields are deprecated for the following reasons: * has_texture is redundant
 * --- if there's no texture, just don't allocate a MayaShaderColorDef.  *
 * has_flat_color and flat_color don't belong here, they belong in the shader.
 * * has_alpha_channel is not needed - there are better ways to determine if a
 * texture stage involves an alpha channel.  * keep_color, keep_alpha, and
 * interpolate are all adjuncts to blend_mode - it would make more sense just
 * to add some more blend_modes.
 */

public:
  bool     _has_texture;       // deprecated, see above.
  bool     _has_flat_color;    // deprecated, see above.
  LColord   _flat_color;        // deprecated, see above.
  bool     _has_alpha_channel; // deprecated, see above.
  bool     _keep_color;        // deprecated, see above.
  bool     _keep_alpha;        // deprecated, see above.
  bool     _interpolate;       // deprecated, see above.

};

#endif
