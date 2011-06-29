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
#include "awWebViewListener.h"

TypeHandle AwWebViewListener::_type_handle;

AwWebViewListener::
AwWebViewListener()  {  
  awesomium_cat.info() << "constructing WebViewListner" ;
}

void AwWebViewListener::onBeginNavigation(const std::string& url, const std::wstring& frameName){
}

void AwWebViewListener::onBeginLoading(const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType) {
  awesomium_cat.info() << "onBeginLoading" ;
}

    /**
    * This event is fired when all loads have finished for a WebView.
    */
void AwWebViewListener::onFinishLoading() {
}

    /**
    * This event is fired when a Client callback has been invoked via Javascript from a page.
    *
    * @param    name    The name of the client callback that was invoked (specifically, "Client._this_name_here_(...)").
    *
    * @param    args    The arguments passed to the callback.
    */
void AwWebViewListener::onCallback(const std::string& name, const Awesomium::JSArguments& args) {
}
    
    /**
    * This event is fired when a page title is received.
    *
    * @param    title   The page title.
    *
    * @param    frameName   The name of the frame that this event originated from.
    */
void AwWebViewListener::onReceiveTitle(const std::wstring& title, const std::wstring& frameName) {
}

    /**
    * This event is fired when a tooltip has changed state.
    *
    * @param    tooltip     The tooltip text (or, is an empty string when the tooltip should disappear).
    */
void AwWebViewListener::onChangeTooltip(const std::wstring& tooltip) {
}

    /**
    * This event is fired when keyboard focus has changed.
    *
    * @param    isFocused   Whether or not the keyboard is currently focused.
    */
void AwWebViewListener::onChangeKeyboardFocus(bool isFocused) {
}

    /**
    * This event is fired when the target URL has changed. This is usually the result of 
    * hovering over a link on the page.
    *
    * @param    url The updated target URL (or empty if the target URL is cleared).
    */
void AwWebViewListener::onChangeTargetURL(const std::string& url) {
}
