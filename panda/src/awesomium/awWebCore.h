// Filename: awWebCore.h
// Created by:  rurbino (12Oct09)
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
#ifndef AWWEBCORE_H
#define AWWEBCORE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "awesomium_includes.h"

class AwWebView;
////////////////////////////////////////////////////////////////////
//       Class : AwWebCore
// Description : Thin wrappings arround WebCore.h
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAAWESOMIUM AwWebCore : public TypedReferenceCount, public Awesomium::WebCore {
PUBLISHED:
  /**
  * An enumeration of the three verbosity settings for the Awesomium Log.
  */
  enum LogLevel
  {
    LOG_NONE ,      // No log is created
    LOG_NORMAL ,        // Logs only errors
    LOG_VERBOSE         // Logs everything
  };

  /**
  * An enumeration of the two output pixel formats that WebView::render will use.
  */
  enum PixelFormat
  {
    PF_BGRA,    // BGRA byte ordering [Blue, Green, Red, Alpha]
    PF_RGBA     // RGBA byte ordering [Red, Green, Blue, Alpha]
  };

  AwWebCore(LogLevel level = LOG_NORMAL, bool enablePlugins = true, PixelFormat pixelFormat = PF_BGRA);

  virtual ~AwWebCore();

  static Awesomium::WebCore& Get();

  static Awesomium::WebCore* GetPointer();

  INLINE void setBaseDirectory(const std::string& baseDirectory);

  AwWebView * createWebView(int width, int height, bool isTransparent = false, bool enableAsyncRendering = false, int maxAsyncRenderPerSec = 70);

  INLINE void setCustomResponsePage(int statusCode, const std::string& filePath);

  INLINE void update();

  INLINE const std::string& getBaseDirectory() const;

  AwWebCore::PixelFormat getPixelFormat() const;

  INLINE bool arePluginsEnabled() const;

  INLINE void pause();

  INLINE void resume();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AwWebCore",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "awWebCore.I"

#endif
