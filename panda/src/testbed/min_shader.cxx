// Filename: min_shader.cxx
// Created by:  jason (28Jun00)
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

//Shaders
#include <projtexShader.h>
#include <spotlightShader.h>
#include <projtexShadower.h>
#include <planarReflector.h>
#include <spheretexReflector.h>
#include <spheretexHighlighter.h>
#include <spheretexReflector.h>
#include <outlineShader.h>

//Channel stuff
#include <chancfg.h>

//Textures
#include <texture.h>
#include <texturePool.h>

//Transitions
#include <textureTransition.h>
#include <shaderTransition.h>
#include <lightTransition.h>
#include <transformTransition.h>
#include <colorBlendTransition.h>

//Light stuff
#include <light.h>
#include <spotlight.h>

//Nodes
#include <pt_NamedNode.h>
#include <geomNode.h>

//Relations (arcs)
#include <renderRelation.h>
#include <nodeRelation.h>
#include <dataRelation.h>

//Misc
#include "dconfig.h"
#include <framework.h>
#include <loader.h>
#include <eventHandler.h>

//Math/Matrix/Vector/Transformation stuff
#include <transform2sg.h>
#include <look_at.h>
#include <perspectiveLens.h>
#include <geomLine.h>

Configure(min_shader);
ConfigureFn(min_shader) {
}

//--------Projective texture stuff--------
PT(LensNode) tex_proj;
PT(Trackball) tex_proj_trackball;
PT(ProjtexShader) proj_shader;
//--------Spotlight stuff-----------------
PT(Spotlight) tex_proj_spot;
PT(Trackball) tex_spot_trackball;
PT(SpotlightShader) spot_shader;
//--------Projective Shadow stuff--------
PT(ProjtexShadower) proj_shadow;
//--------Planar reflecter stuff---------
PT(PlanarReflector) preflect;
//--------Sphere texture stuff---------
PT(SpheretexShader) spheretex;
//--------Sphere texture highlight stuff---------
PT(SpheretexHighlighter) highlight;
//--------Sphere texture reflector stuff---------
PT(SpheretexReflector) sreflect;
//--------Outline shader stuff---------
PT(OutlineShader) outline_shader;

ShaderTransition shader_trans;

RenderRelation* room_arc;
RenderRelation* spot_arc;
RenderRelation* jack_arc;
RenderRelation* camera_model_arc;
RenderRelation* smiley_arc;

PT(LightTransition) light_transition;

//Framework extern variables and functions
extern PT_NamedNode render;
extern NodeAttributes initial_state;
extern RenderRelation* first_arc;
extern PT_NamedNode lights;
extern PT_NamedNode root;
extern PT(GeomNode) geomnode;
extern PT_NamedNode cameras;
extern PT(MouseAndKeyboard) mak;

extern void set_alt_trackball(Node *trackball);

extern int framework_main(int argc, char *argv[]);
extern void (*extra_display_func)();
extern void (*define_keys)(EventHandler&);

extern PT(GraphicsWindow) main_win;

//GLOBALS
LPoint3f center_pos = LPoint3f::origin();
Transform2SG *tball2cam;
Texture* tex;

//VIZ STUFF
class BaseViz : public Shader::Visualize {
public:
  BaseViz(void);
  virtual ~BaseViz(void);
  virtual void DisplayTexture(PT(Texture)&, Shader*);
  virtual void DisplayPixelBuffer(PT(PixelBuffer)&, Shader*);
protected:
  typedef std::plist<PT(Texture) > texlist;
  typedef texlist::iterator texiter;
  typedef std::plist<PT(PixelBuffer) > pblist;
  typedef pblist::iterator pbiter;

  texlist _texs;
  pblist _pbs;
};

BaseViz::BaseViz(void) {}

BaseViz::~BaseViz(void) {}

void BaseViz::DisplayTexture(PT(Texture)& tex, Shader*) {
  if (tex->has_ram_image())
    _texs.push_back(tex);
  else {
    GraphicsStateGuardian* g = main_win->get_gsg();
    PT(PixelBuffer) pb(new PixelBuffer);
    g->texture_to_pixel_buffer(tex->prepare(g), pb);
    _pbs.push_back(pb);
  }
}

