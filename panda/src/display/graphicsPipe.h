// Filename: graphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef GRAPHICSPIPE_H
#define GRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "graphicsWindow.h"
#include "hardwareChannel.h"
#include "pipeSpec.h"

#include <typedReferenceCount.h>
#include <namable.h>
#include <factory.h>
#include <factoryParam.h>

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class GraphicsPipe;
class glxDisplay;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsPipe : public TypedReferenceCount, public Namable {
PUBLISHED:
  GraphicsPipe(const PipeSpecifier &spec);
  virtual ~GraphicsPipe();

  GraphicsWindow *make_window();
  GraphicsWindow *make_window(const GraphicsWindow::Properties&);

  void remove_window(GraphicsWindow *window);

  virtual TypeHandle get_window_type() const=0;

  // Functions for obtaining the set of windows associated with this
  // pipe, and the set of all GraphicsPipes in the world.

  int get_num_windows() const;
  GraphicsWindow *get_window(int n) const;

  static int get_num_pipes();
  static GraphicsPipe *get_pipe(int n);

public:
  // This function's interface must be defined here even though we
  // know nothing about glx displays at this point.
  virtual glxDisplay *get_glx_display();

protected:
  virtual int get_num_hw_channels();
  virtual HardwareChannel *get_hw_channel(GraphicsWindow *, int);

  void add_window(GraphicsWindow *win);

public:
  // Factory stuff
  typedef Factory<GraphicsPipe> PipeFactory;
  typedef FactoryParam PipeParam;

  // Make a factory parameter type for the pipe specifier
  class EXPCL_PANDA PipeSpec : public PipeParam {
  public:
    INLINE PipeSpec(void) : PipeParam() {}
    INLINE PipeSpec(PipeSpecifier& p) : PipeParam(), _p(p) {}
    virtual ~PipeSpec(void);
    INLINE const PipeSpecifier &get_specifier(void) { return _p; }
  public:
    static TypeHandle get_class_type(void);
    static void init_type(void);
    virtual TypeHandle get_type(void) const;
    virtual TypeHandle force_init_type(void);
  private:
    PipeSpecifier _p;

    static TypeHandle _type_handle;
  };

  static PipeFactory _factory;

  static void resolve_modules(void);

private:
  static void read_priorities(void);

protected:

  GraphicsPipe();
  GraphicsPipe(const GraphicsPipe &copy);
  GraphicsPipe &operator = (const GraphicsPipe &copy);

private:
  // Some private type declarations.  These must be declared here so
  // we can declare the public iterator types, below.
  typedef vector<PT(GraphicsWindow)> Windows;
  typedef vector<GraphicsPipe *> Pipes;

  Windows _windows;

  static Pipes *_all_pipes;
  INLINE static Pipes &get_all_pipes();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    Namable::init_type();
    register_type(_type_handle, "GraphicsPipe",
		  TypedReferenceCount::get_class_type(),
		  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
 
  // this is so it can call get_hw_channel
  friend class GraphicsWindow;
};

#include "graphicsPipe.I"

#endif /* GRAPHICSPIPE_H */
