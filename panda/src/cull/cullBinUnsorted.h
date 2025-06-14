/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinUnsorted.h
 * @author drose
 * @date 2002-02-28
 */

#ifndef CULLBINUNSORTED_H
#define CULLBINUNSORTED_H

#include "pandabase.h"

#include "cullBin.h"
#include "pointerTo.h"

/**
 * A specific kind of CullBin that does not reorder the geometry; it simply
 * passes it through to the GSG in the same order it was encountered, which
 * will be in scene-graph order.
 */
class EXPCL_PANDA_CULL CullBinUnsorted : public CullBin {
public:
  INLINE CullBinUnsorted(const std::string &name,
                         GraphicsStateGuardianBase *gsg,
                         const PStatCollector &draw_region_pcollector);
  ~CullBinUnsorted();

  static CullBin *make_bin(const std::string &name,
                           GraphicsStateGuardianBase *gsg,
                           const PStatCollector &draw_region_pcollector);

  virtual void add_object(CullableObject *object, Thread *current_thread);
  virtual void draw(bool force, Thread *current_thread);

protected:
  virtual void fill_result_graph(ResultGraphBuilder &builder);

private:
  typedef pvector<CullableObject *> Objects;
  Objects _objects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinUnsorted",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBinUnsorted.I"

#endif
