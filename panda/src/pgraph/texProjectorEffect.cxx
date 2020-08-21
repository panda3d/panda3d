/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texProjectorEffect.cxx
 * @author drose
 * @date 2004-07-25
 */

#include "texProjectorEffect.h"
#include "cullTraverserData.h"
#include "texMatrixAttrib.h"
#include "lensNode.h"
#include "config_pgraph.h"
#include "nodePath.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

CPT(RenderEffect) TexProjectorEffect::_empty_effect;
TypeHandle TexProjectorEffect::_type_handle;

/**
 *
 */
TexProjectorEffect::
~TexProjectorEffect() {
}

/**
 * Constructs a TexProjectorEffect that modifies no stages at all.
 */
CPT(RenderEffect) TexProjectorEffect::
make() {
  // We make it a special case and store a pointer to the empty effect forever
  // once we find it the first time, as an optimization.
  if (_empty_effect == nullptr) {
    _empty_effect = return_new(new TexProjectorEffect);
  }

  return _empty_effect;
}

/**
 * Returns a new TexProjectorEffect just like this one, with the indicated
 * projection for the given stage.  If this stage already exists, its
 * projection definition is replaced.
 *
 * The relative transform between the "from" and the "to" nodes is
 * automatically applied to the texture transform each frame.
 *
 * Furthermore, if the "to" node is a LensNode, its projection matrix is also
 * applied to the texture transform.  In this case, the lens_index may be used
 * to select the particular lens that should be used.
 */
CPT(RenderEffect) TexProjectorEffect::
add_stage(TextureStage *stage, const NodePath &from, const NodePath &to, int lens_index) const {
  TexProjectorEffect *effect = new TexProjectorEffect(*this);
  StageDef &def = effect->_stages[stage];
  def.set_from(from);
  def.set_to(to);
  def.set_lens_index(lens_index);
  return return_new(effect);
}

/**
 * Returns a new TexProjectorEffect just like this one, with the indicated
 * stage removed.
 */
CPT(RenderEffect) TexProjectorEffect::
remove_stage(TextureStage *stage) const {
  TexProjectorEffect *effect = new TexProjectorEffect(*this);
  effect->_stages.erase(stage);
  return return_new(effect);
}

/**
 * Returns true if no stages are defined in the TexProjectorEffect, false if
 * at least one is.
 */
bool TexProjectorEffect::
is_empty() const {
  return _stages.empty();
}

/**
 * Returns true if there is a transform associated with the indicated stage,
 * or false otherwise (in which case get_transform(stage) will return the
 * identity transform).
 */
bool TexProjectorEffect::
has_stage(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  return (mi != _stages.end());
}

/**
 * Returns the "from" node associated with the TexProjectorEffect on the
 * indicated stage.  The relative transform between the "from" and the "to"
 * nodes is automatically applied to the texture transform each frame.
 */
NodePath TexProjectorEffect::
get_from(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  nassertr(mi != _stages.end(), NodePath::fail());
  return (*mi).second._from;
}

/**
 * Returns the "to" node associated with the TexProjectorEffect on the
 * indicated stage.  The relative transform between the "from" and the "to"
 * nodes is automatically applied to the texture transform each frame.
 *
 * Furthermore, if the "to" node is a LensNode, its projection matrix is also
 * applied to the texture transform.
 */
NodePath TexProjectorEffect::
get_to(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  nassertr(mi != _stages.end(), NodePath::fail());
  return (*mi).second._to;
}

/**
 * Returns the lens_index associated with the TexProjectorEffect on the
 * indicated stage.  This is only used if the "to" node is a LensNode, in
 * which case it specifies the particular lens that should be used.
 */
int TexProjectorEffect::
get_lens_index(TextureStage *stage) const {
  Stages::const_iterator mi = _stages.find(stage);
  nassertr(mi != _stages.end(), 0);
  return (*mi).second._lens_index;
}

/**
 *
 */
void TexProjectorEffect::
output(std::ostream &out) const {
  out << get_type() << ":";

  Stages::const_iterator mi;
  for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
    TextureStage *stage = (*mi).first;
    const StageDef &def = (*mi).second;
    out << " " << stage->get_name() << "(" << def._to
        << ", " << def._from << ", " << def._lens_index << ")";
  }
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this effect during the cull traversal.
 */
