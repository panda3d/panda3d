// Filename: cgShaderAttrib.h
// Created by:  sshodhan (10Jul04)
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

#ifndef CGSHADERATTRIB_H
#define CGSHADERATTRIB_H

#include "pandabase.h"

// In case we don't have HAVE_CG defined, we need to have at least a
// forward reference to the class so the gsg's issue_cg_shader_bind()
// method can be compiled.
class CgShaderAttrib;

#ifdef HAVE_CG

#include "luse.h"
#include "pmap.h"
#include "cgShader.h"
#include "renderAttrib.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "factoryParam.h"
#include "dcast.h"



////////////////////////////////////////////////////////////////////
//       Class : CgShaderAttrib
// Description : Cg is Nvidia's high level shading language
//               Setting this attrib on a node will make that node
//               pass through a vertex and fragment program on the GPU
//               These programs must be passed through a CgShader object
//               to the make function of this attrib
//               All existing states will collapse
//               You will need to pass in the right matrices
//               to transform the object
//               All textures will have to be passed again manually
//               and applied by the fragment program.
//               You can also pass other parameters such as floating
//               point and double precision numbers
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX CgShaderAttrib: public RenderAttrib {

private:
  INLINE CgShaderAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make(CgShader *shader);
  static CPT(RenderAttrib) make_off();

  INLINE bool is_off() const;
  INLINE CgShader *get_cg_shader() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;

protected:
   virtual RenderAttrib *make_default_impl() const;
   virtual int compare_to_impl(const RenderAttrib *other) const;

private:
  PT(CgShader) _cg_shader;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "CgShaderAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};


#include "cgShaderAttrib.I"

#endif  // HAVE_CG

#endif



