// Filename: graphicsChannel.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef GRAPHICSCHANNEL_H
#define GRAPHICSCHANNEL_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "graphicsLayer.h"

#include <typedReferenceCount.h>
#include <pointerTo.h>

#include <vector>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class GraphicsChannel;
class GraphicsPipe;
class GraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsChannel
// Description : This represents a single hardware output.  Typically
//               there is exactly one channel per window, but some
//               implementations (e.g. SGI) support potentially
//               several different video channel ports connected to
//               different parts within a window.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsChannel : public TypedReferenceCount {
protected:

  GraphicsChannel();

public:
  GraphicsChannel(GraphicsWindow *);
  virtual ~GraphicsChannel();

  GraphicsLayer *make_layer(int index = -1);
  int get_num_layers() const;
  GraphicsLayer *get_layer(int index) const;
  void move_layer(int from_index, int to_index);
  void remove_layer(int index);

  GraphicsWindow *get_window() const;
  GraphicsPipe *get_pipe() const;

  virtual void window_resized(int x, int y);

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

private:
  GraphicsWindow *_window;
  bool _is_active;

  typedef vector<PT(GraphicsLayer)> GraphicsLayers;
  GraphicsLayers _layers;

private:

  GraphicsChannel(const GraphicsChannel&);
  GraphicsChannel& operator=(const GraphicsChannel&);


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsChannel",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

  friend class GraphicsWindow;
};

#include "graphicsChannel.I"

#endif /* GRAPHICSCHANNEL_H */
