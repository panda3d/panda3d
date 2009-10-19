// Filename: awWebView.I
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

INLINE void AwWebView::
destroy(void)
{
  _myWebView->destroy();
}


INLINE void AwWebView::
setListener(Awesomium::WebViewListener *  listener) {
  _myWebView->setListener(listener);
}

INLINE Awesomium::WebViewListener* AwWebView::
getListener() {
  return _myWebView->getListener();
}

INLINE void AwWebView::
goToHistoryOffset(int offset) {
  _myWebView->goToHistoryOffset(offset);
}

INLINE void AwWebView::
executeJavascript2(const std::string& javascript, const std::string& frameName ) {
  _myWebView->executeJavascript2(javascript, frameName);
}

INLINE Awesomium::FutureJSValue AwWebView::
executeJavascriptWithResult2(const std::string& javascript, const std::string& frameName ) {
  return _myWebView->executeJavascriptWithResult2(javascript, frameName);
}

INLINE void AwWebView::
setProperty(const std::string& name, const Awesomium::JSValue& value) {
  _myWebView->setProperty(name, value);
}

INLINE void AwWebView::
setCallback(const std::string& name) {
  _myWebView->setCallback(name);
}

INLINE bool AwWebView::
isDirty() {
  return _myWebView->isDirty();
}

INLINE void AwWebView::
render( size_t  destination, int destRowSpan, int destDepth)
{
  _myWebView->render( reinterpret_cast<unsigned char *>(destination), destRowSpan, destDepth, 0);
}



INLINE void AwWebView::
injectMouseUp(AwWebView::MouseButton button) {
  _myWebView->injectMouseUp(static_cast<Awesomium::MouseButton>(button));
}

INLINE void AwWebView::
injectMouseWheelXY(int scrollAmountX, int scrollAmountY){
  _myWebView->injectMouseWheelXY(scrollAmountX, scrollAmountY);
}

INLINE void AwWebView::
injectKeyEvent(bool press, int modifiers, int windowsCode, int nativeCode) {
  _myWebView->injectKeyEvent(press, modifiers, windowsCode, nativeCode);
}

