/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTransformer.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "geomTransformer.h"
#include "sceneGraphReducer.h"
#include "geomNode.h"
#include "geom.h"
#include "geomVertexRewriter.h"
#include "renderState.h"
#include "transformTable.h"
#include "transformBlendTable.h"
#include "sliderTable.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "vector_int.h"
#include "userVertexTransform.h"
#include "geomMunger.h"
#include "texture.h"
#include "texturePeeker.h"
#include "textureAttrib.h"
#include "colorAttrib.h"
#include "config_pgraph.h"

PStatCollector GeomTransformer::_apply_vertex_collector("*:Flatten:apply:vertex");
PStatCollector GeomTransformer::_apply_texcoord_collector("*:Flatten:apply:texcoord");
PStatCollector GeomTransformer::_apply_set_color_collector("*:Flatten:apply:set color");
PStatCollector GeomTransformer::_apply_scale_color_collector("*:Flatten:apply:scale color");
PStatCollector GeomTransformer::_apply_texture_color_collector("*:Flatten:apply:texture color");
PStatCollector GeomTransformer::_apply_set_format_collector("*:Flatten:apply:set format");

TypeHandle GeomTransformer::NewCollectedData::_type_handle;

/**
 *
 */
GeomTransformer::
GeomTransformer() :
  // The default value here comes from the Config file.
  _max_collect_vertices(max_collect_vertices)
{
}

/**
 *
 */
GeomTransformer::
GeomTransformer(const GeomTransformer &copy) :
  _max_collect_vertices(copy._max_collect_vertices)
{
}

/**
 *
 */
GeomTransformer::
~GeomTransformer() {
  finish_collect(false);
}

/**
 * Records the association of the Geom with its GeomVertexData, for the
 * purpose of later removing unused vertices.
 */
void GeomTransformer::
register_vertices(Geom *geom, bool might_have_unused) {
  VertexDataAssoc &assoc = _vdata_assoc[geom->get_vertex_data()];
  assoc._geoms.push_back(geom);
  if (might_have_unused) {
    assoc._might_have_unused = true;
  }
}

/**
 * Records the association of the Geom with its GeomVertexData, for the
 * purpose of later removing unused vertices.
 */
