// Filename: windowHandle.h
// Created by:  drose (30Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef WINDOWHANDLE_H
#define WINDOWHANDLE_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : WindowHandle
// Description : This object represents a window on the desktop, not
//               necessarily a Panda window.  This structure can be
//               assigned to a WindowProperties to indicate a parent
//               window.
//
//               It also has callbacks so the Panda window can
//               communicate with its parent window, which is
//               particularly important when running embedded in a
//               browser.
//
//               To create a WindowHandle, you would typically call
//               GraphicsPipe::make_window_handle, for the particular
//               GraphicsPipe that you have constructed.  (Each
//               OS-specific GraphicsPipe has a make_window_handle()
//               method that receives an OS-specific window handle,
//               whatever that means for a particular OS, and creates
//               a WindowHandle object wrapping it.)
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY WindowHandle : public TypedReferenceCount {
PUBLISHED:
  class OSHandle;

  INLINE WindowHandle(OSHandle *os_handle);
  INLINE WindowHandle(const WindowHandle &copy);
  virtual ~WindowHandle();

  INLINE OSHandle *get_os_handle() const;
  INLINE void set_os_handle(OSHandle *os_handle);

  string get_string_handle() const;

  void output(ostream &out) const;

  // This internal pointer within WindowHandle stores the actual
  // OS-specific window handle type, whatever type that is.  It is
  // subclassed for each OS.
  class OSHandle : public TypedReferenceCount {
  protected:
    INLINE OSHandle();

  PUBLISHED:
    virtual ~OSHandle();
    virtual void format_string_handle(ostream &out) const;
    virtual void output(ostream &out) const;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      TypedReferenceCount::init_type();
      register_type(_type_handle, "WindowHandle::OSHandle",
                    TypedReferenceCount::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
    
  private:
    static TypeHandle _type_handle;
  };

  // This class only exists for backward compatibility; it stores the
  // OS handle as a size_t object, as the WindowProperties object did
  // historically.  New code should use
  // GraphicsPipe::make_window_handle() instead of this.
  class IntHandle : public OSHandle {
  PUBLISHED:
    INLINE IntHandle(size_t handle);
    virtual void format_string_handle(ostream &out) const;
    virtual void output(ostream &out) const;

    INLINE size_t get_handle() const;

  private:
    size_t _handle;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      OSHandle::init_type();
      register_type(_type_handle, "WindowHandle::IntHandle",
                    OSHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
    
  private:
    static TypeHandle _type_handle;
  };
  
    

protected:
  // Callbacks for communication with the parent window.
  virtual void attach_child(WindowHandle *child);
  virtual void detach_child(WindowHandle *child);

  virtual void set_keyboard_focus(WindowHandle *child);

protected:
  PT(OSHandle) _os_handle;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "WindowHandle",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "windowHandle.I"

INLINE ostream &operator << (ostream &out, const WindowHandle &handle) {
  handle.output(out);
  return out;
}

INLINE ostream &operator << (ostream &out, const WindowHandle::OSHandle &handle) {
  handle.output(out);
  return out;
}

#endif
