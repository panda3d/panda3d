// Filename: shaderGenerator.h
// Created by: jyelon (15Dec07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SHADERGENERATOR_H
#define SHADERGENERATOR_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : ShaderGenerator
// Description : The ShaderGenerator is a device that effectively
//               replaces the classic fixed function pipeline with
//               a 'next-gen' fixed function pipeline.  The next-gen
//               fixed function pipeline supports features like 
//               normal mapping, gloss mapping, cartoon lighting,
//               and so forth.  It works by automatically generating
//               a shader from a given RenderState.
//
//               The ShaderGenerator owes its existence to the 
//               'Bamboo Team' at Carnegie Mellon's Entertainment
//               Technology Center.  This is a group of students
//               who, as a semester project, decided that next-gen
//               graphics should be accessible to everyone, even if
//               they don't know shader programming.  The group 
//               consisted of:
//
//               Aaron Lo, Programmer
//               Heegun Lee, Programmer
//               Erin Fernandez, Artist/Tester
//               Joe Grubb, Artist/Tester
//               Ivan Ortega, Technical Artist/Tester
//
//               Thanks to them!
//
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA_PGRAPH ShaderGenerator {
private:
  INLINE ShaderGenerator();
  INLINE ~ShaderGenerator();
  
public:
  static CPT(RenderAttrib) synthesize_shader(const RenderState *rs);
};

#include "shaderGenerator.I"

#endif  // SHADERGENERATOR_H