void GeomTransformer::
register_vertices(GeomNode *node, bool might_have_unused) {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(node->_cycler, current_thread) {
    GeomNode::CDStageWriter cdata(node->_cycler, pipeline_stage, current_thread);
    GeomNode::GeomList::iterator gi;
    PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
    for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
      GeomNode::GeomEntry &entry = (*gi);
      PT(Geom) geom = entry._geom.get_write_pointer();
      register_vertices(geom, might_have_unused);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(node->_cycler);
}

/**
 * Transforms the vertices and the normals in the indicated Geom by the
 * indicated matrix.  Returns true if the Geom was changed, false otherwise.
 */
bool GeomTransformer::
transform_vertices(Geom *geom, const LMatrix4 &mat) {
  PStatTimer timer(_apply_vertex_collector);

  nassertr(geom != nullptr, false);
  SourceVertices sv;
  sv._mat = mat;
  sv._vertex_data = geom->get_vertex_data();

  NewVertexData &new_data = _vertices[sv];
  if (new_data._vdata.is_null()) {
    // We have not yet converted these vertices.  Do so now.
    PT(GeomVertexData) new_vdata = new GeomVertexData(*sv._vertex_data);
    new_vdata->transform_vertices(mat);
    new_data._vdata = new_vdata;
  }

  geom->set_vertex_data(new_data._vdata);
  if (sv._vertex_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[sv._vertex_data]._might_have_unused = true;
  }

  return true;
}


/**
 * Transforms the vertices and the normals in all of the Geoms within the
 * indicated GeomNode by the indicated matrix.  Does not destructively change
 * Geoms; instead, a copy will be made of each Geom to be changed, in case
 * multiple GeomNodes reference the same Geom.  Returns true if the GeomNode
 * was changed, false otherwise.
 */
bool GeomTransformer::
transform_vertices(GeomNode *node, const LMatrix4 &mat) {
  bool any_changed = false;

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(node->_cycler, current_thread) {
    GeomNode::CDStageWriter cdata(node->_cycler, pipeline_stage, current_thread);
    GeomNode::GeomList::iterator gi;
    PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
    for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
      GeomNode::GeomEntry &entry = (*gi);
      PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
      if (transform_vertices(new_geom, mat)) {
        entry._geom = std::move(new_geom);
        any_changed = true;
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(node->_cycler);

  if (any_changed) {
    node->mark_internal_bounds_stale();
  }

  return any_changed;
}


/**
 * Transforms the texture coordinates in the indicated Geom by the indicated
 * matrix.  Returns true if the Geom was changed, false otherwise.
 */
bool GeomTransformer::
transform_texcoords(Geom *geom, const InternalName *from_name,
                    InternalName *to_name, const LMatrix4 &mat) {
  PStatTimer timer(_apply_texcoord_collector);

  nassertr(geom != nullptr, false);

  SourceTexCoords st;
  st._mat = mat;
  st._from = from_name;
  st._to = to_name;
  st._vertex_data = geom->get_vertex_data();

  NewVertexData &new_data = _texcoords[st];
  if (new_data._vdata.is_null()) {
    if (!st._vertex_data->has_column(from_name)) {
      // No from_name column; no change.
      return false;
    }

    PT(GeomVertexData) new_vdata;

    // We have not yet converted these texcoords.  Do so now.
    if (st._vertex_data->has_column(to_name)) {
      new_vdata = new GeomVertexData(*st._vertex_data);
    } else {
      const GeomVertexColumn *old_column =
        st._vertex_data->get_format()->get_column(from_name);
      new_vdata = st._vertex_data->replace_column
        (to_name, old_column->get_num_components(),
         old_column->get_numeric_type(),
         old_column->get_contents());
    }

    CPT(GeomVertexFormat) format = new_vdata->get_format();

    GeomVertexWriter tdata(new_vdata, to_name);
    GeomVertexReader fdata(new_vdata, from_name);

    while (!fdata.is_at_end()) {
      const LPoint4 &coord = fdata.get_data4();
      tdata.set_data4(coord * mat);
    }
    new_data._vdata = new_vdata;
  }

  geom->set_vertex_data(new_data._vdata);
  if (st._vertex_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[st._vertex_data]._might_have_unused = true;
  }

  return true;
}


/**
 * Transforms the texture coordinates in all of the Geoms within the indicated
 * GeomNode by the indicated matrix.  Does not destructively change Geoms;
 * instead, a copy will be made of each Geom to be changed, in case multiple
 * GeomNodes reference the same Geom.  Returns true if the GeomNode was
 * changed, false otherwise.
 */
bool GeomTransformer::
transform_texcoords(GeomNode *node, const InternalName *from_name,
                    InternalName *to_name, const LMatrix4 &mat) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (transform_texcoords(new_geom, from_name, to_name, mat)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}


/**
 * Overrides the color indicated within the Geom with the given replacement
 * color.  Returns true if the Geom was changed, false otherwise.
 */
bool GeomTransformer::
set_color(Geom *geom, const LColor &color) {
  PStatTimer timer(_apply_set_color_collector);

  SourceColors sc;
  sc._color = color;
  sc._vertex_data = geom->get_vertex_data();

  NewVertexData &new_data = _fcolors[sc];
  if (new_data._vdata.is_null()) {
    // We have not yet converted these colors.  Do so now.
    if (sc._vertex_data->has_column(InternalName::get_color())) {
      new_data._vdata = sc._vertex_data->set_color(color);
    } else {
      new_data._vdata = sc._vertex_data->set_color
        (color, 1, Geom::NT_packed_dabc, Geom::C_color);
    }
  }

  geom->set_vertex_data(new_data._vdata);
  if (sc._vertex_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[sc._vertex_data]._might_have_unused = true;
  }

  return true;
}


/**
 * Overrides the color indicated within the GeomNode with the given
 * replacement color.  Returns true if any Geom in the GeomNode was changed,
 * false otherwise.
 */
bool GeomTransformer::
set_color(GeomNode *node, const LColor &color) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (set_color(new_geom, color)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}

/**
 * Transforms the colors in the indicated Geom by the indicated scale.
 * Returns true if the Geom was changed, false otherwise.
 */
bool GeomTransformer::
transform_colors(Geom *geom, const LVecBase4 &scale) {
  PStatTimer timer(_apply_scale_color_collector);

  nassertr(geom != nullptr, false);

  SourceColors sc;
  sc._color = scale;
  sc._vertex_data = geom->get_vertex_data();

  NewVertexData &new_data = _tcolors[sc];
  if (new_data._vdata.is_null()) {
    // We have not yet converted these colors.  Do so now.
    if (sc._vertex_data->has_column(InternalName::get_color())) {
      new_data._vdata = sc._vertex_data->scale_color(scale);
    } else {
      new_data._vdata = sc._vertex_data->set_color
        (scale, 1, Geom::NT_packed_dabc, Geom::C_color);
    }
  }

  geom->set_vertex_data(new_data._vdata);
  if (sc._vertex_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[sc._vertex_data]._might_have_unused = true;
  }

  return true;
}


/**
 * Transforms the colors in all of the Geoms within the indicated GeomNode by
 * the indicated scale.  Does not destructively change Geoms; instead, a copy
 * will be made of each Geom to be changed, in case multiple GeomNodes
 * reference the same Geom.  Returns true if the GeomNode was changed, false
 * otherwise.
 */
bool GeomTransformer::
transform_colors(GeomNode *node, const LVecBase4 &scale) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (transform_colors(new_geom, scale)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}


/**
 * Removes textures from Geoms by applying the texture colors to the vertices.
 *
 * See apply_texure_colors(GeomNode *, RenderState *).
 */
bool GeomTransformer::
apply_texture_colors(Geom *geom, TextureStage *ts, Texture *tex,
                     const TexMatrixAttrib *tma, const LColor &base_color,
                     bool keep_vertex_color) {
  PStatTimer timer(_apply_texture_color_collector);

  nassertr(geom != nullptr, false);

  PT(TexturePeeker) peeker = tex->peek();
  if (peeker == nullptr) {
    return false;
  }

  if (peeker->get_x_size() == 1 &&
      peeker->get_y_size() == 1 &&
      peeker->get_z_size() == 1) {
    // If it's just a one-pixel texture (e.g.  a simple ram image), don't
    // bother scanning the UV's.  Just extract the color and apply it.
    LColor color;
    peeker->lookup(color, 0.0f, 0.0f);
    color.set(color[0] * base_color[0],
              color[1] * base_color[1],
              color[2] * base_color[2],
              color[3] * base_color[3]);
    if (keep_vertex_color) {
      return transform_colors(geom, color);
    } else {
      return set_color(geom, color);
    }
  }

  bool got_mat = false;
  LMatrix4 mat = LMatrix4::ident_mat();
  if (tma != nullptr && tma->has_stage(ts)) {
    mat = tma->get_mat(ts);
    got_mat = !mat.almost_equal(LMatrix4::ident_mat());
  }

  // This version of the code just applied one overall flat color to the
  // entire mesh.  Turned out not to be good enough.  Instead, we'll look up
  // each vertex in the texture map and apply the nearest color to the vertex.
  /*
  // Scan the UV's to get the used range.  This is particularly necessary for
  // palettized textures.

  LPoint3 min_point, max_point;
  bool found_any = false;
  geom->calc_tight_bounds(min_point, max_point, found_any,
                          geom->get_vertex_data(),
                          got_mat, mat,
                          ts->get_texcoord_name(),
                          Thread::get_current_thread());
  if (found_any) {
    // Now use that UV range to determine the overall color of the geom's
    // texture.
    LColor color;
    peeker->filter_rect(color,
                        min_point[0], min_point[1], min_point[2],
                        max_point[0], max_point[1], max_point[2]);
    color.set(color[0] * base_color[0],
              color[1] * base_color[1],
              color[2] * base_color[2],
              color[3] * base_color[3]);
    if (keep_vertex_color) {
      return transform_colors(geom, color);
    } else {
      return set_color(geom, color);
    }
  }

  return false;
  */

  SourceTextureColors stc;
  stc._ts = ts;
  stc._tex = tex;
  stc._tma = tma;
  stc._base_color = base_color;
  stc._keep_vertex_color = keep_vertex_color;
  stc._vertex_data = geom->get_vertex_data();

  NewVertexData &new_data = _tex_colors[stc];
  if (new_data._vdata.is_null()) {
    // We have not yet applied these texture colors.  Do so now.

    PT(GeomVertexData) vdata;

    // Make sure the vdata has a color column.
    if (stc._vertex_data->has_column(InternalName::get_color())) {
      vdata = new GeomVertexData(*stc._vertex_data);
    } else {
      // Create a color column where there wasn't one before.
      vdata = new GeomVertexData(*stc._vertex_data->set_color
                                 (LColor(1.0f, 1.0f, 1.0f, 1.0f), 1, Geom::NT_packed_dabc, Geom::C_color));
      keep_vertex_color = false;
    }

    // Check whether it has 2-d or 3-d texture coordinates.
    bool tex3d = false;
    const GeomVertexColumn *column = vdata->get_format()->get_column(ts->get_texcoord_name());
    if (column == nullptr) {
      return false;
    }
    if (column->get_num_components() >= 3) {
      tex3d = true;
    }

    // Now walk through the vertices and apply each color from the texture as
    // we go.
    if (keep_vertex_color) {
      // We want to modulate the existing vertex color.
      GeomVertexReader gtexcoord(vdata, ts->get_texcoord_name());
      GeomVertexRewriter gcolor(vdata, InternalName::get_color());

      if (got_mat || tex3d) {
        while (!gtexcoord.is_at_end()) {
          LTexCoord3 p = gtexcoord.get_data3();
          LColor c = gcolor.get_data4();
          p = p * mat;
          LColor color;
          peeker->lookup(color, p[0], p[1], p[2]);
          color.set(color[0] * base_color[0] * c[0],
                    color[1] * base_color[1] * c[1],
                    color[2] * base_color[2] * c[2],
                    color[3] * base_color[3] * c[3]);
          gcolor.set_data4(color);
        }
      } else {
        while (!gtexcoord.is_at_end()) {
          LTexCoord p = gtexcoord.get_data2();
          LColor c = gcolor.get_data4();
          LColor color;
          peeker->lookup(color, p[0], p[1]);
          color.set(color[0] * base_color[0] * c[0],
                    color[1] * base_color[1] * c[1],
                    color[2] * base_color[2] * c[2],
                    color[3] * base_color[3] * c[3]);
          gcolor.set_data4(color);
        }
      }
    } else {
      // We want to replace any existing vertex color.
      GeomVertexReader gtexcoord(vdata, ts->get_texcoord_name());
      GeomVertexWriter gcolor(vdata, InternalName::get_color());

      if (got_mat || tex3d) {
        while (!gtexcoord.is_at_end()) {
          LTexCoord3 p = gtexcoord.get_data3();
          p = p * mat;
          LColor color;
          peeker->lookup(color, p[0], p[1], p[2]);
          color.set(color[0] * base_color[0],
                    color[1] * base_color[1],
                    color[2] * base_color[2],
                    color[3] * base_color[3]);
          gcolor.set_data4(color);
        }
      } else {
        while (!gtexcoord.is_at_end()) {
          LTexCoord p = gtexcoord.get_data2();
          LColor color;
          peeker->lookup(color, p[0], p[1]);
          color.set(color[0] * base_color[0],
                    color[1] * base_color[1],
                    color[2] * base_color[2],
                    color[3] * base_color[3]);
          gcolor.set_data4(color);
        }
      }
    }

    new_data._vdata = vdata;
  }

  geom->set_vertex_data(new_data._vdata);
  if (stc._vertex_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[stc._vertex_data]._might_have_unused = true;
  }

  return true;
}

/**
 * Removes textures from Geoms by applying the texture colors to the vertices.
 * This is primarily useful to simplify a low-LOD model.
 *
 * Only the bottommost texture is used (if there is more than one), and it is
 * applied as if it were M_modulate, and WM_repeat, regardless of its actual
 * settings.  If the texture has a simple_ram_image, this may be used if the
 * main image isn't resident.
 *
 * After this call, there will be no texturing specified on the GeomNode
 * level.  Of course, there might still be texturing inherited from above.
 */
bool GeomTransformer::
apply_texture_colors(GeomNode *node, const RenderState *state) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    CPT(RenderState) geom_state = state->compose(entry._state);

    const TextureAttrib *ta = DCAST(TextureAttrib, geom_state->get_attrib(TextureAttrib::get_class_slot()));
    if (ta != nullptr) {
      CPT(TextureAttrib) ta2 = ta->filter_to_max(1);
      if (ta2->get_num_on_stages() > 0) {
        TextureStage *ts = ta2->get_on_stage(0);
        Texture *tex = ta2->get_on_texture(ts);
        const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, geom_state->get_attrib(TexMatrixAttrib::get_class_slot()));

        const ColorAttrib *ca = DCAST(ColorAttrib, geom_state->get_attrib(ColorAttrib::get_class_slot()));
        LColor base_color(1.0f, 1.0f, 1.0f, 1.0f);
        bool keep_vertex_color = true;
        if (ca != nullptr && ca->get_color_type() == ColorAttrib::T_flat) {
          base_color = ca->get_color();
          keep_vertex_color = false;
        }

        PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
        if (apply_texture_colors(new_geom, ts, tex, tma, base_color, keep_vertex_color)) {
          entry._geom = new_geom;
          any_changed = true;

          if (new_geom->get_vertex_data()->has_column(InternalName::get_color())) {
            // Ensure we have a ColorAttrib::make_vertex() attrib.
            CPT(RenderState) color_state = entry._state->set_attrib(ColorAttrib::make_vertex());
            if (entry._state != color_state) {
              entry._state = color_state;
              any_changed = true;
            }
          }
        }

        // Also remove any texture references from the GeomState.
        CPT(RenderState) no_tex_state = entry._state->remove_attrib(TextureAttrib::get_class_slot());
        if (entry._state != no_tex_state) {
          entry._state = no_tex_state;
          any_changed = true;
        }
      }
    }
  }

  return any_changed;
}

