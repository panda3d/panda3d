// Filename: texMatrixAttrib.cxx
// Created by:  drose (14Mar02)
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

#include "texMatrixAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

CPT(RenderAttrib) TexMatrixAttrib::_empty_attrib;
TypeHandle TexMatrixAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TexMatrixAttrib::
~TexMatrixAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make
//       Access: Published, Static
//  Description: Constructs a TexMatrixAttrib that applies
//               no stages at all.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib
  // forever once we find it the first time, as an optimization.
  if (_empty_attrib == (RenderAttrib *)NULL) {
    _empty_attrib = return_new(new TexMatrixAttrib);
  }

  return _empty_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make
//       Access: Published, Static
//  Description: Constructs a TexMatrixAttrib that applies the
//               indicated matrix to the default texture stage.  This
//               interface is deprecated.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
make(const LMatrix4f &mat) {
  pgraph_cat.warning()
    << "Using deprecated TexMatrixAttrib interface.\n";
  if (mat == LMatrix4f::ident_mat()) {
    return make();
  }
  CPT(TransformState) transform = TransformState::make_mat(mat);
  return make(TextureStage::get_default(), transform);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make
//       Access: Published, Static
//  Description: Constructs a TexMatrixAttrib that applies the
//               indicated transform to the named texture stage.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
make(TextureStage *stage, const TransformState *transform) {
  if (transform->is_identity()) {
    return make();
  }
  TexMatrixAttrib *attrib = new TexMatrixAttrib;
  attrib->_stages.insert(Stages::value_type(stage, transform));
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::add_stage
//       Access: Published, Static
//  Description: Returns a new TexMatrixAttrib just like this one,
//               with the indicated transform for the given stage.  If
//               this stage already exists, its transform is replaced.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
add_stage(TextureStage *stage, const TransformState *transform) const {
  if (transform->is_identity()) {
    return remove_stage(stage);
  }
  TexMatrixAttrib *attrib = new TexMatrixAttrib(*this);
  attrib->_stages[stage] = transform;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::remove_stage
//       Access: Published, Static
//  Description: Returns a new TexMatrixAttrib just like this one,
//               with the indicated stage removed.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
remove_stage(TextureStage *stage) const {
  TexMatrixAttrib *attrib = new TexMatrixAttrib(*this);
  attrib->_stages.erase(stage);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::get_mat
//       Access: Published
//  Description: Returns the transformation matrix associated with
//               the default texture stage.
////////////////////////////////////////////////////////////////////
const LMatrix4f &TexMatrixAttrib::
get_mat() const {
  return get_mat(TextureStage::get_default());
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::is_empty
//       Access: Published
//  Description: Returns true if no stages are defined in the
//               TexMatrixAttrib, false if at least one is.
////////////////////////////////////////////////////////////////////
bool TexMatrixAttrib::
is_empty() const {
  return _stages.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::has_stage
//       Access: Published
//  Description: Returns true if there is a transform associated with
//               the indicated stage, or false otherwise (in which
//               case get_transform(stage) will return the identity
//               transform).
////////////////////////////////////////////////////////////////////
bool TexMatrixAttrib::
has_stage(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  return (mi != _stages.end());
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::get_num_stages
//       Access: Published
//  Description: Returns the number of stages that are represented by
//               this attrib.
////////////////////////////////////////////////////////////////////
int TexMatrixAttrib::
get_num_stages() const {
  return _stages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::get_stage
//       Access: Published
//  Description: Returns the nth stage that is represented by this
//               attrib.  The TextureStages are in no particular
//               order.
////////////////////////////////////////////////////////////////////
TextureStage *TexMatrixAttrib::
get_stage(int n) const {
  nassertr(n >= 0 && n < (int)_stages.size(), NULL);
  check_stage_list();
  return _stage_list[n];
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::get_mat
//       Access: Published
//  Description: Returns the transformation matrix associated with
//               the named texture stage, or identity matrix if
//               nothing is associated with the indicated stage.
////////////////////////////////////////////////////////////////////
const LMatrix4f &TexMatrixAttrib::
get_mat(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  if (mi != _stages.end()) {
    return (*mi).second->get_mat();
  }
  return LMatrix4f::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::get_transform
//       Access: Published
//  Description: Returns the transformation associated with
//               the named texture stage, or identity matrix if
//               nothing is associated with the indicated stage.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TexMatrixAttrib::
get_transform(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  if (mi != _stages.end()) {
    return (*mi).second;
  }
  return TransformState::make_identity();
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
output(ostream &out) const {
  out << get_type() << ":";

  Stages::const_iterator mi;
  for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
    TextureStage *stage = (*mi).first;
    const TransformState *transform = (*mi).second;
    out << " " << stage->get_name() << "(" << *transform << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexMatrixAttrib
//               types to return a unique number indicating whether
//               this TexMatrixAttrib is equivalent to the other one.
//
//               This should return 0 if the two TexMatrixAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TexMatrixAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TexMatrixAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  
  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai).first < (*bi).first) {
      // This stage is in a but not in b.
      return -1;

    } else if ((*bi).first < (*ai).first) {
      // This stage is in b but not in a.
      return 1;

    } else {
      // This stage is in both; compare the stages.
      if ((*ai).second != (*bi).second) {
        return (*ai).second < (*bi).second ? -1 : 1;
      }
      ++ai;
      ++bi;
    }
  }

  if (bi != ta->_stages.end()) {
    // a ran out first; b was longer.
    return -1;
  }

  if (ai != _stages.end()) {
    // b ran out first; a was longer.
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
compose_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // The composition is the union of the two attribs.  In the case
  // when a stage is in both attribs, we compose the stages.

  TexMatrixAttrib *attrib = new TexMatrixAttrib;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai).first < (*bi).first) {
      // This stage is in a but not in b.
      attrib->_stages.insert(attrib->_stages.end(), *ai);
      ++ai;

    } else if ((*bi).first < (*ai).first) {
      // This stage is in b but not in a.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;

    } else {
      // This stage is in both; compose the stages.
      CPT(TransformState) new_transform = (*ai).second->compose((*bi).second);
      attrib->_stages.insert(attrib->_stages.end(), Stages::value_type((*ai).first, new_transform));
      ++ai;
      ++bi;
    }
  }

  while (ai != _stages.end()) {
    // This stage is in a but not in b.
    attrib->_stages.insert(attrib->_stages.end(), *ai);
    ++ai;
  }

  while (bi != ta->_stages.end()) {
    // This stage is in b but not in a.
    attrib->_stages.insert(attrib->_stages.end(), *bi);
    ++bi;
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // The inverse composition works a lot like the composition, except
  // we invert the ai stages.

  TexMatrixAttrib *attrib = new TexMatrixAttrib;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai).first < (*bi).first) {
      // This stage is in a but not in b.
      CPT(TransformState) inv_a = 
        (*ai).second->invert_compose(TransformState::make_identity());
      attrib->_stages.insert(attrib->_stages.end(), Stages::value_type((*ai).first, inv_a));
      ++ai;

    } else if ((*bi).first < (*ai).first) {
      // This stage is in b but not in a.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;

    } else {
      // This stage is in both; compose the stages.
      CPT(TransformState) new_transform = 
        (*ai).second->invert_compose((*bi).second);
      attrib->_stages.insert(attrib->_stages.end(), Stages::value_type((*ai).first, new_transform));
      ++ai;
      ++bi;
    }
  }

  while (ai != _stages.end()) {
    // This stage is in a but not in b.
    CPT(TransformState) inv_a = 
      (*ai).second->invert_compose(TransformState::make_identity());
    attrib->_stages.insert(attrib->_stages.end(), Stages::value_type((*ai).first, inv_a));
    ++ai;
  }

  while (bi != ta->_stages.end()) {
    // This stage is in b but not in a.
    attrib->_stages.insert(attrib->_stages.end(), *bi);
    ++bi;
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexMatrixAttrib
//               types to specify what the default property for a
//               TexMatrixAttrib of this type should be.
//
//               This should return a newly-allocated TexMatrixAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of TexMatrixAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *TexMatrixAttrib::
make_default_impl() const {
  return new TexMatrixAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: When attribs are stored in a slot-based attrib array,
//               this returns the index of the appropriate slot
//               for this attrib type.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_tex_matrix = this;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::rebuild_stage_list
//       Access: Private
//  Description: Builds the linear list of TextureStages, the first
//               time someone asks for it.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
rebuild_stage_list() {
  _stage_list.clear();
  _stage_list.reserve(_stages.size());

  Stages::const_iterator si;
  for (si = _stages.begin(); si != _stages.end(); ++si) {
    _stage_list.push_back((*si).first);
  }

  _stage_list_stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TexMatrixAttrib.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint16(_stages.size());

  Stages::const_iterator si;
  for (si = _stages.begin(); si != _stages.end(); ++si) {
    TextureStage *stage = (*si).first;
    const TransformState *transform = (*si).second;

    manager->write_pointer(dg, stage);
    manager->write_pointer(dg, transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TexMatrixAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  for (size_t i = 0; i < _num_stages; i++) {
    TextureStage *stage = DCAST(TextureStage, p_list[pi++]);
    const TransformState *transform = DCAST(TransformState, p_list[pi++]);
    _stages[stage] = transform;
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TexMatrixAttrib is encountered
//               in the Bam file.  It should create the TexMatrixAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TexMatrixAttrib::
make_from_bam(const FactoryParams &params) {
  TexMatrixAttrib *attrib = new TexMatrixAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TexMatrixAttrib.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _num_stages = scan.get_uint16();
  for (size_t i = 0; i < _num_stages; i++) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
  }
}
