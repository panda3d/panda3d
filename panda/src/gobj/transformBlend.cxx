// Filename: transformBlend.cxx
// Created by:  drose (24Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "transformBlend.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"


////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::compare_to
//       Access: Published
//  Description: Defines an arbitrary ordering for TransformBlend
//               objects.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::add_transform
//       Access: Published
//  Description: Adds a new transform to the blend.  If the transform
//               already existed, increases its weight factor.
////////////////////////////////////////////////////////////////////
void TransformBlend::
add_transform(const VertexTransform *transform, float weight) {
  if (!IS_NEARLY_ZERO(weight)) {
    TransformEntry entry;
    entry._transform = transform;
    entry._weight = weight;
    pair<Entries::iterator, bool> result = _entries.insert(entry);
    if (!result.second) {
      // If the new value was not inserted, it was already there;
      // increment the existing weight factor.
      Entries::iterator ei = result.first;
      (*ei)._weight += weight;
      if (IS_NEARLY_ZERO((*ei)._weight)) {
        // If we have just zeroed out the weight, remove it.
        _entries.erase(ei);
      }
    }
    clear_result();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::remove_transform
//       Access: Published
//  Description: Removes the indicated transform to the blend.
////////////////////////////////////////////////////////////////////
void TransformBlend::
remove_transform(const VertexTransform *transform) {
  TransformEntry entry;
  entry._transform = transform;
  entry._weight = 0.0f;
  Entries::iterator ei = _entries.find(entry);
  if (ei != _entries.end()) {
    _entries.erase(ei);
  }
  clear_result();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::normalize_weights
//       Access: Published
//  Description: Rescales all of the weights on the various transforms
//               so that they sum to 1.0.  It is generally a good idea
//               to call this after adding or removing transforms from
//               the blend.
////////////////////////////////////////////////////////////////////
void TransformBlend::
normalize_weights() {
  float net_weight = 0.0f;
  Entries::iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    net_weight += (*ei)._weight;
  }
  if (net_weight != 0.0f) {
    for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
      (*ei)._weight /= net_weight;
    }
  }
  clear_result();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::has_transform
//       Access: Published
//  Description: Returns true if the blend has the indicated
//               transform, false otherwise.
////////////////////////////////////////////////////////////////////
bool TransformBlend::
has_transform(const VertexTransform *transform) const {
  TransformEntry entry;
  entry._transform = transform;
  entry._weight = 0.0f;
  Entries::const_iterator ei = _entries.find(entry);
  return (ei != _entries.end());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::get_weight
//       Access: Published
//  Description: Returns the weight associated with the indicated
//               transform, or 0 if there is no entry for the
//               transform.
////////////////////////////////////////////////////////////////////
float TransformBlend::
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

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformBlend::
output(ostream &out) const {
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

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformBlend::
write(ostream &out, int indent_level) const {
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    indent(out, indent_level)
      << *(*ei)._transform << ":" << (*ei)._weight << "\n";
    LMatrix4f mat;
    (*ei)._transform->get_matrix(mat);
    mat.write(out, indent_level + 4);
  }
  LMatrix4f blend;
  get_blend(blend);
  indent(out, indent_level)
    << "Blended result =\n";
  blend.write(out, indent_level + 2);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::recompute_result
//       Access: Private
//  Description: Recomputes the blend result from the various
//               VertexTransform objects, if necessary.
////////////////////////////////////////////////////////////////////
void TransformBlend::
recompute_result(TransformBlend::CDWriter &cdata) {
  // Update the global_modified sequence number first, to prevent race
  // conditions.
  cdata->_global_modified = VertexTransform::get_global_modified();

  // Now see if we really need to recompute.
  UpdateSeq seq;
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    seq = max(seq, (*ei)._transform->get_modified());
  }

  if (cdata->_modified != seq) {
    // We do need to recompute.
    cdata->_modified = seq;

    cdata->_result.set(0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f);
    for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
      (*ei)._transform->accumulate_matrix(cdata->_result, (*ei)._weight);
    }
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::clear_result
//       Access: Private
//  Description: Removes the computed result to force it to be
//               recomputed.
////////////////////////////////////////////////////////////////////
void TransformBlend::
clear_result() {
  CDWriter cdata(_cycler);
  cdata->_global_modified = UpdateSeq();
  if (cdata->_modified != UpdateSeq()) {
    cdata->_modified = UpdateSeq();
    cdata->_result = LMatrix4f::ident_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::write_datagram
//       Access: Public
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformBlend::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::complete_pointers
//       Access: Public
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TransformBlend::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::fillin
//       Access: Public
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void TransformBlend::
fillin(DatagramIterator &scan, BamReader *manager) {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *TransformBlend::CData::
make_copy() const {
  return new CData(*this);
}
