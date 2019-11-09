/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texMatrixAttrib.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "texMatrixAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "textureStagePool.h"

CPT(RenderAttrib) TexMatrixAttrib::_empty_attrib;
TypeHandle TexMatrixAttrib::_type_handle;
int TexMatrixAttrib::_attrib_slot;

/**
 *
 */
TexMatrixAttrib::
~TexMatrixAttrib() {
}

/**
 * Constructs a TexMatrixAttrib that applies no stages at all.
 */
CPT(RenderAttrib) TexMatrixAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib forever
  // once we find it the first time, as an optimization.
  if (_empty_attrib == nullptr) {
    _empty_attrib = return_new(new TexMatrixAttrib);
  }

  return _empty_attrib;
}

/**
 * Constructs a TexMatrixAttrib that applies the indicated matrix to the
 * default texture stage.  This interface is deprecated.
 *
 * @deprecated Use the constructor that takes a TextureStage instead.
 */
CPT(RenderAttrib) TexMatrixAttrib::
make(const LMatrix4 &mat) {
  pgraph_cat.warning()
    << "Using deprecated TexMatrixAttrib interface.\n";
  if (mat.is_identity()) {
    return make();
  }
  CPT(TransformState) transform = TransformState::make_mat(mat);
  return make(TextureStage::get_default(), transform);
}

/**
 * Constructs a TexMatrixAttrib that applies the indicated transform to the
 * named texture stage.
 */
CPT(RenderAttrib) TexMatrixAttrib::
make(TextureStage *stage, const TransformState *transform) {
  return DCAST(TexMatrixAttrib, make())->add_stage(stage, transform);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) TexMatrixAttrib::
make_default() {
  return return_new(new TexMatrixAttrib);
}

/**
 * Returns a new TexMatrixAttrib just like this one, with the indicated
 * transform for the given stage.  If this stage already exists, its transform
 * is replaced.
 */
CPT(RenderAttrib) TexMatrixAttrib::
add_stage(TextureStage *stage, const TransformState *transform,
          int override) const {
  TexMatrixAttrib *attrib = new TexMatrixAttrib(*this);
  Stages::iterator si = attrib->_stages.insert(StageNode(stage)).first;
  (*si)._transform = transform;
  (*si)._override = override;

  return return_new(attrib);
}

/**
 * Returns a new TexMatrixAttrib just like this one, with the indicated stage
 * removed.
 */
CPT(RenderAttrib) TexMatrixAttrib::
remove_stage(TextureStage *stage) const {
  TexMatrixAttrib *attrib = new TexMatrixAttrib(*this);
  attrib->_stages.erase(StageNode(stage));
  return return_new(attrib);
}

/**
 * Returns the transformation matrix associated with the default texture
 * stage.
 */
const LMatrix4 &TexMatrixAttrib::
get_mat() const {
  return get_mat(TextureStage::get_default());
}

/**
 * Returns true if no stages are defined in the TexMatrixAttrib, false if at
 * least one is.
 */
bool TexMatrixAttrib::
is_empty() const {
  return _stages.empty();
}

/**
 * Returns true if there is a transform associated with the indicated stage,
 * or false otherwise (in which case get_transform(stage) will return the
 * identity transform).
 */
bool TexMatrixAttrib::
has_stage(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(StageNode(stage));
  return (mi != _stages.end());
}

/**
 * Returns the number of stages that are represented by this attrib.
 */
int TexMatrixAttrib::
get_num_stages() const {
  return _stages.size();
}

/**
 * Returns the nth stage that is represented by this attrib.  The
 * TextureStages are in no particular order.
 */
TextureStage *TexMatrixAttrib::
get_stage(int n) const {
  nassertr(n >= 0 && n < (int)_stages.size(), nullptr);
  return _stages[n]._stage;
}

/**
 * Returns the transformation matrix associated with the indicated texture
 * stage, or identity matrix if nothing is associated with the indicated
 * stage.
 */
const LMatrix4 &TexMatrixAttrib::
get_mat(TextureStage *stage) const {
  return get_transform(stage)->get_mat();
}

/**
 * Returns the transformation associated with the indicated texture stage, or
 * identity matrix if nothing is associated with the indicated stage.
 */
CPT(TransformState) TexMatrixAttrib::
get_transform(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(StageNode(stage));
  if (mi != _stages.end()) {
    return (*mi)._transform;
  }
  return TransformState::make_identity();
}

/**
 *
 */
void TexMatrixAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";

  Stages::const_iterator mi;
  for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
    const StageNode &sn = (*mi);
    out << " " << sn._stage->get_name() << "(" << *sn._transform << ")";
    if (sn._override != 0) {
      out << "^" << sn._override;
    }
  }
}

/**
 * Intended to be overridden by derived TexMatrixAttrib types to return a
 * unique number indicating whether this TexMatrixAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two TexMatrixAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two TexMatrixAttrib objects whose get_type()
 * functions return the same.
 */
int TexMatrixAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta = (const TexMatrixAttrib *)other;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai) < (*bi)) {
      // This stage is in a but not in b.
      return -1;

    } else if ((*bi) < (*ai)) {
      // This stage is in b but not in a.
      return 1;

    } else {
      // This stage is in both.
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

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t TexMatrixAttrib::
get_hash_impl() const {
  size_t hash = 0;
  Stages::const_iterator si;
  for (si = _stages.begin(); si != _stages.end(); ++si) {
    const StageNode &sn = (*si);

    hash = pointer_hash::add_hash(hash, sn._stage);
    hash = pointer_hash::add_hash(hash, sn._transform);
    hash = int_hash::add_hash(hash, sn._override);
  }

  return hash;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return the result of applying the other RenderAttrib to a node
 * in the scene graph below this RenderAttrib, which was already applied.  In
 * most cases, the result is the same as the other RenderAttrib (that is, a
 * subsequent RenderAttrib completely replaces the preceding one).  On the
 * other hand, some kinds of RenderAttrib (for instance, ColorTransformAttrib)
 * might combine in meaningful ways.
 */
CPT(RenderAttrib) TexMatrixAttrib::
compose_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta = (const TexMatrixAttrib *)other;

  // The composition is the union of the two attribs.  In the case when a
  // stage is in both attribs, we compose the stages.

  TexMatrixAttrib *attrib = new TexMatrixAttrib;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai)._stage < (*bi)._stage) {
      // This stage is in a but not in b.
      attrib->_stages.insert(attrib->_stages.end(), *ai);
      ++ai;

    } else if ((*bi)._stage < (*ai)._stage) {
      // This stage is in b but not in a.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;

    } else {
      // This stage is in both.
      if ((*ai)._override == (*bi)._override) {
        // Same override; compose them.
        CPT(TransformState) new_transform = (*ai)._transform->compose((*bi)._transform);
        StageNode sn((*ai)._stage);
        sn._transform = new_transform;
        sn._override = (*ai)._override;
        attrib->_stages.insert(attrib->_stages.end(), sn);
      } else if ((*ai)._override < (*bi)._override) {
        // Override b wins.
        attrib->_stages.insert(attrib->_stages.end(), *bi);
      } else {
        // Override a wins.
        attrib->_stages.insert(attrib->_stages.end(), *ai);
      }

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

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) TexMatrixAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta = (const TexMatrixAttrib *)other;

  // The inverse composition works a lot like the composition, except we
  // invert the ai stages.

  TexMatrixAttrib *attrib = new TexMatrixAttrib;

  Stages::const_iterator ai, bi;
  ai = _stages.begin();
  bi = ta->_stages.begin();
  while (ai != _stages.end() && bi != ta->_stages.end()) {
    if ((*ai)._stage < (*bi)._stage) {
      // This stage is in a but not in b.
      CPT(TransformState) inv_a =
        (*ai)._transform->invert_compose(TransformState::make_identity());
      StageNode sn((*ai)._stage);
      sn._transform = inv_a;
      sn._override = (*ai)._override;
      attrib->_stages.insert(attrib->_stages.end(), sn);
      ++ai;

    } else if ((*bi)._stage < (*ai)._stage) {
      // This stage is in b but not in a.
      attrib->_stages.insert(attrib->_stages.end(), *bi);
      ++bi;

    } else {
      // This stage is in both.
      if ((*ai)._override == (*bi)._override) {
        // Same override; compose them.
        CPT(TransformState) new_transform = (*ai)._transform->invert_compose((*bi)._transform);
        StageNode sn((*ai)._stage);
        sn._transform = new_transform;
        sn._override = (*ai)._override;
        attrib->_stages.insert(attrib->_stages.end(), sn);

      } else if ((*ai)._override < (*bi)._override) {
        // Override b wins.
        attrib->_stages.insert(attrib->_stages.end(), *bi);

      } else {
        // Override a wins.
        CPT(TransformState) inv_a =
          (*ai)._transform->invert_compose(TransformState::make_identity());
        StageNode sn((*ai)._stage);
        sn._transform = inv_a;
        sn._override = (*ai)._override;
        attrib->_stages.insert(attrib->_stages.end(), sn);
      }

      ++ai;
      ++bi;
    }
  }

  while (ai != _stages.end()) {
    // This stage is in a but not in b.
    CPT(TransformState) inv_a =
      (*ai)._transform->invert_compose(TransformState::make_identity());
    StageNode sn((*ai)._stage);
    sn._transform = inv_a;
    sn._override = (*ai)._override;
    attrib->_stages.insert(attrib->_stages.end(), sn);
    ++ai;
  }

  while (bi != ta->_stages.end()) {
    // This stage is in b but not in a.
    attrib->_stages.insert(attrib->_stages.end(), *bi);
    ++bi;
  }

  return return_new(attrib);
}

/**
 * Tells the BamReader how to create objects of type TexMatrixAttrib.
 */
void TexMatrixAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TexMatrixAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint16(_stages.size());

  Stages::const_iterator si;
  for (si = _stages.begin(); si != _stages.end(); ++si) {
    const StageNode &sn = (*si);

    manager->write_pointer(dg, sn._stage);
    manager->write_pointer(dg, sn._transform);

    if (manager->get_file_minor_ver() >= 24) {
      dg.add_int32(sn._override);
    }
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int TexMatrixAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  for (size_t sni = 0; sni < _stages.size(); ++sni) {
    // Filter the TextureStage through the TextureStagePool.
    PT(TextureStage) ts = DCAST(TextureStage, p_list[pi++]);
    ts = TextureStagePool::get_stage(ts);

    const TransformState *transform = DCAST(TransformState, p_list[pi++]);

    StageNode &sn = _stages[sni];
    sn._stage = ts;
    sn._transform = transform;
  }
  _stages.sort();

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type TexMatrixAttrib is encountered in the Bam file.  It should create the
 * TexMatrixAttrib and extract its information from the file.
 */
TypedWritable *TexMatrixAttrib::
make_from_bam(const FactoryParams &params) {
  TexMatrixAttrib *attrib = new TexMatrixAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new TexMatrixAttrib.
 */
void TexMatrixAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  size_t num_stages = scan.get_uint16();
  for (size_t i = 0; i < num_stages; i++) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    int override = 0;
    if (manager->get_file_minor_ver() >= 24) {
      override = scan.get_int32();
    }

    StageNode sn(nullptr);
    sn._override = override;
    _stages.push_back(sn);
  }
}
