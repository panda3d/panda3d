// Filename: shader_test.cxx
// Created by:  
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

#include "eventHandler.h"
#include "projtexShader.h"
#include "shaderTransition.h"
#include "renderRelation.h"
#include "nodeRelation.h"
#include "chancfg.h"
#include <string>
#include "texGenTransition.h"
#include "colorBlendTransition.h"
#include "colorMaskTransition.h"
#include "textureTransition.h"
#include "texMatrixTransition.h"
#include "spotlightShader.h"
#include "light.h"
#include "eggLoader.h"
#include "look_at.h"
#include "transformTransition.h"
#include "geomNode.h"
#include "lightTransition.h"
#include "spotlight.h"
#include "projtexShadower.h"
#include "spheretexHighlighter.h"
#include "spheretexReflector.h"
#include "trackball.h"
#include "transform2sg.h"
#include "mouse.h"
#include "graphicsWindow.h"
#include "planarReflector.h"
#include "stencilTransition.h"
#include "plane.h"
#include "renderBuffer.h"
#include "outlineShader.h"
#include "pt_NamedNode.h"
#include "dataRelation.h"
#include "geomLine.h"

#include "dconfig.h"

#ifdef SHADER_VERBOSE
#include "indent.h"
#endif

Configure(shader_test);
ConfigureFn(shader_test) {
}

PT(LensNode) tex_proj;
PT(Trackball) tex_proj_trackball;
PT(ProjtexShader) proj_shader;
PT(ProjtexShadower) proj_shadow;
PT(SpheretexShader) spheretex;
PT(SpheretexHighlighter) highlight;
PT(SpheretexReflector) sreflect;
PT(PlanarReflector) preflect;
PT(OutlineShader) outline_shader;

ShaderTransition shader_trans;

PT(Spotlight) tex_proj_spot;
PT(Trackball) tex_spot_trackball;
PT(SpotlightShader) spot_shader;

RenderRelation* room_arc;
RenderRelation* spot_arc;
RenderRelation* jack_arc;
RenderRelation* camera_model_arc;

PT(LightTransition) light_transition;

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

