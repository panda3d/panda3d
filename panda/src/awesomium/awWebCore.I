// Filename: awWebCore.I
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


INLINE void AwWebCore::
setBaseDirectory(const std::string& baseDirectory) {
  WebCore::setBaseDirectory(baseDirectory);
}

INLINE void AwWebCore::
setCustomResponsePage(int statusCode, const std::string& filePath) {
  WebCore::setCustomResponsePage(statusCode, filePath);
}

INLINE void AwWebCore::
update() {
  WebCore::update();
}

INLINE const std::string& AwWebCore::
getBaseDirectory() const {
  return WebCore::getBaseDirectory();
}

INLINE bool AwWebCore::
arePluginsEnabled() const {
  return WebCore::arePluginsEnabled();
}


INLINE void AwWebCore::
pause(){
  WebCore::pause();
}

INLINE void AwWebCore::
resume() {
  WebCore::resume();
}


