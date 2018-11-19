/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullResult.cxx
 * @author drose
 * @date 2002-02-28
 */

#include "cullResult.h"
#include "cullBinManager.h"
#include "cullBinAttrib.h"
#include "textureAttrib.h"
#include "lightAttrib.h"
#include "colorAttrib.h"
#include "alphaTestAttrib.h"
#include "depthWriteAttrib.h"
#include "colorScaleAttrib.h"
#include "fogAttrib.h"
#include "transparencyAttrib.h"
#include "renderState.h"
#include "rescaleNormalAttrib.h"
#include "clockObject.h"
#include "config_pgraph.h"
#include "depthOffsetAttrib.h"
#include "colorBlendAttrib.h"

TypeHandle CullResult::_type_handle;

/*
 * This value is used instead of 1.0 to represent the alpha level of a pixel
 * that is to be considered "opaque" for the purposes of M_dual.  Ideally, 1.0
 * is the only correct value for this.  Realistically, we have to fudge it
 * lower for two reasons: (1) The modelers tend to paint textures with very
 * slight transparency levels in places that are not intended to be
 * transparent, without realizing it.  These very faint transparency regions
 * are normally (almost) invisible, but when rendered with M_dual they may be
 * revealed as regions of poor alpha sorting.  (2) There seems to be some
 * problem in DX where, in certain circumstances apparently related to
 * automatic texture management, it spontaneously drops out the bottom two
 * bits of an eight-bit alpha channel, causing a value of 255 to become a
 * value of 252 instead.  We use 256 as the denominator here (instead of, say,
 * 255) because a fractional power of two will have a terminating
 * representation in base 2, and thus will be more likely to have a precise
 * value in whatever internal representation the graphics API will use.
 */
static const PN_stdfloat dual_opaque_level = 252.0 / 256.0;
static const double bin_color_flash_rate = 1.0;  // 1 state change per second

/**
 *
 */
CullResult::
CullResult(GraphicsStateGuardianBase *gsg,
           const PStatCollector &draw_region_pcollector) :
  _gsg(gsg),
  _draw_region_pcollector(draw_region_pcollector)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif

#ifndef NDEBUG
  _show_transparency = show_transparency.get_value();
#endif
}

/**
 * Returns a newly-allocated CullResult object that contains a copy of just
 * the subset of the data from this CullResult object that is worth keeping
 * around for next frame.
 */
PT(CullResult) CullResult::
make_next() const {
  PT(CullResult) new_result = new CullResult(_gsg, _draw_region_pcollector);
  new_result->_bins.reserve(_bins.size());

  CullBinManager *bin_manager = CullBinManager::get_global_ptr();

  for (size_t i = 0; i < _bins.size(); ++i) {
    CullBin *old_bin = _bins[i];
    if (old_bin == nullptr ||
        old_bin->get_bin_type() != bin_manager->get_bin_type(i)) {
      new_result->_bins.push_back(nullptr);
    } else {
      new_result->_bins.push_back(old_bin->make_next());
    }
  }

  return new_result;
}

/**
 * Adds the indicated CullableObject to the appropriate bin.  The bin becomes
 * the owner of the object pointer, and will eventually delete it.
 */
