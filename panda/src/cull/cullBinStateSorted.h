/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinStateSorted.h
 * @author drose
 * @date 2005-03-22
 */

#ifndef CULLBINSTATESORTED_H
#define CULLBINSTATESORTED_H

#include "pandabase.h"

#include "cullBin.h"
#include "cullableObject.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pointerTo.h"

/**
 * A specific kind of CullBin that sorts geometry to collect items of the same
 * state together, so that minimal state changes are required on the GSG to
 * render them.
 *
 * This also sorts objects front-to-back within a particular state, to take
 * advantage of hierarchical Z-buffer algorithms which can early-out when an
 * object appears behind another one.
 */
class EXPCL_PANDA_CULL CullBinStateSorted : public CullBin {
public:
  INLINE CullBinStateSorted(const std::string &name,
                            GraphicsStateGuardianBase *gsg,
                            const PStatCollector &draw_region_pcollector);
  virtual ~CullBinStateSorted();

  static CullBin *make_bin(const std::string &name,
                           GraphicsStateGuardianBase *gsg,
                           const PStatCollector &draw_region_pcollector);

  virtual void add_object(CullableObject *object, Thread *current_thread);
  virtual void finish_cull(SceneSetup *scene_setup, Thread *current_thread);
  virtual void draw(bool force, Thread *current_thread);

protected:
  virtual void fill_result_graph(ResultGraphBuilder &builder);

private:
  class ObjectData {
  public:
    INLINE ObjectData(CullableObject *object);
    INLINE bool operator < (const ObjectData &other) const;

    CullableObject *_object;
    const GeomVertexFormat *_format;
  };

  typedef pvector<ObjectData> Objects;
  Objects _objects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinStateSorted",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBinStateSorted.I"

#endif
