// Filename: graphicsPipe.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "graphicsPipe.h"
#include "config_display.h"

#include <load_dso.h>
#include <filename.h>

#include <algorithm>

// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle GraphicsPipe::_type_handle;
TypeHandle GraphicsPipe::PipeSpec::_type_handle;

GraphicsPipe::PipeFactory GraphicsPipe::_factory;

// These static members are pointers rather than concrete objects so
// we can guarantee order of creation at static init time.
GraphicsPipe::Pipes *GraphicsPipe::_all_pipes = NULL;

GraphicsPipe::PipeSpec::~PipeSpec(void) {}

TypeHandle GraphicsPipe::PipeSpec::get_class_type(void) {
  return _type_handle;
}

void GraphicsPipe::PipeSpec::init_type(void) {
  PipeParam::init_type();
  register_type(_type_handle, "GraphicsPipe::PipeSpec",
		PipeParam::get_class_type());
}

TypeHandle GraphicsPipe::PipeSpec::get_type(void) const {
  return get_class_type();
}

TypeHandle GraphicsPipe::PipeSpec::force_init_type(void) {
  init_type();
  return get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe(const PipeSpecifier &spec) :
  Namable(spec.get_name()) 
{
  // Add ourself to the global list of pipes.
  get_all_pipes().push_back(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Default Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe() {
  display_cat.error()
    << "GraphicsPipes should not be called with default constructor" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe(const GraphicsPipe&) {
  display_cat.error()
    << "GraphicsPipes should not be copied" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Assignment Operator
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsPipe &GraphicsPipe::
operator=(const GraphicsPipe&) {
  display_cat.error()
  << "GraphicsPipes should not be assigned" << endl;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphicsPipe::
~GraphicsPipe() {
  // We don't have to destruct our child windows explicitly, since
  // they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  Windows::const_iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    (*wi)->_pipe = NULL;
  }

  // Remove ourself from the global list.
  Pipes &all_pipes = get_all_pipes();
  Pipes::iterator pi = 
    find(all_pipes.begin(), all_pipes.end(), this);
  if (pi != all_pipes.end()) {
    all_pipes.erase(pi);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_window
//       Access: Public
//  Description: Creates and returns a new window on the pipe.  The
//               window will automatically be added to the pipe's
//               reference-countling list of windows; it must later be
//               explicitly removed to destroy it (you should not
//               attempt to delete it).
////////////////////////////////////////////////////////////////////
GraphicsWindow *GraphicsPipe::
make_window() {
  FactoryParams params;
  params.add_param(new GraphicsWindow::WindowPipe(this));

  GraphicsWindow *win = GraphicsWindow::_factory.
    make_instance(get_window_type(), params);
  nassertr(win != (GraphicsWindow *)NULL, NULL);

  add_window(win);
  return win;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_window
//       Access: Public
//  Description: Creates and returns a new window on the pipe.  The
//               window will automatically be added to the pipe's
//               reference-countling list of windows; it must later be
//               explicitly removed to destroy it (you should not
//               attempt to delete it).
////////////////////////////////////////////////////////////////////
GraphicsWindow *GraphicsPipe::
make_window(const GraphicsWindow::Properties &props) {
  FactoryParams params;
  params.add_param(new GraphicsWindow::WindowPipe(this));
  params.add_param(new GraphicsWindow::WindowProps(props));

  GraphicsWindow *win = GraphicsWindow::_factory.
    make_instance(get_window_type(), params);
  nassertr(win != (GraphicsWindow *)NULL, NULL);

  add_window(win);
  return win;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::remove_window
//       Access: Public
//  Description: Deletes a previously-created window by removing it
//               from the reference-counting list of windows owned by
//               the pipe.  Note that the window will not actually be
//               deleted until all pointers to it (for instance, on
//               mouse objects, etc.) are cleared.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
remove_window(GraphicsWindow *window) {
  // For whatever reason, VC++ considers == ambiguous unless we
  // compare it to a PT(GraphicsWindow) instead of a GraphicsWindow*.
  PT(GraphicsWindow) ptwin = window;
  Windows::iterator wi = 
    find(_windows.begin(), _windows.end(), ptwin);
  if (wi != _windows.end()) {
    _windows.erase(wi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_num_windows
//       Access: Public
//  Description: Returns the number of windows that have been created
//               on this pipe.
////////////////////////////////////////////////////////////////////
int GraphicsPipe::
get_num_windows() const {
  return _windows.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_window
//       Access: Public
//  Description: Returns the nth window associated with this pipe.  n
//               must be between 0 and get_num_windows().
////////////////////////////////////////////////////////////////////
GraphicsWindow *GraphicsPipe::
get_window(int n) const {
  nassertr(n >= 0 && n < get_num_windows(), NULL);
  return _windows[n];
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_num_pipes
//       Access: Public, Static
//  Description: Returns the total number of pipes in the universe.
////////////////////////////////////////////////////////////////////
int GraphicsPipe::
get_num_pipes() {
  return get_all_pipes().size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_pipe
//       Access: Public, Static
//  Description: Returns the nth pipe in the universe.  n must be
//               between 0 and get_num_pipes().
////////////////////////////////////////////////////////////////////
GraphicsPipe *GraphicsPipe::
get_pipe(int n) {
  nassertr(n >= 0 && n < get_num_pipes(), NULL);
  return get_all_pipes()[n];
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_num_hw_channels
//       Access: Protected, Virtual
//  Description: Returns the number of hardware channels available for
//               pipes of this type.  See get_hw_channel().
////////////////////////////////////////////////////////////////////
int GraphicsPipe::
get_num_hw_channels() {
  return 0;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_hw_channel
//       Access: Public, Virtual
//  Description: Creates and returns an accessor to the
//               HardwareChannel at the given index number, which must
//               be in the range 0 <= index < get_num_hw_channels().
//               This function will return NULL if the index number is
//               out of range or the hardware channel at that index is
//               unavailable.
//
//               Most kinds of GraphicsPipes do not have any special
//               hardware channels available, and this function will
//               always return NULL.
////////////////////////////////////////////////////////////////////
HardwareChannel *GraphicsPipe::
get_hw_channel(GraphicsWindow*, int) {
  return (HardwareChannel*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::add_window
//       Access: Protected
//  Description: Adds a newly-created window to the set of windows
//               owned by the pipe.  This function is intended to be
//               called from a derived class' make_window() function.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
add_window(GraphicsWindow *win) {
  _windows.push_back(win);
}

void GraphicsPipe::read_priorities(void) {
  if (_factory.get_num_preferred() == 0) {
    Config::ConfigTable::Symbol::iterator i;
    for (i = preferred_pipe_begin(); i != preferred_pipe_end(); ++i) {
      ConfigString type_name = (*i).Val();
      TypeHandle type = TypeRegistry::ptr()->find_type(type_name);
      if (type == TypeHandle::none()) {
	display_cat.warning()
	  << "Unknown type requested for pipe preference: " << type_name
	  << "\n";
      } else {
	display_cat.debug()
	  << "Specifying type " << type << " for pipe preference.\n";
	_factory.add_preferred(type);
      }
    }
  }
}

void GraphicsPipe::resolve_modules(void) {
  Config::ConfigTable::Symbol::iterator i;

  for (i=pipe_modules_begin(); i!=pipe_modules_end(); ++i) {
    Filename dlname = Filename::dso_filename("lib" + (*i).Val() + ".so");
    display_cat.info()
      << "loading display module: " << dlname.to_os_specific() << endl;
    void *tmp = load_dso(dlname.to_os_specific());
    if (tmp == (void*)0L) {
      display_cat.info()
	<< "Unable to load: " << load_dso_error() << endl;
    }
  }
  for (i=gsg_modules_begin(); i!=gsg_modules_end(); ++i) {
    Filename dlname = Filename::dso_filename("lib" + (*i).Val() + ".so");
    display_cat.info()
      << "loading GSG module: " << dlname.to_os_specific() << endl;
    void *tmp = load_dso(dlname.to_os_specific());
    if (tmp == (void*)0L) {
      display_cat.info()
	<< "Unable to load: " << load_dso_error() << endl;
    }
  }

  read_priorities();
  GraphicsWindow::read_priorities();
  GraphicsPipe::read_priorities();
}
