// Filename: mouse.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef MOUSE_H
#define MOUSE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <dataNode.h>
#include <modifierButtonDataTransition.h>
#include <modifierButtonDataAttribute.h>
#include <vec3DataTransition.h>
#include <vec3DataAttribute.h>
#include <buttonEventDataTransition.h>
#include <buttonEventDataAttribute.h>
#include <graphicsWindow.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
const int MIN_MOVE = 2;

////////////////////////////////////////////////////////////////////
//       Class : MouseAndKeyboard
// Description : Reads the mouse and/or keyboard data sent from a
//               GraphicsWindow, and transmits it down the data graph.
//
//               The mouse and keyboard devices are bundled together
//               into one device here, because they interrelate so
//               much.  A mouse might be constrained by the holding
//               down of the shift key, for instance, or the clicking
//               of the mouse button might be handled in much the same
//               way as a keyboard key.
//
//               Mouse data is sent down the data graph as an x,y
//               position as well as the set of buttons currently
//               being held down; keyboard data is sent down as a set
//               of keypress events in an EventDataAttribute.  To
//               throw these events to the system, you must child an
//               EventThrower to the MouseAndKeyboard object;
//               otherwise, the events will be discarded.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseAndKeyboard : public DataNode {
public:
	
  MouseAndKeyboard(GraphicsWindow *window, int device,
		   const string& name = "");

  virtual void transmit_data(NodeAttributes &data);

public:
  NodeAttributes _got_mouse_attrib;
  NodeAttributes _no_mouse_attrib;
  PT(ModifierButtonDataAttribute) _mods;
  PT(Vec3DataAttribute) _pixel_xyz;
  PT(Vec3DataAttribute) _xyz;
  PT(ButtonEventDataAttribute) _button_events;
  
  static TypeHandle _mods_type;
  static TypeHandle _pixel_xyz_type;
  static TypeHandle _xyz_type;
  static TypeHandle _button_events_type;
  
protected:
  PT(GraphicsWindow) _window;
  int _device;
  
public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#endif
