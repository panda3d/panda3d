/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spriteParticleRenderer.cxx
 * @author charles
 * @date 2000-07-13
 */

#include "spriteParticleRenderer.h"
#include "boundingSphere.h"
#include "geomNode.h"
#include "sequenceNode.h"
#include "nodePath.h"
#include "dcast.h"
#include "geom.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "renderModeAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"
#include "textureCollection.h"
#include "nodePathCollection.h"
#include "indent.h"
#include "config_particlesystem.h"
#include "pStatTimer.h"

using std::max;
using std::min;

PStatCollector SpriteParticleRenderer::_render_collector("App:Particles:Sprite:Render");

/**
 * constructor
 */
SpriteParticleRenderer::
SpriteParticleRenderer(Texture *tex) :
  BaseParticleRenderer(PR_ALPHA_NONE),
  _color(LColor(1.0f, 1.0f, 1.0f, 1.0f)),
  _height(1.0f),
  _width(1.0f),
  _initial_x_scale(1.0f),
  _final_x_scale(1.0f),
  _initial_y_scale(1.0f),
  _final_y_scale(1.0f),
  _theta(0.0f),
  _base_y_scale(1.0f),
  _aspect_ratio(1.0f),
  _animate_frames_rate(0.0f),
  _animate_frames_index(0),
  _animate_x_ratio(false),
  _animate_y_ratio(false),
  _animate_theta(false),
  _alpha_disable(false),
  _animate_frames(false),
  _animation_removed(true),
  _blend_method(PP_BLEND_LINEAR),
  _color_interpolation_manager(new ColorInterpolationManager(_color)),
  _pool_size(0) {
  set_texture(tex);
  init_geoms();
}

/**
 * copy constructor
 */
SpriteParticleRenderer::
SpriteParticleRenderer(const SpriteParticleRenderer& copy) :
  BaseParticleRenderer(copy),
  _anims(copy._anims),
  _color(copy._color),
  _height(copy._height),
  _width(copy._width),
  _initial_x_scale(copy._initial_x_scale),
  _final_x_scale(copy._final_x_scale),
  _initial_y_scale(copy._initial_y_scale),
  _final_y_scale(copy._final_y_scale),
  _theta(copy._theta),
  _base_y_scale(copy._base_y_scale),
  _aspect_ratio(copy._aspect_ratio),
  _animate_frames_rate(copy._animate_frames_rate),
  _animate_frames_index(copy._animate_frames_index),
  _animate_x_ratio(copy._animate_x_ratio),
  _animate_y_ratio(copy._animate_y_ratio),
  _animate_theta(copy._animate_theta),
  _alpha_disable(copy._alpha_disable),
  _animate_frames(copy._animate_frames),
  _animation_removed(true),
  _blend_method(copy._blend_method),
  _color_interpolation_manager(copy._color_interpolation_manager),
  _pool_size(0),
  _birth_list(copy._birth_list) {
  init_geoms();
}

/**
 * destructor
 */
SpriteParticleRenderer::
~SpriteParticleRenderer() {
  get_render_node()->remove_all_geoms();
}

/**
 * child dynamic copy
 */
BaseParticleRenderer *SpriteParticleRenderer::
make_copy() {
  return new SpriteParticleRenderer(*this);
}


/**
 * Pull either a set of textures from a SequenceNode or a single texture from
 * a GeomNode.  This function is called in both set_from_node() and
 * add_from_node().  Notice the second parameter.  This nodepath will
 * reference the GeomNode holding the first texture in the returned
 * TextureCollection.
 */
