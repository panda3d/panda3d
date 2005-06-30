// Filename: spriteParticleRenderer.cxx
// Created by:  charles (13Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "spriteParticleRenderer.h"
#include "boundingSphere.h"
#include "geomNode.h"
#include "nodePath.h"
#include "dcast.h"
#include "geom.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "renderModeAttrib.h"
#include "texMatrixAttrib.h"
#include "texGenAttrib.h"
#include "textureAttrib.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::SpriteParticleRenderer
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
SpriteParticleRenderer(Texture *tex) :
  BaseParticleRenderer(PR_ALPHA_NONE),
  _color(Colorf(1.0f, 1.0f, 1.0f, 1.0f)),
  _ll_uv(0.0f, 0.0f),
  _ur_uv(1.0f, 1.0f),
  _height(1.0f),
  _width(1.0f),
  _initial_x_scale(0.02f),
  _final_x_scale(0.02f),
  _initial_y_scale(0.02f),
  _final_y_scale(0.02f),
  _theta(0.0f),
  _base_y_scale(1.0f),
  _aspect_ratio(1.0f),
  _animate_x_ratio(false),
  _animate_y_ratio(false),
  _animate_theta(false),
  _alpha_disable(false),
  _blend_method(PP_BLEND_LINEAR),
  _color_interpolation_manager(new ColorInterpolationManager(_color)),
  _pool_size(0),
  _source_type(ST_texture)
{
  init_geoms();  
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::SpriteParticleRenderer
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
SpriteParticleRenderer(const SpriteParticleRenderer& copy) :
  BaseParticleRenderer(copy), _pool_size(0) {
  _texture = copy._texture;
  _animate_x_ratio = copy._animate_x_ratio;
  _animate_y_ratio = copy._animate_y_ratio;
  _animate_theta = copy._animate_theta;
  _alpha_disable = copy._alpha_disable;
  _blend_method = copy._blend_method;
  _ll_uv = copy._ll_uv;
  _ur_uv = copy._ur_uv;
  _initial_x_scale = copy._initial_x_scale;
  _final_x_scale = copy._final_x_scale;
  _initial_y_scale = copy._initial_y_scale;
  _final_y_scale = copy._final_y_scale;
  _theta = copy._theta;
  _color = copy._color;
  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::~SpriteParticleRenderer
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////
SpriteParticleRenderer::
~SpriteParticleRenderer() {
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::make_copy
//      Access : public
// Description : child dynamic copy
////////////////////////////////////////////////////////////////////
BaseParticleRenderer *SpriteParticleRenderer::
make_copy() {
  return new SpriteParticleRenderer(*this);
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::set_from_node
//      Access : public
// Description : Sets the properties on this render from the geometry
//               referenced by the indicated NodePath.  This should be
//               a reference to a GeomNode; it extracts out the
//               Texture and UV range from the GeomNode.
//
//               If size_from_texels is true, the particle size is
//               based on the number of texels in the source image;
//               otherwise, it is based on the size of the polygon
//               found in the GeomNode.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
set_from_node(const NodePath &node_path, bool size_from_texels) {
  nassertv(!node_path.is_empty());

  // The bottom node must be a GeomNode.  If it is not, find the first
  // GeomNode beneath it.
  NodePath geom_node_path = node_path;
  if (!geom_node_path.node()->is_geom_node()) {
    geom_node_path = node_path.find("**/+GeomNode");
    if (geom_node_path.is_empty()) {
      particlesystem_cat.error()
        << node_path << " does not contain a GeomNode.\n";
      return;
    }
  }
  GeomNode *gnode = DCAST(GeomNode, geom_node_path.node());

  // Get the texture off the node.  We'll take just the first texture.
  Texture *tex = geom_node_path.find_texture("*");

  if (tex == (Texture *)NULL) {
    particlesystem_cat.error()
      << geom_node_path << " has no texture.\n";
    return;
  }

  // Now examine the UV's of the first Geom within the GeomNode.
  nassertv(gnode->get_num_geoms() > 0);
  const Geom *geom = gnode->get_geom(0);

  bool got_texcoord = false;
  TexCoordf min_uv(0.0f, 0.0f);
  TexCoordf max_uv(0.0f, 0.0f);

  bool got_vertex = false;
  Vertexf min_xyz(0.0f, 0.0f, 0.0f);
  Vertexf max_xyz(0.0f, 0.0f, 0.0f);

  GeomVertexReader texcoord(geom->get_vertex_data(),
                              InternalName::get_texcoord());
  if (texcoord.has_column()) {
    for (int pi = 0; pi < geom->get_num_primitives(); ++pi) {
      const GeomPrimitive *primitive = geom->get_primitive(pi);
      for (int vi = 0; vi < primitive->get_num_vertices(); ++vi) {
        int vert = primitive->get_vertex(vi);
        texcoord.set_row(vert);
        
        if (!got_texcoord) {
          min_uv = max_uv = texcoord.get_data2f();
          got_texcoord = true;
          
        } else {
          const LVecBase2f &uv = texcoord.get_data2f();
          
          min_uv[0] = min(min_uv[0], uv[0]);
          max_uv[0] = max(max_uv[0], uv[0]);
          min_uv[1] = min(min_uv[1], uv[1]);
          max_uv[1] = max(max_uv[1], uv[1]);
        }
      }
    }
  }
  
  GeomVertexReader vertex(geom->get_vertex_data(),
                            InternalName::get_vertex());
  if (vertex.has_column()) {
    for (int pi = 0; pi < geom->get_num_primitives(); ++pi) {
      const GeomPrimitive *primitive = geom->get_primitive(pi);
      for (int vi = 0; vi < primitive->get_num_vertices(); ++vi) {
        int vert = primitive->get_vertex(vi);
        vertex.set_row(vert);
        
        if (!got_vertex) {
          min_xyz = max_xyz = vertex.get_data3f();
          got_vertex = true;
          
        } else {
          const LVecBase3f &xyz = vertex.get_data3f();
          
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

  _texture = tex;
  if (got_texcoord) {
    // We don't really pay attention to orientation of UV's here; a
    // minor flaw.  We assume the minimum is in the lower-left, and
    // the maximum is in the upper-right.
    set_ll_uv(min_uv);
    set_ur_uv(max_uv);
  }

  if (got_vertex) {
    float width = max_xyz[0] - min_xyz[0];
    float height = max(max_xyz[1] - min_xyz[1],
                       max_xyz[2] - min_xyz[2]);
    
    if (size_from_texels && got_texcoord) {
      // If size_from_texels is true, we get the particle size from the
      // number of texels in the source image.
      float y_texels = _texture->get_y_size() * fabs(_ur_uv[1] - _ll_uv[1]);
      set_size(y_texels * width / height, y_texels);
      
    } else {
      // If size_from_texels is false, we get the particle size from
      // the size of the polygon.
      set_size(width, height);
    }

  } else {
    // With no vertices, just punt.
    set_size(1.0f, 1.0f);
  }

  _source_type = ST_from_node;

  init_geoms();
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::resize_pool
//      Access : private
// Description : reallocate the vertex pool.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
resize_pool(int new_size) {
  if (new_size != _pool_size) {
    _pool_size = new_size;
    
    init_geoms();
  }
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::init_geoms
//      Access : private
// Description : initializes everything, called on traumatic events
//               such as construction and serious particlesystem
//               modifications
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
init_geoms() {
  CPT(RenderState) state = _render_state;

  PT(Geom) geom = new Geom; 
  _sprite_primitive = geom;
  
  PT(GeomVertexArrayFormat) array_format = new GeomVertexArrayFormat
    (InternalName::get_vertex(), 3, Geom::NT_float32, Geom::C_point,
     InternalName::get_color(), 1, Geom::NT_packed_dabc, Geom::C_color);
  
  if (_animate_theta || _theta != 0.0f) {
    array_format->add_column
      (InternalName::get_rotate(), 1, Geom::NT_float32, Geom::C_other);
  }
  
  _base_y_scale = _initial_y_scale;
  _aspect_ratio = _width / _height;
  
  float final_x_scale = _animate_x_ratio ? _final_x_scale : _initial_x_scale;
  float final_y_scale = _animate_y_ratio ? _final_y_scale : _initial_y_scale;
  
  if (_animate_y_ratio) {
    _base_y_scale = max(_initial_y_scale, _final_y_scale);
    array_format->add_column
      (InternalName::get_size(), 1, Geom::NT_float32, Geom::C_other);
  }
  
  if (_aspect_ratio * _initial_x_scale != _initial_y_scale ||
      _aspect_ratio * final_x_scale != final_y_scale) {
    array_format->add_column
      (InternalName::get_aspect_ratio(), 1, Geom::NT_float32,
       Geom::C_other);
  }
  
  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format
    (new GeomVertexFormat(array_format));
  
  _vdata = new GeomVertexData
    ("particles", format, Geom::UH_dynamic);
  geom->set_vertex_data(_vdata);
  _sprites = new GeomPoints(Geom::UH_dynamic);
  geom->add_primitive(_sprites);
  
  state = state->add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_unchanged, _base_y_scale * _height, true));
  
  if (_texture != (Texture *)NULL) {
    state = state->add_attrib(TextureAttrib::make(_texture));
    state = state->add_attrib(TexGenAttrib::make(TextureStage::get_default(), TexGenAttrib::M_point_sprite));
    
    // Build a matrix to convert the texture coordinates to the ul, lr
    // space.
    LPoint2f ul(_ll_uv[0], _ur_uv[1]);
    LPoint2f lr(_ur_uv[0], _ll_uv[1]);
    LVector2f sc = lr - ul;
    
    LMatrix4f mat
      (sc[0], 0.0f, 0.0f, 0.0f,
       0.0f, sc[1], 0.0f, 0.0f,
       0.0f, 0.0f,  1.0f, 0.0f,
       ul[0], ul[1], 0.0f, 1.0f);
    state = state->add_attrib(TexMatrixAttrib::make(mat));
  }

  GeomNode *render_node = get_render_node();
  render_node->remove_all_geoms();
  render_node->add_geom(_sprite_primitive, state);
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::birth_particle
//      Access : private
// Description : child birth, one of those 'there-if-we-want-it'
//               things.  not really too useful here, so it turns
//               out we don't really want it.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
birth_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::kill_particle
//      Access : private
// Description : child death
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
kill_particle(int) {
}

////////////////////////////////////////////////////////////////////
//    Function : SpriteParticleRenderer::render
//      Access : private
// Description : big child render.  populates the geom node.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
render(pvector< PT(PhysicsObject) >& po_vector, int ttl_particles) {
  BaseParticle *cur_particle;

  int remaining_particles = ttl_particles;
  int i;

  GeomVertexWriter vertex(_vdata, InternalName::get_vertex());
  GeomVertexWriter color(_vdata, InternalName::get_color());
  GeomVertexWriter rotate(_vdata, InternalName::get_rotate());
  GeomVertexWriter size(_vdata, InternalName::get_size());
  GeomVertexWriter aspect_ratio(_vdata, InternalName::get_aspect_ratio());

  // init the aabb
  _aabb_min.set(99999.0f, 99999.0f, 99999.0f);
  _aabb_max.set(-99999.0f, -99999.0f, -99999.0f);

  // run through every filled slot
  for (i = 0; i < (int)po_vector.size(); i++) {
    cur_particle = (BaseParticle *) po_vector[i].p();

    if (!cur_particle->get_alive())
      continue;

    LPoint3f position = cur_particle->get_position();

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


    float t = cur_particle->get_parameterized_age();

    // Calculate the color
    // This is where we'll want to give the renderer the new color
    //Colorf c = _color;
    Colorf c = _color_interpolation_manager->generateColor(t);
    

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
    
    vertex.add_data3f(position);
    color.add_data4f(c);

    float current_x_scale = _initial_x_scale;
    float current_y_scale = _initial_y_scale;
    
    if (_animate_x_ratio || _animate_y_ratio) {
      float t = cur_particle->get_parameterized_age();
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
    
    if (size.has_column()) {
      size.add_data1f(current_y_scale * _height);
    }
    if (aspect_ratio.has_column()) {
      aspect_ratio.add_data1f(_aspect_ratio * current_x_scale / current_y_scale);
    }
    
    if (_animate_theta) {
      rotate.add_data1f(cur_particle->get_theta());
    } else if (rotate.has_column()) {
      rotate.add_data1f(_theta);
    }

    // maybe jump out early?
    remaining_particles--;
    if (remaining_particles == 0) {
      break;
    }
  }

  _sprites->clear_vertices();
  _sprites->add_next_vertices(ttl_particles);

  // done filling geompoint node, now do the bb stuff
  LPoint3f aabb_center = _aabb_min + ((_aabb_max - _aabb_min) * 0.5f);
  float radius = (aabb_center - _aabb_min).length();

  _sprite_primitive->set_bound(BoundingSphere(aabb_center, radius));
  get_render_node()->mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function : output
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
output(ostream &out) const {
  #ifndef NDEBUG //[
  out<<"SpriteParticleRenderer";
  #endif //] NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void SpriteParticleRenderer::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << "SpriteParticleRenderer:\n";
  indent(out, indent_level + 2) << "_sprite_primitive "<<_sprite_primitive<<"\n";
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
  indent(out, indent_level + 2) << "_source_type "<<_source_type<<"\n";
  BaseParticleRenderer::write(out, indent_level + 2);
}
