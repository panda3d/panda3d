// Filename: demo.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "framework.h"

#include "eventHandler.h"
#include "chancfg.h"
#include "string"
#include "renderModeTransition.h"
#include "colorTransition.h"
#include "colorBlendTransition.h"
#include "cullFaceTransition.h"
#include "depthTestTransition.h"
#include "depthWriteTransition.h"
#include "textureTransition.h"
#include "textureApplyTransition.h"
#include "lightTransition.h"
#include "materialTransition.h"
#include "transformTransition.h"
#include "transparencyTransition.h"
#include "drawBoundsTransition.h"
#include "pruneTransition.h"
#include "get_rel_pos.h"
#include "boundingSphere.h"
#include "geomSphere.h"
#include "geomNode.h"
#include "notify.h"
#include "directionalLight.h"
#include "renderRelation.h"
#include "camera.h"
#include "frustum.h"
#include "orthoProjection.h"
#include "perspectiveProjection.h"
#include "textNode.h"
#include "colorMatrixTransition.h"
#include "alphaTransformTransition.h"
#include "lensFlareNode.h"
#include "texture.h"
#include "texturePool.h"
#include "spotlight.h"
#include "nodePath.h"
#include "pta_Colorf.h"
#include "pta_float.h"
#include "pt_Node.h"
#include "panda.h"

// If we're doing a static link, we should explicitly initialize some
// of our external libraries, or they may not get linked in.
#ifdef LINK_ALL_STATIC
  #ifdef HAVE_GL
    #include "pandagl.h"
  #endif
  #ifdef HAVE_DX
    #include "pandadx.h"
  #endif
  #include "pandaegg.h"
#endif

//From framework
extern PT(GeomNode) geomnode;
extern RenderRelation* first_arc;

// These variables are used to implement the set_highlight() and
// related functions to allow the user to walk through the scene graph
// with the arrow keys.

// highlight_render_node is a special node that has a funny render
// mode set on it to draw things in red wireframe, but it has no
// geometry of its own.  The node that we're currently highlighting
// (current_node) is instanced under highlight_render_node via
// current_arc, so that the highlight applies to that node.
RenderRelation *highlight_arc = NULL;
PT_NamedNode highlight_render_node;
RenderRelation *current_arc = NULL;

NodePath selected_node;

PT_NamedNode render2d_top;
RenderRelation *render2d_arc = NULL;
PT_NamedNode render2d;

PT(TextNode) label2d;

//Global Node used by LensFlare and NodePath color/alpha demos
PT_NamedNode sky = (NamedNode *)NULL;
RenderRelation *sky_arc = (RenderRelation *)NULL;
RenderRelation *flare_arc = (RenderRelation *)NULL;


