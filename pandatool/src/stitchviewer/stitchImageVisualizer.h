// Filename: stitchImageVisualizer.h
// Created by:  drose (05Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGEVISUALIZER_H
#define STITCHIMAGEVISUALIZER_H

#include "stitchImage.h"
#include "stitchImageOutputter.h"

#include <pointerTo.h>
#include <graphicsPipe.h>
#include <graphicsWindow.h>
#include <namedNode.h>
#include <renderRelation.h>
#include <mouse.h>
#include <trackball.h>
#include <nodeTransition.h>
#include <eventHandler.h>
#include <texture.h>

class PNMImage;
class ChanCfgOverrides;

class StitchImageVisualizer : public StitchImageOutputter,
			      public GraphicsWindow::Callback {
public:
  StitchImageVisualizer();
  virtual void add_input_image(StitchImage *image);
  virtual void add_output_image(StitchImage *image);
  virtual void add_stitcher(Stitcher *stitcher);

  virtual void execute();
  
protected:
  void setup();

  class Image {
  public:
    Image(StitchImage *image, int index, bool scale);
    Image(const Image &copy);
    void operator = (const Image &copy);

    StitchImage *_image;
    RenderRelation *_arc;
    PT(Texture) _tex;
    bool _viz;
    int _index;
  };

  virtual void override_chan_cfg(ChanCfgOverrides &override);
  virtual void setup_camera(const RenderRelation &camera_arc);
  virtual bool is_interactive() const;

  void toggle_viz(Image &im);
  virtual void create_image_geometry(Image &im);

  static void static_handle_event(CPT(Event) event);
  void handle_event(CPT(Event) event);

  virtual void draw(bool);
  virtual void idle();

  typedef vector<Image> Images;
  Images _images;

  PT(GraphicsPipe) _main_pipe;
  PT(GraphicsWindow) _main_win;
  NodeAttributes _initial_state;
  PT(NamedNode) _render;
  PT(NamedNode) _cameras;
  PT(NamedNode) _data_root;
  PT(MouseAndKeyboard) _mak;
  PT(Trackball) _trackball;
  EventHandler _event_handler;

  static StitchImageVisualizer *_static_siv;
  bool _running;
};

#endif