void CullResult::
add_object(CullableObject *object, const CullTraverser *traverser) {
  static const LColor flash_alpha_color(0.92, 0.96, 0.10, 1.0f);
  static const LColor flash_binary_color(0.21f, 0.67f, 0.24, 1.0f);
  static const LColor flash_multisample_color(0.78f, 0.05f, 0.81f, 1.0f);
  static const LColor flash_dual_color(0.92, 0.01f, 0.01f, 1.0f);

  nassertv(object->_draw_callback != nullptr || object->_geom != nullptr);

  bool force = !traverser->get_effective_incomplete_render();
  Thread *current_thread = traverser->get_current_thread();
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();

  // This is probably a good time to check for an auto rescale setting.
  const RescaleNormalAttrib *rescale;
  object->_state->get_attrib_def(rescale);
  if (rescale->get_mode() == RescaleNormalAttrib::M_auto) {
    RescaleNormalAttrib::Mode mode;

    if (object->_internal_transform->has_identity_scale()) {
      mode = RescaleNormalAttrib::M_none;
    } else if (object->_internal_transform->has_uniform_scale()) {
      mode = RescaleNormalAttrib::M_rescale;
    } else {
      mode = RescaleNormalAttrib::M_normalize;
    }

    object->_state = object->_state->compose(get_rescale_normal_state(mode));
  }

  // Check for a special wireframe setting.
  const RenderModeAttrib *rmode;
  if (object->_state->get_attrib(rmode)) {
    if (rmode->get_mode() == RenderModeAttrib::M_filled_wireframe) {
      CullableObject *wireframe_part = new CullableObject(*object);
      wireframe_part->_state = get_wireframe_overlay_state(rmode);

      if (wireframe_part->munge_geom
          (_gsg, _gsg->get_geom_munger(wireframe_part->_state, current_thread),
           traverser, force)) {
        int wireframe_bin_index = bin_manager->find_bin("fixed");
        CullBin *bin = get_bin(wireframe_bin_index);
        nassertv(bin != nullptr);
        check_flash_bin(wireframe_part->_state, bin_manager, wireframe_bin_index);
        bin->add_object(wireframe_part, current_thread);
      } else {
        delete wireframe_part;
      }

      object->_state = object->_state->compose(get_wireframe_filled_state());
    }
  }

  // Check to see if there's a special transparency setting.
  const TransparencyAttrib *trans;
  if (object->_state->get_attrib(trans)) {
    switch (trans->get_mode()) {
    case TransparencyAttrib::M_alpha:
    case TransparencyAttrib::M_premultiplied_alpha:
      // M_alpha implies an alpha-write test, so we don't waste time writing
      // 0-valued pixels.
      object->_state = object->_state->compose(get_alpha_state());
      check_flash_transparency(object->_state, flash_alpha_color);
      break;

    case TransparencyAttrib::M_binary:
      // M_binary is implemented by explicitly setting the alpha test.
      object->_state = object->_state->compose(get_binary_state());
      check_flash_transparency(object->_state, flash_binary_color);
      break;

    case TransparencyAttrib::M_multisample:
    case TransparencyAttrib::M_multisample_mask:
      // The multisample modes are implemented using M_binary if the GSG in
      // use doesn't support multisample.
      if (!_gsg->get_supports_multisample()) {
        object->_state = object->_state->compose(get_binary_state());
      }
      check_flash_transparency(object->_state, flash_multisample_color);
      break;

    case TransparencyAttrib::M_dual:
#ifndef NDEBUG
      check_flash_transparency(object->_state, flash_dual_color);
#endif
      if (!m_dual) {
        // If m_dual is configured off, it becomes M_alpha.
        break;
      }

      // M_dual is implemented by drawing the opaque parts first, without
      // transparency, then drawing the transparent parts later.  This means
      // we must copy the object and add it to both bins.  We can only do this
      // if we do not have an explicit bin already applied; otherwise, M_dual
      // falls back to M_alpha.
      {
        const CullBinAttrib *bin_attrib;
        if (!object->_state->get_attrib(bin_attrib) ||
            bin_attrib->get_bin_name().empty()) {
          // We make a copy of the object to draw the transparent part; this
          // gets placed in the transparent bin.
#ifndef NDEBUG
          if (m_dual_transparent)
#endif
            {
              CullableObject *transparent_part = new CullableObject(*object);
              CPT(RenderState) transparent_state = get_dual_transparent_state();
              transparent_part->_state = object->_state->compose(transparent_state);
              if (transparent_part->munge_geom
                  (_gsg, _gsg->get_geom_munger(transparent_part->_state, current_thread),
                   traverser, force)) {
                int transparent_bin_index = transparent_part->_state->get_bin_index();
                CullBin *bin = get_bin(transparent_bin_index);
                nassertv(bin != nullptr);
                check_flash_bin(transparent_part->_state, bin_manager, transparent_bin_index);
                bin->add_object(transparent_part, current_thread);
              } else {
                delete transparent_part;
              }
            }

          // Now we can draw the opaque part.  This will end up in the opaque
          // bin.
          object->_state = object->_state->compose(get_dual_opaque_state());
#ifndef NDEBUG
          if (!m_dual_opaque) {
            delete object;
            return;
          }
#endif
        }
        // The object is assigned to a specific bin; M_dual becomes M_alpha.
      }
      break;

    default:
      // Other kinds of transparency need no special handling.
      break;
    }
  }

  int bin_index = object->_state->get_bin_index();
  CullBin *bin = get_bin(bin_index);
  nassertv(bin != nullptr);
  check_flash_bin(object->_state, bin_manager, bin_index);

  // Munge vertices as needed for the GSG's requirements, and the object's
  // current state.
  if (object->munge_geom(_gsg, _gsg->get_geom_munger(object->_state, current_thread), traverser, force)) {
    // The object may or may not now be fully resident, but this may not
    // matter, since the GSG may have the necessary buffers already loaded.
    // We'll let the GSG ultimately decide whether to render it.
    bin->add_object(object, current_thread);
  } else {
    delete object;
  }
}

/**
 * Called after all the geoms have been added, this indicates that the cull
 * process is finished for this frame and gives the bins a chance to do any
 * post-processing (like sorting) before moving on to draw.
 */
