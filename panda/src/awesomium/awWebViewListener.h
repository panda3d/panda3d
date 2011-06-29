// Filename: awWebViewListener.h
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
#ifndef AWWEBVIEWLISTENER_H
#define AWWEBVIEWLISTENER_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"

#include "awesomium_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : AwWebViewListener
// Description : Thin bindings, wraps a WebViewListener
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAAWESOMIUM AwWebViewListener : public TypedReferenceCount, public Awesomium::WebCore {
PUBLISHED:


PUBLISHED:
        AwWebViewListener();
  
        virtual ~AwWebViewListener() {}
   
    /**
    * This event is fired when a WebView begins navigating to a new URL.
    *
    * @param    url     The URL that is being navigated to.
    *
    * @param    frameName   The name of the frame that this event originated from.
    */
    void onBeginNavigation(const std::string& url, const std::wstring& frameName) ;

    /**
    * This event is fired when a WebView begins to actually receive data from a server.
    *
    * @param    url     The URL of the frame that is being loaded.
    *
    * @param    frameName   The name of the frame that this event originated from.
    *
    * @param    statusCode  The HTTP status code returned by the server.
    *
    * @param    mimeType    The mime-type of the content that is being loaded.
    */
        void onBeginLoading(const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType);

    /**
    * This event is fired when all loads have finished for a WebView.
    */
        void onFinishLoading();

    /**
    * This event is fired when a Client callback has been invoked via Javascript from a page.
    *
    * @param    name    The name of the client callback that was invoked (specifically, "Client._this_name_here_(...)").
    *
    * @param    args    The arguments passed to the callback.
    */
  void onCallback(const std::string& name, const Awesomium::JSArguments& args);
    
    /**
    * This event is fired when a page title is received.
    *
    * @param    title   The page title.
    *
    * @param    frameName   The name of the frame that this event originated from.
    */
  void onReceiveTitle(const std::wstring& title, const std::wstring& frameName) ;

    /**
    * This event is fired when a tooltip has changed state.
    *
    * @param    tooltip     The tooltip text (or, is an empty string when the tooltip should disappear).
    */
    void onChangeTooltip(const std::wstring& tooltip);

    /**
    * This event is fired when keyboard focus has changed.
    *
    * @param    isFocused   Whether or not the keyboard is currently focused.
    */
    void onChangeKeyboardFocus(bool isFocused) ;

    /**
    * This event is fired when the target URL has changed. This is usually the result of 
    * hovering over a link on the page.
    *
    * @param    url The updated target URL (or empty if the target URL is cleared).
    */
    void onChangeTargetURL(const std::string& url) ;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AwWebViewListener",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

// #include "awWebViewListener.I"

#endif