int SpriteParticleRenderer::
extract_textures_from_node(const NodePath &node_path, NodePathCollection &np_col, TextureCollection &tex_col) {
  NodePath tex_node_path = node_path;
  NodePath geom_node_path;

  // Look for a sequence node first, in case they want animated texture
  // sprites
  if (!tex_node_path.is_empty() && tex_node_path.node()->get_type() != SequenceNode::get_class_type()) {
    tex_node_path = node_path.find("**/+SequenceNode");
  }

  // Nodepath contains a sequence node, attempt to read its textures.
  if (!tex_node_path.is_empty()) {
    int frame_count = tex_node_path.get_num_children();
    // We do it this way in order to preserve the order of the textures in the
    // sequence.  If we use a find_all_textures() that order is lost.
    for (int i = 0; i < frame_count; ++i) {
      geom_node_path = tex_node_path.get_child(i);
      if (!geom_node_path.is_empty()) {
        // Since this is a SequenceNode, there will be only one texture on
        // this geom_node_path.
        tex_col.add_textures_from(geom_node_path.find_all_textures());
        np_col.add_path(geom_node_path);
      }
    }
    // If unsuccessful, try again as if the node were a normal GeomNode.
    if (tex_col.get_num_textures() == 0) {
      geom_node_path = NodePath();
      tex_col.clear();
      np_col.clear();
    }
  }

  // If a sequence node is not found, we just want to look for a regular geom
  // node.
  if (geom_node_path.is_empty()) {
    // Find the first GeomNode.
    if (!node_path.is_empty() && node_path.node()->get_type() != GeomNode::get_class_type()) {
      geom_node_path = node_path.find("**/+GeomNode");
      if (geom_node_path.is_empty()) {
        particlesystem_cat.error();
        return 0;
      }
    } else {
      geom_node_path = node_path;
    }

    // Grab the first texture.
    tex_col.add_texture(geom_node_path.find_texture("*"));
    if (tex_col.get_num_textures() < 1) {
      particlesystem_cat.error()
        << geom_node_path << " does not contain a texture.\n";
      return 0;
    } else {
      np_col.add_path(geom_node_path);
    }
  }
  return 1;
}

/**
 * If the source type is important, use this one.
 *
 * model and node should lead to node_path like this: node_path =
 * loader.loadModel(model).find(node)
 *
 * This will remove all previously add textures and resize the renderer to
 * match the new geometry.
 */
void SpriteParticleRenderer::
set_from_node(const NodePath &node_path, const std::string &model, const std::string &node, bool size_from_texels) {
  // Clear all texture information
  _anims.clear();
  add_from_node(node_path,model,node,size_from_texels,true);
}

/**
 * Sets the properties on this renderer from the geometry referenced by the
 * indicated NodePath.  This should be a reference to a GeomNode or a
 * SequenceNode; it extracts out the texture and UV range from the node.
 *
 * This will remove all previously added textures and animations.  It will
 * also resize the renderer to match this new geometry.
 *
 * If node_path refers to a GeomNode(or has one beneath it) the texture, its
 * size, and UV data will be extracted from that.
 *
 * If node_path references a SequenceNode(or has one beneath it) with multiple
 * GeomNodes beneath it, the size data will correspond only to the first
 * GeomNode found with a valid texture, while the texture and UV information
 * will be stored for each individual node.
 *
 * If size_from_texels is true, the particle size is based on the number of
 * texels in the source image; otherwise, it is based on the size of the first
 * polygon found in the node.
 *
 * model and node are the two items used to construct node_path.  If the
 * source type is important, use set_from_node(NodePath,string,string,bool)
 * instead.
 */
void SpriteParticleRenderer::
set_from_node(const NodePath &node_path, bool size_from_texels) {
  nassertv(!node_path.is_empty());
  // Clear all texture information
  _anims.clear();
  add_from_node(node_path,size_from_texels,true);
}

/**
 * This will allow the renderer to randomly choose from more than one texture
 * or sequence at particle birth.
 *
 * If the source type is important, use this one.
 *
 * model and node should lead to node_path like this: node_path =
 * loader.loadModel(model).find(node)
 *
 * If resize is true, or if there are no textures currently on the renderer,
 * it will force the renderer to use the size information from this node from
 * now on.  (Default is false)
 */