void CullResult::
finish_cull(SceneSetup *scene_setup, Thread *current_thread) {
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();

  for (size_t i = 0; i < _bins.size(); ++i) {
    if (!bin_manager->get_bin_active(i)) {
      // If the bin isn't active, don't sort it, and don't draw it.  In fact,
      // clear it.
      _bins[i] = nullptr;

    } else {
      CullBin *bin = _bins[i];
      if (bin != nullptr) {
        bin->finish_cull(scene_setup, current_thread);
      }
    }
  }
}

/**
 * Asks all the bins to draw themselves in the correct order.
 */
void CullResult::
draw(Thread *current_thread) {
  bool force = !_gsg->get_effective_incomplete_render();

  // Ask the bin manager for the correct order to draw all the bins.
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  int num_bins = bin_manager->get_num_bins();
  for (int i = 0; i < num_bins; i++) {
    int bin_index = bin_manager->get_bin(i);
    nassertv(bin_index >= 0);

    if (bin_index < (int)_bins.size() && _bins[bin_index] != nullptr) {

      _gsg->push_group_marker(_bins[bin_index]->get_name());
      _bins[bin_index]->draw(force, current_thread);
      _gsg->pop_group_marker();
    }
  }
}

/**
 * Returns a special scene graph constructed to represent the results of the
 * cull.  This will be a hierarchy of nodes, one node for each bin, each of
 * which will in term be a parent of a number of GeomNodes, representing the
 * geometry drawn in each bin.
 *
 * This is useful mainly for high-level debugging and abstraction tools; it
 * should not be mistaken for the low-level cull result itself.  For the low-
 * level cull result, use draw() to efficiently draw the culled scene.
 */
PT(PandaNode) CullResult::
make_result_graph() {
  PT(PandaNode) root_node = new PandaNode("cull_result");

  // Ask the bin manager for the correct order to draw all the bins.
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  int num_bins = bin_manager->get_num_bins();
  for (int i = 0; i < num_bins; i++) {
    int bin_index = bin_manager->get_bin(i);
    nassertr(bin_index >= 0, nullptr);

    if (bin_index < (int)_bins.size() && _bins[bin_index] != nullptr) {
      root_node->add_child(_bins[bin_index]->make_result_graph());
    }
  }

  return root_node;
}

/**
 * Intended to be called by CullBinManager::remove_bin(), this informs all the
 * CullResults in the world to remove the indicated bin_index from their cache
 * if it has been cached.
 */
void CullResult::
bin_removed(int bin_index) {
  // Do something here.
  nassertv(false);
}

/**
 * Allocates a new CullBin for the given bin_index and stores it for next
 * time.
 */
CullBin *CullResult::
make_new_bin(int bin_index) {
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  PT(CullBin) bin = bin_manager->make_new_bin(bin_index, _gsg,
                                              _draw_region_pcollector);
  CullBin *bin_ptr = bin.p();

  if (bin_ptr != nullptr) {
    // Now store it in the vector.
    while (bin_index >= (int)_bins.size()) {
      _bins.push_back(nullptr);
    }
    nassertr(bin_index >= 0 && bin_index < (int)_bins.size(), nullptr);

    // Prevent unnecessary refunref by swapping the PointerTos.
    std::swap(_bins[bin_index], bin);
  }

  return bin_ptr;
}

/**
 * Returns a RenderState containing the given rescale normal attribute.
 */
const RenderState *CullResult::
get_rescale_normal_state(RescaleNormalAttrib::Mode mode) {
  static CPT(RenderState) states[RescaleNormalAttrib::M_auto + 1];
  if (states[mode].is_null()) {
    states[mode] = RenderState::make(RescaleNormalAttrib::make(mode),
                                     RenderState::get_max_priority());
  }
  return states[mode].p();
}

/**
 * Returns a RenderState that changes the alpha test to > 0, for implementing
 * M_alpha.
 */
const RenderState *CullResult::
get_alpha_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    // We don't monkey with the priority, since we want to allow the user to
    // override this if he desires.
    state = RenderState::make(AlphaTestAttrib::make(AlphaTestAttrib::M_greater, 0.0f));
  }
  return state.p();
}

/**
 * Returns a RenderState that applies the effects of M_binary.
 */
const RenderState *CullResult::
get_binary_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make(AlphaTestAttrib::make(AlphaTestAttrib::M_greater_equal, 0.5f),
                              TransparencyAttrib::make(TransparencyAttrib::M_none),
                              RenderState::get_max_priority());
  }
  return state.p();
}

#ifndef NDEBUG
/**
 * Update the object's state to flash the geometry with a solid color.
 */