/*
void
setup_panda2d() {
  static bool already_setup = false;
  if (already_setup) {
    nout << "Already have 2-d layer set up.\n";
    return;
  }
  already_setup = true;
  nout << "Setting up 2-d layer.\n";

  render2d_top = new NamedNode("render2d_top");
  render2d = new NamedNode("render2d");
  render2d_arc = new RenderRelation(render2d_top, render2d);

  // Set up some overrides to turn off certain properties which we
  // probably won't need for 2-d objects.
  render2d_arc->set_transition(new DepthTestTransition(DepthTestProperty::M_none), 1);
  render2d_arc->set_transition(new DepthWriteTransition(DepthWriteTransition::off()), 1);
  render2d_arc->set_transition(new LightTransition(LightTransition::all_off()), 1);
  render2d_arc->set_transition(new MaterialTransition(MaterialTransition::off()), 1);
  render2d_arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none), 1);

  // Create a 2-d camera.
  PT(Camera) cam2d = new Camera("cam2d");
  new RenderRelation(render2d, cam2d);
  cam2d->set_scene(render2d_top);

  Frustumf frustum2d;
  frustum2d.make_ortho_2D();
  cam2d->set_projection(OrthoProjection(frustum2d));

  // Now create a new layer.
  GraphicsChannel *chan = main_win->get_channel(0);
  nassertv(chan != (GraphicsChannel *)NULL);

  GraphicsLayer *layer = chan->make_layer();
  nassertv(layer != (GraphicsLayer *)NULL);

  DisplayRegion *dr = layer->make_display_region();
  nassertv(dr != (DisplayRegion *)NULL);
  dr->set_camera(cam2d);


  // Put some interesting things in the 2-d scene graph.
  PT_Node font = loader.load_sync("ttf-comic");

  if (font != (NamedNode *)NULL) {
    label2d = new TextNode("label2d");
    new RenderRelation(render2d, label2d);

    LMatrix4f mat =
      LMatrix4f::scale_mat(0.1) *
      LMatrix4f::translate_mat(0.0, 0.0, 0.8);

    label2d->set_transform(mat);
    label2d->set_font(font.p());
    label2d->set_card_color(0.5, 0.5, 0.5, 0.5);
    label2d->set_card_as_margin(0.5, 0.5, 0.2, 0.2);
    label2d->set_frame_color(1.0, 1.0, 1.0, 1.0);
    label2d->set_frame_as_margin(0.5, 0.5, 0.2, 0.2);
    label2d->set_align(TM_ALIGN_CENTER);
    label2d->set_text_color(0.2, 0.5, 1.0, 1.0);
    label2d->set_text("Now presenting Panda 2-D!");

    MouseWatcherRegion *region =
      new MouseWatcherRegion("label2d", label2d->get_card_transformed());
    region->set_suppress_below(true);
    mouse_watcher->add_region(region);

  } else {
    nout << "Couldn't load font ttf-comic\n";
  }

  PT_Node p2egg = loader.load_sync("panda2d");
  if (p2egg != (NamedNode *)NULL) {
    new RenderRelation(render2d, p2egg);
  } else {
    nout << "Couldn't find panda2d.egg\n";
  }

  // Load up a mouse cursor for fun.
  PT_Node lilsmiley = loader.load_sync("lilsmiley");
  if (lilsmiley != (NamedNode *)NULL) {
    NamedNode *n2 = new NamedNode("cursor");
    RenderRelation *s1_arc = new RenderRelation(render2d, n2);
    RenderRelation *s2_arc = new RenderRelation(n2, lilsmiley);

    // Make up a scale to make it 32x32 pixels (it starts out 1x1
    // units).
    LMatrix4f scale_mat = LMatrix4f::scale_mat(32.0 / main_win->get_width(), 1.0, 32.0 / main_win->get_height());
    s2_arc->set_transition(new TransformTransition(scale_mat));
    mouse_watcher->set_geometry(s1_arc);
  } else {
    nout << "Couldn't find lilsmiley.egg\n";
  }
}

static void
event_in_label2d(CPT_Event) {
  // The mouse moved over the label.
  nassertv(!label2d.is_null());

  label2d->set_card_color(0.8, 0.2, 1.0, 1.0);
}

static void
event_out_label2d(CPT_Event) {
  // The mouse moved out from the label.
  nassertv(!label2d.is_null());

  label2d->set_card_color(0.5, 0.5, 0.5, 0.5);
}
*/

static void
clear_highlight() {
  if (selected_node.has_arcs()) {
    selected_node.hide_bounds();
  }

  if (current_arc != NULL) {
    remove_arc(current_arc);
    current_arc = NULL;
  }
}

static void
set_highlight() {
  // Transform the highlight to the coordinate space of the node.
  LMatrix4f mat = selected_node.get_mat(NodePath(render));
  highlight_arc->set_transition(new TransformTransition(mat));

  nout << "Highlighting " << selected_node.as_string(1) << "\n";

  nout << "Bounding volume of node is " << selected_node.node()->get_bound() << "\n";

  if (selected_node.has_arcs()) {
    nout << "Bounding volume of arc is " << *selected_node.get_bounds() << "\n";

    nout << "Transitions on arc:\n";
    selected_node.arc()->write_transitions(nout, 2);
    selected_node.show_bounds();
  }

  current_arc = new RenderRelation(highlight_render_node, selected_node.node());
}

static void
event_up(CPT_Event) {
  if (selected_node.has_arcs() && selected_node.get_node(1) != root) {
    clear_highlight();
    selected_node.shorten(1);
    set_highlight();
  }
}

static void
event_down(CPT_Event) {
  if (!selected_node.is_empty() &&
      selected_node.get_num_children() != 0) {
    clear_highlight();
    selected_node = selected_node.get_child(0);
    set_highlight();
  }
}

static void
event_left(CPT_Event) {
  // Go to the previous child in the sibling list, if there is one.
  if (selected_node.has_arcs()) {
    NodePath parent = selected_node;
    parent.shorten(1);
    int num_children = parent.get_num_children();

    for (int i = 0; i < num_children; i++) {
      if (parent.get_child(i) == selected_node) {
        // We've currently got child i; now select child i-1.
        if (i - 1 >=  0) {
          clear_highlight();
          selected_node = parent.get_child(i - 1);
          set_highlight();
        }
        return;
      }
    }
  }
}

