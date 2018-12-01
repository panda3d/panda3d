/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file occluderEffect.cxx
 * @author drose
 * @date 2011-03-17
 */

#include "occluderEffect.h"
#include "pandaNode.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraph.h"
#include "attribNodeRegistry.h"

CPT(RenderEffect) OccluderEffect::_empty_effect;
TypeHandle OccluderEffect::_type_handle;

/**
 * Constructs a new OccluderEffect object that does nothing.
 */
CPT(RenderEffect) OccluderEffect::
make() {
  // We make it a special case and store a pointer to the empty effect forever
  // once we find it the first time, as an optimization.
  if (_empty_effect == nullptr) {
    _empty_effect = return_new(new OccluderEffect);
  }

  return _empty_effect;
}

/**
 * Returns a new OccluderEffect, just like this one, but with the indicated
 * occluder added to the list of occluders enabled by this effect.
 */
CPT(RenderEffect) OccluderEffect::
add_on_occluder(const NodePath &occluder) const {
  nassertr(!occluder.is_empty() && occluder.node()->is_of_type(OccluderNode::get_class_type()), this);
  OccluderEffect *effect = new OccluderEffect(*this);
  effect->_on_occluders.insert(occluder);
  return return_new(effect);
}

/**
 * Returns a new OccluderEffect, just like this one, but with the indicated
 * occluder removed from the list of occluders enabled by this effect.
 */
CPT(RenderEffect) OccluderEffect::
remove_on_occluder(const NodePath &occluder) const {
  nassertr(!occluder.is_empty() && occluder.node()->is_of_type(OccluderNode::get_class_type()), this);
  OccluderEffect *effect = new OccluderEffect(*this);
  effect->_on_occluders.erase(occluder);
  return return_new(effect);
}

/**
 *
 */
void OccluderEffect::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (_on_occluders.empty()) {
    out << "identity";
  } else {
    out << "on";

    Occluders::const_iterator li;
    for (li = _on_occluders.begin(); li != _on_occluders.end(); ++li) {
      NodePath occluder = (*li);
      out << " " << occluder;
    }
  }
}

/**
 * Intended to be overridden by derived OccluderEffect types to return a
 * unique number indicating whether this OccluderEffect is equivalent to the
 * other one.
 *
 * This should return 0 if the two OccluderEffect objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two OccluderEffect objects whose get_type()
 * functions return the same.
 */
int OccluderEffect::
compare_to_impl(const RenderEffect *other) const {
  const OccluderEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  Occluders::const_iterator li = _on_occluders.begin();
  Occluders::const_iterator oli = ta->_on_occluders.begin();

  while (li != _on_occluders.end() && oli != ta->_on_occluders.end()) {
    NodePath occluder = (*li);
    NodePath other_occluder = (*oli);

    int compare = occluder.compare_to(other_occluder);
    if (compare != 0) {
      return compare;
    }

    ++li;
    ++oli;
  }

  if (li != _on_occluders.end()) {
    return 1;
  }
  if (oli != ta->_on_occluders.end()) {
    return -1;
  }

  return 0;
}

/**
 * Tells the BamReader how to create objects of type OccluderEffect.
 */
void OccluderEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void OccluderEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  // write the number of on occluders
  dg.add_uint16(get_num_on_occluders());

  // write the on occluders pointers if any
  Occluders::const_iterator nti;
  if (manager->get_file_minor_ver() < 40) {
    for (nti = _on_occluders.begin(); nti != _on_occluders.end(); ++nti) {
      manager->write_pointer(dg, nti->node());
    }
  } else {
    for (nti = _on_occluders.begin(); nti != _on_occluders.end(); ++nti) {
      (*nti).write_datagram(manager, dg);
    }
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int OccluderEffect::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderEffect::complete_pointers(p_list, manager);
  AttribNodeRegistry *areg = AttribNodeRegistry::get_global_ptr();

  if (manager->get_file_minor_ver() >= 40) {
    for (size_t i = 0; i < _on_occluders.size(); ++i) {
      pi += _on_occluders[i].complete_pointers(p_list + pi, manager);

      int n = areg->find_node(_on_occluders[i]);
      if (n != -1) {
        // If it's in the registry, replace it.
        _on_occluders[i] = areg->get_node(n);
      }
    }

  } else {
    Occluders::iterator ci;
    ci = _on_occluders.begin();
    while (ci != _on_occluders.end()) {
      PandaNode *node;
      DCAST_INTO_R(node, p_list[pi++], pi);

      int ni = areg->find_node(node->get_type(), node->get_name());
      if (ni != -1) {
        (*ci) = areg->get_node(ni);
      } else {
        (*ci) = NodePath(node);
      }
      ++ci;
    }
  }
  _on_occluders.sort();

  return pi;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool OccluderEffect::
require_fully_complete() const {
  return true;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type OccluderEffect is encountered in the Bam file.  It should create the
 * OccluderEffect and extract its information from the file.
 */
TypedWritable *OccluderEffect::
make_from_bam(const FactoryParams &params) {
  OccluderEffect *effect = new OccluderEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new OccluderEffect.
 */
void OccluderEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  // Push back an empty NodePath for each Occluder for now, until we get the
  // actual list of pointers later in complete_pointers().
  int num_on_occluders = scan.get_uint16();
  _on_occluders.resize(num_on_occluders);
  if (manager->get_file_minor_ver() >= 40) {
    for (int i = 0; i < num_on_occluders; i++) {
      _on_occluders[i].fillin(scan, manager);
    }
  } else {
    manager->read_pointers(scan, num_on_occluders);
  }
}
