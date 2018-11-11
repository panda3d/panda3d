/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file speedtree_api.h
 * @author drose
 * @date 2010-10-05
 */

#ifndef SPEEDTREE_API_H
#define SPEEDTREE_API_H

// This header file should be included first, to pull in any of the required
// headers from the SpeedTree API, needed in this directory.

#include "speedtree_parameters.h"
#include <Core/Core.h>
#include <Forest/Forest.h>

#if defined(SPEEDTREE_OPENGL)
  #include <Renderers/OpenGL/OpenGLRenderer.h>
#elif defined(SPEEDTREE_DIRECTX9)
  #undef Configure
  #include <Renderers/DirectX9/DirectX9Renderer.h>
#else
  #error Unexpected graphics API.
#endif

#endif  // SPEEDTREE_API_H
