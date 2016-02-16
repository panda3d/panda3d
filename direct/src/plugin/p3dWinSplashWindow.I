/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dWinSplashWindow.I
 * @author drose
 * @date 2009-06-17
 */

/**

 */
inline P3DWinSplashWindow::WinImageData::
WinImageData() {
  _filename_changed = false;
  _bitmap = NULL;
}

/**

 */
inline P3DWinSplashWindow::WinImageData::
~WinImageData() {
  dump_image();
}