void SpriteParticleRenderer::
add_from_node(const NodePath &node_path, const std::string &model, const std::string &node, bool size_from_texels, bool resize) {
  int anim_count = _anims.size();
  if (anim_count == 0)
    resize = true;
  add_from_node(node_path,size_from_texels,resize);
  if (anim_count < (int)_anims.size()) {
    get_last_anim()->set_source_info(model,node);
  }
}

/**
 * This will allow the renderer to randomly choose from more than one texture
 * or sequence at particle birth.
 *
 * If resize is true, or if there are no textures currently on the renderer,
 * it will force the renderer to use the size information from this node from
 * now on.  (Default is false)
 */
void SpriteParticleRenderer::
add_from_node(const NodePath &node_path, bool size_from_texels, bool resize) {
  nassertv(!node_path.is_empty());

  NodePathCollection np_col;
  TextureCollection tex_col;

  if (_anims.empty())
    resize = true;

  // Load the found textures into the renderer.
  if (extract_textures_from_node(node_path,np_col,tex_col)) {
    pvector< LTexCoord > ll,ur;
    GeomNode *gnode = nullptr;
    const Geom *geom;
    const GeomPrimitive *primitive;

    for (int i = 0; i < np_col.get_num_paths(); ++i) {
      // Get the node from which we'll extract the geometry information.
      gnode = DCAST(GeomNode, np_col[i].node());

      // Now examine the UV's of the first Geom within the GeomNode.
      nassertv(gnode->get_num_geoms() > 0);
      geom = gnode->get_geom(0);

      bool got_texcoord = false;
      LTexCoord min_uv(0.0f, 0.0f);
      LTexCoord max_uv(0.0f, 0.0f);

      GeomVertexReader texcoord(geom->get_vertex_data(),
                                InternalName::get_texcoord());
      if (texcoord.has_column()) {
        for (size_t pi = 0; pi < geom->get_num_primitives(); ++pi) {
          primitive = geom->get_primitive(pi);
          for (int vi = 0; vi < primitive->get_num_vertices(); ++vi) {
            int vert = primitive->get_vertex(vi);
            texcoord.set_row_unsafe(vert);

            if (!got_texcoord) {
                min_uv = max_uv = texcoord.get_data2();
                got_texcoord = true;

            } else {
              const LVecBase2 &uv = texcoord.get_data2();

              min_uv[0] = min(min_uv[0], uv[0]);
              max_uv[0] = max(max_uv[0], uv[0]);
              min_uv[1] = min(min_uv[1], uv[1]);
              max_uv[1] = max(max_uv[1], uv[1]);
            }
          }
        }
      }

      if (got_texcoord) {
        // We don't really pay attention to orientation of UV's here; a minor
        // flaw.  We assume the minimum is in the lower-left, and the maximum
        // is in the upper-right.
        ll.push_back(min_uv);
        ur.push_back(max_uv);
      }
    }

    _anims.push_back(new SpriteAnim(tex_col,ll,ur));

    if (resize) {
      gnode = DCAST(GeomNode, np_col[0].node());
      geom = gnode->get_geom(0);

      bool got_vertex = false;
      LVertex min_xyz(0.0f, 0.0f, 0.0f);
      LVertex max_xyz(0.0f, 0.0f, 0.0f);

      GeomVertexReader vertex(geom->get_vertex_data(),
                              InternalName::get_vertex());
      if (vertex.has_column()) {
        for (size_t pi = 0; pi < geom->get_num_primitives(); ++pi) {
          primitive = geom->get_primitive(pi);
          for (int vi = 0; vi < primitive->get_num_vertices(); ++vi) {
            int vert = primitive->get_vertex(vi);
            vertex.set_row_unsafe(vert);

            if (!got_vertex) {
              min_xyz = max_xyz = vertex.get_data3();
              got_vertex = true;

            } else {
              const LVecBase3 &xyz = vertex.get_data3();

              min_xyz[0] = min(min_xyz[0], xyz[0]);
              max_xyz[0] = max(max_xyz[0], xyz[0]);
              min_xyz[1] = min(min_xyz[1], xyz[1]);
              max_xyz[1] = max(max_xyz[1], xyz[1]);
              min_xyz[2] = min(min_xyz[2], xyz[2]);
              max_xyz[2] = max(max_xyz[2], xyz[2]);
            }
          }
        }
      }

      if (got_vertex) {
        PN_stdfloat width = max_xyz[0] - min_xyz[0];
        PN_stdfloat height = max(max_xyz[1] - min_xyz[1],
                           max_xyz[2] - min_xyz[2]);

        if (size_from_texels) {
          // If size_from_texels is true, we get the particle size from the
          // number of texels in the source image.
          PN_stdfloat y_texels = _anims[0]->get_frame(0)->get_y_size() * fabs(_anims[0]->get_ur(0)[1] - _anims[0]->get_ll(0)[1]);
          set_size(y_texels * width / height, y_texels);
        } else {
          // If size_from_texels is false, we get the particle size from the
          // size of the polygon.
          set_size(width, height);
        }

      } else {
        // With no vertices, just punt.
        set_size(1.0f, 1.0f);
      }
    }
    init_geoms();
  }
}