/**
 * Applies the indicated render state to all the of Geoms.  Returns true if
 * the GeomNode was changed, false otherwise.
 */
bool GeomTransformer::
apply_state(GeomNode *node, const RenderState *state) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    CPT(RenderState) new_state = state->compose(entry._state);
    if (entry._state != new_state) {
      entry._state = new_state;
      any_changed = true;
    }
  }

  return any_changed;
}

/**
 * Changes the GeomVertexData of the indicated Geom to use the specified
 * format.
 */
bool GeomTransformer::
set_format(Geom *geom, const GeomVertexFormat *new_format) {
  PStatTimer timer(_apply_set_format_collector);

  nassertr(geom != nullptr, false);

  SourceFormat sf;
  sf._format = new_format;
  sf._vertex_data = geom->get_vertex_data();

  NewVertexData &new_data = _format[sf];
  if (new_data._vdata.is_null()) {
    if (sf._vertex_data->get_format() == new_format) {
      // No change.
      return false;
    }

    // We have not yet converted this vertex data.  Do so now.
    PT(GeomVertexData) new_vdata = new GeomVertexData(*sf._vertex_data);
    new_vdata->set_format(new_format);
    new_data._vdata = new_vdata;
  }

  geom->set_vertex_data(new_data._vdata);
  if (sf._vertex_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[sf._vertex_data]._might_have_unused = true;
  }

  return true;
}

