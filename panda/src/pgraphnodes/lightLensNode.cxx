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

TypeHandle LightLensNode::_type_handle;

/**
 *
 */
LightLensNode::
LightLensNode(const string &name, Lens *lens) :
  Camera(name, lens)
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
  _sb_sort(-10)
{
}

/**
 * Clears the shadow buffers, meaning they will be automatically recreated
 * when the Shader Generator needs them.
 */
void LightLensNode::
clear_shadow_buffers() {
  ShadowBuffers::iterator it;
  for(it = _sbuffers.begin(); it != _sbuffers.end(); ++it) {
    PT(Texture) tex = (*it).second->get_texture();
    if (tex) {
      // Clear it to all ones, so that any shaders that might still be using
      // it will see the shadows being disabled.
      tex->set_clear_color(LColor(1));
      tex->clear_image();
    }
    (*it).first->remove_window((*it).second);
  }
  _sbuffers.clear();
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
output(ostream &out) const {
  LensNode::output(out);
}

/**
 *
 */
void LightLensNode::
write(ostream &out, int indent_level) const {
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