/**
 * reallocate the vertex pool.
 */
void SpriteParticleRenderer::
resize_pool(int new_size) {
  if (new_size != _pool_size) {
    _pool_size = new_size;
    init_geoms();
  }
}

/**
 * initializes everything, called on traumatic events such as construction and
 * serious particlesystem modifications
 */
void SpriteParticleRenderer::
init_geoms() {
  CPT(RenderState) state = _render_state;
  SpriteAnim *anim;
  int anim_count = _anims.size();
  int i,j;

  // Setup format
  PT(GeomVertexArrayFormat) array_format = new GeomVertexArrayFormat
    (InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point,
     InternalName::get_color(), 1, Geom::NT_packed_dabc, Geom::C_color);

  if (_animate_theta || _theta != 0.0f) {
    array_format->add_column
      (InternalName::get_rotate(), 1, Geom::NT_stdfloat, Geom::C_other);
  }

  _base_y_scale = _initial_y_scale;
  _aspect_ratio = _width / _height;

  PN_stdfloat final_x_scale = _animate_x_ratio ? _final_x_scale : _initial_x_scale;
  PN_stdfloat final_y_scale = _animate_y_ratio ? _final_y_scale : _initial_y_scale;

  if (_animate_y_ratio) {
    _base_y_scale = max(_initial_y_scale, _final_y_scale);
    array_format->add_column
      (InternalName::get_size(), 1, Geom::NT_stdfloat, Geom::C_other);
  }

  if (_aspect_ratio * _initial_x_scale != _initial_y_scale ||
      _aspect_ratio * final_x_scale != final_y_scale) {
    array_format->add_column
      (InternalName::get_aspect_ratio(), 1, Geom::NT_stdfloat,
       Geom::C_other);
  }

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format
    (new GeomVertexFormat(array_format));

  // Reset render() data structures
  for (i = 0; i < (int)_ttl_count.size(); ++i) {
    PANDA_FREE_ARRAY(_ttl_count[i]);
  }
  _anim_size.resize(anim_count);
  _ttl_count.clear();
  _ttl_count.resize(anim_count);

  // Reset sprite primitive data in order to prepare for next pass.
  _sprite_primitive.clear();
  _sprites.clear();
  _vdata.clear();
  _sprite_writer.clear();

  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();

  // For each animation...
  for (i = 0; i < anim_count; ++i) {
    anim = _anims[i];
    _anim_size[i] = anim->get_num_frames();

    _sprite_primitive.push_back(pvector<PT(Geom)>());
    _sprites.push_back(pvector<PT(GeomPoints)>());
    _vdata.push_back(pvector<PT(GeomVertexData)>());
    _sprite_writer.push_back(pvector<SpriteWriter>());

    // For each frame of the animation...
    for (j = 0; j < _anim_size[i]; ++j) {
      _ttl_count[i] = (int *)PANDA_MALLOC_ARRAY(_anim_size[i] * sizeof(int));
      _vdata[i].push_back(new GeomVertexData("sprite_particles", format, Geom::UH_stream));
      PT(Geom) geom = new Geom(_vdata[i][j]);
      _sprite_primitive[i].push_back((Geom*)geom);
      _sprites[i].push_back(new GeomPoints(Geom::UH_stream));
      geom->add_primitive(_sprites[i][j]);

      // This will be overwritten in render(), but we had to have some initial
      // value
      _sprite_writer[i].push_back(SpriteWriter());

      state = state->add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_unchanged, _base_y_scale * _height, true));
      if (anim->get_frame(j) != nullptr) {
        state = state->add_attrib(TextureAttrib::make(anim->get_frame(j)));
        state = state->add_attrib(TexGenAttrib::make(TextureStage::get_default(), TexGenAttrib::M_point_sprite));

        // Build a transform to convert the texture coordinates to the ll, ur
        // space.
        LPoint2 ul(anim->get_ll(j)[0], anim->get_ur(j)[1]);
        LPoint2 lr(anim->get_ur(j)[0], anim->get_ll(j)[1]);
        LVector2 sc = lr - ul;

        CPT(TransformState) ts = TransformState::make_pos_rotate_scale2d(ul, 0.0f, sc);
        state = state->add_attrib(TexMatrixAttrib::make(TextureStage::get_default(), ts));
      }

      render_node->add_geom(_sprite_primitive[i][j], state);
    }
  }

  nassertv(render_node->check_valid());
}

