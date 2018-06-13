/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformBlend.cxx
 * @author drose
 * @date 2005-03-24
 */

#include "transformBlend.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle TransformBlend::_type_handle;

/**
 * Defines an arbitrary ordering for TransformBlend objects.
 */
int TransformBlend::
compare_to(const TransformBlend &other) const {
  if (_entries.size() != other._entries.size()) {
    return (int)_entries.size() - (int)other._entries.size();
  }

  Entries::const_iterator ai, bi;
  ai = _entries.begin();
  bi = other._entries.begin();
  while (ai != _entries.end() && bi != other._entries.end()) {
    if ((*ai)._transform != (*bi)._transform) {
      return (*ai)._transform < (*bi)._transform ? -1 : 1;
    }
    if (!IS_NEARLY_EQUAL((*ai)._weight, (*bi)._weight)) {
      return (*ai)._weight < (*bi)._weight ? -1 : 1;
    }
    ++ai;
    ++bi;
  }

  return 0;
}

/**
 * Adds a new transform to the blend.  If the transform already existed,
 * increases its weight factor.
 */
void TransformBlend::
add_transform(const VertexTransform *transform, PN_stdfloat weight) {
  if (!IS_NEARLY_ZERO(weight)) {
    TransformEntry entry;
    entry._transform = transform;
    entry._weight = weight;
    std::pair<Entries::iterator, bool> result = _entries.insert(entry);
    if (!result.second) {
      // If the new value was not inserted, it was already there; increment
      // the existing weight factor.
      Entries::iterator ei = result.first;
      (*ei)._weight += weight;
      if (IS_NEARLY_ZERO((*ei)._weight)) {
        // If we have just zeroed out the weight, remove it.
        _entries.erase(ei);
      }
    }
    Thread *current_thread = Thread::get_current_thread();
    clear_result(current_thread);
  }
}

/**
 * Removes the indicated transform from the blend.
 */
void TransformBlend::
remove_transform(const VertexTransform *transform) {
  TransformEntry entry;
  entry._transform = transform;
  entry._weight = 0.0f;
  Entries::iterator ei = _entries.find(entry);
  if (ei != _entries.end()) {
    _entries.erase(ei);
  }
  Thread *current_thread = Thread::get_current_thread();
  clear_result(current_thread);
}

/**
 * If the total number of transforms in the blend exceeds max_transforms,
 * removes the n least-important transforms as needed to reduce the number of
 * transforms to max_transforms.
 */
void TransformBlend::
limit_transforms(int max_transforms) {
  if (max_transforms <= 0) {
    _entries.clear();
    return;
  }

  while ((int)_entries.size() > max_transforms) {
    // Repeatedly find and remove the least-important transform.
    nassertv(!_entries.empty());
    Entries::iterator ei_least = _entries.begin();
    Entries::iterator ei = ei_least;
    ++ei;
    while (ei != _entries.end()) {
      if ((*ei)._weight < (*ei_least)._weight) {
        ei_least = ei;
      }
      ++ei;
    }

    _entries.erase(ei_least);
  }
}

/**
 * Rescales all of the weights on the various transforms so that they sum to
 * 1.0.  It is generally a good idea to call this after adding or removing
 * transforms from the blend.
 */
void TransformBlend::
normalize_weights() {
  PN_stdfloat net_weight = 0.0f;
  Entries::iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    net_weight += (*ei)._weight;
  }
  if (net_weight != 0.0f) {
    for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
      (*ei)._weight /= net_weight;
    }
  }
  Thread *current_thread = Thread::get_current_thread();
  clear_result(current_thread);
}

/**
 * Returns true if the blend has the indicated transform, false otherwise.
 */
bool TransformBlend::
has_transform(const VertexTransform *transform) const {
  TransformEntry entry;
  entry._transform = transform;
  entry._weight = 0.0f;
  Entries::const_iterator ei = _entries.find(entry);
  return (ei != _entries.end());
}

/**
 * Returns the weight associated with the indicated transform, or 0 if there
 * is no entry for the transform.
 */
PN_stdfloat TransformBlend::
get_weight(const VertexTransform *transform) const {
  TransformEntry entry;
  entry._transform = transform;
  entry._weight = 0.0f;
  Entries::const_iterator ei = _entries.find(entry);
  if (ei != _entries.end()) {
    return (*ei)._weight;
  }
  return 0.0f;
}

/**
 *
 */
void TransformBlend::
output(std::ostream &out) const {
  if (_entries.empty()) {
    out << "empty";
  } else {
    Entries::const_iterator ei = _entries.begin();
    out << *(*ei)._transform << ":" << (*ei)._weight;
    ++ei;
    while (ei != _entries.end()) {
      out << " " << *(*ei)._transform << ":" << (*ei)._weight;
      ++ei;
    }
  }
}

/**
 *
 */
void TransformBlend::
write(std::ostream &out, int indent_level) const {
  Thread *current_thread = Thread::get_current_thread();
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    indent(out, indent_level)
      << *(*ei)._transform << ":" << (*ei)._weight << "\n";
    LMatrix4 mat;
    (*ei)._transform->get_matrix(mat);
    mat.write(out, indent_level + 4);
  }
  LMatrix4 blend;
  update_blend(current_thread);
  get_blend(blend, current_thread);
  indent(out, indent_level)
    << "Blended result =\n";
  blend.write(out, indent_level + 2);
}

/**
 * Recomputes the blend result from the various VertexTransform objects, if
 * necessary.
 */
void TransformBlend::
recompute_result(CData *cdata, Thread *current_thread) {
  // Update the global_modified sequence number first, to prevent race
  // conditions.
  cdata->_global_modified = VertexTransform::get_global_modified(current_thread);

  // Now see if we really need to recompute.
  UpdateSeq seq;
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    seq = std::max(seq, (*ei)._transform->get_modified(current_thread));
  }

  if (cdata->_modified != seq) {
    // We do need to recompute.
    cdata->_modified = seq;

    cdata->_result = LMatrix4::zeros_mat();
    for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
      (*ei)._transform->accumulate_matrix(cdata->_result, (*ei)._weight);
    }
  }
}

/**
 * Removes the computed result to force it to be recomputed.
 */
void TransformBlend::
clear_result(Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_global_modified = UpdateSeq();
  if (cdata->_modified != UpdateSeq()) {
    cdata->_modified = UpdateSeq();
    cdata->_result = LMatrix4::ident_mat();
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TransformBlend::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint16(_entries.size());

  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    manager->write_pointer(dg, (*ei)._transform);
    dg.add_stdfloat((*ei)._weight);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int TransformBlend::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = 0;

  Entries::iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    (*ei)._transform = DCAST(VertexTransform, p_list[pi++]);
  }

  // Now that we have actual pointers, we can sort the list of entries.
  _entries.sort();

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PandaNode.
 */
void TransformBlend::
fillin(DatagramIterator &scan, BamReader *manager) {
  size_t num_entries = scan.get_uint16();
  _entries.reserve(num_entries);
  for (size_t i = 0; i < num_entries; ++i) {
    TransformEntry entry;
    manager->read_pointer(scan);
    entry._weight = scan.get_stdfloat();
    _entries.push_back(entry);
  }
}

/**
 *
 */
CycleData *TransformBlend::CData::
make_copy() const {
  return new CData(*this);
}