/**
 * Removes the named column from the vertex data in the Geom.  Returns true if
 * the Geom was changed, false otherwise.
 */
bool GeomTransformer::
remove_column(Geom *geom, const InternalName *column) {
  CPT(GeomVertexFormat) format = geom->get_vertex_data()->get_format();
  if (!format->has_column(column)) {
    return false;
  }

  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*format);
  new_format->remove_column(column);
  new_format->pack_columns();
  format = GeomVertexFormat::register_format(new_format);

  return set_format(geom, format);
}


/**
 * Removes the named column from the vertex datas within the GeomNode.
 * Returns true if the GeomNode was changed, false otherwise.
 */
bool GeomTransformer::
remove_column(GeomNode *node, const InternalName *column) {
  bool any_changed = false;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    if (remove_column(new_geom, column)) {
      entry._geom = new_geom;
      any_changed = true;
    }
  }

  return any_changed;
}

/**
 * Checks if the different geoms in the GeomNode have different RenderStates.
 * If so, tries to make the RenderStates the same.  It does this by
 * canonicalizing the ColorAttribs, and in the future, possibly other attribs.
 */
bool GeomTransformer::
make_compatible_state(GeomNode *node) {
  if (node->get_num_geoms() < 2) {
    return false;
  }

  GeomNode::CDWriter cdata(node->_cycler);
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();

  // For each geom, calculate a canonicalized RenderState, and classify all
  // the geoms according to that.  By "canonicalize" here, we simply mean
  // removing the ColorAttrib.

  typedef pmap <CPT(RenderState), pvector<int> > StateTable;
  StateTable state_table;

  for (int i = 0; i < (int)geoms->size(); i++) {
    GeomNode::GeomEntry &entry = (*geoms)[i];
    CPT(RenderState) canon = entry._state->remove_attrib(ColorAttrib::get_class_slot());
    state_table[canon].push_back(i);
  }

  // For each group of geoms, check for mismatch.

  bool any_changed = false;
  StateTable::iterator si;
  for (si = state_table.begin(); si != state_table.end(); si++) {

    // If the geoms in the group already have the same RenderStates, then
    // nothing needs to be done to this group.

    const pvector<int> &indices = (*si).second;
    bool mismatch = false;
    for (int i = 1; i < (int)indices.size(); i++) {
      if ((*geoms)[indices[i]]._state != (*geoms)[indices[0]]._state) {
        mismatch = true;
        break;
      }
    }
    if (!mismatch) {
      continue;
    }

    // The geoms do not have the same RenderState, but they could, since their
    // canonicalized states are the same.  Canonicalize them, by applying the
    // colors to the vertices.

    const RenderState *canon_state = (*si).first;
    for (int i = 0; i < (int)indices.size(); i++) {
      GeomNode::GeomEntry &entry = (*geoms)[indices[i]];
      const RenderAttrib *ra = entry._state->get_attrib_def(ColorAttrib::get_class_slot());
      const ColorAttrib *ca = DCAST(ColorAttrib, ra);
      if (ca->get_color_type() == ColorAttrib::T_vertex) {
        // All we need to do is ensure that the geom has a color column.
        if (!entry._geom.get_read_pointer()->get_vertex_data()->has_column(InternalName::get_color())) {
          PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
          if (set_color(new_geom, LColor(1,1,1,1))) {
            entry._geom = new_geom;
          }
        }
      } else {
        // A flat color (or "off", which is white).  Set the vertices to the
        // indicated flat color.
        LColor c = ca->get_color();
        PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
        if (set_color(new_geom, c)) {
          entry._geom = new_geom;
        }
      }
      entry._state = canon_state->add_attrib(ColorAttrib::make_vertex());
      any_changed = true;
    }
  }

  return any_changed;
}

/**
 * Reverses the lighting normals on the vertex data, if any.  Returns true if
 * the Geom was changed, false otherwise.
 */
bool GeomTransformer::
reverse_normals(Geom *geom) {
  nassertr(geom != nullptr, false);
  CPT(GeomVertexData) orig_data = geom->get_vertex_data();
  NewVertexData &new_data = _reversed_normals[orig_data];
  if (new_data._vdata.is_null()) {
    new_data._vdata = orig_data->reverse_normals();
  }

  if (new_data._vdata == orig_data) {
    // No change.
    return false;
  }

  geom->set_vertex_data(new_data._vdata);
  if (orig_data->get_ref_count() > 1) {
    _vdata_assoc[new_data._vdata]._might_have_unused = true;
    _vdata_assoc[orig_data]._might_have_unused = true;
  }

  return true;
}