/**
 * child birth, one of those 'there-if-we-want-it' things.  not really too
 * useful here, so it turns out we don't really want it.
 */
void SpriteParticleRenderer::
birth_particle(int index) {
  _birth_list.push_back(index);
}

/**
 * child death
 */
void SpriteParticleRenderer::
kill_particle(int) {
}

/**
 * big child render.  populates the geom node.
 */
void SpriteParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  PStatTimer t1(_render_collector);
  // There is no texture data available, exit.
  if (_anims.empty()) {
    return;
  }

  BaseParticle *cur_particle;
  int remaining_particles = ttl_particles;
  int i,j;                                  // loop counters
  int anim_count = _anims.size();           // number of animations
  int frame;                                // frame index, used in indicating which frame to use when not animated
  // First, since this is the only time we have access to the actual
  // particles, do some delayed initialization.
  if (_animate_frames || anim_count) {
    if (!_birth_list.empty()) {
      for (vector_int::iterator vIter = _birth_list.begin(); vIter != _birth_list.end(); ++vIter) {
        cur_particle = (BaseParticle*)po_vector[*vIter].p();
        i = int(NORMALIZED_RAND()*anim_count);

        // If there are multiple animations to choose from, choose one at
        // random for this new particle
        cur_particle->set_index(i < anim_count?i:i-1);

        // This is an experimental age offset so that the animations don't
        // appear synchronized.  If we are using animations, try to vary the
        // frame flipping a bit for particles in the same litter.  A similar
        // effect might be a achieved by using a small lifespan spread value
        // on the factory.

        // Perhaps we should look into other methods.  The age offset doesn't
        // seem to be cutting it.
        if (_animate_frames) {
          cur_particle->set_age(cur_particle->get_age()+i/10.0*cur_particle->get_lifespan());
        }
      }
    }
  }
  _birth_list.clear();

  // Create vertex writers for each of the possible geoms.  Could possibly be
  // changed to only create writers for geoms that would be used according to
  // the animation configuration.
  for (i = 0; i < anim_count; ++i) {
    for (j = 0; j < _anim_size[i]; ++j) {
      // Set the particle per frame counts to 0.
      memset(_ttl_count[i], 0, _anim_size[i]*sizeof(int));
      _sprite_writer[i][j].vertex = GeomVertexWriter(_vdata[i][j], InternalName::get_vertex());
      _sprite_writer[i][j].color = GeomVertexWriter(_vdata[i][j], InternalName::get_color());
      _sprite_writer[i][j].rotate = GeomVertexWriter(_vdata[i][j], InternalName::get_rotate());
      _sprite_writer[i][j].size = GeomVertexWriter(_vdata[i][j], InternalName::get_size());
      _sprite_writer[i][j].aspect_ratio = GeomVertexWriter(_vdata[i][j], InternalName::get_aspect_ratio());
    }
  }

  // init the aabb
  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through every filled slot
  for (i = 0; i < (int)po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (!cur_particle->get_alive()) {
      continue;
    }

    LPoint3 position = cur_particle->get_position();

    // x aabb adjust
    if (position[0] > _aabb_max[0])
      _aabb_max[0] = position[0];
    else if (position[0] < _aabb_min[0])
      _aabb_min[0] = position[0];

    // y aabb adjust
    if (position[1] > _aabb_max[1])
      _aabb_max[1] = position[1];
    else if (position[1] < _aabb_min[1])
      _aabb_min[1] = position[1];

    // z aabb adjust
    if (position[2] > _aabb_max[2])
      _aabb_max[2] = position[2];
    else if (position[2] < _aabb_min[2])
      _aabb_min[2] = position[2];


    PN_stdfloat t = cur_particle->get_parameterized_age();
    int anim_index = cur_particle->get_index();

    // If an animation has been removed, we need to reassign those particles
    // assigned to the removed animation.
    if(_animation_removed && (anim_index >= anim_count)) {
      anim_index = int(NORMALIZED_RAND()*anim_count);
      anim_index = anim_index<anim_count?anim_index:anim_index-1;
      cur_particle->set_index(anim_index);
    }

    // Find the frame
    if (_animate_frames) {
      if (_animate_frames_rate == 0.0f) {
        frame = (int)(t*_anim_size[anim_index]);
      } else {
        frame = (int)fmod(cur_particle->get_age()*_animate_frames_rate+1,_anim_size[anim_index]);
      }
    } else {
      frame = _animate_frames_index;
    }

    // Quick check make sure our math above didn't result in an invalid frame.
    frame = (frame < _anim_size[anim_index]) ? frame : (_anim_size[anim_index]-1);
    ++_ttl_count[anim_index][frame];

    // Calculate the color This is where we'll want to give the renderer the
    // new color
    LColor c = _color_interpolation_manager->generateColor(t);

    int alphamode=get_alpha_mode();
    if (alphamode != PR_ALPHA_NONE) {
      if (alphamode == PR_ALPHA_OUT)
        c[3] *= (1.0f - t) * get_user_alpha();
      else if (alphamode == PR_ALPHA_IN)
        c[3] *= t * get_user_alpha();
      else if (alphamode == PR_ALPHA_IN_OUT) {
        c[3] *= 2.0f * min(t, 1.0f - t) * get_user_alpha();
      }
      else {
        assert(alphamode == PR_ALPHA_USER);
        c[3] *= get_user_alpha();
      }
    }

    // Send the data on its way...
    _sprite_writer[anim_index][frame].vertex.add_data3(position);
    _sprite_writer[anim_index][frame].color.add_data4(c);

    PN_stdfloat current_x_scale = _initial_x_scale;
    PN_stdfloat current_y_scale = _initial_y_scale;

    if (_animate_x_ratio || _animate_y_ratio) {
      if (_blend_method == PP_BLEND_CUBIC) {
        t = CUBIC_T(t);
      }

      if (_animate_x_ratio) {
        current_x_scale = (_initial_x_scale +
                           (t * (_final_x_scale - _initial_x_scale)));
      }
      if (_animate_y_ratio) {
        current_y_scale = (_initial_y_scale +
                           (t * (_final_y_scale - _initial_y_scale)));
      }
    }

    if (_sprite_writer[anim_index][frame].size.has_column()) {
      _sprite_writer[anim_index][frame].size.add_data1f(current_y_scale * _height);
    }
    if (_sprite_writer[anim_index][frame].aspect_ratio.has_column()) {
      _sprite_writer[anim_index][frame].aspect_ratio.add_data1f(_aspect_ratio * current_x_scale / current_y_scale);
    }
    if (_animate_theta) {
      _sprite_writer[anim_index][frame].rotate.add_data1f(cur_particle->get_theta());
    } else if (_sprite_writer[anim_index][frame].rotate.has_column()) {
      _sprite_writer[anim_index][frame].rotate.add_data1f(_theta);
    }

    // maybe jump out early?
    remaining_particles--;
    if (remaining_particles == 0) {
      break;
    }
  }
  int n = 0;
  GeomNode *render_node = get_render_node();

  for (i = 0; i < anim_count; ++i) {
    for (j = 0; j < _anim_size[i]; ++j) {
      _sprites[i][j]->clear_vertices();
      _sprite_writer[i][j].clear();

      // We have to reassign the GeomVertexData and GeomPrimitive to the Geom,
      // and the Geom to the GeomNode, in case it got flattened away.
      _sprite_primitive[i][j]->set_primitive(0, _sprites[i][j]);
      _sprite_primitive[i][j]->set_vertex_data(_vdata[i][j]);

      render_node->set_geom(n, _sprite_primitive[i][j]);
      ++n;
    }
  }

  if (_animate_frames) {
    for (i = 0; i < anim_count; ++i) {
      for (j = 0; j < _anim_size[i]; ++j) {
        _sprites[i][j]->add_next_vertices(_ttl_count[i][j]);
      }
    }
  } else {
    for (i = 0; i < anim_count; ++i) {
      _sprites[i][_animate_frames_index]->add_next_vertices(_ttl_count[i][_animate_frames_index]);
    }
  }

  // done filling geompoint node, now do the bb stuff
  LPoint3 aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  PN_stdfloat radius = (aabb_center - _aabb_min).length();

  for (i = 0; i < anim_count; ++i) {
    for (j = 0; j < _anim_size[i]; ++j) {
      nassertv(_sprite_primitive[i][j]->check_valid());
      BoundingSphere sphere(aabb_center, radius);
      _sprite_primitive[i][j]->set_bounds(&sphere);
    }
  }

  get_render_node()->mark_internal_bounds_stale();
  nassertv(render_node->check_valid());
  _animation_removed = false;
}