static void
event_right(CPT_Event) {
  // Go to the next child in the sibling list, if there is one.
  if (selected_node.has_arcs()) {
    NodePath parent = selected_node;
    parent.shorten(1);
    int num_children = parent.get_num_children();

    for (int i = 0; i < num_children; i++) {
      if (parent.get_child(i) == selected_node) {
        // We've currently got child i; now select child i + 11.
        if (i + 1 < num_children) {
          clear_highlight();
          selected_node = parent.get_child(i + 1);
          set_highlight();
        }
        return;
      }
    }
  }
}

static void
event_fkey(CPT_Event event) {
  if (selected_node.has_arcs()) {
    string name = event->get_name();
    if (name.substr(0, 6) == "shift-") {
      // Shift-fkey: work with color scale.
      if (name == "shift-f9") {
        selected_node.clear_color_scale();
        selected_node.clear_transparency();
        cerr << "Clearing color scale on " << selected_node << "\n";

      } else {
        Colorf color_scale;
        if (name == "shift-f1") {
          color_scale.set(0.5, 0.5, 1.0, 1.0);
        } else if (name == "shift-f2") {
          color_scale.set(0.5, 1.0, 0.5, 1.0);
        } else if (name == "shift-f3") {
          color_scale.set(0.5, 1.0, 1.0, 1.0);
        } else if (name == "shift-f4") {
          color_scale.set(1.0, 0.5, 0.5, 1.0);
        } else if (name == "shift-f5") {
          color_scale.set(1.0, 0.5, 1.0, 1.0);
        } else if (name == "shift-f6") {
          color_scale.set(1.0, 1.0, 0.5, 1.0);
        } else if (name == "shift-f7") {
          color_scale.set(1.0, 1.0, 1.0, 1.0);
        } else if (name == "shift-f8") {
          color_scale.set(1.0, 1.0, 1.0, 0.5);
        }
        selected_node.set_color_scale(color_scale);
        if (color_scale[3] != 1.0) { 
          selected_node.set_transparency(true);
        }
        cerr << "Setting color scale on " << selected_node << " to " << color_scale << "\n";
      }
    } else {
      // Non shifted fkey: work with flat color.
        
      if (name == "f9") {
        // F9: restore the natural color.
        selected_node.clear_color();
        selected_node.clear_transparency();
        cerr << "Clearing color on " << selected_node << "\n";
        
      } else {
        Colorf color;
        if (name == "f1") {
          color.set(0.0, 0.0, 1.0, 1.0);
        } else if (name == "f2") {
          color.set(0.0, 1.0, 0.0, 1.0);
        } else if (name == "f3") {
          color.set(0.0, 1.0, 1.0, 1.0);
        } else if (name == "f4") {
          color.set(1.0, 0.0, 0.0, 1.0);
        } else if (name == "f5") {
          color.set(1.0, 0.0, 1.0, 1.0);
        } else if (name == "f6") {
          color.set(1.0, 1.0, 0.0, 1.0);
        } else if (name == "f7") {
          color.set(1.0, 1.0, 1.0, 1.0);
        } else if (name == "f8") {
          color.set(1.0, 1.0, 1.0, 0.5);
        }
        selected_node.set_color(color, 1);
        if (color[3] != 1.0) { 
          selected_node.set_transparency(true);
        }
        cerr << "Setting color on " << selected_node << " to " << color << "\n";
      }
    }
  }
}

static void
event_B(CPT_Event event) {
  // List everything under the selected bounding volume and
  // recompute the volume.
  selected_node.ls();
  selected_node.analyze();

  if (selected_node.has_arcs()) {
    selected_node.arc()->force_bound_stale();
  }
}

