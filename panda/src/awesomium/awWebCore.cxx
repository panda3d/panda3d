// Filename: awWebCore.cxx
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

#include "config_awesomium.h"
#include "awWebCore.h"
#include "WebCore.h"

TypeHandle AwWebCore::_type_handle;

AwWebCore::
AwWebCore(AwWebCore::LogLevel level, bool enablePlugins , AwWebCore::PixelFormat pixelFormat) 
#ifndef CPPPARSER
:
  WebCore(static_cast<Awesomium::LogLevel>(level), enablePlugins, static_cast<Awesomium::PixelFormat>(pixelFormat)) 
#endif
  {  
  awesomium_cat.info() << "constructing webcore\n";
}

AwWebCore::
~AwWebCore() {
  awesomium_cat.info() << "destructor webcore\n";
}

Awesomium::WebCore& AwWebCore::
Get() {
  return WebCore::Get();
}

Awesomium::WebCore* AwWebCore::
GetPointer() {
  return WebCore::GetPointer();
}

AwWebView *  AwWebCore::
createWebView(int width, int height, bool isTransparent , bool enableAsyncRendering , int maxAsyncRenderPerSec ) {
  Awesomium::WebView * newView = WebCore::createWebView(width, height, isTransparent, enableAsyncRendering, maxAsyncRenderPerSec);
  AwWebView * result = new AwWebView(newView);
  return result;
}

AwWebCore::PixelFormat AwWebCore::
getPixelFormat() const {
  return ( static_cast<AwWebCore::PixelFormat>( WebCore::getPixelFormat()) );
}