/**
 * Write a string representation of this instance to <out>.
 */
void SpriteParticleRenderer::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SpriteParticleRenderer";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void SpriteParticleRenderer::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "SpriteParticleRenderer:\n";
  // indent(out, indent_level + 2) << "_sprite_primitive
  // "<<_sprite_primitive<<"\n";
  indent(out, indent_level + 2) << "_color "<<_color<<"\n";
  indent(out, indent_level + 2) << "_initial_x_scale "<<_initial_x_scale<<"\n";
  indent(out, indent_level + 2) << "_final_x_scale "<<_final_x_scale<<"\n";
  indent(out, indent_level + 2) << "_initial_y_scale "<<_initial_y_scale<<"\n";
  indent(out, indent_level + 2) << "_final_y_scale "<<_final_y_scale<<"\n";
  indent(out, indent_level + 2) << "_theta "<<_theta<<"\n";
  indent(out, indent_level + 2) << "_animate_x_ratio "<<_animate_x_ratio<<"\n";
  indent(out, indent_level + 2) << "_animate_y_ratio "<<_animate_y_ratio<<"\n";
  indent(out, indent_level + 2) << "_animate_theta "<<_animate_theta<<"\n";
  indent(out, indent_level + 2) << "_blend_method "<<_blend_method<<"\n";
  indent(out, indent_level + 2) << "_aabb_min "<<_aabb_min<<"\n";
  indent(out, indent_level + 2) << "_aabb_max "<<_aabb_max<<"\n";
  indent(out, indent_level + 2) << "_pool_size "<<_pool_size<<"\n";
  BaseParticleRenderer::write(out, indent_level + 2);
}