void BaseViz::DisplayPixelBuffer(PT(PixelBuffer)& pb, Shader*) {
  _pbs.push_back(pb);
}

class Oldviz : public BaseViz {
public:
  Oldviz(PT(GraphicsWindow)&);
  virtual ~Oldviz(void);
  virtual void Flush(void);
protected:
  PT(GraphicsWindow) _w;
};

Oldviz::Oldviz(PT(GraphicsWindow)& win) : _w(win) {}

Oldviz::~Oldviz(void) {}

void Oldviz::Flush(void) {
  GraphicsStateGuardian* g = _w->get_gsg();
  const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);
  PT(DisplayRegion) d(_w->make_scratch_display_region(256, 256));

  for (texiter i=_texs.begin(); i!=_texs.end(); ++i)
    (*i)->draw(g, d, r);
  for (pbiter j=_pbs.begin(); j!=_pbs.end(); ++j)
    (*j)->draw(g, d, r);
  _texs.erase(_texs.begin(), _texs.end());
  _pbs.erase(_pbs.begin(), _pbs.end());
}

class Viztex : public BaseViz {
public:
  Viztex(PT(GraphicsPipe)&);
  virtual ~Viztex(void);
  virtual void Flush(void);
protected:
  typedef std::plist< PT(GraphicsWindow) > winlist;
  typedef winlist::iterator winiter;

  winlist _wins;
  PT(GraphicsPipe) _pipe;
};

Viztex::Viztex(PT(GraphicsPipe)& p) : _pipe(p) {}

Viztex::~Viztex(void) {}

void Viztex::Flush(void) {
  winiter _witer = _wins.begin();

  for (texiter i=_texs.begin(); i!=_texs.end(); ++i) {
    GraphicsWindow* w;

    if (_witer == _wins.end()) {
      ChanCfgOverrides override;

      override.setField(ChanCfgOverrides::Mask,
                        ((unsigned int)(W_SINGLE)));
      override.setField(ChanCfgOverrides::Title, "Multipass partial");
      override.setField(ChanCfgOverrides::SizeX, 256);
      override.setField(ChanCfgOverrides::SizeY, 256);
      override.setField(ChanCfgOverrides::Cameras, false);
      w = ChanConfig(_pipe, "single", (Node *)NULL, render, override);
      _wins.push_back(w);
    }
    else
    {
      w = *_witer;
      ++_witer;
    }
    GraphicsStateGuardian* g = w->get_gsg();
    const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);
    PT(DisplayRegion) d(w->make_scratch_display_region(w->get_width(),
                                                       w->get_height()));
    // g->prepare_display_region(d);
    (*i)->draw(g, d, r);
  }
  for (pbiter j=_pbs.begin(); j!=_pbs.end(); ++j) {
    GraphicsWindow* w;

    if (_witer == _wins.end()) {
      ChanCfgOverrides override;

      override.setField(ChanCfgOverrides::Mask,
                        ((unsigned int)(W_SINGLE)));
      override.setField(ChanCfgOverrides::Title, "Multipass partial");
      override.setField(ChanCfgOverrides::SizeX, 256);
      override.setField(ChanCfgOverrides::SizeY, 256);
      override.setField(ChanCfgOverrides::Cameras, false);
      w = ChanConfig(_pipe, "single", cameras, render, override);
      _wins.push_back(w);
    } else {
      w = *_witer;
      ++_witer;
    }
    GraphicsStateGuardian* g = w->get_gsg();
    const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);
    PT(DisplayRegion) d(w->make_scratch_display_region(w->get_width(),
                                                       w->get_height()));
    // g->prepare_display_region(d);
    (*j)->draw(g, d, r);
  }
  _texs.erase(_texs.begin(), _texs.end());
  _pbs.erase(_pbs.begin(), _pbs.end());
}

class Tiledviz : public Viztex {
public:
  Tiledviz(PT(GraphicsPipe)&);
  virtual ~Tiledviz(void);
  virtual void Flush(void);
};

Tiledviz::Tiledviz(PT(GraphicsPipe)& p) : Viztex(p) {}

Tiledviz::~Tiledviz(void) {}

