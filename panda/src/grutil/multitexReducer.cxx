// Filename: multitexReducer.cxx
// Created by:  drose (30Nov04)
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

#include "multitexReducer.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "geom.h"
#include "renderState.h"
#include "transformState.h"
#include "graphicsOutput.h"
#include "graphicsChannel.h"
#include "graphicsLayer.h"
#include "displayRegion.h"
#include "nodePath.h"
#include "camera.h"
#include "orthographicLens.h"
#include "cardMaker.h"
#include "colorBlendAttrib.h"
#include "config_grutil.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
MultitexReducer::
MultitexReducer() {
  _target_stage = TextureStage::get_default();
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
MultitexReducer::
~MultitexReducer() {
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::clear
//       Access: Published
//  Description: Removes the record of nodes that were previously
//               discovered by scan().
////////////////////////////////////////////////////////////////////
void MultitexReducer::
clear() {
  _stages.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::scan
//       Access: Published
//  Description: Starts scanning the hierarchy beginning at the
//               indicated node.  Any GeomNodes discovered in the
//               hierarchy with multitexture will be added to internal
//               structures in the MultitexReducer so that a future
//               call to flatten() will operate on all of these at
//               once.
//
//               The indicated transform and state are the state
//               inherited from the node's ancestors; any multitexture
//               operations will be accumulated from the indicated
//               starting state.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
scan(PandaNode *node, const RenderState *state, const TransformState *transform) {
  CPT(RenderState) next_state = node->get_state()->compose(state);
  CPT(TransformState) next_transform = node->get_transform()->compose(transform);

  if (node->is_geom_node()) {
    scan_geom_node(DCAST(GeomNode, node), next_state, next_transform);
  }

  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();
  for (int i = 0; i < num_children; i++) {
    scan(cr.get_child(i), next_state, next_transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::set_target
//       Access: Published
//  Description: Specifies the target TextureStage (and TexCoordName)
//               that will be left on each multitexture node after the
//               flatten operation has completed.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
set_target(TextureStage *stage) {
  _target_stage = stage;
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::flatten
//       Access: Published
//  Description: Actually performs the reducing operations on the
//               nodes that were previously scanned.
//
//               A window that can be used to create texture buffers
//               suitable for rendering this geometry must be
//               supplied.  This specifies the particular GSG that
//               will be used to composite the textures.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
flatten(GraphicsOutput *window) {
  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Beginning flatten operation\n";
    Stages::const_iterator mi;
    for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
      const StageList &stage_list = (*mi).first;
      const GeomList &geom_list = (*mi).second;
      grutil_cat.debug(false)
        << "stage_list for:";
      for (GeomList::const_iterator gi = geom_list.begin();
           gi != geom_list.end(); 
           ++gi) {
        const GeomInfo &geom_info = (*gi);
        grutil_cat.debug(false)
          << " (" << geom_info._geom_node->get_name() << " g" 
          << geom_info._index << ")";
      }
      grutil_cat.debug(false) << ":\n";

      StageList::const_iterator si;
      for (si = stage_list.begin(); si != stage_list.end(); ++si) {
        const StageInfo &stage_info = (*si);
        grutil_cat.debug(false)
          << "  " << *stage_info._stage << " " << *stage_info._tex
          << " " << *stage_info._tex_mat << "\n";
      }
    }
  }

  Stages::const_iterator mi;
  for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
    const StageList &stage_list = (*mi).first;
    const GeomList &geom_list = (*mi).second;

    // Create an offscreen buffer in which to render the new texture.
    int x_size, y_size, aniso_degree;
    Texture::FilterType minfilter, magfilter;
    determine_size(x_size, y_size, aniso_degree,
                   minfilter, magfilter, stage_list);

    GraphicsOutput *buffer = 
      window->make_texture_buffer("multitex", x_size, y_size);
    buffer->set_one_shot(true);
    Texture *tex = buffer->get_texture();
    tex->set_anisotropic_degree(aniso_degree);
    tex->set_minfilter(minfilter);
    tex->set_magfilter(magfilter);

    // Set up the offscreen buffer to render 0,0 to 1,1.  This will be
    // the whole texture, but nothing outside the texture.
    GraphicsChannel *chan = buffer->get_channel(0);
    GraphicsLayer *layer = chan->make_layer(0);
    DisplayRegion *dr = layer->make_display_region();
    PT(Camera) cam_node = new Camera("multitexCam");
    PT(Lens) lens = new OrthographicLens();
    lens->set_film_size(1.0f, 1.0f);
    lens->set_film_offset(0.5f, 0.5f);
    lens->set_near_far(-1000.0f, 1000.0f);
    cam_node->set_lens(lens);

    // Create a root node for the buffer's scene graph, and set up
    // some appropriate properties for it.
    NodePath render("buffer");
    cam_node->set_scene(render);
    render.set_bin("unsorted", 0);
    render.set_depth_test(false);
    render.set_depth_write(false);

    NodePath cam = render.attach_new_node(cam_node);
    dr->set_camera(cam);

    // Put one plain white card in the background for the first
    // texture layer to apply onto.
    CardMaker cm("background");
    cm.set_frame(0.0f, 1.0f, 0.0f, 1.0f);
    render.attach_new_node(cm.generate());

    StageList::const_iterator si;
    for (si = stage_list.begin(); si != stage_list.end(); ++si) {
      const StageInfo &stage_info = (*si);

      make_texture_layer(render, stage_info, geom_list);
    }

    // Now modify the geometry to apply the new texture, instead of
    // the old multitexture.
    CPT(RenderAttrib) new_ta = DCAST(TextureAttrib, TextureAttrib::make())->
      add_on_stage(_target_stage, tex);

    GeomList::const_iterator gi;
    for (gi = geom_list.begin(); gi != geom_list.end(); ++gi) {
      const GeomInfo &geom_info = (*gi);
      
      CPT(RenderState) geom_state = 
        geom_info._geom_node->get_geom_state(geom_info._index);
      geom_state = geom_state->add_attrib(new_ta);
      geom_info._geom_node->set_geom_state(geom_info._index, geom_state);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::scan_geom_node
//       Access: Private
//  Description: Adds the Geoms in the indicated GeomNode to the
//               internal database of multitexture elements.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
scan_geom_node(GeomNode *node, const RenderState *state, 
               const TransformState *transform) {
  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "scan_geom_node(" << *node << ", " << *state << ", "
      << *transform << ")\n";
  }

  int num_geoms = node->get_num_geoms();
  for (int gi = 0; gi < num_geoms; gi++) {
    CPT(RenderState) geom_net_state = 
      node->get_geom_state(gi)->compose(state);

    // Get out the net TextureAttrib and TexMatrixAttrib from the state.
    const RenderAttrib *attrib;
    const TextureAttrib *ta = NULL;

    attrib = geom_net_state->get_attrib(TextureAttrib::get_class_type());
    if (attrib != (const RenderAttrib *)NULL) {
      ta = DCAST(TextureAttrib, attrib);
    }

    if (ta != (TextureAttrib *)NULL && ta->get_num_on_stages() >= 2) {
      // Ok, we have multitexture.  Record the Geom.
      CPT(TexMatrixAttrib) tma = DCAST(TexMatrixAttrib, TexMatrixAttrib::make());
      attrib = geom_net_state->get_attrib(TexMatrixAttrib::get_class_type());
      if (attrib != (const RenderAttrib *)NULL) {
        tma = DCAST(TexMatrixAttrib, attrib);
      }

      StageList stage_list;
      
      int num_stages = ta->get_num_on_stages();
      for (int si = 0; si < num_stages; si++) {
        TextureStage *stage = ta->get_on_stage(si);
        
        stage_list.push_back(StageInfo(stage, ta, tma));
      }

      record_stage_list(stage_list, GeomInfo(node, gi));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::record_stage_list
//       Access: Private
//  Description: Adds the record of this one Geom and its associated
//               StageList.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
record_stage_list(const MultitexReducer::StageList &stage_list, 
                  const MultitexReducer::GeomInfo &geom_info) {
  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "record_stage_list for " << geom_info._geom_node->get_name() << " g" 
      << geom_info._index << ":\n";
    StageList::const_iterator si;
    for (si = stage_list.begin(); si != stage_list.end(); ++si) {
      const StageInfo &stage_info = (*si);
      grutil_cat.debug(false)
        << "  " << *stage_info._stage << " " << *stage_info._tex
        << " " << *stage_info._tex_mat << "\n";
    }
  }

  _stages[stage_list].push_back(geom_info);
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::determine_size
//       Access: Private
//  Description: Tries to guess what size to make the new, collapsed
//               texture based on the sizes of all of the textures
//               used in the stage_list.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
determine_size(int &x_size, int &y_size, int &aniso_degree,
               Texture::FilterType &minfilter, Texture::FilterType &magfilter, 
               const MultitexReducer::StageList &stage_list) const {
  x_size = 0;
  y_size = 0;
  aniso_degree = 0;
  minfilter = Texture::FT_nearest;
  magfilter = Texture::FT_nearest;

  StageList::const_iterator si;
  for (si = stage_list.begin(); si != stage_list.end(); ++si) {
    const StageInfo &stage_info = (*si);
    Texture *tex = stage_info._tex;
    PixelBuffer *pbuffer = tex->_pbuffer;

    if (stage_info._stage == _target_stage) {
      // If we find the target stage, use that.
      x_size = pbuffer->get_xsize();
      y_size = pbuffer->get_ysize();
      aniso_degree = tex->get_anisotropic_degree();
      minfilter = tex->get_minfilter();
      magfilter = tex->get_magfilter();
      return;
    }

    // If we never run across the target stage, just use the maximum
    // of all encountered textures.
    x_size = max(x_size, pbuffer->get_xsize());
    y_size = max(y_size, pbuffer->get_ysize());
    aniso_degree = max(aniso_degree, tex->get_anisotropic_degree());
    minfilter = (Texture::FilterType)max((int)minfilter, (int)tex->get_minfilter());
    magfilter = (Texture::FilterType)max((int)magfilter, (int)tex->get_magfilter());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::make_texture_layer
//       Access: Private
//  Description: Creates geometry to render the texture into the
//               offscreen buffer using the same effects that were
//               requested by its multitexture specification.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
make_texture_layer(const NodePath &render, 
                   const MultitexReducer::StageInfo &stage_info, 
                   const MultitexReducer::GeomList &geom_list) {
  CPT(RenderAttrib) cba;

  switch (stage_info._stage->get_mode()) {
  case TextureStage::M_modulate:
    cba = ColorBlendAttrib::make
      (ColorBlendAttrib::M_add, ColorBlendAttrib::O_fbuffer_color,
       ColorBlendAttrib::O_zero);
    break;

  case TextureStage::M_decal:
    cba = ColorBlendAttrib::make
      (ColorBlendAttrib::M_add, ColorBlendAttrib::O_incoming_alpha,
       ColorBlendAttrib::O_one_minus_incoming_alpha);
    break;

  case TextureStage::M_blend:
    cba = ColorBlendAttrib::make
      (ColorBlendAttrib::M_add, ColorBlendAttrib::O_constant_color,
       ColorBlendAttrib::O_one_minus_incoming_color,
       stage_info._stage->get_color());
    break;

  case TextureStage::M_replace:
    cba = ColorBlendAttrib::make_off();
    break;

  case TextureStage::M_add:
    cba = ColorBlendAttrib::make
      (ColorBlendAttrib::M_add, ColorBlendAttrib::O_one,
       ColorBlendAttrib::O_one);
    break;

  case TextureStage::M_combine:
    // We don't support the texture combiner here right now.  For now,
    // this is the same as modulate.
    cba = ColorBlendAttrib::make
      (ColorBlendAttrib::M_add, ColorBlendAttrib::O_fbuffer_color,
       ColorBlendAttrib::O_zero);
    break;
  }

  NodePath geom;

  if (stage_info._stage->get_texcoord_name() == _target_stage->get_texcoord_name()) {
    // If this TextureStage uses the target texcoords, we can just
    // generate a simple card the fills the entire buffer.
    CardMaker cm(stage_info._tex->get_name());
    cm.set_uv_range(TexCoordf(0.0f, 0.0f), TexCoordf(1.0f, 1.0f));
    cm.set_has_uvs(true);
    cm.set_frame(0.0f, 1.0f, 0.0f, 1.0f);
    
    geom = render.attach_new_node(cm.generate());

  } else {
    // If this TextureStage uses some other texcoords, we have to
    // generate geometry that maps the texcoords to the target space.
    // This will work only for very simple cases where the geometry is
    // not too extensive and doesn't repeat over the same UV's.
    PT(GeomNode) geom_node = new GeomNode(stage_info._tex->get_name());
    transfer_geom(geom_node, stage_info._stage->get_texcoord_name(), 
                  geom_list);
    
    geom = render.attach_new_node(geom_node);

    // Make sure we override the vertex color, so we don't pollute the
    // texture with geometry color.
    geom.set_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  }

  if (!stage_info._tex_mat->is_identity()) {
    geom.set_tex_transform(TextureStage::get_default(), stage_info._tex_mat);
  }

  geom.set_texture(stage_info._tex);
  geom.node()->set_attrib(cba);
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::transfer_geom
//       Access: Private
//  Description: Copy the vertices from the indicated geom_list,
//               mapping the vertex coordinates so that the geometry
//               will render the appropriate distortion on the texture
//               to map UV's from the specified set of texture
//               coordinates to the target set.
////////////////////////////////////////////////////////////////////
void MultitexReducer::
transfer_geom(GeomNode *geom_node, const TexCoordName *texcoord_name,
              const MultitexReducer::GeomList &geom_list) {
  GeomList::const_iterator gi;
  for (gi = geom_list.begin(); gi != geom_list.end(); ++gi) {
    const GeomInfo &geom_info = (*gi);
    
    PT(Geom) geom = 
      geom_info._geom_node->get_geom(geom_info._index)->make_copy();

    PTA_Vertexf coords = PTA_Vertexf::empty_array(0);
    PTA_TexCoordf texcoords = geom->get_texcoords_array(_target_stage->get_texcoord_name());
    if (!texcoords.empty()) {
      coords.reserve(texcoords.size());
      for (size_t i = 0; i < texcoords.size(); i++) {
        const TexCoordf &tc = texcoords[i];
        Vertexf v(tc[0], 0.0f, tc[1]);
        coords.push_back(v);
      }
      
      geom->set_coords(coords, geom->get_texcoords_index(_target_stage->get_texcoord_name()));
      geom->set_texcoords(TexCoordName::get_default(),
                          geom->get_texcoords_array(texcoord_name),
                          geom->get_texcoords_index(texcoord_name));
      
      geom_node->add_geom(geom);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::StageInfo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MultitexReducer::StageInfo::
StageInfo(TextureStage *stage, const TextureAttrib *ta, 
          const TexMatrixAttrib *tma) :
  _stage(stage),
  _tex_mat(TransformState::make_identity())
{
  _tex = ta->get_on_texture(_stage);
  if (tma->has_stage(stage)) {
    _tex_mat = tma->get_transform(stage);
  }
}
