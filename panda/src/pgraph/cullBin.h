// Filename: cullBin.h
// Created by:  drose (27Feb02)
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

#ifndef CULLBIN_H
#define CULLBIN_H

#include "pandabase.h"
#include "cullBinEnums.h"
#include "typedReferenceCount.h"
#include "pStatCollector.h"
#include "pointerTo.h"
#include "luse.h"

class CullableObject;
class GraphicsStateGuardianBase;
class SceneSetup;

////////////////////////////////////////////////////////////////////
//       Class : CullBin
// Description : A collection of Geoms and their associated state, for
//               a particular scene.  The cull traversal (and the
//               BinCullHandler) assigns Geoms to bins as it comes
//               across them.
//
//               This is an abstract base class; derived classes like
//               CullBinStateSorted and CullBinBackToFront provide the
//               actual implementation.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBin : public TypedReferenceCount, public CullBinEnums {
protected:
  INLINE CullBin(const CullBin &copy);
public:
  INLINE CullBin(const string &name, BinType bin_type,
                 GraphicsStateGuardianBase *gsg,
                 const PStatCollector &draw_region_pcollector);
  virtual ~CullBin();

  INLINE const string &get_name() const;
  INLINE BinType get_bin_type() const;

  virtual PT(CullBin) make_next() const;

  virtual void add_object(CullableObject *object, Thread *current_thread)=0;
  virtual void finish_cull(SceneSetup *scene_setup, Thread *current_thread);

  virtual void draw(Thread *current_thread)=0;

  INLINE bool has_flash_color() const;
  INLINE const Colorf &get_flash_color() const;

private:
  void check_flash_color();

protected:
  string _name;
  BinType _bin_type;
  GraphicsStateGuardianBase *_gsg;

  bool _has_flash_color;
  Colorf _flash_color;

  static PStatCollector _cull_bin_pcollector;
  PStatCollector _cull_this_pcollector;
  PStatCollector _draw_this_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CullBin",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBin.I"

#endif


  