/**
 * Duplicates triangles in this GeomNode so that each triangle is back-to-back
 * with another triangle facing in the opposite direction.  If the geometry
 * has vertex normals, this will also duplicate and reverse the normals, so
 * that lighting will work correctly from both sides.  Note that calling this
 * when the geometry is already doublesided (with back-to-back polygons) will
 * result in multiple redundant coplanar polygons.
 *
 * Also see CullFaceAttrib, which can enable rendering of both sides of a
 * triangle without having to duplicate it (but which doesn't necessarily work
 * in the presence of lighting).
 *
 * Returns true if any Geoms are modified, false otherwise.
 */
bool GeomTransformer::
doubleside(GeomNode *node) {
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; ++i) {
    CPT(Geom) orig_geom = node->get_geom(i);
    bool has_normals = (orig_geom->get_vertex_data()->has_column(InternalName::get_normal()));
    if (has_normals) {
      // If the geometry has normals, we have to duplicate it to reverse the
      // normals on the duplicate copy.
      PT(Geom) new_geom = orig_geom->reverse();
      reverse_normals(new_geom);
      node->add_geom(new_geom, node->get_geom_state(i));

    } else {
      // If there are no normals, we can just doubleside it in place.  This is
      // preferable because we can share vertices.
      orig_geom.clear();
      node->modify_geom(i)->doubleside_in_place();
    }
  }

  return (num_geoms != 0);
}


/**
 * Reverses the winding order of triangles in this GeomNode so that each
 * triangle is facing in the opposite direction.  If the geometry has vertex
 * normals, this will also reverse the normals, so that lighting will work
 * correctly.
 *
 * Also see CullFaceAttrib, which can effectively change the facing of a
 * triangle having to modify its vertices (but which doesn't necessarily work
 * in the presence of lighting).
 *
 * Returns true if any Geoms are modified, false otherwise.
 */
bool GeomTransformer::
reverse(GeomNode *node) {
  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; ++i) {
    PT(Geom) geom = node->modify_geom(i);
    geom->reverse_in_place();
    reverse_normals(geom);
  }

  return (num_geoms != 0);
}

/**
 * Should be called after performing any operations--particularly
 * PandaNode::apply_attribs_to_vertices()--that might result in new
 * GeomVertexData objects being duplicated and modified.  This walks through
 * those newly duplicated objects and ensures that redundant unused vertices
 * have not been created, removing them if they have.
 */
void GeomTransformer::
finish_apply() {
  VertexDataAssocMap::iterator vi;
  for (vi = _vdata_assoc.begin(); vi != _vdata_assoc.end(); ++vi) {
    const GeomVertexData *vdata = (*vi).first;
    VertexDataAssoc &assoc = (*vi).second;
    if (assoc._might_have_unused) {
      assoc.remove_unused_vertices(vdata);
    }
  }
  _vdata_assoc.clear();

  _texcoords.clear();
  _fcolors.clear();
  _tcolors.clear();
  _format.clear();
  _reversed_normals.clear();
}

/**
 * Collects together GeomVertexDatas from different geoms into one big (or
 * several big) GeomVertexDatas.  Returns the number of unique GeomVertexDatas
 * created.
 *
 * If format_only is true, this only makes GeomVertexFormats compatible; it
 * does not otherwise combine vertices.
 *
 * You should follow this up with a call to finish_collect(), but you probably
 * don't want to call this method directly anyway.  Call
 * SceneGraphReducer::collect_vertex_data() instead.
 */
int GeomTransformer::
collect_vertex_data(Geom *geom, int collect_bits, bool format_only) {
  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  if (vdata->get_num_rows() > _max_collect_vertices) {
    // Don't even bother.
    return 0;
  }

  CPT(GeomVertexFormat) format = vdata->get_format();

  NewCollectedKey key;
  if ((collect_bits & SceneGraphReducer::CVD_name) != 0) {
    key._name = vdata->get_name();
  }
  if ((collect_bits & SceneGraphReducer::CVD_format) != 0) {
    key._format = format;
  }
  if ((collect_bits & SceneGraphReducer::CVD_usage_hint) != 0) {
    key._usage_hint = vdata->get_usage_hint();
  } else {
    key._usage_hint = Geom::UH_unspecified;
  }
  if ((collect_bits & SceneGraphReducer::CVD_animation_type) != 0) {
    key._animation_type = format->get_animation().get_animation_type();
  } else {
    key._animation_type = Geom::AT_none;
  }

  AlreadyCollectedMap::const_iterator ai;
  ai = _already_collected_map.find(vdata);
  if (ai != _already_collected_map.end()) {
    // We've previously collected this vertex data; reuse it.
    const AlreadyCollectedData &acd = (*ai).second;
    SourceGeom source_geom;
    source_geom._geom = geom;
    source_geom._vertex_offset = acd._vertex_offset;
    acd._ncd->_source_geoms.push_back(source_geom);
    return 0;
  }

  // We haven't collected this vertex data yet; associate it with a new data.
  NewCollectedMap::iterator ni = _new_collected_map.find(key);
  NewCollectedData *ncd;
  if (ni != _new_collected_map.end()) {
    ncd = (*ni).second;

  } else {
    // We haven't encountered a compatible GeomVertexData before.  Create a
    // new entry.
    ncd = new NewCollectedData(vdata);
    _new_collected_list.push_back(ncd);
    _new_collected_map[key] = ncd;
  }

  if (ncd->_new_format != format) {
    ncd->_new_format = format->get_union_format(ncd->_new_format);
  }

  int this_num_vertices = vdata->get_num_rows();

  if (!format_only &&
      ncd->_num_vertices + this_num_vertices > _max_collect_vertices) {
    // Whoa, hold the phone!  Too many vertices going into this one
    // GeomVertexData object; we'd better start over.
    ncd = new NewCollectedData(vdata);
    _new_collected_list.push_back(ncd);
    _new_collected_map[key] = ncd;
  }

  int vertex_offset = ncd->_num_vertices;

  AlreadyCollectedData &acd = _already_collected_map[vdata];
  acd._ncd = ncd;
  acd._vertex_offset = vertex_offset;

  SourceGeom source_geom;
  source_geom._geom = geom;
  source_geom._vertex_offset = vertex_offset;
  ncd->_source_geoms.push_back(source_geom);

  SourceData source_data;
  source_data._vdata = vdata;
  source_data._num_vertices = this_num_vertices;

  ncd->_source_datas.push_back(source_data);
  ncd->_num_vertices += this_num_vertices;

  return 0;
}


