// Filename: planarReflector.h
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
#ifndef PLANARREFLECTOR_H
#define PLANARREFLECTOR_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "casterShader.h"
#include <pixelBuffer.h>
#include <planeNode.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : PlanarReflector
// Description : Reflects about a specified plane
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER PlanarReflector : public CasterShader
{
  public:

    PlanarReflector(void);
    PlanarReflector(PlaneNode* plane_node);
    PlanarReflector(const Colorf& c);
    PlanarReflector(PlaneNode* plane_node, const Colorf& c);

    virtual void pre_apply(Node *node, const AllAttributesWrapper &init_state,
                       const AllTransitionsWrapper &net_trans,
                       GraphicsStateGuardian *gsg);
    virtual void apply(Node *node, const AllAttributesWrapper &init_state,
                       const AllTransitionsWrapper &net_trans,
                       GraphicsStateGuardian *gsg);

    INLINE void set_save_color_buffer(bool val) { _save_color_buffer = val; }
    INLINE void set_save_depth_buffer(bool val) { _save_depth_buffer = val; }
    INLINE void set_clip_to_plane(bool val) { _clip_to_plane = val; }
    INLINE void set_plane_node(PlaneNode* p) { _plane_node = p; }
    INLINE void set_color(const Colorf& c) { _color = c; }

  protected:

    void init(PlaneNode* plane_node, const Colorf& c);

  protected:

    bool                        _save_color_buffer;
    bool                        _save_depth_buffer;
    bool                        _clip_to_plane;
    PT(PixelBuffer)             _color_buffer;
    PT(PixelBuffer)             _depth_buffer;
    PT(PlaneNode)               _plane_node;
    Colorf                      _color;

  public:

    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      CasterShader::init_type();
      register_type(_type_handle, "PlanarReflector",
                    CasterShader::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:

    static TypeHandle _type_handle;
};

#endif
