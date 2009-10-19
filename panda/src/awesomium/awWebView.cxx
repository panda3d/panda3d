// Filename: awWebView.cxx
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
#include "awWebView.h"

TypeHandle AwWebView::_type_handle;

AwWebView::
AwWebView(Awesomium::WebView * webViewPtr)  {  
  _myWebView = webViewPtr;

}

AwWebView::
~AwWebView() {
}


void AwWebView::
loadURL2(const string& url, const string& frameName , const string& username , const string& password )
{
  _myWebView->loadURL2(url, frameName, username, password);

}

void AwWebView::
loadHTML2(const std::string& html, const std::string& frameName )
{
  _myWebView->loadHTML2(html, frameName);
}


void AwWebView::
loadFile2(const std::string& file, const std::string& frameName )
{
  _myWebView->loadFile2(file, frameName);
}


void AwWebView::
render(size_t destination, int destRowSpan, int destDepth, AwWebView::Rect * renderedRect) {
  if (renderedRect) {
    Awesomium::Rect rect(renderedRect->x, renderedRect->y, renderedRect->width, renderedRect->height);
    _myWebView->Awesomium::WebView::render( reinterpret_cast<unsigned char *>(destination), destRowSpan, destDepth, &rect);    
  }
  else
  {
    AwWebView::render(destination, destRowSpan, destDepth, 0);
  }
}

void AwWebView::
injectMouseDown(AwWebView::MouseButton button) {
  awesomium_cat.debug() <<"got mouse down " << button << "\n";
  _myWebView->injectMouseDown(static_cast<Awesomium::MouseButton>(button));
}


void AwWebView::
injectMouseMove(int x, int y) {
  //awesomium_cat.debug() <<"got mouse move " << x << " " << y << "\n";
  _myWebView->injectMouseMove(x,y);
}

