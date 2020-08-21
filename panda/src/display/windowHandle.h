/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowHandle.h
 * @author drose
 * @date 2009-09-30
 */

#ifndef WINDOWHANDLE_H
#define WINDOWHANDLE_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "pointerTo.h"

/**
 * This object represents a window on the desktop, not necessarily a Panda
 * window.  This structure can be assigned to a WindowProperties to indicate a
 * parent window.
 *
 * It also has callbacks so the Panda window can communicate with its parent
 * window, which is particularly important when running embedded in a browser.
 *
 * To create a WindowHandle, you would usually call one of the
 * NativeWindowHandle::make_*() methods, depending on the kind of native
 * window handle object you already have.
 */
class EXPCL_PANDA_DISPLAY WindowHandle : public TypedReferenceCount {
PUBLISHED:
  class OSHandle;

  INLINE WindowHandle(OSHandle *os_handle);
  INLINE WindowHandle(const WindowHandle &copy);
  virtual ~WindowHandle();

  INLINE OSHandle *get_os_handle() const;
  INLINE void set_os_handle(OSHandle *os_handle);
  MAKE_PROPERTY(os_handle, get_os_handle, set_os_handle);

  void send_windows_message(unsigned int msg, int wparam, int lparam);

  size_t get_int_handle() const;

  void output(std::ostream &out) const;

public:
  // Callbacks for communication with the parent window.
  virtual void attach_child(WindowHandle *child);
  virtual void detach_child(WindowHandle *child);

  virtual void request_keyboard_focus(WindowHandle *child);
  virtual void receive_windows_message(unsigned int msg, int wparam, int lparam);

PUBLISHED:
  // This internal pointer within WindowHandle stores the actual OS-specific
  // window handle type, whatever type that is.  It is subclassed for each OS.
  class EXPCL_PANDA_DISPLAY OSHandle : public TypedReferenceCount {
  protected:
    INLINE OSHandle();

  PUBLISHED:
    virtual ~OSHandle();
    virtual size_t get_int_handle() const;
    virtual void output(std::ostream &out) const;

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

protected:
  PT(OSHandle) _os_handle;

  PT(WindowHandle) _keyboard_window;

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

INLINE std::ostream &operator << (std::ostream &out, const WindowHandle &handle) {
  handle.output(out);
  return out;
}

INLINE std::ostream &operator << (std::ostream &out, const WindowHandle::OSHandle &handle) {
  handle.output(out);
  return out;
}

#endif