void Tiledviz::Flush(void) {
  int count = 0;
  int layer_count = 0;
  winiter _witer = _wins.begin();

  for (texiter texi=_texs.begin(); texi!=_texs.end(); ++texi) {
    GraphicsWindow* w;
    DisplayRegion* d;

    if (_witer == _wins.end()) {
      ChanCfgOverrides override;

      override.setField(ChanCfgOverrides::Mask,
                        ((unsigned int)(W_SINGLE)));
      override.setField(ChanCfgOverrides::Title, "Multipass partial");
      override.setField(ChanCfgOverrides::SizeX, main_win->get_width());
      override.setField(ChanCfgOverrides::SizeY, main_win->get_height());
      override.setField(ChanCfgOverrides::Cameras, false);
      w = ChanConfig(_pipe, "multipass-tile", cameras, render, override);
      _wins.push_back(w);
      _witer = --(_wins.end());
    } else
      w = *_witer;

    int dridx;
    GraphicsStateGuardian* g = w->get_gsg();
    GraphicsChannel *chan = w->get_channel(0);
    GraphicsLayer *layer = chan->get_layer(layer_count);
    const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);

    d = layer->get_dr(count);
    (*texi)->draw(g, d, r);

    ++count;
    if (count == layer->get_num_drs()) {
      count = 0;
      ++layer_count;
    }

    if (layer_count == chan->get_num_layers())
    {
      layer_count = 0;
      ++_witer;
    }
  }

  for (pbiter pbi=_pbs.begin(); pbi!=_pbs.end(); ++pbi) {
    GraphicsWindow* w;
    DisplayRegion* d;

    if (_witer == _wins.end()) {
      ChanCfgOverrides override;

      override.setField(ChanCfgOverrides::Mask,
                        ((unsigned int)(W_SINGLE)));
      override.setField(ChanCfgOverrides::Title, "Multipass partial");
      override.setField(ChanCfgOverrides::SizeX, main_win->get_width());
      override.setField(ChanCfgOverrides::SizeY, main_win->get_height());
      override.setField(ChanCfgOverrides::Cameras, false);
      w = ChanConfig(_pipe, "multipass-tile", cameras, render, override);
      _wins.push_back(w);
      _witer = --(_wins.end());
    } else
      w = *_witer;

    int dridx;
    GraphicsStateGuardian* g = w->get_gsg();
    GraphicsChannel *chan = w->get_channel(0);
    GraphicsLayer *layer = chan->get_layer(layer_count);
    const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);

    d = layer->get_dr(count);
    (*pbi)->draw(g, d, r);

     ++count;
    if (count == layer->get_num_drs()) {
      count = 0;
      ++layer_count;
    }

    if (layer_count == chan->get_num_layers())
    {
      layer_count = 0;
      ++_witer;
    }

  }
  _texs.erase(_texs.begin(), _texs.end());
  _pbs.erase(_pbs.begin(), _pbs.end());
}

void shader_display_func(void) {
  Shader::Visualize* v = Shader::get_viz();
  if (v != (Shader::Visualize*)0L)
    v->Flush();
}

//END VIZ STUFF

