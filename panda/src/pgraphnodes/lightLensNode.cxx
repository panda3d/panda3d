/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightLensNode.cxx
 * @author drose
 * @date 2002-03-26
 */

#include "lightLensNode.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "renderState.h"
#include "cullFaceAttrib.h"
#include "colorWriteAttrib.h"
#include "graphicsStateGuardianBase.h"

TypeHandle LightLensNode::_type_handle;

/**
 *
 */
LightLensNode::
LightLensNode(const std::string &name, Lens *lens) :
  Camera(name, lens),
  _has_specular_color(false),
  _attrib_count(0),
  _used_by_auto_shader(false)
{
  set_active(false);
  _shadow_caster = false;
  _sb_size.set(512, 512);
  _sb_sort = -10;
  // set_initial_state(RenderState::make(ShaderAttrib::make_off(), 1000));
  // Backface culling helps eliminating artifacts.
  set_initial_state(RenderState::make(CullFaceAttrib::make_reverse(),
                    ColorWriteAttrib::make(ColorWriteAttrib::C_off)));
}

/**
 *
 */
LightLensNode::
~LightLensNode() {
  set_active(false);
  clear_shadow_buffers();

  // If this triggers, the number of attrib_ref() didn't match the number of
  // attrib_unref() calls, probably indicating a bug in LightAttrib.
  nassertv(AtomicAdjust::get(_attrib_count) == 0);
}

/**
 *
 */
LightLensNode::
LightLensNode(const LightLensNode &copy) :
  Light(copy),
  Camera(copy),
  _shadow_caster(copy._shadow_caster),
  _sb_size(copy._sb_size),
  _sb_sort(-10),
  _has_specular_color(copy._has_specular_color),
  _attrib_count(0),
  _used_by_auto_shader(false)
{
  if (_shadow_caster) {
    setup_shadow_map();
  }
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * This is the variant without buffer size, meaning that the current buffer
 * size will be kept (512x512 is the default). Note that enabling shadows will
 * require the shader generator to be enabled on the scene.
 */
void LightLensNode::
set_shadow_caster(bool caster) {
  if (_shadow_caster && !caster) {
    clear_shadow_buffers();
  }
  if (_shadow_caster != caster && _used_by_auto_shader) {
    // Make sure any shaders using this light are regenerated.
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _shadow_caster = caster;
  set_active(caster);
  if (caster) {
    setup_shadow_map();
  }
}

/**
 * Sets the flag indicating whether this light should cast shadows or not.
 * The xsize and ysize parameters specify the size of the shadow buffer that
 * will be set up, the sort parameter specifies the sort.  Note that enabling
 * shadows will require the shader generator to be enabled on the scene.
 */
void LightLensNode::
set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize, int buffer_sort) {
  if ((_shadow_caster && !caster) || buffer_xsize != _sb_size[0] || buffer_ysize != _sb_size[1]) {
    clear_shadow_buffers();
  }
  if (_shadow_caster != caster && _used_by_auto_shader) {
    // Make sure any shaders using this light are regenerated.
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _shadow_caster = caster;
  _sb_size.set(buffer_xsize, buffer_ysize);

  if (buffer_sort != _sb_sort) {
    ShadowBuffers::iterator it;
    for(it = _sbuffers.begin(); it != _sbuffers.end(); ++it) {
      (*it).second->set_sort(buffer_sort);
    }
    _sb_sort = buffer_sort;
  }
  set_active(caster);
  if (caster) {
    setup_shadow_map();
  }
}

/**
 * Clears the shadow buffers, meaning they will be automatically recreated
 * when the Shader Generator needs them.
 */
void LightLensNode::
clear_shadow_buffers() {
  if (_shadow_map) {
    // Clear it to all ones, so that any shaders that might still be using
    // it will see the shadows being disabled.
    _shadow_map->clear_image();
  }

  ShadowBuffers::iterator it;
  for(it = _sbuffers.begin(); it != _sbuffers.end(); ++it) {
    (*it).first->remove_window((*it).second);
  }
  _sbuffers.clear();
}

/**
 * Creates the shadow map texture.  Can be overridden.
 */
void LightLensNode::
setup_shadow_map() {
  if (_shadow_map != nullptr &&
      _shadow_map->get_x_size() == _sb_size[0] &&
      _shadow_map->get_y_size() == _sb_size[1]) {
    // Nothing to do.
    return;
  }

  if (_shadow_map == nullptr) {
    _shadow_map = new Texture(get_name());
  }

  _shadow_map->setup_2d_texture(_sb_size[0], _sb_size[1], Texture::T_unsigned_byte, Texture::F_depth_component);
  _shadow_map->set_clear_color(LColor(1));
  _shadow_map->set_wrap_u(SamplerState::WM_border_color);
  _shadow_map->set_wrap_v(SamplerState::WM_border_color);
  _shadow_map->set_border_color(LColor(1));
  _shadow_map->set_minfilter(SamplerState::FT_shadow);
  _shadow_map->set_magfilter(SamplerState::FT_shadow);
}

/**
 * This is called when the light is added to a LightAttrib.
 */
void LightLensNode::
attrib_ref() {
  AtomicAdjust::inc(_attrib_count);
}

/**
 * This is called when the light is removed from a LightAttrib.
 */
void LightLensNode::
attrib_unref() {
  // When it is removed from the last LightAttrib, destroy the shadow buffers.
  // This is necessary to break the circular reference that the buffer holds
  // on this node, via the display region's camera.
  if (!AtomicAdjust::dec(_attrib_count)) {
    clear_shadow_buffers();
  }
}

/**
 * Returns the Light object upcast to a PandaNode.
 */
PandaNode *LightLensNode::
as_node() {
  return this;
}

/**
 * Cross-casts the node to a Light pointer, if it is one of the four kinds of
 * Light nodes, or returns NULL if it is not.
 */
Light *LightLensNode::
as_light() {
  return this;
}

/**
 *
 */
void LightLensNode::
output(std::ostream &out) const {
  LensNode::output(out);
}

/**
 *
 */
void LightLensNode::
write(std::ostream &out, int indent_level) const {
  LensNode::write(out, indent_level);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void LightLensNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  Camera::write_datagram(manager, dg);
  Light::write_datagram(manager, dg);

  dg.add_bool(_shadow_caster);
  dg.add_int32(_sb_size[0]);
  dg.add_int32(_sb_size[1]);
  dg.add_int32(_sb_sort);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new LightLensNode.
 */
void LightLensNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  Camera::fillin(scan, manager);
  Light::fillin(scan, manager);

  bool shadow_caster = scan.get_bool();
  int sb_xsize = scan.get_int32();
  int sb_ysize = scan.get_int32();
  int sb_sort = scan.get_int32();
  set_shadow_caster(shadow_caster, sb_xsize, sb_ysize, sb_sort);
}
