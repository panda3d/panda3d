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
#ifndef AWWEBVIEW_H
#define AWWEBVIEW_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "awesomium_includes.h"

class WebViewListener;

////////////////////////////////////////////////////////////////////
//       Class : AwWebView
// Description : Thin bindings, wraps a WebView * returned from WebCore.createWebView
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAAWESOMIUM AwWebView : public TypedReferenceCount{
PUBLISHED:

  /**
  * Mouse button enumerations, used with WebView::injectMouseDown 
  * and WebView::injectMouseUp
  */
  enum MouseButton {
    LEFT_MOUSE_BTN,
    MIDDLE_MOUSE_BTN,
    RIGHT_MOUSE_BTN
  };

  /**
   * A simple rectangle class, used with WebView::render
   */
  struct Rect {
    int x, y, width, height;
    
    Rect();
    Rect(int x, int y, int width, int height);
    bool isEmpty() const;
  };


PUBLISHED:
  AwWebView(Awesomium::WebView * webView);
  
  virtual ~AwWebView();
  
  INLINE void destroy(void);
  
  INLINE void setListener(Awesomium::WebViewListener * listener);

  INLINE Awesomium::WebViewListener* getListener();
  
  // VC7 linker doesn't like wstring from VS2008, hence using the all regular string version
  void loadURL2(const string& url, const string& frameName ="", const string& username="" , const string& password="");
  
  // VC7 linker doesn't like wstring from VS2008, hence using the all regular string version
  void loadHTML2(const std::string& html, const std::string& frameName = "");
  
  // VC7 linker doesn't like wstring from VS2008, hence using the all regular string version
  void loadFile2(const std::string& file, const std::string& frameName = "" );
  
  INLINE void goToHistoryOffset(int offset);

  // VC7 linker doesn't like wstring from VS2008, hence using the all regular string version
  INLINE void executeJavascript2(const std::string& javascript, const std::string& frameName = "" );

  INLINE Awesomium::FutureJSValue executeJavascriptWithResult2(const std::string& javascript, const std::string& frameName = "");

  INLINE void setProperty(const std::string& name, const Awesomium::JSValue& value);

  INLINE void setCallback(const std::string& name);

  INLINE bool isDirty();

  INLINE void render(size_t destination, int destRowSpan, int destDepth);

  void render(size_t destination, int destRowSpan, int destDepth, AwWebView::Rect * renderedRect);

  void injectMouseMove(int x, int y);

  void injectMouseDown(AwWebView::MouseButton button);

  INLINE void injectMouseUp(AwWebView::MouseButton button);

  INLINE void injectMouseWheelXY(int scrollAmountX, int scrollAmountY);

  INLINE void injectMouseWheel(int scrollAmountY) {
    injectMouseWheelXY(0, scrollAmountY);
  }

  INLINE void injectKeyEvent(bool press, int modifiers, int windowsCode, int nativeCode=0);
  
private:
  Awesomium::WebView * _myWebView;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AwWebView",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "awWebView.I"

#endif