////////////////////////////////////////////////////////////////////
//     Function: event_p
//       Access: Public
//  Description: Toggle the projective texture being on and off
////////////////////////////////////////////////////////////////////
void event_p(CPT_Event) {
  static bool projtex_mode = false;

  projtex_mode = !projtex_mode;
  if (!projtex_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, proj_shader);
    //clear_shader(first_arc, proj_shader);
    //clear_shader(jack_arc, proj_shader);

    set_alt_trackball(NULL);

  } else {
    // Set an override on the initial state.
    set_shader(room_arc, proj_shader);
    //set_shader(first_arc, proj_shader);
    //set_shader(jack_arc, proj_shader);

    set_alt_trackball(tex_proj_trackball);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: event_s
//       Access: Public
//  Description: Toggle the spotlight being on and off
////////////////////////////////////////////////////////////////////
void event_s(CPT_Event) {
  static bool projtexspot_mode = false;

  projtexspot_mode = !projtexspot_mode;
  if (!projtexspot_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, spot_shader);

    set_alt_trackball(NULL);
  } else {
    // Set an override on the initial state.
    set_shader(room_arc, spot_shader);

    set_alt_trackball(tex_spot_trackball);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: event_d
//       Access: Public
//  Description: Toggle projected shadows
////////////////////////////////////////////////////////////////////
void event_d(CPT_Event)
{
  static bool projtex_shadow_mode = false;

  projtex_shadow_mode = !projtex_shadow_mode;
  if (!projtex_shadow_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, proj_shadow);
    set_alt_trackball(NULL);

  }
  else
  {
    set_shader(room_arc, proj_shadow);
    set_alt_trackball(tex_spot_trackball);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: event_r
//       Access: Public
//  Description: Toggle planar reflection
////////////////////////////////////////////////////////////////////
void event_r(CPT_Event)
{
  static bool plane_reflect_mode = false;

  plane_reflect_mode = !plane_reflect_mode;
  if (!plane_reflect_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, preflect);
  }
  else
  {
    // Set an override on the initial state.
    set_shader(room_arc, preflect);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: event_e
//       Access: Public
//  Description: Toggle sphere texture
////////////////////////////////////////////////////////////////////
void event_e(CPT_Event) {
  static bool spheretex_mode = false;

  spheretex_mode = !spheretex_mode;
  if (!spheretex_mode) {
    // Set the normal mode on the render arc.
    //clear_shader(first_arc, spheretex);
    clear_shader(jack_arc, spheretex);

  } else {
    // Set an override on the initial state.
    //set_shader(first_arc, spheretex);
    set_shader(jack_arc, spheretex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: event_h
//       Access: Public
//  Description: Toggle sphere texture highlight
////////////////////////////////////////////////////////////////////
void event_h(CPT_Event) {
  static bool highlight_mode = false;

  highlight_mode = !highlight_mode;
  if (!highlight_mode) {
    // Set the normal mode on the render arc.
    clear_shader(jack_arc, highlight);

  } else {
    set_shader(jack_arc, highlight);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: event_h
//       Access: Public
//  Description: Toggle sphere texture reflector
////////////////////////////////////////////////////////////////////
void event_m(CPT_Event) {
  static bool sphere_reflect_mode = false;

  sphere_reflect_mode = !sphere_reflect_mode;
  if (!sphere_reflect_mode) {
    // Set the normal mode on the render arc.
    clear_shader(jack_arc, sreflect);

  } else {
    // Set an override on the initial state.
    set_shader(jack_arc, sreflect);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: event_o
//       Access: Public
//  Description: Toggle outline shader
////////////////////////////////////////////////////////////////////
void event_o(CPT_Event) {
  static bool outline_mode = false;

  outline_mode = !outline_mode;
  if (!outline_mode) {
    // Set the normal mode on the render arc.
    clear_shader(jack_arc, outline_shader);

  } else {
    set_shader(jack_arc, outline_shader);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: setup_projtex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void setup_projtex(void)
{
  // Create a projected texture shader

  // Put the texture projector into the scene graph
  tex_proj = new LensNode("texture_projector");
  RenderRelation* proj_arc = new RenderRelation(render, tex_proj);

  // Create a trackball to spin this around.
  tex_proj_trackball = new Trackball("tex_proj_trackball");
  tex_proj_trackball->set_invert(false);
  tex_proj_trackball->set_rel_to(cameras);
  tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(proj_arc);
  new DataRelation(tex_proj_trackball, tball2cam);

  // Raise it and aim it at the origin
  LMatrix4f proj_mat;
  LPoint3f proj_pos = LPoint3f::rfu(2., 3., 8.);
  LVector3f fwd_vec = proj_pos - center_pos;
  look_at(proj_mat, -fwd_vec);
  proj_mat.set_row(3, proj_pos);
  tex_proj_trackball->set_mat(proj_mat);
  proj_arc->set_transition(new TransformTransition(proj_mat));

  // Create a shader for the texture projector
  proj_shader = new ProjtexShader(tex);
  proj_shader->set_priority(150);
  proj_shader->add_frustum(tex_proj);

#define DISPLAY_TEXPROJFRUST
#ifdef DISPLAY_TEXPROJFRUST
  // Display a wireframe representation of the texture projector frustum
  PT(Geom) proj_geom =
    tex_proj->get_lens()->make_geometry();
  GeomNode* proj_geom_node = new GeomNode("proj_geometry");
  proj_geom_node->add_geom(proj_geom);
  RenderRelation *prr = new RenderRelation(tex_proj, proj_geom_node);
  LightTransition *plt = new LightTransition(LightTransition::all_off());
  prr->set_transition(plt);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: setup_spotlight
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void setup_spotlight(void)
{
  // Create a projected texture spotlight shader
  tex_proj_spot = new Spotlight("tex_proj_spotlight");
  //Push out the far clipping plane of the spotlight frustum
  PT(Lens) lens = new PerspectiveLens;
  lens->set_fov(45.0f);
  lens->set_near(f._fnear);
  lens->set_far(13.0f);
  tex_proj_spot->set_lens(lens);

  spot_arc = new RenderRelation(render, tex_proj_spot, 10);

  // Create a trackball to spin this around.
  tex_spot_trackball = new Trackball("tex_spot_trackball");
  tex_spot_trackball->set_invert(false);
  tex_spot_trackball->set_rel_to(cameras);
  tball2cam = new Transform2SG("tball2cam");
  tball2cam->set_arc(spot_arc);
  new DataRelation(tex_spot_trackball, tball2cam);

  // Raise it and aim it at the origin
  LMatrix4f spot_mat;
  LPoint3f spot_pos = LPoint3f::rfu(-4., -3., 8.);
  LVector3f spot_vec = spot_pos - center_pos;
  look_at(spot_mat, -spot_vec);
  spot_mat.set_row(3, spot_pos);
  tex_spot_trackball->set_mat(spot_mat);
  spot_arc->set_transition(new TransformTransition(spot_mat));

  // Create a shader for the spotlight
  spot_shader = new SpotlightShader;
  spot_shader->set_priority(150);
  spot_shader->add_frustum(tex_proj_spot);

#define DISPLAY_TEXPROJSPOTFRUST
#ifdef DISPLAY_TEXPROJSPOTFRUST
  // Display a wireframe representation of the spotlight frustum
  Colorf color_red(1., 0., 0., 1.);
  PT(Geom) frust_geom =
    tex_proj_spot->get_lens()->make_geometry();
  GeomNode* frust_geom_node = new GeomNode("frustum_geometry");
  frust_geom_node->add_geom(frust_geom);
  RenderRelation *rr = new RenderRelation(tex_proj_spot, frust_geom_node);
  LightTransition *lt = new LightTransition(LightTransition::all_off());
  rr->set_transition(lt);
#endif

#define DISPLAY_SHAFT
#ifdef DISPLAY_SHAFT
  // Draw a light shaft for the spotlight
  NamedNode* shaft = tex_proj_spot->make_geometry(0.05, 8.0, 36);
  RenderRelation *sr = new RenderRelation(tex_proj_spot, shaft, 10);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: setup_planar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void setup_planar(void)
{
  // Create a planar reflection
  Planef p(LVector3f(0., 0., 1.), LVector3f(0., 0., -10.));
  PlaneNode* plane_node = new PlaneNode;
  plane_node->set_plane(p);
  preflect = new PlanarReflector(plane_node);
}

////////////////////////////////////////////////////////////////////
//     Function: setup_projshadow
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void setup_projshadow(void)
{
  // Create a projected texture shadower
  proj_shadow = new ProjtexShadower;
  proj_shadow->add_frustum(tex_proj_spot);
  proj_shadow->set_priority(150);
  if (root != (NamedNode *)NULL) {
    proj_shadow->add_caster(root);
    preflect->add_caster(root);
  } else if (geomnode != (GeomNode *)NULL) {
    proj_shadow->add_caster(geomnode);
    preflect->add_caster(geomnode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: setup_vizes
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void setup_vizes(void)
{
  // and now for some multipass partial visualization
  Shader::Visualize* v = Shader::get_viz();
  std::string viztype = min_shader.GetString("multipass-viz", "none");

 if (viztype == "old-style")
  {
    PT(GraphicsWindow) w(main_win);
    v = new Oldviz(w);
  }
  else if (viztype == "new-single")
  {
    PT(GraphicsPipe) p(((GraphicsPipe*)(main_win->get_pipe())));
    v = new Viztex(p);
  }
  else if (viztype == "new-tile")
  {
    PT(GraphicsPipe) p(((GraphicsPipe*)(main_win->get_pipe())));
    v = new Tiledviz(p);
  }

  Shader::set_viz(v);
}

////////////////////////////////////////////////////////////////////
//     Function: min_shader_keys
//       Access: Public
//  Description: Set event handlers for the various keys needed, and
//               do any initialization necessary
////////////////////////////////////////////////////////////////////
void min_shader_keys(EventHandler &eh) {
  Loader loader;

  eh.add_hook("p", event_p);
  eh.add_hook("s", event_s);
  eh.add_hook("d", event_d);
  eh.add_hook("r", event_r);
  eh.add_hook("e", event_e);
  eh.add_hook("h", event_h);
  eh.add_hook("m", event_m);
  eh.add_hook("o", event_o);

  // Load a texture to project
  tex = TexturePool::load_texture("smiley.rgba");
  tex->set_minfilter(Texture::FT_linear);
  tex->set_magfilter(Texture::FT_linear);
  tex->set_wrapu(Texture::WM_clamp);
  tex->set_wrapv(Texture::WM_clamp);

//--------------------PROJECTED TEXTURE SHADER-------------------
  setup_projtex();
//--------------------SPOTLIGHT SHADER-------------------
  setup_spotlight();
//--------------------PLANAR REFLECTOR SHADER-------------------
  setup_planar();
//--------------------SHADOW SHADER-------------------
  setup_projshadow();
//--------------------SPHERE TEXTURE SHADER-------------------
  spheretex = new SpheretexShader(tex);
  spheretex->set_priority(150);
//--------------------SPHERE HIGHLIGHTER TEXTURE SHADER-------------------
  highlight = new SpheretexHighlighter(32);
  highlight->add_frustum(tex_proj_spot);
  highlight->set_priority(150);
//--------------------SPHERE REFLECTOR TEXTURE SHADER-------------------
  sreflect = new SpheretexReflector;
  sreflect->add_frustum(tex_proj_spot);
  sreflect->set_priority(150);
  sreflect->add_caster(root);
//--------------------OUTLINE SHADER-------------------
  outline_shader = new OutlineShader;
  outline_shader->set_priority(150);

  // Load the room file
  PT_NamedNode room = DCAST(NamedNode, loader.load_sync("big-room.egg"));
  if (room != (NamedNode *)NULL) {
    room_arc = new RenderRelation(render, room, 20);

    sreflect->add_caster(room);
  }

  // Load jack
  PT_NamedNode jack = DCAST(NamedNode, loader.load_sync("jack.egg"));
  if (jack != (NamedNode *)NULL) {
    jack_arc = new RenderRelation(render, jack);
    LMatrix4f jack_mat = LMatrix4f::ident_mat();
    LPoint3f jack_pos = LPoint3f::rfu(-2., -2., -6.);
    jack_mat.set_row(3, jack_pos);
    jack_arc->set_transition(new TransformTransition(jack_mat));

    proj_shadow->add_caster(jack);
    preflect->add_caster(jack);
  }

  // Load jack
  PT_NamedNode smiley = DCAST(NamedNode, loader.load_sync("smiley.egg"));
  if (jack != (NamedNode *)NULL) {
    smiley_arc = new RenderRelation(render, smiley);

    proj_shadow->add_caster(smiley);
    preflect->add_caster(smiley);
  }

  // Load up a camera model to visualize our eyepoint.
  PT_NamedNode camera_model = DCAST(NamedNode, loader.load_sync("camera.egg"));
  if (camera_model != (NamedNode *)NULL) {
    camera_model_arc = new RenderRelation(cameras, camera_model);

    sreflect->add_caster(camera_model);
    proj_shadow->add_caster(camera_model);
    preflect->add_caster(camera_model);
  }

  // Set up a transition for the spotlight
  light_transition = new LightTransition;
  if (first_arc != (NodeRelation *)NULL) {
    first_arc->set_transition(light_transition);
  }
  if (jack_arc != (NodeRelation *)NULL) {
    jack_arc->set_transition(light_transition);
  }

  setup_vizes();
}

int main(int argc, char *argv[]) {
  extra_display_func = &shader_display_func;
  define_keys = &min_shader_keys;
  return framework_main(argc, argv);
}

