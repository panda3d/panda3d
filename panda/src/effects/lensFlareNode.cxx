// Filename: lensFlareNode.cxx
// Created by:  jason (18Jul00)
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

#if 0  // temporarily disabled until we can port to new scene graph.

#include "lensFlareNode.h"
#include "config_effects.h"

#include "sequenceNode.h"
#include "geomNode.h"
#include "geomSprite.h"
#include "textureTransition.h"
#include "transformTransition.h"
#include "billboardTransition.h"
#include "transformTransition.h"
#include "transparencyTransition.h"
#include "renderTraverser.h"
#include "lens.h"
#include "get_rel_pos.h"
#include "clockObject.h"
#include "allTransitionsWrapper.h"
#include "allTransitionsWrapper.h"
#include "graphicsStateGuardian.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramFloat.h"
#include "ioPtaDatagramLinMath.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle LensFlareNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::add_bloom
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
add_flare(PT(Texture) flare, PTA_float scales, PTA_float offsets,
          PTA_float angle_scales, PTA_Colorf colors)
{
  nassertv(scales.size() == offsets.size());
  nassertv(colors.size() == offsets.size());
  nassertv(angle_scales.size() == offsets.size());

  _flare_scales.push_back(scales);
  _flare_offsets.push_back(offsets);
  _flare_colors.push_back(colors);
  _flare_angle_scales.push_back(angle_scales);
  _flares.push_back(flare);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::add_blind
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
add_blind(PT(Texture) blind)
{
  _blind = blind;
  GeomSprite *sprite = new GeomSprite();
  GeomNode *node = new GeomNode();

  //We don't want to set any geometry right now, as that will be
  //taken care of later (and on each subsequent render), but
  //Geoms requires a certain amount of info or else they crash,
  //so simply give it the minimum it needs not to crash

  //The lengths and number of prims will never change, so give
  //it valid values for those, but pass it an empty array of
  //vertices
  PTA_Vertexf coords=PTA_Vertexf::empty_array(0);
  PTA_float tex_scales=PTA_float::empty_array(0);

  tex_scales.push_back(_texel_scale);

  sprite->set_coords(coords);
  sprite->set_num_prims(1);
  sprite->set_texture(_blind);

  node->add_geom(sprite);

  _blind_arc = new RenderRelation(this, node);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_geometry
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
set_geometry(GeomSprite *sprite, const PTA_float &geom_scales,
             const PTA_float &geom_offsets, const PTA_float &geom_angle_scales,
             const PTA_Colorf &geom_colors, const LVector3f &delta,
             const LPoint3f &light, const float &angle)
{

  PTA_Vertexf coords=PTA_Vertexf::empty_array(0);
  PTA_float tex_scales=PTA_float::empty_array(0);
  PTA_Colorf colors=PTA_Colorf::empty_array(0);

  //Sanity check
  nassertv(geom_scales.size() == geom_offsets.size());
  nassertv(geom_colors.size() == geom_offsets.size());

  float world_scale = _texel_scale * _global_scale;
  for(int i = 0; i < (int)geom_scales.size(); i++)
  {
    LVector3f position = (delta * geom_offsets[i]) + light;
    float view_scale;
    //If this is true then we are supposed to invert the meaning of
    //the scale. I.E.  As the angle between the viewing direction and
    //the light direction increases we want the size of the thing to
    //get smaller and as it increases we want it to get bigger.  This
    //will actually be the normal use of angle scales
    if (geom_angle_scales[i] < 0)
    {
      view_scale = (1.0f-pow(angle, 15.0f)) * -geom_angle_scales[i];
    }
    else
    {
      view_scale = pow(angle, 15.0f) * geom_angle_scales[i];
    }
    float offset = (angle - 1.0f) / _flare_fall_off;
    offset = (offset < 0.0f) ? 0.0f : ((offset > 1.0f) ? 1.0f : offset);
    float r = geom_colors[i][0] - offset;
    float g = geom_colors[i][1] - offset;
    float b = geom_colors[i][2] - offset;
    r = (r < 0.0f) ? 0.0f : r;
    g = (g < 0.0f) ? 0.0f : g;
    b = (b < 0.0f) ? 0.0f : b;

    coords.push_back(position); tex_scales.push_back(geom_scales[i] * (world_scale + view_scale));
    colors.push_back(Colorf(r, g, b, 1));
  }

  sprite->set_coords(coords);
  sprite->set_x_texel_ratio(tex_scales, G_PER_PRIM);
  sprite->set_y_texel_ratio(tex_scales, G_PER_PRIM);
  sprite->set_colors(colors, G_PER_PRIM);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::prepare_flares
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
prepare_flares(const LVector3f &delta, const LPoint3f &light, const float &angle)
{
  if (_flares.size() > 0)
  {
    if (_flares.size() > _flare_arcs.size())
    {
      for(int i = _flare_arcs.size(); i < (int)_flares.size(); i++)
      {
        //Sanity check
        nassertv(_flare_offsets[i].size() == _flare_scales[i].size());

        GeomSprite *sprite = new GeomSprite();
        GeomNode *node = new GeomNode();

        //We don't want to set any geometry right now, as that will be
        //taken care of later (and on each subsequent render), but
        //Geoms requires a certain amount of info or else they crash,
        //so simply give it the minimum it needs not to crash

        //The lengths and number of prims will never change, so give
        //it valid values for those, but pass it an empty array of
        //vertices
        PTA_Vertexf coords=PTA_Vertexf::empty_array(0);

        sprite->set_coords(coords);
        sprite->set_num_prims(_flare_offsets[i].size());

        node->add_geom(sprite);

        RenderRelation *arc = new RenderRelation(this, node);
        //arc->set_transition(new TransparencyTransition(TransparencyProperty::M_alpha));
        _flare_arcs.push_back(arc);
      }
    }

    for(int i = 0; i < (int)_flares.size(); i++)
    {
      GeomNode *node = DCAST(GeomNode, _flare_arcs[i]->get_child());
      GeomSprite *sprite = DCAST(GeomSprite, node->get_geom(0));

      set_geometry(sprite, _flare_scales[i], _flare_offsets[i],
                   _flare_angle_scales[i], _flare_colors[i],
                   delta, light, angle);
      sprite->set_texture(_flares[i]);

      //Tell them to recompute their bounding volumes
      sprite->mark_bound_stale();
      node->mark_bound_stale();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::prepare_blind
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
prepare_blind(const float &angle, const float &tnear)
{
  if (_blind != (Texture*) NULL)
  {
    GeomNode *node = DCAST(GeomNode, _blind_arc->get_child());
    GeomSprite *sprite = DCAST(GeomSprite, node->get_geom(0));

    float offset = (angle - 1.0f) / _blind_fall_off;
    //Make sure that it always blends some
    offset = (offset < 0.3f) ? 0.3f : ((offset > 1.0f) ? 1.0f : offset);

    PTA_Vertexf coords=PTA_Vertexf::empty_array(0);
    PTA_Colorf colors=PTA_Colorf::empty_array(0);
    PTA_float x_tex_scales=PTA_float::empty_array(0);
    PTA_float y_tex_scales=PTA_float::empty_array(0);

    //The height and the width are set to two as sprites are always
    //drawn in a frustum of size 2.
    float width = sprite->get_frustum_right() - sprite->get_frustum_left();
    float height = sprite->get_frustum_top() - sprite->get_frustum_bottom();
    float x_offset_scale = width / _blind->_pbuffer->get_xsize();
    float y_offset_scale = height / _blind->_pbuffer->get_ysize();

    float inten = 1.0f - offset;

    coords.push_back(Vertexf(0.0f, 0.0f, -tnear ));
    colors.push_back(Colorf(inten,inten,inten,1.0f));
    x_tex_scales.push_back(x_offset_scale); y_tex_scales.push_back(y_offset_scale);

    sprite->set_x_texel_ratio(x_tex_scales, G_PER_PRIM);
    sprite->set_y_texel_ratio(y_tex_scales, G_PER_PRIM);
    sprite->set_coords(coords);
    sprite->set_colors(colors, G_PER_PRIM);

    //Tell it to recompute it's bounding volume
    sprite->mark_bound_stale();
    node->mark_bound_stale();
  }
}
////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::render_child
//       Access: Pribate
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
render_child(RenderRelation *arc, const AllTransitionsWrapper &trans,
             GraphicsStateGuardian *gsg)
{

  AllTransitionsWrapper new_trans(trans);
  new_trans.clear_transition(TransformTransition::get_class_type());

  AllTransitionsWrapper arc_trans;
  arc_trans.extract_from(arc);

  new_trans.compose_in_place(arc_trans);

  // Now render everything from this node and below.
  gsg->render_subgraph(gsg->get_render_traverser(),
                       arc->get_child(), new_trans);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::render_children
//       Access: Pribate
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
render_children(const vector_relation &arcs, 
                const AllTransitionsWrapper &trans,
                GraphicsStateGuardian *gsg)
{
  for(int i = 0; i < (int)arcs.size(); i++)
  {
    render_child(arcs[i], trans, gsg);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::sub_render
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool LensFlareNode::
sub_render(const AllTransitionsWrapper &input_trans,
           AllTransitionsWrapper &, RenderTraverser *trav) {
  GraphicsStateGuardian *gsg = trav->get_gsg();

  nassertr(_light_node != (Node*) NULL, false);

  //First we need the light position
  LensNode *camera_node = gsg->get_current_camera();
  Lens *lens = camera_node->get_lens();

  LPoint3f light_pos = get_rel_pos(_light_node, camera_node);

  LMatrix4f light_mat;
  get_rel_mat(_light_node, camera_node, light_mat);

  LMatrix4f modelview_mat;

  const TransformTransition *ta;
  if (!get_transition_into(ta, input_trans))
    modelview_mat = LMatrix4f::ident_mat();
  else
    modelview_mat = ta->get_matrix();

  LMatrix4f inv_light_mat = invert(light_mat);
  light_pos = light_pos * inv_light_mat * modelview_mat;

  //Now figure out where the center of the screen is.  Since we are
  //doing everything in camera space, this should merely be the
  //distance between the camera and the near clipping plane projected
  //along Y into the screen
  LPoint3f center = LPoint3f::origin() + LPoint3f::rfu(0,lens->get_near(),0);
  center = center * inv_light_mat * modelview_mat;

  //Now lets get the vector from the light to the center.
  LPoint3f delta = center - light_pos;
  delta.set_z(light_pos.get_z());

  //Now perform the angle caclulationss for increasing the brightness
  //as we look at the light dead on
  LPoint3f origin = LPoint3f::origin() * inv_light_mat * modelview_mat;
  LVector3f light_dir = light_pos - origin;
  light_dir.normalize();
  LVector3f view_dir = center - origin;

  float dot = view_dir.dot(light_dir);
  dot = (dot < 0.0f) ? -dot : dot;

  prepare_flares(delta, light_pos, dot);
  prepare_blind(dot, lens->get_near());

  render_children(_flare_arcs, input_trans, gsg);
  render_child(_blind_arc, input_trans, gsg);

  //Short circuit the rendering
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool LensFlareNode::
has_sub_render() const
{
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::write_object
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LensFlareNode::
write_datagram(BamWriter *manager, Datagram &me) {
  int i;

  Node::write_datagram(manager, me);

  me.add_uint16(_flares.size());
  for(i = 0; i < (int)_flares.size(); i++)
  {
    manager->write_pointer(me, _flares[i]);
  }
  manager->write_pointer(me, _blind);

  me.add_uint16(_flare_arcs.size());
  for(i = 0; i < (int)_flare_arcs.size(); i++)
  {
    manager->write_pointer(me, _flare_arcs[i]);
  }

  me.add_uint16(_flare_scales.size());
  for(i = 0; i < (int)_flare_scales.size(); i++)
  {
    WRITE_PTA(manager, me, IPD_float::write_datagram, _flare_scales[i])
  }

  me.add_uint16(_flare_angle_scales.size());
  for(i = 0; i < (int)_flare_angle_scales.size(); i++)
  {
    WRITE_PTA(manager, me, IPD_float::write_datagram, _flare_angle_scales[i])
  }

  me.add_uint16(_flare_offsets.size());
  for(i = 0; i < (int)_flare_offsets.size(); i++)
  {
    WRITE_PTA(manager, me, IPD_float::write_datagram, _flare_offsets[i])
  }

  me.add_uint16(_flare_colors.size());
  for(i = 0; i < (int)_flare_colors.size(); i++)
  {
    WRITE_PTA(manager, me, IPD_Colorf::write_datagram, _flare_colors[i])
  }

  me.add_float32(_global_scale);
  me.add_float32(_texel_scale);
  me.add_float32(_blind_fall_off);
  me.add_float32(_flare_fall_off);

  manager->write_pointer(me, _light_node);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_LensFlareNode to
//               read in all of the relevant data from the BamFile for
//               the new LensFlareNode.
////////////////////////////////////////////////////////////////////
void LensFlareNode::
fillin(DatagramIterator &scan, BamReader *manager)
{
  int i, size;
  Node::fillin(scan, manager);

  Node::fillin(scan, manager);

  _num_flares = scan.get_uint16();
  for(i = 0; i < _num_flares; i++)
  {
    manager->read_pointer(scan);
  }
  manager->read_pointer(scan);

  _num_arcs = scan.get_uint16();
  for(i = 0; i < _num_arcs; i++)
  {
    manager->read_pointer(scan);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++)
  {
    PTA_float temp=PTA_float::empty_array(0);
    READ_PTA(manager, scan, IPD_float::read_datagram, temp)
    _flare_scales.push_back(temp);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++)
  {
    PTA_float temp=PTA_float::empty_array(0);
    READ_PTA(manager, scan, IPD_float::read_datagram, temp)
    _flare_angle_scales.push_back(temp);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++)
  {
    PTA_float temp=PTA_float::empty_array(0);
    READ_PTA(manager, scan, IPD_float::read_datagram, temp)
    _flare_offsets.push_back(temp);
  }

  size = scan.get_uint16();
  for(i = 0; i < size; i++)
  {
    PTA_Colorf temp=PTA_Colorf::empty_array(0);
    READ_PTA(manager, scan, IPD_Colorf::read_datagram, temp)
    _flare_colors.push_back(temp);
  }

  _global_scale = scan.get_float32();
  _texel_scale = scan.get_float32();
  _blind_fall_off = scan.get_float32();
  _flare_fall_off = scan.get_float32();

  manager->read_pointer(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int LensFlareNode::
complete_pointers(TypedWritable **p_list, BamReader *manager)
{
  int i;
  int start = Node::complete_pointers(p_list, manager);
  int end = _num_flares + start;

  for(i = start; i < end; i++)
  {
    _flares.push_back(DCAST(Texture, p_list[i]));
  }

  _blind = DCAST(Texture, p_list[end]);

  end += 1 + _num_arcs;
  for(; i < end; i++)
  {
    _flare_arcs.push_back(DCAST(RenderRelation, p_list[i]));
  }

  _light_node = DCAST(Node, p_list[end]);

  return end+1;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::make_LensFlareNode
//       Access: Protected
//  Description: This function is called by the BamReader's factory
//               when a new object of type LensFlareNode is encountered in
//               the Bam file.  It should create the LensFlareNode and
//               extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *LensFlareNode::
make_LensFlareNode(const FactoryParams &params) {
  LensFlareNode *me = new LensFlareNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LensFlareNode.
////////////////////////////////////////////////////////////////////
void LensFlareNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_LensFlareNode);
}


/***************
 OLD SPARKLE CODE

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LensFlareNode::
LensFlareNode(void) :
  _global_scale(1), _next_switch(-1), _sparkle_fps(0.2),
  _texel_scale(0.1), _inv_sparkle_fps(5), _exp_scale(15)
{
  _global_clock = ClockObject::get_global_clock();
  _next_switch =  _global_clock->get_real_time() + _sparkle_fps;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_sparkle_fps
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
set_sparkle_fps(float fps)
{
  nassertv(fps > 0);
  _next_switch = _next_switch - _sparkle_fps + fps;
  _sparkle_fps = fps;
  _inv_sparkle_fps = 1. / _sparkle_fps;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::compute_current
//       Access: Private
//  Description: Determines the current sparkle index
////////////////////////////////////////////////////////////////////
int LensFlareNode::
compute_current(int &current_sparkle, vector_texture sparkles)
{
  double current_time = _global_clock->get_real_time();
  unsigned int increment = (unsigned int) ((current_time - _next_switch) * _inv_sparkle_fps);

  _next_switch += _sparkle_fps * increment;
  current_sparkle = (current_sparkle + increment) % sparkles.size();

  return current_sparkle;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::add_sparkle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
add_sparkle(PT_Node source, PT(Texture) sparkle)
{
  set_light(source);
  _sparkles.push_back(sparkle);
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_sparkles_attributes
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
set_sparkles_attributes(PT_Node source, vector_float scales,
                        vector_float offsets, vector_Colorf colors)
{
  nassertv(scales.size() == offsets.size());

  set_light(source);

  _sparkle_scales = scales;
  _sparkle_offsets = offsets;
  _sparkle_colors = colors;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_light
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
set_light(PT_Node light)
{
 _lights.insert(light);
 if (_current_sparkles.find(light) == _current_sparkles.end())
 {
   _current_sparkles[light] = 0;
 }
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::prepare_sparkles
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void LensFlareNode::
prepare_sparkles(vector_relation &arcs, const vector_texture &sparkles,
                 const vector_float &scales, const vector_float &offsets,
                 const vector_Colorf &colors, const LVector3f &delta,
                 const LPoint3f &light, const BoundingVolume &bound, int &old_sparkle)
{
  //Sanity check
  nassertv(scales.size() == offsets.size());

  if (scales.size() > 0)
  {
    if (arcs.size() == 0)
    {
      for(int i = 0; i < scales.size(); i++)
      {
        GeomSprite *sprite = new GeomSprite();
        GeomNode *node = new GeomNode();

        //We don't want to set any geometry right now, as that will be
        //taken care of later (and on each subsequent render), but
        //Geoms requires a certain amount of info or else they crash,
        //so simply give it the minimum it needs not to crash

        //The lengths and number of prims will never change, so give
        //it valid values for those, but pass it an empty array of
        //vertices
        PTA_Vertexf coords(0);

        sprite->set_coords(coords);
        sprite->set_num_prims(1);

        node->add_geom(sprite);

        arcs.push_back(new RenderRelation(this, node));
      }
    }

    //Unfortunately, we can't use set_geometry here because only a
    //certain number of sparkles are active at once, and set_geometry
    //knows nothing about switching between them

    for(int i = 0; i < scales.size(); i++)
    {
      int index = (compute_current(old_sparkle, sparkles)+i) % sparkles.size();
      LVector3f position = (delta * offsets[i]) + light;

      GeomNode *node = DCAST(GeomNode, arcs[i]->get_child());
      GeomSprite *sprite = DCAST(GeomSprite, node->get_geom(0));

      PTA_Vertexf coords(0);
      PTA_float tex_scales(0);
      PTA_Colorf sprite_colors(0);

      coords.push_back(position); tex_scales.push_back(scales[i] * _texel_scale * _global_scale);

      sprite_colors.push_back(colors[i]);

      sprite->set_coords(coords);
      sprite->set_x_texel_ratio(tex_scales, G_PER_PRIM);
      sprite->set_y_texel_ratio(tex_scales, G_PER_PRIM);
      sprite->set_colors(sprite_colors, G_PER_PRIM);
      sprite->set_texture(sparkles[index]);

      //Tell them to recompute their bounding volumes
      sprite->set_bound(bound);
      sprite->mark_bound_stale();
      node->mark_bound_stale();
    }
  }
}

****************/

#endif  // temporarily disabled until we can port to new scene graph.
