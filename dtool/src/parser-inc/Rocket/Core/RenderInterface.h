/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file RenderInterface.h
 * @author rdb
 * @date 2011-11-25
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

namespace Rocket {
  namespace Core {
    class Context;
    class RenderInterface;
    class String;
    class Vector2f;
    class Vector2i;
    class Vertex;

    typedef unsigned char byte;
    typedef void* CompiledGeometryHandle;
    typedef void* TextureHandle;
  }
}
