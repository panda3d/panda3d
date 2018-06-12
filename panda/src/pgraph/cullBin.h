/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBin.h
 * @author drose
 * @date 2002-02-27
 */

#ifndef CULLBIN_H
#define CULLBIN_H

#include "pandabase.h"
#include "cullBinEnums.h"
#include "typedReferenceCount.h"
#include "pStatCollector.h"
#include "pointerTo.h"
#include "luse.h"
#include "geomNode.h"

class CullableObject;
class GraphicsStateGuardianBase;
class SceneSetup;
class TransformState;
class RenderState;
class PandaNode;

/**
 * A collection of Geoms and their associated state, for a particular scene.
 * The cull traversal (and the BinCullHandler) assigns Geoms to bins as it
 * comes across them.
 *
 * This is an abstract base class; derived classes like CullBinStateSorted and
 * CullBinBackToFront provide the actual implementation.
 */
class EXPCL_PANDA_PGRAPH CullBin : public TypedReferenceCount, public CullBinEnums {
protected:
  INLINE CullBin(const CullBin &copy);
public:
  INLINE CullBin(const std::string &name, BinType bin_type,
                 GraphicsStateGuardianBase *gsg,
                 const PStatCollector &draw_region_pcollector);
  virtual ~CullBin();

  INLINE const std::string &get_name() const;
  INLINE BinType get_bin_type() const;

  virtual PT(CullBin) make_next() const;

  virtual void add_object(CullableObject *object, Thread *current_thread)=0;
  virtual void finish_cull(SceneSetup *scene_setup, Thread *current_thread);

  virtual void draw(bool force, Thread *current_thread)=0;

  PT(PandaNode) make_result_graph();

  INLINE bool has_flash_color() const;
  INLINE const LColor &get_flash_color() const;

protected:
  class ResultGraphBuilder;
  virtual void fill_result_graph(ResultGraphBuilder &builder)=0;

private:
  void check_flash_color();

protected:
  std::string _name;
  BinType _bin_type;
  GraphicsStateGuardianBase *_gsg;

  // Used in make_result_graph() and fill_result_graph().
  class EXPCL_PANDA_PGRAPH ResultGraphBuilder {
  public:
    ResultGraphBuilder(PandaNode *root_node);
    void add_object(CullableObject *object);

  private:
    void record_one_object(GeomNode *node, CullableObject *object);

  private:
    int _object_index;
    CPT(TransformState) _current_transform;
    CPT(RenderState) _current_state;
    PT(PandaNode) _root_node;
    PT(GeomNode) _current_node;
  };

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