/**
 * Collects together individual GeomVertexData structures that share the same
 * format into one big GeomVertexData structure.  This is intended to minimize
 * context switches on the graphics card.
 *
 * If format_only is true, this only makes GeomVertexFormats compatible; it
 * does not otherwise combine vertices.
 *
 * You should follow this up with a call to finish_collect(), but you probably
 * don't want to call this method directly anyway.  Call
 * SceneGraphReducer::collect_vertex_data() instead.
 */
int GeomTransformer::
collect_vertex_data(GeomNode *node, int collect_bits, bool format_only) {
  int num_adjusted = 0;
  GeomTransformer *dynamic = nullptr;

  GeomNode::CDWriter cdata(node->_cycler);
  GeomNode::GeomList::iterator gi;
  PT(GeomNode::GeomList) geoms = cdata->modify_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomNode::GeomEntry &entry = (*gi);
    PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
    entry._geom = new_geom;

    if ((collect_bits & SceneGraphReducer::CVD_avoid_dynamic) != 0 &&
        new_geom->get_vertex_data()->get_usage_hint() < Geom::UH_static) {
      // This one has some dynamic properties.  Collect it independently of
      // the outside world.
      if (dynamic == nullptr) {
        dynamic = new GeomTransformer(*this);
      }
      num_adjusted += dynamic->collect_vertex_data(new_geom, collect_bits, format_only);

    } else {
      num_adjusted += collect_vertex_data(new_geom, collect_bits, format_only);
    }
  }

  if (dynamic != nullptr) {
    num_adjusted += dynamic->finish_collect(format_only);
    delete dynamic;
  }

  return num_adjusted;
}

/**
 * This should be called after a call to collect_vertex_data() to finalize the
 * changes and apply them to the vertices in the graph.  If this is not
 * called, it will be called automatically by the GeomTransformer destructor.
 *
 * If format_only is true, this returns the number of GeomVertexDatas modified
 * to use a new format.  If false, it returns the number of GeomVertexDatas
 * created.
 */
int GeomTransformer::
finish_collect(bool format_only) {
  int num_adjusted = 0;

  NewCollectedList::iterator nci;
  for (nci = _new_collected_list.begin();
       nci != _new_collected_list.end();
       ++nci) {
    NewCollectedData *ncd = (*nci);
    if (format_only) {
      num_adjusted += ncd->apply_format_only_changes();
    } else {
      num_adjusted += ncd->apply_collect_changes();
    }
    delete ncd;
  }

  _new_collected_list.clear();
  _new_collected_map.clear();
  _already_collected_map.clear();

  return num_adjusted;
}

/**
 * Uses the indicated munger to premunge the given Geom to optimize it for
 * eventual rendering.  See SceneGraphReducer::premunge().
 */
PT(Geom) GeomTransformer::
premunge_geom(const Geom *geom, GeomMunger *munger) {
  // This method had been originally provided to cache the result for a
  // particular geommunger and vdatamunger combination, similar to the way
  // other GeomTransformer methods work.  On reflection, this additional
  // caching is not necessary, since the GeomVertexFormat does its own
  // caching, and there's no danger of that cache filling up during the span
  // of one frame.

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  vdata = munger->premunge_data(vdata);
  CPT(Geom) pgeom = geom;
  munger->premunge_geom(pgeom, vdata);

  PT(Geom) geom_copy = pgeom->make_copy();
  geom_copy->set_vertex_data(vdata);

  return geom_copy;
}

/**
 *
 */
GeomTransformer::NewCollectedData::
NewCollectedData(const GeomVertexData *source_data) {
  _new_format = source_data->get_format();
  _vdata_name = source_data->get_name();
  _usage_hint = source_data->get_usage_hint();
  _num_vertices = 0;
}

/**
 * Actually adjusts the GeomVertexDatas found in a collect_vertex_data()
 * format-only call to have the same vertex format.  Returns the number of
 * vdatas modified.
 */
int GeomTransformer::NewCollectedData::
apply_format_only_changes() {
  int num_modified = 0;

  // We probably don't need to use a map, since GeomVertexData::convert_to()
  // already caches its result, but we do it anyway just in case there's
  // danger of overflowing the cache.  What the heck, it's easy to do.
  typedef pmap< CPT(GeomVertexData), CPT(GeomVertexData) > VDataMap;
  VDataMap vdata_map;

  SourceGeoms::iterator sgi;
  for (sgi = _source_geoms.begin(); sgi != _source_geoms.end(); ++sgi) {
    SourceGeom &sg = (*sgi);
    CPT(GeomVertexData) orig_data = sg._geom->get_vertex_data();

    if (orig_data->get_format() != _new_format) {
      VDataMap::iterator mi = vdata_map.find(orig_data);
      if (mi != vdata_map.end()) {
        // Already modified this vdata.
        sg._geom->set_vertex_data((*mi).second);

      } else {
        // Modify this vdata to the new format.
        CPT(GeomVertexData) new_data = orig_data->convert_to(_new_format);
        vdata_map[orig_data] = new_data;
        ++num_modified;

        sg._geom->set_vertex_data(new_data);
      }
    }
  }

  return num_modified;
}

/**
 * Actually combines all of the vertex datas found in a previous call to
 * collect_vertex_data().
 */
int GeomTransformer::NewCollectedData::
apply_collect_changes() {
  if (_num_vertices == 0) {
    return 0;
  }

  _new_data =
    new GeomVertexData(_vdata_name, _new_format, _usage_hint);

  _new_data->unclean_set_num_rows(_num_vertices);

  // Copy each source data into the new GeomVertexData, one at a time.
  int vertex_offset = 0;
  SourceDatas::iterator sdi;
  for (sdi = _source_datas.begin(); sdi != _source_datas.end(); ++sdi) {
    SourceData &sd = (*sdi);
    CPT(GeomVertexData) vdata = sd._vdata;

    if (_new_format != vdata->get_format()) {
      // Convert (non-destructively) the current Geom's vertex data to the new
      // format, so we can just blindly append the vertices to _new_data,
      // within append_vdata().
      vdata = vdata->convert_to(_new_format);
    }

    append_vdata(vdata, vertex_offset);
    vertex_offset += sd._num_vertices;
  }

  nassertr(vertex_offset == _num_vertices, 0);

  if (_new_btable != nullptr) {
    _new_btable->set_rows(_new_btable_rows);
    _new_data->set_transform_blend_table(_new_btable);
  }

  update_geoms();

  _new_data.clear();
  _new_btable.clear();
  _new_btable_rows.clear();

  return 1;
}