bool TexProjectorEffect::
has_cull_callback() const {
  return !_stages.empty();
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
void TexProjectorEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  CPT(TexMatrixAttrib) tex_matrix = DCAST(TexMatrixAttrib, TexMatrixAttrib::make());

  Stages::const_iterator mi;
  for (mi = _stages.begin(); mi != _stages.end(); ++mi) {
    TextureStage *stage = (*mi).first;
    const StageDef &def = (*mi).second;

    CPT(TransformState) transform = def._from.get_transform(def._to);

    if (def._to_lens_node != nullptr &&
        def._to_lens_node->get_lens() != nullptr) {

      // Get the lens's projection matrix, as a TransformState.
      Lens *lens = def._to_lens_node->get_lens(def._lens_index);
      if (lens != nullptr) {
        CPT(TransformState) projmat = TransformState::make_mat(lens->get_projection_mat());

        // We need a special transform to convert the -0.5, 0.5 centering of
        // the lens's projection matrix to UV's in the range of (0, 1).
        static CPT(TransformState) fixmat;
        if (fixmat == nullptr) {
          fixmat = TransformState::make_pos_hpr_scale
            (LVecBase3(0.5f, 0.5f, 0.0f),
             LVecBase3(0.0f, 0.0f, 0.0f),
             LVecBase3(0.5f, 0.5f, 1.0f));
        }

        // Now apply both to the current transform.
        transform = fixmat->compose(projmat)->compose(transform);
      }
    }

    if (!transform->is_identity()) {
      tex_matrix = DCAST(TexMatrixAttrib,
                         tex_matrix->add_stage(stage, transform));
    }
  }

  if (!tex_matrix->is_empty()) {
    node_state = node_state->compose(RenderState::make(tex_matrix));
  }
}

/**
 * Intended to be overridden by derived TexProjectorEffect types to return a
 * unique number indicating whether this TexProjectorEffect is equivalent to
 * the other one.
 *
 * This should return 0 if the two TexProjectorEffect objects are equivalent,
 * a number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two TexProjectorEffect objects whose
 * get_type() functions return the same.
 */
int TexProjectorEffect::
compare_to_impl(const RenderEffect *other) const {
  const TexProjectorEffect *ta;
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
      int compare = (*ai).second.compare_to((*bi).second);
      if (compare != 0) {
        return compare;
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

/**
 * Tells the BamReader how to create objects of type TexProjectorEffect.
 */
void TexProjectorEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TexProjectorEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  // For now, we don't write anything to the bam file for a
  // TexProjectorEffect, except a bogus 0 "size" which may one day indicate
  // the number of stages in the map that we might write out.
  dg.add_uint16(0);

  // One day we will write the whole map out.  We don't do this yet, because
  // (a) we don't have an interface for writing out NodePaths to a bam file,
  // and (b) it won't matter until we have the Panda-monium system in place,
  // since you can't load a TexProjectorEffect from an egg file.
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int TexProjectorEffect::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderEffect::complete_pointers(p_list, manager);

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type TexProjectorEffect is encountered in the Bam file.  It should create
 * the TexProjectorEffect and extract its information from the file.
 */
TypedWritable *TexProjectorEffect::
make_from_bam(const FactoryParams &params) {
  TexProjectorEffect *effect = new TexProjectorEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new TexProjectorEffect.
 */
void TexProjectorEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  size_t num_stages = scan.get_uint16();

  // Since we don't support full reading and writing of TexProjectorEffects
  // yet, this value had better be zero.  If it's not, maybe we're trying to
  // read a bam file that was generated by some future version of Panda that
  // does support these things.
  nassertv(num_stages == 0);
}

/**
 *
 */
void TexProjectorEffect::StageDef::
set_to(const NodePath &to) {
  _to = to;
  if (!_to.is_empty() && _to.node()->is_of_type(LensNode::get_class_type())) {
    DCAST_INTO_V(_to_lens_node, _to.node());
  } else {
    _to_lens_node = nullptr;
  }
}
