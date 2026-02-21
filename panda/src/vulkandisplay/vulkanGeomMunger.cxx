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

#include "vulkanGeomMunger.h"
#include "vulkanGraphicsStateGuardian.h"

TypeHandle VulkanGeomMunger::_type_handle;

ALLOC_DELETED_CHAIN_DEF(VulkanGeomMunger);

/**
 *
 */
VulkanGeomMunger::
VulkanGeomMunger(VulkanGraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 1, NT_packed_dabc, C_color) {

}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) VulkanGeomMunger::
munge_format_impl(const GeomVertexFormat *orig,
                  const GeomVertexAnimationSpec &animation) {

  if (animation.get_animation_type() == AT_hardware) {
    // If we want hardware animation, we need to reserve space for the blend
    // weights.

    PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);
    new_format->set_animation(animation);

    // Make sure the old weights and indices are removed, just in case.
    new_format->remove_column(InternalName::get_transform_weight());
    new_format->remove_column(InternalName::get_transform_index());

    // And we don't need the transform_blend table any more.
    new_format->remove_column(InternalName::get_transform_blend());

    PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;

    // For now, limited to 4 simultaneous joints.
    new_array_format->add_column
      (InternalName::get_transform_weight(), 4,
       NT_float32, C_other);

    // This allows for up to 65536 joints, more than anyone will ever need.
    new_array_format->add_column
      (InternalName::get_transform_index(), 4,
       NT_uint16, C_index);

    new_format->add_array(new_array_format);

    return GeomVertexFormat::register_format(new_format);
  } else {
    return orig;
  }
}