/**
 * Appends the vertices from the indicated source GeomVertexData to the end of
 * the working data.
 */
void GeomTransformer::NewCollectedData::
append_vdata(const GeomVertexData *vdata, int vertex_offset) {
  for (size_t i = 0; i < vdata->get_num_arrays(); ++i) {
    PT(GeomVertexArrayDataHandle) new_handle = _new_data->modify_array_handle(i);
    CPT(GeomVertexArrayDataHandle) old_handle = vdata->get_array_handle(i);
    size_t stride = (size_t)_new_format->get_array(i)->get_stride();
    size_t start_byte = (size_t)vertex_offset * stride;
    size_t copy_bytes = old_handle->get_data_size_bytes();
    nassertv(start_byte + copy_bytes <= new_handle->get_data_size_bytes());

    new_handle->copy_subdata_from(start_byte, copy_bytes, old_handle, 0, copy_bytes);
  }

  // Also, copy the animation data (if any).  This means combining transform
  // andor slider tables, and might therefore mean remapping transform indices
  // in the vertices.  Each of these has a slightly different way to handle
  // the remapping, because they have slightly different kinds of data.

  if (vdata->get_transform_table() != nullptr ||
      _new_data->get_transform_table() != nullptr) {
    // The TransformTable.
    CPT(TransformTable) old_table;
    if (vdata->get_transform_table() != nullptr) {
      old_table = vdata->get_transform_table();
    } else {
      PT(TransformTable) temp_table = new TransformTable;
      // There's an implicit identity transform for all nodes.
      PT(VertexTransform) identity_transform = new UserVertexTransform("identity");
      temp_table->add_transform(identity_transform);
      old_table = TransformTable::register_table(temp_table);
    }

    // First, build a mapping of the transforms we already have in the current
    // table.  We must do this because the TransformTable doesn't
    // automatically unquify index numbers for us (it doesn't store an index).
    typedef pmap<const VertexTransform *, int> AddedTransforms;
    AddedTransforms added_transforms;

    int num_old_transforms = old_table->get_num_transforms();
    for (int i = 0; i < num_old_transforms; i++) {
      added_transforms[old_table->get_transform(i)] = i;
    }

    // Now create a new table.  We have to create a new table instead of
    // modifying the existing one, since a registered TransformTable cannot be
    // modified.
    PT(TransformTable) new_table;
    if (_new_data->get_transform_table() != nullptr) {
      new_table = new TransformTable(*_new_data->get_transform_table());
    } else {
      new_table = new TransformTable;
    }

    // Now walk through the old table and copy over its transforms.  We will
    // build up an IndexMap of old index numbers to new index numbers while we
    // go, which we can use to modify the vertices.
    IndexMap transform_map;

    int num_transforms = old_table->get_num_transforms();
    transform_map.reserve(num_transforms);
    for (int ti = 0; ti < num_transforms; ++ti) {
      const VertexTransform *transform = old_table->get_transform(ti);
      AddedTransforms::iterator ai = added_transforms.find(transform);
      if (ai != added_transforms.end()) {
        // Already got this one in the table.
        transform_map.push_back((*ai).second);
      } else {
        // This is a new one.
        int tj = new_table->add_transform(transform);
        transform_map.push_back(tj);
        added_transforms[transform] = tj;
      }
    }
    _new_data->set_transform_table(TransformTable::register_table(new_table));

    // And now modify the vertices to update the indices to their new values
    // in the new table.  This requires a nested loop, since each column of
    // transform_index might define multiple index values.
    GeomVertexRewriter index(_new_data, InternalName::get_transform_index());
    if (index.has_column()) {
      int num_values = index.get_column()->get_num_values();
      int num_rows = vdata->get_num_rows();

      index.set_row_unsafe(vertex_offset);
      for (int ci = 0; ci < num_rows; ++ci) {
        LVecBase4i indices = index.get_data4i();
        for (int i = 0; i < num_values; i++) {
          nassertv(indices[i] >= 0 && indices[i] < (int)transform_map.size());
          indices[i] = transform_map[indices[i]];
        }
        index.set_data4i(indices);
      }
    }
  }

  if (vdata->get_transform_blend_table() != nullptr) {
    // The TransformBlendTable.  This one is the easiest, because we can
    // modify it directly, and it will uniquify blend objects for us.

    // We have few special optimizations to handle the TransformBlendTable,
    // since it's a very common case and therefore worth spending a bit of
    // effort to optimize deeply.

    CPT(TransformBlendTable) old_btable = vdata->get_transform_blend_table();

    if (_new_btable == nullptr) {
      _new_btable = new TransformBlendTable;
      _new_btable->add_blend(TransformBlend());
    }

    SparseArray new_rows = old_btable->get_rows();
    new_rows <<= vertex_offset;
    _new_btable_rows |= new_rows;

    // We still need to build up the IndexMap.
    IndexMap blend_map;

    int num_blends = old_btable->get_num_blends();
    blend_map.reserve(num_blends);
    for (int bi = 0; bi < num_blends; ++bi) {
      int bj = _new_btable->add_blend(old_btable->get_blend(bi));
      blend_map.push_back(bj);
    }

    // Modify the indices.  This is simpler than the transform_index, above,
    // because each column of transform_blend may only define one index value.
    GeomVertexRewriter index(_new_data, InternalName::get_transform_blend());
    if (index.has_column()) {
      int num_rows = vdata->get_num_rows();
      index.set_row_unsafe(vertex_offset);

      for (int ci = 0; ci < num_rows; ++ci) {
        int orig_index = index.get_data1i();
        nassertv(orig_index >= 0 && orig_index < (int)blend_map.size());
        int new_index = blend_map[orig_index];
        index.set_data1i(new_index);
      }
    }
  }

  if (vdata->get_slider_table() != nullptr) {
    // The SliderTable.  This one requires making a copy, like the
    // TransformTable (since it can't be modified once registered either), but
    // at least it uniquifies sliders added to it.  Also, it doesn't require
    // indexing into it, so we don't have to build an IndexMap to modify the
    // vertices with.
    const SliderTable *old_sliders = vdata->get_slider_table();
    PT(SliderTable) new_sliders;
    if (_new_data->get_slider_table() != nullptr) {
      new_sliders = new SliderTable(*_new_data->get_slider_table());
    } else {
      new_sliders = new SliderTable;
    }
    int num_sliders = old_sliders->get_num_sliders();
    for (int si = 0; si < num_sliders; ++si) {
      SparseArray new_rows = old_sliders->get_slider_rows(si);
      new_rows <<= vertex_offset;
      new_sliders->add_slider(old_sliders->get_slider(si), new_rows);
    }
    _new_data->set_slider_table(SliderTable::register_table(new_sliders));
  }
}