static void
enable_highlight() {
  if (highlight_render_node == NULL) {
    highlight_render_node = new NamedNode("highlight");
    highlight_arc = new RenderRelation(render, highlight_render_node);

    // Set up the funny rendering attributes on the highlighted
    // geometry.
    RenderModeTransition *rmt =
      new RenderModeTransition(RenderModeProperty::M_wireframe);
    ColorTransition *ct =
      new ColorTransition(1.0, 0.0, 0.0, 1.0);
    CullFaceTransition *cft =
      new CullFaceTransition(CullFaceProperty::M_cull_none);
    TextureTransition *tt =
      new TextureTransition;
    LightTransition *lt =
      new LightTransition;

    rmt->set_priority(100);
    ct->set_priority(100);
    cft->set_priority(100);
    tt->set_priority(100);
    lt->set_priority(100);

    highlight_arc->set_transition(rmt);
    highlight_arc->set_transition(ct);
    highlight_arc->set_transition(cft);
    highlight_arc->set_transition(tt);
    highlight_arc->set_transition(lt);
  }

  // Add a temporary arc from the highlight render node to the node we
  // are highlighting.
  selected_node = NodePath(root);
  if (selected_node.get_num_children() == 0) {
    nout << "No nodes.\n";
    selected_node.clear();

  } else {
    selected_node = selected_node.get_child(0);
    set_highlight();
  }
}

static void
disable_highlight() {
  nout << "Disabling highlight\n";
  clear_highlight();
  selected_node.clear();
}

static void
event_h(CPT_Event) {
  if (selected_node.is_empty()) {
    enable_highlight();
  } else {
    disable_highlight();
  }
}

static void attach_sky() {
  // Load the sun and sky
  sky = DCAST(NamedNode, loader.load_sync("sky"));
  if (sky != (NamedNode *)NULL) {
    if (sky_arc == (RenderRelation *)NULL) {
      sky_arc = new RenderRelation(render, sky);
    }
  }
}

static void
event_k(CPT_Event) {
  static bool is_color_mat = false;

  if (!is_color_mat) {
    LMatrix4f color_mat = LMatrix4f::scale_mat(0,0,0) * LMatrix4f::translate_mat(1,0,0);

    first_arc->set_transition(new ColorMatrixTransition(color_mat));
  }
  else {
    LMatrix4f color_mat = LMatrix4f::ident_mat();

    first_arc->set_transition(new ColorMatrixTransition(color_mat));
  }
  is_color_mat = !is_color_mat;
}

static void
event_a(CPT_Event) {
  static bool is_alpha = false;

  if (!is_alpha) {
    first_arc->set_transition(new AlphaTransformTransition(-0.5, 1));
    first_arc->set_transition(new TransparencyTransition(TransparencyProperty::M_alpha));
  }
  else {
    first_arc->clear_transition(AlphaTransformTransition::get_class_type());
    first_arc->clear_transition(TransparencyTransition::get_class_type());
  }
  is_alpha = !is_alpha;
}


static void
event_v(CPT_Event) {
  static bool is_color_scale = false;

  attach_sky();

  NodePath search(sky);
  NodePath sky_search = search.find("**/sun");

  if (!is_color_scale) {
    sky_search.set_color_scale(1, 0.5, 0.5, 0.5);
    sky_search.set_transparency(true);
  }
  else {
    sky_search.clear_color_scale();
    sky_search.clear_transparency();
  }
  is_color_scale = !is_color_scale;
}

