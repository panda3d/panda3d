/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nativeWindowHandle.h
 * @author drose
 * @date 2009-09-30
 */

#ifndef NATIVEWINDOWHANDLE_H
#define NATIVEWINDOWHANDLE_H

#include "pandabase.h"

#include "windowHandle.h"
#include "get_x11.h"

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

/**
 * This subclass of WindowHandle exists to allow simple creation of a
 * WindowHandle of the appropriate type to the current OS.
 *
 * This class exists for name scoping only.  Don't use the constructor
 * directly; use one of the make_* methods.
 */
class EXPCL_PANDA_DISPLAY NativeWindowHandle final : public WindowHandle {
private:
  INLINE NativeWindowHandle();
  INLINE NativeWindowHandle(const NativeWindowHandle &copy);

PUBLISHED:
  static PT(WindowHandle) make_int(size_t window);
  static PT(WindowHandle) make_subprocess(const Filename &filename);

public:
#if defined(HAVE_X11) && !defined(CPPPARSER)
  static PT(WindowHandle) make_x11(X11_Window window);
#endif  // HAVE_X11

#if defined(WIN32) && !defined(CPPPARSER)
  static PT(WindowHandle) make_win(HWND window);
#endif  // WIN32

public:
  class EXPCL_PANDA_DISPLAY IntHandle : public OSHandle {
  public:
    INLINE IntHandle(size_t handle);
    virtual size_t get_int_handle() const;
    virtual void output(std::ostream &out) const;

    INLINE size_t get_handle() const;

  private:
    size_t _handle;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      OSHandle::init_type();
      register_type(_type_handle, "NativeWindowHandle::IntHandle",
                    OSHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

  class EXPCL_PANDA_DISPLAY SubprocessHandle : public OSHandle {
  public:
    INLINE SubprocessHandle(const Filename &filename);
    virtual void output(std::ostream &out) const;

    INLINE const Filename &get_filename() const;

  private:
    Filename _filename;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      OSHandle::init_type();
      register_type(_type_handle, "NativeWindowHandle::SubprocessHandle",
                    OSHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

#if defined(HAVE_X11) && !defined(CPPPARSER)
  class EXPCL_PANDA_DISPLAY X11Handle : public OSHandle {
  public:
    INLINE X11Handle(X11_Window handle);
    virtual size_t get_int_handle() const;
    virtual void output(std::ostream &out) const;

    INLINE X11_Window get_handle() const;

  private:
    X11_Window _handle;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      OSHandle::init_type();
      register_type(_type_handle, "NativeWindowHandle::X11Handle",
                    OSHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };
#endif  // HAVE_X11


#if defined(WIN32) && !defined(CPPPARSER)
  class EXPCL_PANDA_DISPLAY WinHandle : public OSHandle {
  public:
    INLINE WinHandle(HWND handle);
    virtual size_t get_int_handle() const;
    virtual void output(std::ostream &out) const;

    INLINE HWND get_handle() const;

  private:
    HWND _handle;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      OSHandle::init_type();
      register_type(_type_handle, "NativeWindowHandle::WinHandle",
                    OSHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };
#endif  // WIN32

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WindowHandle::init_type();
    register_type(_type_handle, "NativeWindowHandle",
                  WindowHandle::get_class_type());

    IntHandle::init_type();
    SubprocessHandle::init_type();
#if defined(HAVE_X11) && !defined(CPPPARSER)
    X11Handle::init_type();
#endif
#if defined(WIN32) && !defined(CPPPARSER)
    WinHandle::init_type();
#endif
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "nativeWindowHandle.I"

#endif