/**
 * Updates all of the source Geoms to reference the new vertex data.
 */
void GeomTransformer::NewCollectedData::
update_geoms() {
  SourceGeoms::iterator sgi;
  for (sgi = _source_geoms.begin(); sgi != _source_geoms.end(); ++sgi) {
    SourceGeom &sg = (*sgi);
    sg._geom->offset_vertices(_new_data, sg._vertex_offset);
  }
}

/**
 *
 */
void GeomTransformer::VertexDataAssoc::
remove_unused_vertices(const GeomVertexData *vdata) {
  if (_geoms.empty()) {
    // Trivial case.
    return;
  }

  PT(Thread) current_thread = Thread::get_current_thread();

  BitArray referenced_vertices;
  bool any_referenced = false;
  GeomList::iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    Geom *geom = (*gi);
    if (geom->get_vertex_data() != vdata) {
      continue;
    }

    any_referenced = true;
    int num_primitives = geom->get_num_primitives();
    for (int i = 0; i < num_primitives; ++i) {
      GeomPrimitivePipelineReader reader(geom->get_primitive(i), current_thread);
      reader.get_referenced_vertices(referenced_vertices);
    }
  }

  if (!any_referenced) {
    return;
  }

  int num_vertices = vdata->get_num_rows();
  int new_num_vertices = referenced_vertices.get_num_on_bits();
  if (num_vertices <= new_num_vertices) {
    // All vertices are used.
    nassertv(num_vertices == new_num_vertices);
    return;
  }

  // Remap the vertices.
  int *remap_array = (int *)alloca(sizeof(int) * num_vertices);
  int new_index = 0;
  int index;
  int next_index = 0;
  for (index = 0; index < num_vertices; ++index) {
    if (referenced_vertices.get_bit(index)) {
      while (next_index <= index) {
        remap_array[next_index] = new_index;
        ++next_index;
      }
      ++new_index;
    }
  }
  while (next_index < num_vertices) {
    remap_array[next_index] = new_num_vertices - 1;
    ++next_index;
  }

  // Now recopy the actual vertex data, one array at a time.
  PT(GeomVertexData) new_vdata = new GeomVertexData(*vdata);
  new_vdata->unclean_set_num_rows(new_num_vertices);

  size_t num_arrays = vdata->get_num_arrays();
  nassertv(num_arrays == new_vdata->get_num_arrays());

  GeomVertexDataPipelineReader reader(vdata, current_thread);
  reader.check_array_readers();
  GeomVertexDataPipelineWriter writer(new_vdata, true, current_thread);
  writer.check_array_writers();

  for (size_t a = 0; a < num_arrays; ++a) {
    const GeomVertexArrayDataHandle *array_reader = reader.get_array_reader(a);
    GeomVertexArrayDataHandle *array_writer = writer.get_array_writer(a);

    int stride = array_reader->get_array_format()->get_stride();
    nassertv(stride == array_writer->get_array_format()->get_stride());

    int new_index = 0;
    int index;
    for (index = 0; index < num_vertices; ++index) {
      if (referenced_vertices.get_bit(index)) {
        array_writer->copy_subdata_from(new_index * stride, stride,
                                        array_reader,
                                        index * stride, stride);
        ++new_index;
      }
    }
  }

  // Update the subranges in the TransformBlendTable, if any.
  PT(TransformBlendTable) tbtable = new_vdata->modify_transform_blend_table();
  if (!tbtable.is_null()) {
    const SparseArray &rows = tbtable->get_rows();
    SparseArray new_rows;
    int num_subranges = rows.get_num_subranges();
    for (int si = 0; si < num_subranges; ++si) {
      int from = rows.get_subrange_begin(si);
      int to = rows.get_subrange_end(si);
      nassertv(from >= 0 && from < num_vertices && to > from && to <= num_vertices);
      int new_from = remap_array[from];
      int new_to = remap_array[to - 1] + 1;
      nassertv(new_from >= 0 && new_from < new_num_vertices && new_to >= new_from && new_to <= new_num_vertices);
      new_rows.set_range(new_from, new_to - new_from);
    }
    tbtable->set_rows(new_rows);
  }

  // Finally, reindex the Geoms.
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    Geom *geom = (*gi);
    if (geom->get_vertex_data() != vdata) {
      continue;
    }

    int num_primitives = geom->get_num_primitives();
    for (int i = 0; i < num_primitives; ++i) {
      PT(GeomPrimitive) prim = geom->modify_primitive(i);
      prim->make_indexed();
      PT(GeomVertexArrayData) vertices = prim->modify_vertices();
      GeomVertexRewriter rewriter(vertices, 0, current_thread);

      while (!rewriter.is_at_end()) {
        index = rewriter.get_data1i();
        nassertv(index >= 0 && index < num_vertices);
        new_index = remap_array[index];
        nassertv(new_index >= 0 && new_index < new_num_vertices);
        rewriter.set_data1i(new_index);
      }
    }

    geom->set_vertex_data(new_vdata);
  }
}