void CullResult::
apply_flash_color(CPT(RenderState) &state, const LColor &flash_color) {
  int cycle = (int)(ClockObject::get_global_clock()->get_frame_time() * bin_color_flash_rate);
  if ((cycle & 1) == 0) {
    state = state->remove_attrib(TextureAttrib::get_class_slot());
    state = state->remove_attrib(LightAttrib::get_class_slot());
    state = state->remove_attrib(ColorScaleAttrib::get_class_slot());
    state = state->remove_attrib(FogAttrib::get_class_slot());
    state = state->add_attrib(ColorAttrib::make_flat(flash_color),
                              RenderState::get_max_priority());
  }
}
#endif  // NDEBUG

/**
 * Returns a RenderState that renders only the transparent parts of an object,
 * in support of M_dual.
 */
const RenderState *CullResult::
get_dual_transparent_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    // The alpha test for > 0 prevents us from drawing empty pixels, and hence
    // filling up the depth buffer with large empty spaces that may obscure
    // other things.  However, this does mean we draw pixels twice where the
    // alpha == 1.0 (since they were already drawn in the opaque pass).  This
    // is not normally a problem.
    state = RenderState::make(AlphaTestAttrib::make(AlphaTestAttrib::M_greater, 0.0f),
                              TransparencyAttrib::make(TransparencyAttrib::M_alpha),
                              DepthWriteAttrib::make(DepthWriteAttrib::M_off),
                              RenderState::get_max_priority());
  }

#ifndef NDEBUG
  if (m_dual_flash) {
    int cycle = (int)(ClockObject::get_global_clock()->get_frame_time() * bin_color_flash_rate);
    if ((cycle & 1) == 0) {
      static CPT(RenderState) flash_state = nullptr;
      if (flash_state == nullptr) {
        flash_state = state->add_attrib(ColorAttrib::make_flat(LColor(0.8f, 0.2, 0.2, 1.0f)),
                                        RenderState::get_max_priority());

        flash_state = flash_state->add_attrib(ColorScaleAttrib::make(LVecBase4(1.0f, 1.0f, 1.0f, 1.0f)),
                                              RenderState::get_max_priority());

        flash_state = flash_state->add_attrib(AlphaTestAttrib::make(AlphaTestAttrib::M_less, 1.0f),
                                              RenderState::get_max_priority());
      }
      return flash_state.p();
    }
  }
#endif  // NDEBUG

  return state.p();
}

/**
 * Returns a RenderState that renders only the opaque parts of an object, in
 * support of M_dual.
 */
const RenderState *CullResult::
get_dual_opaque_state() {
  static CPT(RenderState) state = nullptr;
  if (state == nullptr) {
    state = RenderState::make(AlphaTestAttrib::make(AlphaTestAttrib::M_greater_equal, dual_opaque_level),
                              TransparencyAttrib::make(TransparencyAttrib::M_none),
                              RenderState::get_max_priority());
  }

#ifndef NDEBUG
  if (m_dual_flash) {
    int cycle = (int)(ClockObject::get_global_clock()->get_frame_time() * bin_color_flash_rate);
    if ((cycle & 1) == 0) {
      static CPT(RenderState) flash_state = nullptr;
      if (flash_state == nullptr) {
        flash_state = state->add_attrib(ColorAttrib::make_flat(LColor(0.2, 0.2, 0.8f, 1.0f)),
                                        RenderState::get_max_priority());
        flash_state = flash_state->add_attrib(ColorScaleAttrib::make(LVecBase4(1.0f, 1.0f, 1.0f, 1.0f)),
                                              RenderState::get_max_priority());

      }
      return flash_state.p();
    }
  }
#endif  // NDEBUG

  return state.p();
}

/**
 * Returns a RenderState that is composed with the filled part of an
 * M_filled_wireframe model.
 */
const RenderState *CullResult::
get_wireframe_filled_state() {
  static CPT(RenderState) state = RenderState::make(
    RenderModeAttrib::make(RenderModeAttrib::M_filled),
    RenderState::get_max_priority());
  return state.p();
}

/**
 * Returns a RenderState that renders only the wireframe part of an
 * M_filled_wireframe model.
 */
CPT(RenderState) CullResult::
get_wireframe_overlay_state(const RenderModeAttrib *rmode) {
  return RenderState::make(
    DepthOffsetAttrib::make(1, 0, 0.99999f),
    ColorAttrib::make_flat(rmode->get_wireframe_color()),
    ColorBlendAttrib::make(ColorBlendAttrib::M_add,
                           ColorBlendAttrib::O_incoming_alpha,
                           ColorBlendAttrib::O_one_minus_incoming_alpha),
    RenderModeAttrib::make(RenderModeAttrib::M_wireframe,
                           rmode->get_thickness(),
                           rmode->get_perspective()));
}