class SomeViz : public Shader::Visualize {
public:
  SomeViz(void);
  virtual ~SomeViz(void);
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

SomeViz::SomeViz(void) {}

SomeViz::~SomeViz(void) {}

void SomeViz::DisplayTexture(PT(Texture)& tex, Shader*) {
  if (tex->has_ram_image())
    _texs.push_back(tex);
  else {
    GraphicsStateGuardian* g = main_win->get_gsg();
    PT(PixelBuffer) pb(new PixelBuffer);
    g->texture_to_pixel_buffer(tex->prepare(g), pb);
    _pbs.push_back(pb);
  }
}

void SomeViz::DisplayPixelBuffer(PT(PixelBuffer)& pb, Shader*) {
  _pbs.push_back(pb);
}

class Oldviz : public SomeViz {
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

class Viztex : public SomeViz {
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
      //JMC: Addeded cameras and render for parameters of ChanConfig
      w = ChanConfig(_pipe, "single", (Node *)NULL, render, override);
      //JMC: Removed set_active calls to channels
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
      //JMC: Addeded cameras and render for parameters of ChanConfig
      w = ChanConfig(_pipe, "single", cameras, render, override);
       //JMC: Removed set_active calls to channels
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
  //JMC: Added layer count
  int layer_count = 0;
  winiter _witer = _wins.begin();

#ifdef SHADER_VERBOSE
    int _level = 2;
    if (!_texs.empty())
    {
      nout << "Tiledvid::Flush:" << endl;
    }
#endif

#ifdef SHADER_VERBOSE
    if (!_texs.empty())
    {
      indent(nout, _level) << "TexList Loop" << endl;
    }
    _level += 2;
#endif
  for (texiter texi=_texs.begin(); texi!=_texs.end(); ++texi) {
    GraphicsWindow* w;
    DisplayRegion* d;

    if (_witer == _wins.end()) {
      ChanCfgOverrides override;

      override.setField(ChanCfgOverrides::Mask,
                        ((unsigned int)(W_SINGLE)));
      override.setField(ChanCfgOverrides::Title, "Multipass partial");
      override.setField(ChanCfgOverrides::SizeX, 512);
      override.setField(ChanCfgOverrides::SizeY, 512);
      override.setField(ChanCfgOverrides::Cameras, false);
      //JMC: Addeded cameras and render for parameters of ChanConfig
      w = ChanConfig(_pipe, "multipass-tile", cameras, render, override);
      //JMC: Removed set_active calls to channels
      _wins.push_back(w);
      _witer = --(_wins.end());
    } else
      w = *_witer;

    int dridx;
    GraphicsStateGuardian* g = w->get_gsg();
    GraphicsChannel *chan = w->get_channel(0);
    //JMC: Added call to get_layer
    GraphicsLayer *layer = chan->get_layer(layer_count);
    const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);
    //JMC: Removed display region iterators for GraphicsChannel.  They
    //no longer exist.
    //JMC: Added call to get_dr
#ifdef SHADER_VERBOSE
    indent(nout, _level) << "Getting display region " << count
                         << " from graphics layer"  << endl;
#endif
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
#ifdef SHADER_VERBOSE
    _level -= 2;
#endif
  for (pbiter pbi=_pbs.begin(); pbi!=_pbs.end(); ++pbi) {
    GraphicsWindow* w;
    DisplayRegion* d;

    if (_witer == _wins.end()) {
      ChanCfgOverrides override;

      override.setField(ChanCfgOverrides::Mask,
                        ((unsigned int)(W_SINGLE)));
      override.setField(ChanCfgOverrides::Title, "Multipass partial");
      override.setField(ChanCfgOverrides::SizeX, 512);
      override.setField(ChanCfgOverrides::SizeY, 512);
      override.setField(ChanCfgOverrides::Cameras, false);
      //JMC: Addeded cameras and render for parameters of ChanConfig
      w = ChanConfig(_pipe, "multipass-tile", cameras, render, override);
      //JMC: Removed set_active calls to channels
      _wins.push_back(w);
      _witer = --(_wins.end());
    } else
      w = *_witer;

    int dridx;
    GraphicsStateGuardian* g = w->get_gsg();
    GraphicsChannel *chan = w->get_channel(0);
    //JMC: Added call to get_layer
    GraphicsLayer *layer = chan->get_layer(0);
    const RenderBuffer& r = g->get_render_buffer(RenderBuffer::T_front);
    //JMC: Removed display region iterators for GraphicsChannel.  They
    //no longer exist.
    //JMC: Added call to get_dr
    d = layer->get_dr(count);
    // g->prepare_display_region(d);
    (*pbi)->draw(g, d, r);
    ++count;
    if (count == 16) {
      count = 0;
      ++_witer;
    }
  }
  _texs.erase(_texs.begin(), _texs.end());
  _pbs.erase(_pbs.begin(), _pbs.end());
}

// nothing extra to do for either of these
void shader_display_func(void) {
  Shader::Visualize* v = Shader::get_viz();
  if (v != (Shader::Visualize*)0L)
    v->Flush();
}

void event_p(CPT_Event) {
  // The "p" key was pressed.  Toggle projected texture.
  static bool projtex_mode = false;

  projtex_mode = !projtex_mode;
  if (!projtex_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, proj_shader);
    clear_shader(first_arc, proj_shader);
    clear_shader(jack_arc, proj_shader);

    set_alt_trackball(NULL);

  } else {
    // Set an override on the initial state.
    set_shader(room_arc, proj_shader);
    set_shader(first_arc, proj_shader);
    set_shader(jack_arc, proj_shader);

    set_alt_trackball(tex_proj_trackball);
  }
}

void event_s(CPT_Event) {
  // The "s" key was pressed.  Toggle projected texture spotlight.
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

void event_d(CPT_Event) {
  // The "d" key was pressed.  Toggle projected texture shadows.
  static bool projtex_shadow_mode = false;

  projtex_shadow_mode = !projtex_shadow_mode;
  if (!projtex_shadow_mode) {
    // Set the normal mode on the render arc.
    clear_shader(room_arc, proj_shadow);
    set_alt_trackball(NULL);

  } else {
    set_shader(room_arc, proj_shadow);
    set_alt_trackball(tex_spot_trackball);
  }
}

void event_h(CPT_Event) {
  // The "h" key was pressed.  Toggle highlight.
  static bool highlight_mode = false;

  highlight_mode = !highlight_mode;
  if (!highlight_mode) {
    // Set the normal mode on the render arc.
    clear_shader(first_arc, highlight);

  } else {
    set_shader(first_arc, highlight);
  }
}

void event_e(CPT_Event) {
  // The "e" key was pressed.  Toggle sphere texture.
  static bool spheretex_mode = false;

  spheretex_mode = !spheretex_mode;
  if (!spheretex_mode) {
    // Set the normal mode on the render arc.
    clear_shader(first_arc, spheretex);

  } else {
    // Set an override on the initial state.
    set_shader(first_arc, spheretex);
  }
}

void event_m(CPT_Event) {
  // The "m" key was pressed.  Toggle sphere reflection.
  static bool sphere_reflect_mode = false;

  sphere_reflect_mode = !sphere_reflect_mode;
  if (!sphere_reflect_mode) {
    // Set the normal mode on the render arc.
    clear_shader(first_arc, sreflect);

  } else {
    // Set an override on the initial state.
    set_shader(first_arc, sreflect);
  }
}

void event_r(CPT_Event) {
  // The "r" key was pressed.  Toggle planar reflection.
  static bool plane_reflect_mode = false;

  plane_reflect_mode = !plane_reflect_mode;
  if (!plane_reflect_mode) {
    // Set the normal mode on the render arc.
    clear_shader(first_arc, preflect);

  } else {
    // Set an override on the initial state.
    set_shader(first_arc, preflect);
  }
}

void event_n(CPT_Event) {
  // The "n" key was pressed.  Toggle the spot light.
  static bool spotlight_on = false;
  spotlight_on = !spotlight_on;
  if (!spotlight_on) {
    light_transition->set_off(tex_proj_spot.p());
    set_alt_trackball(NULL);
  } else {
    light_transition->set_on(tex_proj_spot.p());
    set_alt_trackball(tex_spot_trackball);
  }
}

void event_o(CPT_Event) {
  // The "o" key was pressed.  Toggle outlining.
  static bool outline_mode = false;

  outline_mode = !outline_mode;
  if (!outline_mode) {
    // Set the normal mode on the render arc.
    clear_shader(first_arc, outline_shader);

  } else {
    set_shader(first_arc, outline_shader);
  }
}

void shader_keys(EventHandler &eh) {
  eh.add_hook("p", event_p);
  eh.add_hook("s", event_s);
  eh.add_hook("d", event_d);
  eh.add_hook("h", event_h);
  eh.add_hook("e", event_e);
  eh.add_hook("m", event_m);
  eh.add_hook("r", event_r);
  eh.add_hook("n", event_n);
  eh.add_hook("o", event_o);

  LPoint3f center_pos = LPoint3f::origin();

  // Create a projected texture shader
  // Load a texture to project
  Texture* tex = new Texture;
  tex->read("smiley.rgba");
  tex->set_name("smiley.rgba");

  // Put the texture projector into the scene graph
  tex_proj = new LensNode("texture_projector");
  RenderRelation* proj_arc = new RenderRelation(render, tex_proj);

  // Create a trackball to spin this around.
  tex_proj_trackball = new Trackball("tex_proj_trackball");
  tex_proj_trackball->set_invert(false);
  tex_proj_trackball->set_rel_to(cameras);
  Transform2SG *tball2cam = new Transform2SG("tball2cam");
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
  proj_shader->add_frustum(tex_proj);

#define DISPLAY_TEXPROJFRUST
#ifdef DISPLAY_TEXPROJFRUST
  // Display a wireframe representation of the texture projector frustum
  PT(Geom) proj_geom =
        tex_proj->get_lens()->make_geometry();
  GeomNode* proj_geom_node = new GeomNode("proj_geometry");
  //JMC: Removed _drawable_vector.push_back call and added add_geom
  //call
  proj_geom_node->add_geom(proj_geom);
  RenderRelation *prr = new RenderRelation(tex_proj, proj_geom_node);
  LightTransition *plt = new LightTransition(LightTransition::all_off());
  prr->set_transition(plt);
#endif

  // Create a projected texture spotlight shader
  tex_proj_spot = new Spotlight("tex_proj_spotlight");
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
  spot_shader->add_frustum(tex_proj_spot);

#define DISPLAY_TEXPROJSPOTFRUST
#ifdef DISPLAY_TEXPROJSPOTFRUST
  // Display a wireframe representation of the spotlight frustum
  Colorf color_red(1., 0., 0., 1.);
  PT(Geom) frust_geom =
        tex_proj_spot->get_lens()->make_geometry(color_red);
  GeomNode* frust_geom_node = new GeomNode("frustum_geometry");
  //JMC: Removed _drawable_vector.push_back call and added add_geom
  //call
  frust_geom_node->add_geom(frust_geom);
  RenderRelation *rr = new RenderRelation(tex_proj_spot, frust_geom_node);
  LightTransition *lt = new LightTransition(LightTransition::all_off());
  rr->set_transition(lt);
#endif

#ifdef DISPLAY_SHAFT
  // Draw a light shaft for the spotlight
  NamedNode* shaft = tex_proj_spot->make_geometry(0.05, 8.0, 36);
  new RenderRelation(tex_proj_spot, shaft, 10);
#endif

  // Create a planar reflection
  Planef p(LVector3f(0., 0., 1.), LVector3f(0., 0., -10.));
  PlaneNode* plane_node = new PlaneNode;
  plane_node->set_plane(p);
  preflect = new PlanarReflector(plane_node);

  // Create a projected texture shadower
  proj_shadow = new ProjtexShadower;
  proj_shadow->add_frustum(tex_proj_spot);
  if (root != (NamedNode *)NULL) {
    proj_shadow->add_caster(root);
    preflect->add_caster(root);
  } else if (geomnode != (GeomNode *)NULL) {
    proj_shadow->add_caster(geomnode);
    preflect->add_caster(geomnode);
  }

  // Create a spheretex shader
  spheretex = new SpheretexShader(tex);

  // Create a highlight
  highlight = new SpheretexHighlighter;
  highlight->add_frustum(tex_proj_spot);

  // Create a spheremap reflection
  sreflect = new SpheretexReflector;
  sreflect->add_frustum(tex_proj_spot);

  // Create an outline shader
  outline_shader = new OutlineShader;

  // Load the room file
  PT_NamedNode room = loader.load_sync("big-room.egg");
  if (room != (NamedNode *)NULL) {
    room_arc = new RenderRelation(render, room, 20);

    sreflect->add_caster(room);
    new RenderRelation(room, plane_node);
  }

  // Load jack
  PT_NamedNode jack = loader.load_sync("jack.egg");
  if (jack != (NamedNode *)NULL) {
    jack_arc = new RenderRelation(render, jack);
    LMatrix4f jack_mat = LMatrix4f::ident_mat();
    LPoint3f jack_pos = LPoint3f::rfu(-2., -2., 2.);
    jack_mat.set_row(3, jack_pos);
    jack_arc->set_transition(new TransformTransition(jack_mat));

    sreflect->add_caster(jack);
    proj_shadow->add_caster(jack);
    preflect->add_caster(jack);
  }

  // Load up a camera model to visualize our eyepoint.
  PT_NamedNode camera_model = loader.load_sync("camera.egg");
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

  // and now for some multipass partial visualization
  Shader::Visualize* v = Shader::get_viz();
  std::string viztype = shader_test.GetString("multipass-viz", "none");
  if (viztype == "old-style") {
    PT(GraphicsWindow) w(main_win);
    v = new Oldviz(w);
  } else if (viztype == "new-single") {
    PT(GraphicsPipe) p(((GraphicsPipe*)(main_win->get_pipe())));
    v = new Viztex(p);
  } else if (viztype == "new-tile") {
    PT(GraphicsPipe) p(((GraphicsPipe*)(main_win->get_pipe())));
    v = new Tiledviz(p);
  }
  Shader::set_viz(v);
}

int main(int argc, char *argv[]) {
  extra_display_func = &shader_display_func;
  define_keys = &shader_keys;
  return framework_main(argc, argv);
}