static void
event_L(CPT_Event) {
  static bool is_flare = false;

  attach_sky();

  if (!is_flare) {
    //Texture *shine = TexturePool::load_texture("MyShine.bw");
    Texture *shine = TexturePool::load_texture("bigsmileycrop.rgba");

    Texture *f4 = TexturePool::load_texture("Flare4.bw");
    Texture *f5 = TexturePool::load_texture("Flare5.bw");
    Texture *f6 = TexturePool::load_texture("Flare6.bw");

    LensFlareNode *flare = new LensFlareNode();

    PTA_float scales, offsets, angles;
    PTA_Colorf colors;

    scales.push_back(0.5); offsets.push_back(0.2); angles.push_back(0); colors.push_back(Colorf(0.3, 0.6, 0.3, 1));
    scales.push_back(0.5); offsets.push_back(0.4); angles.push_back(0); colors.push_back(Colorf(0.6, 0.6, 0.6, 1));
    scales.push_back(0.75); offsets.push_back(0.7); angles.push_back(0.2); colors.push_back(Colorf(0.3, 0.6, 0.3, 1));
    scales.push_back(1.5); offsets.push_back(1.2); angles.push_back(0); colors.push_back(Colorf(0.6, 0.6, 0.6, 1));
    scales.push_back(0.75); offsets.push_back(1.5); angles.push_back(0); colors.push_back(Colorf(0.6, 0.6, 0.6, 1));

    flare->add_flare(f6, scales, offsets, angles, colors);

    scales.clear(); offsets.clear(); angles.clear(); colors.clear();

    scales.push_back(0.3); offsets.push_back(0.55); angles.push_back(0); colors.push_back(Colorf(0.0, 0.0, 0.6, 1));
    scales.push_back(0.3); offsets.push_back(0.8); angles.push_back(0); colors.push_back(Colorf(0.0, 0.0, 0.6, 1));
    scales.push_back(0.5); offsets.push_back(1.1); angles.push_back(0); colors.push_back(Colorf(0.0, 0.0, 0.6, 1));
    scales.push_back(0.15); offsets.push_back(1.35); angles.push_back(0.2); colors.push_back(Colorf(0.3, 0.6, 0.3, 1));

    flare->add_flare(f5, scales, offsets, angles, colors);

    scales.clear(); offsets.clear(); angles.clear(); colors.clear();

    scales.push_back(1); offsets.push_back(0.0); angles.push_back(-0.3); colors.push_back(Colorf(1.0, 0.0, 0.0, 1));
    scales.push_back(1.05); offsets.push_back(0.0); angles.push_back(-0.3); colors.push_back(Colorf(0.0, 1.0, 0.0, 1));
    scales.push_back(1.1); offsets.push_back(0.0); angles.push_back(-0.3); colors.push_back(Colorf(0.0, 0.0, 1.0, 1));

    flare->add_flare(f4, scales, offsets, angles, colors);

    flare->add_blind(shine);
    flare->set_blind_falloff(5);
    flare->set_flare_falloff(45);

    NodePath search(sky);
    NodePath sky_search = search.find("**/sun");
    PT_Node light = sky_search.node();

    flare->set_light_source(light);
    flare_arc = new RenderRelation(light, flare, 10);
    ColorBlendTransition *cb = new ColorBlendTransition(ColorBlendProperty::M_add);
    flare_arc->set_transition(cb);

    TextureApplyTransition *ta =
      new TextureApplyTransition(TextureApplyProperty::M_decal);
    flare_arc->set_transition(ta);

    DepthTestTransition *dta =
      new DepthTestTransition(DepthTestProperty::M_none);
    flare_arc->set_transition(dta);
  }
  else {
    remove_arc(flare_arc);
  }

  is_flare = !is_flare;
}

void demo_keys(EventHandler&) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  /*
  event_handler.add_hook("mw-in-label2d", event_in_label2d);
  event_handler.add_hook("mw-out-label2d", event_out_label2d);
  */

  event_handler.add_hook("h", event_h);
  event_handler.add_hook("up", event_up);
  event_handler.add_hook("down", event_down);
  event_handler.add_hook("left", event_left);
  event_handler.add_hook("right", event_right);
  event_handler.add_hook("f1", event_fkey);
  event_handler.add_hook("f2", event_fkey);
  event_handler.add_hook("f3", event_fkey);
  event_handler.add_hook("f4", event_fkey);
  event_handler.add_hook("f5", event_fkey);
  event_handler.add_hook("f6", event_fkey);
  event_handler.add_hook("f7", event_fkey);
  event_handler.add_hook("f8", event_fkey);
  event_handler.add_hook("f9", event_fkey);
  event_handler.add_hook("shift-f1", event_fkey);
  event_handler.add_hook("shift-f2", event_fkey);
  event_handler.add_hook("shift-f3", event_fkey);
  event_handler.add_hook("shift-f4", event_fkey);
  event_handler.add_hook("shift-f5", event_fkey);
  event_handler.add_hook("shift-f6", event_fkey);
  event_handler.add_hook("shift-f7", event_fkey);
  event_handler.add_hook("shift-f8", event_fkey);
  event_handler.add_hook("shift-f9", event_fkey);
  event_handler.add_hook("shift-B", event_B);

  event_handler.add_hook("shift-L", event_L);
  event_handler.add_hook("k", event_k);
  event_handler.add_hook("a", event_a);
  event_handler.add_hook("v", event_v);
}

int main(int argc, char *argv[]) {
  // We call init_libpanda() to be paranoid.  It's not supposed to be
  // necessary, but it turns out that static init isn't dependable in
  // all cases.
  init_libpanda();

  // If we're doing a static link, we should explicitly initialize some
  // of our external libraries, or they may not get linked in.
#ifdef LINK_ALL_STATIC
  #ifdef HAVE_GL
  init_libpandagl();
  #endif
  #ifdef HAVE_DX
  init_libpandadx();
  #endif
  init_libpandaegg();
#endif

  define_keys = &demo_keys;
  return framework_main(argc, argv);
}
