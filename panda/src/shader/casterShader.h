// Filename: casterShader.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef CASTERSHADER_H
#define CASTERSHADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "shader.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : CasterShader
// Description : Frustum Shader that computes effect based on a list
//               of "casting" objects
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER CasterShader : public FrustumShader
{
  protected:

    CasterShader(void) : FrustumShader() { }

  public:

    virtual ~CasterShader(void) { }

    INLINE int get_num_casters(void) const { return _casters.size(); }
    INLINE int add_caster(NamedNode* node);
    INLINE int remove_caster(NamedNode* node);

    typedef pvector<NamedNode *> NamedNodeVector;

  protected:

    NamedNodeVector                     _casters;

  public:

    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      FrustumShader::init_type();
      register_type(_type_handle, "CasterShader",
                          FrustumShader::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:

    static TypeHandle _type_handle;
};

#include "casterShader.I"

#endif
