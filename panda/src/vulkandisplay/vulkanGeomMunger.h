/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vulkanGeomMunger.h
 * @author rdb
 * @date 2026-02-20
 */

#ifndef VULKANGEOMMUNGER_H
#define VULKANGEOMMUNGER_H

#include "pandabase.h"
#include "standardMunger.h"

class VulkanGraphicsStateGuardian;

/**
 * This specialization on GeomMunger finesses vertices for Vulkan rendering.
 */
class EXPCL_VULKANDISPLAY VulkanGeomMunger final : public StandardMunger {
public:
  VulkanGeomMunger(VulkanGraphicsStateGuardian *gsg, const RenderState *state);
  ALLOC_DELETED_CHAIN_DECL(VulkanGeomMunger);

protected:
  virtual CPT(GeomVertexFormat) munge_format_impl(const GeomVertexFormat *orig,
                                                  const GeomVertexAnimationSpec &animation);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    StandardMunger::init_type();
    register_type(_type_handle, "VulkanGeomMunger",
                  StandardMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
