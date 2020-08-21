/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJointEffect.cxx
 * @author drose
 * @date 2006-07-26
 */

#include "characterJointEffect.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "nodePath.h"
#include "look_at.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle CharacterJointEffect::_type_handle;

/**
 * Constructs a new CharacterJointEffect object that references the indicated
 * character.  When a relative get_transform() is called on the node that
 * contains the CharacterJointEffect, it will implicitly call
 * character->update() first.
 */
CPT(RenderEffect) CharacterJointEffect::
make(Character *character) {
  CharacterJointEffect *effect = new CharacterJointEffect;
  effect->_character = character;

  CPT(RenderEffect) new_effect_raw = return_new(effect);
  const CharacterJointEffect *new_effect;
  DCAST_INTO_R(new_effect, new_effect_raw, new_effect_raw);

  // It is possible that the CharacterJointEffect we have now is a different
  // CharacterJointEffect to a different Character which has since been
  // deleted, but which had the same memory address of our current character.
  // If this happened, we have to force-update the CharacterJointEffect to
  // tell its weak pointer that it is no longer invalid (and that it now
  // points to this once-again-live Character object).

  // This is a little weird, because it means any nodes that used to be
  // pointing to a deleted Character object (and knew they were pointing to a
  // deleted Character object) will suddenly be pointing to a new, non-deleted
  // Character object--and the wrong Character object, no less.  But there's
  // no other way to handle this, since we can't make the
  // CharacterJointEffect's compare function base itself on whether its
  // pointer is valid or not.

  if (!new_effect->_character.is_valid_pointer()) {
    nassertr(new_effect->_character.get_orig() == character, new_effect_raw);
    ((CharacterJointEffect *)new_effect)->_character = character;
  }

  return new_effect_raw;
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * RenderEffect by calling the xform() method, false otherwise.
 */
bool CharacterJointEffect::
safe_to_transform() const {
  // We now accept that it will be OK to transform the joint--we allow this on
  // the assumption that anything that transforms the joint will also
  // transform the Character node, above the joint.
  return true;
}

/**
 * Returns true if this kind of effect can safely be combined with sibling
 * nodes that share the exact same effect, or false if this is not a good
 * idea.
 */
bool CharacterJointEffect::
safe_to_combine() const {
  return false;
}

/**
 *
 */
void CharacterJointEffect::
output(std::ostream &out) const {
  out << get_type();
  PT(Character) character = get_character();
  if (character != nullptr) {
    out << "(" << character->get_name() << ")";
  } else {
    out << "(**invalid**)";
  }
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this effect during the cull traversal.
 */
bool CharacterJointEffect::
has_cull_callback() const {
  return true;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.  This may include additional manipulation of render
 * state or additional visible/invisible decisions, or any other arbitrary
 * operation.
 *
 * At the time this function is called, the current node's transform and state
 * have not yet been applied to the net_transform and net_state.  This
 * callback may modify the node_transform and node_state to apply an effective
 * change to the render state at this level.
 */
void CharacterJointEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &) const {
  if (auto character = _character.lock()) {
    character->update();
  }
  node_transform = data.node()->get_transform();
}

/**
 * Should be overridden by derived classes to return true if
 * adjust_transform() has been defined, and therefore the RenderEffect has
 * some effect on the node's apparent local and net transforms.
 */
bool CharacterJointEffect::
has_adjust_transform() const {
  return true;
}

/**
 * Performs some operation on the node's apparent net and/or local transforms.
 * This will only be called if has_adjust_transform() is redefined to return
 * true.
 *
 * Both parameters are in/out.  The original transforms will be passed in, and
 * they may (or may not) be modified in-place by the RenderEffect.
 */
void CharacterJointEffect::
adjust_transform(CPT(TransformState) &net_transform,
                 CPT(TransformState) &node_transform,
                 const PandaNode *node) const {
  if (auto character = _character.lock()) {
    character->update();
  }
  node_transform = node->get_transform();
}


/**
 * Intended to be overridden by derived CharacterJointEffect types to return a
 * unique number indicating whether this CharacterJointEffect is equivalent to
 * the other one.
 *
 * This should return 0 if the two CharacterJointEffect objects are
 * equivalent, a number less than zero if this one should be sorted before the
 * other one, and a number greater than zero otherwise.
 *
 * This will only be called with two CharacterJointEffect objects whose
 * get_type() functions return the same.
 */
int CharacterJointEffect::
compare_to_impl(const RenderEffect *other) const {
  const CharacterJointEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_character.get_orig() != ta->_character.get_orig()) {
    return _character.get_orig() < ta->_character.get_orig() ? -1 : 1;
  }

  // As tempting as it is to include the sense of whether the character
  // pointer is valid in this sorting, we can't, because that property might
  // change without warning--which would invalidate the CharacterJointEffect's
  // position in any maps if we used it to determine its sort.

  return 0;
}

/**
 * Tells the BamReader how to create objects of type CharacterJointEffect.
 */
void CharacterJointEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CharacterJointEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  PT(Character) character = get_character();
  manager->write_pointer(dg, character);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int CharacterJointEffect::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderEffect::complete_pointers(p_list, manager);

  _character = DCAST(Character, p_list[pi++]);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CharacterJointEffect is encountered in the Bam file.  It should create
 * the CharacterJointEffect and extract its information from the file.
 */
TypedWritable *CharacterJointEffect::
make_from_bam(const FactoryParams &params) {
  CharacterJointEffect *effect = new CharacterJointEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CharacterJointEffect.
 */
void CharacterJointEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  manager->read_pointer(scan);
}
