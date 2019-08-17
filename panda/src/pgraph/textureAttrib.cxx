/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureAttrib.cxx
 * @author drose
 * @date 2002-02-21
 */

#include "textureAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "internalName.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "dcast.h"
#include "textureStagePool.h"

CPT(RenderAttrib) TextureAttrib::_empty_attrib;
CPT(RenderAttrib) TextureAttrib::_all_off_attrib;
TypeHandle TextureAttrib::_type_handle;
int TextureAttrib::_attrib_slot;


/**
 * Constructs a new TextureAttrib object suitable for rendering the indicated
 * texture onto geometry, using the default TextureStage.
 */
CPT(RenderAttrib) TextureAttrib::
make(Texture *texture) {
  return DCAST(TextureAttrib, make())->add_on_stage(TextureStage::get_default(), texture);
}

/**
 * Constructs a new TextureAttrib object suitable for rendering untextured
 * geometry.
 */
CPT(RenderAttrib) TextureAttrib::
make_off() {
  return make_all_off();
}

/**
 * Constructs a new TextureAttrib object that does nothing.
 */
CPT(RenderAttrib) TextureAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib forever
  // once we find it the first time, as an optimization.
  if (_empty_attrib == nullptr) {
    _empty_attrib = return_new(new TextureAttrib);
  }

  return _empty_attrib;
}

/**
 * Constructs a new TextureAttrib object that turns off all stages (and hence
 * disables texturing).
 */
CPT(RenderAttrib) TextureAttrib::
make_all_off() {
  // We make it a special case and store a pointer to the off attrib forever
  // once we find it the first time, as an optimization.
  if (_all_off_attrib == nullptr) {
    TextureAttrib *attrib = new TextureAttrib;
    attrib->_off_all_stages = true;
    _all_off_attrib = return_new(attrib);
  }

  return _all_off_attrib;
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) TextureAttrib::
make_default() {
  return make();
}


/**
 * Returns the index number of the indicated TextureStage within the list of
 * on_stages, or -1 if the indicated stage is not listed.
 */
int TextureAttrib::
find_on_stage(const TextureStage *stage) const {
  Stages::const_iterator si = _on_stages.find(StageNode(stage));
  if (si != _on_stages.end()) {
    return (int)(si - _on_stages.begin());
  }

  return -1;
}

/**
 * Returns a new TextureAttrib, just like this one, but with the indicated
 * stage added to the list of stages turned on by this attrib.
 */
CPT(RenderAttrib) TextureAttrib::
add_on_stage(TextureStage *stage, Texture *tex, int override) const {
  nassertr(tex != nullptr, this);

  TextureAttrib *attrib = new TextureAttrib(*this);
  Stages::iterator si = attrib->_on_stages.insert(StageNode(stage)).first;
  (*si)._override = override;
  (*si)._texture = tex;
  (*si)._implicit_sort = attrib->_next_implicit_sort;
  (*si)._has_sampler = false;
  ++(attrib->_next_implicit_sort);

  return return_new(attrib);
}

/**
 * Returns a new TextureAttrib, just like this one, but with the indicated
 * stage added to the list of stages turned on by this attrib.
 */
CPT(RenderAttrib) TextureAttrib::
add_on_stage(TextureStage *stage, Texture *tex, const SamplerState &sampler, int override) const {
  nassertr(tex != nullptr, this);

  TextureAttrib *attrib = new TextureAttrib(*this);
  Stages::iterator si = attrib->_on_stages.insert(StageNode(stage)).first;
  (*si)._override = override;
  (*si)._texture = tex;
  (*si)._sampler = sampler;
  (*si)._implicit_sort = attrib->_next_implicit_sort;
  (*si)._has_sampler = true;
  ++(attrib->_next_implicit_sort);

  return return_new(attrib);
}

/**
 * Returns a new TextureAttrib, just like this one, but with the indicated
 * stage removed from the list of stages turned on by this attrib.
 */
CPT(RenderAttrib) TextureAttrib::
remove_on_stage(TextureStage *stage) const {
  TextureAttrib *attrib = new TextureAttrib(*this);

  Stages::iterator si = attrib->_on_stages.find(StageNode(stage));
  if (si != attrib->_on_stages.end()) {
    attrib->_on_stages.erase(si);
  }

  return return_new(attrib);
}

/**
 * Returns a new TextureAttrib, just like this one, but with the indicated
 * stage added to the list of stages turned off by this attrib.
 */
CPT(RenderAttrib) TextureAttrib::
add_off_stage(TextureStage *stage, int override) const {
  TextureAttrib *attrib = new TextureAttrib(*this);
  if (!_off_all_stages) {
    StageNode sn(stage);
    Stages::iterator sfi = attrib->_off_stages.insert(sn).first;
    (*sfi)._override = override;

    // Also ensure it is removed from the on_stages list.
    Stages::iterator si = attrib->_on_stages.find(sn);
    if (si != attrib->_on_stages.end()) {
      attrib->_on_stages.erase(si);
    }
  }
  return return_new(attrib);
}

/**
 * Returns a new TextureAttrib, just like this one, but with the indicated
 * stage removed from the list of stages turned off by this attrib.
 */
CPT(RenderAttrib) TextureAttrib::
remove_off_stage(TextureStage *stage) const {
  TextureAttrib *attrib = new TextureAttrib(*this);
  attrib->_off_stages.erase(StageNode(stage));
  return return_new(attrib);
}

/**
 * Returns a new TextureAttrib, just like this one, but with any included
 * TextureAttribs that happen to have the same name as the given object
 * replaced with the object.
 */
CPT(RenderAttrib) TextureAttrib::
unify_texture_stages(TextureStage *stage) const {
  PT(TextureAttrib) attrib = new TextureAttrib;

  attrib->_off_all_stages = _off_all_stages;
  bool any_changed = false;

  Stages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    TextureStage *this_stage = (*si)._stage;

    if (this_stage->get_name() == stage->get_name()) {
      this_stage = stage;
      any_changed = true;
    }

    Stages::iterator osi = attrib->_on_stages.insert(StageNode(this_stage)).first;
    (*osi)._texture = (*si)._texture;
    (*osi)._ff_tc_index = (*si)._ff_tc_index;
    (*osi)._implicit_sort = (*si)._implicit_sort;
    (*osi)._override = (*si)._override;
  }

  attrib->_next_implicit_sort = _next_implicit_sort;

  Stages::const_iterator fsi;
  for (fsi = _off_stages.begin(); fsi != _off_stages.end(); ++fsi) {
    TextureStage *this_stage = (*fsi)._stage;

    if (this_stage != stage &&
        this_stage->get_name() == stage->get_name()) {
      this_stage = stage;
      any_changed = true;
    }

    attrib->_off_stages.insert(StageNode(this_stage));
  }

  if (!any_changed) {
    return this;
  }

  return return_new(attrib);
}

/**
 * Returns a new TextureAttrib, just like this one, but with all references to
 * the given texture replaced with the new texture.
 *
 * @since 1.10.4
 */
CPT(RenderAttrib) TextureAttrib::
replace_texture(Texture *tex, Texture *new_tex) const {
  TextureAttrib *attrib = nullptr;

  for (size_t i = 0; i < _on_stages.size(); ++i) {
    const StageNode &sn = _on_stages[i];
    if (sn._texture == tex) {
      if (attrib == nullptr) {
        attrib = new TextureAttrib(*this);
      }

      attrib->_on_stages[i]._texture = new_tex;
    }
  }

  if (attrib != nullptr) {
    return return_new(attrib);
  } else {
    return this;
  }
}

/**
 * Returns a new TextureAttrib, very much like this one, but with the number
 * of on_stages reduced to be no more than max_texture_stages.  The number of
 * off_stages in the new TextureAttrib is undefined.
 */
CPT(TextureAttrib) TextureAttrib::
filter_to_max(int max_texture_stages) const {
  if ((int)_on_stages.size() <= max_texture_stages) {
    // Trivial case: this TextureAttrib qualifies.
    return this;
  }

  if (_filtered_seq != TextureStage::get_sort_seq()) {
    ((TextureAttrib *)this)->_filtered.clear();
    ((TextureAttrib *)this)->_filtered_seq = TextureStage::get_sort_seq();
  }

  Filtered::const_iterator fi;
  fi = _filtered.find(max_texture_stages);
  if (fi != _filtered.end()) {
    // Easy case: we have already computed this for this particular
    // TextureAttrib.
    return (*fi).second;
  }

  // Harder case: we have to compute it now.  We must choose the n stages with
  // the highest priority in our list of stages.  In the case of equal
  // priority, we prefer the stage with the lower sort.
  check_sorted();

  RenderStages priority_stages = _render_stages;

  // This sort function uses the STL function object defined above.
  sort(priority_stages.begin(), priority_stages.end(),
       CompareTextureStagePriorities());

  // Now lop off all of the stages after the first max_texture_stages.
  priority_stages.erase(priority_stages.begin() + max_texture_stages,
                        priority_stages.end());

  // And create a new attrib reflecting these stages.
  PT(TextureAttrib) attrib = new TextureAttrib;

  RenderStages::const_iterator ri;
  for (ri = priority_stages.begin(); ri != priority_stages.end(); ++ri) {
    attrib->_on_stages.insert(*(*ri));
  }

  attrib->_next_implicit_sort = _next_implicit_sort;

  CPT(RenderAttrib) new_attrib = return_new(attrib);

  // Finally, record this newly-created attrib in the map for next time.

  // TODO: if new_attrib == this, have we just created a circular reference
  // count?  Whoops!  Fix this!

  CPT(TextureAttrib) tex_attrib = (const TextureAttrib *)new_attrib.p();
  ((TextureAttrib *)this)->_filtered[max_texture_stages] = tex_attrib;
  return tex_attrib;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return false if a RenderAttrib on a higher node will compose
 * into a RenderAttrib on a lower node that has a higher override value, or
 * false if the lower RenderAttrib will completely replace the state.
 *
 * The default behavior is false: normally, a RenderAttrib in the graph cannot
 * completely override a RenderAttrib above it, regardless of its override
 * value--instead, the two attribs are composed.  But for some kinds of
 * RenderAttribs, it is useful to allow this kind of override.
 *
 * This method only handles the one special case of a lower RenderAttrib with
 * a higher override value.  If the higher RenderAttrib has a higher override
 * value, it always completely overrides.  And if both RenderAttribs have the
 * same override value, they are always composed.
 */
bool TextureAttrib::
lower_attrib_can_override() const {
  // A TextureAttrib doesn't compose through an override.  Normally, there
  // won't be a scene-graph override on a TextureAttrib anyway, since the
  // NodePath::set_texture() override is applied to the per-TextureStage
  // override value.  But there might be a scene-graph override if
  // NodePath::adjust_all_priorities() is used, and in this case, we'd like
  // for it to stick.
  return true;
}

/**
 *
 */
void TextureAttrib::
output(std::ostream &out) const {
  check_sorted();

  out << get_type() << ":";
  if (_off_stages.empty()) {
    if (_on_stages.empty()) {
      if (_off_all_stages) {
        out << "all off";
      } else {
        out << "identity";
      }
    } else {
      if (_off_all_stages) {
        out << "set";
      } else {
        out << "on";
      }
    }

  } else {
    out << "off";
    Stages::const_iterator fi;
    for (fi = _off_stages.begin(); fi != _off_stages.end(); ++fi) {
      TextureStage *stage = (*fi)._stage;
      out << " " << stage->get_name();
      if ((*fi)._override != 0) {
        out << "^" << (*fi)._override;
      }
    }

    if (!_on_stages.empty()) {
      out << " on";
    }
  }

  RenderStages::const_iterator ri;
  for (ri = _render_stages.begin(); ri != _render_stages.end(); ++ri) {
    const StageNode &sn = *(*ri);
    TextureStage *stage = sn._stage;
    Texture *tex = sn._texture;
    out << " " << stage->get_name();
    if (tex != nullptr) {
      out << ":" << tex->get_name();
    }
    if (sn._override != 0) {
      out << "^" << sn._override;
    }
  }
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this node during the cull traversal.
 */
bool TextureAttrib::
has_cull_callback() const {
  Stages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    Texture *texture = (*si)._texture;
    if (texture->has_cull_callback()) {
      return true;
    }
  }

  return false;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.
 *
 * This is called each time the RenderAttrib is discovered applied to a Geom
 * in the traversal.  It should return true if the Geom is visible, false if
 * it should be omitted.
 */
bool TextureAttrib::
cull_callback(CullTraverser *trav, const CullTraverserData &data) const {
  Stages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    Texture *texture = (*si)._texture;
    if (!texture->cull_callback(trav, data)) {
      return false;
    }
  }

  return true;
}

/**
 * Intended to be overridden by derived TextureAttrib types to return a unique
 * number indicating whether this TextureAttrib is equivalent to the other
 * one.
 *
 * This should return 0 if the two TextureAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two TextureAttrib objects whose get_type()
 * functions return the same.
 */
int TextureAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TextureAttrib *ta = (const TextureAttrib *)other;

  if (_off_all_stages != ta->_off_all_stages) {
    return (int)_off_all_stages - (int)ta->_off_all_stages;
  }

  Stages::const_iterator si = _on_stages.begin();
  Stages::const_iterator osi = ta->_on_stages.begin();

  while (si != _on_stages.end() && osi != ta->_on_stages.end()) {
    TextureStage *stage = (*si)._stage;
    TextureStage *other_stage = (*osi)._stage;

    if (stage != other_stage) {
      return stage < other_stage ? -1 : 1;
    }

    Texture *texture = (*si)._texture;
    Texture *other_texture = (*osi)._texture;

    if (texture != other_texture) {
      return texture < other_texture ? -1 : 1;
    }

    int implicit_sort = (*si)._implicit_sort;
    int other_implicit_sort = (*osi)._implicit_sort;

    if (implicit_sort != other_implicit_sort) {
      return implicit_sort < other_implicit_sort ? -1 : 1;
    }

    int override = (*si)._override;
    int other_override = (*osi)._override;

    if (override != other_override) {
      return override < other_override ? -1 : 1;
    }

    int has_sampler = (*si)._has_sampler;
    int other_has_sampler = (*osi)._has_sampler;

    if (has_sampler != other_has_sampler) {
      return has_sampler < other_has_sampler ? -1 : 1;
    }

    if (has_sampler) {
      const SamplerState &sampler = (*si)._sampler;
      const SamplerState &other_sampler = (*osi)._sampler;

      if (sampler != other_sampler) {
        return sampler < other_sampler ? -1 : 1;
      }
    }

    ++si;
    ++osi;
  }

  if (si != _on_stages.end()) {
    return 1;
  }
  if (osi != ta->_on_stages.end()) {
    return -1;
  }

  // Finally, ensure that the set of off stages is the same.
  Stages::const_iterator fi = _off_stages.begin();
  Stages::const_iterator ofi = ta->_off_stages.begin();

  while (fi != _off_stages.end() && ofi != ta->_off_stages.end()) {
    TextureStage *stage = (*fi)._stage;
    TextureStage *other_stage = (*ofi)._stage;

    if (stage != other_stage) {
      return stage < other_stage ? -1 : 1;
    }

    int override = (*fi)._override;
    int other_override = (*ofi)._override;

    if (override != other_override) {
      return override < other_override ? -1 : 1;
    }

    ++fi;
    ++ofi;
  }

  if (fi != _off_stages.end()) {
    return 1;
  }
  if (ofi != ta->_off_stages.end()) {
    return -1;
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t TextureAttrib::
get_hash_impl() const {
  check_sorted();

  size_t hash = 0;
  Stages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    const StageNode &sn = (*si);

    hash = pointer_hash::add_hash(hash, sn._stage);
    hash = pointer_hash::add_hash(hash, sn._texture);
    hash = int_hash::add_hash(hash, (int)sn._implicit_sort);
    hash = int_hash::add_hash(hash, sn._override);
  }

  // This bool value goes here, between the two lists, to differentiate
  // between the two.
  hash = int_hash::add_hash(hash, (int)_off_all_stages);

  for (si = _off_stages.begin(); si != _off_stages.end(); ++si) {
    const StageNode &sn = (*si);

    hash = pointer_hash::add_hash(hash, sn._stage);
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
CPT(RenderAttrib) TextureAttrib::
compose_impl(const RenderAttrib *other) const {
  const TextureAttrib *ta = (const TextureAttrib *)other;

  if (ta->_off_all_stages) {
    // If the other type turns off all stages, it doesn't matter what we are.
    return ta;
  }

  // This is a three-way merge between ai, bi, and ci, except that bi and ci
  // should have no intersection and therefore needn't be compared to each
  // other.
  Stages::const_iterator ai = _on_stages.begin();
  Stages::const_iterator bi = ta->_on_stages.begin();
  Stages::const_iterator ci = ta->_off_stages.begin();

  // Create a new TextureAttrib that will hold the result.
  TextureAttrib *attrib = new TextureAttrib;

  while (ai != _on_stages.end() &&
         bi != ta->_on_stages.end() &&
         ci != ta->_off_stages.end()) {
    if ((*ai)._stage < (*bi)._stage) {
      if ((*ai)._stage < (*ci)._stage) {
        // Here is a stage that we have in the original, which is not present
        // in the secondary.
        attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
        ++ai;

      } else if ((*ci)._stage < (*ai)._stage) {
        // Here is a stage that is turned off in the secondary, but was not
        // present in the original.
        ++ci;

      } else { // (*ci)._stage == (*ai)._stage
        // Here is a stage that is turned off in the secondary, and was
        // present in the original.
        if ((*ai)._override > (*ci)._override) {
          // But never mind, keep it anyway.
          attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
        }

        ++ai;
        ++ci;
      }

    } else if ((*bi)._stage < (*ai)._stage) {
      // Here is a new stage we have in the secondary, that was not present in
      // the original.
      attrib->_on_stages.insert(attrib->_on_stages.end(), *bi);
      ++bi;

    } else {  // (*bi)._stage == (*ai)._stage
      // Here is a stage we have in both.
      if ((*ai)._override > (*bi)._override) {
        attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
      } else {
        attrib->_on_stages.insert(attrib->_on_stages.end(), *bi);
      }
      ++ai;
      ++bi;
    }
  }

  while (ai != _on_stages.end() && bi != ta->_on_stages.end()) {
    if ((*ai)._stage < (*bi)._stage) {
      // Here is a stage that we have in the original, which is not present in
      // the secondary.
      attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
      ++ai;

    } else if ((*bi)._stage < (*ai)._stage) {
      // Here is a new stage we have in the secondary, that was not present in
      // the original.
      attrib->_on_stages.insert(attrib->_on_stages.end(), *bi);
      ++bi;

    } else {
      // Here is a stage we have in both.
      if ((*ai)._override > (*bi)._override) {
        attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
      } else {
        attrib->_on_stages.insert(attrib->_on_stages.end(), *bi);
      }
      ++ai;
      ++bi;
    }
  }

  while (ai != _on_stages.end() && ci != ta->_off_stages.end()) {
    if ((*ai)._stage < (*ci)._stage) {
      // Here is a stage that we have in the original, which is not present in
      // the secondary.
      attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
      ++ai;

    } else if ((*ci)._stage < (*ai)._stage) {
      // Here is a stage that is turned off in the secondary, but was not
      // present in the original.
      ++ci;

    } else { // (*ci)._stage == (*ai)._stage
      // Here is a stage that is turned off in the secondary, and was present
      // in the original.
      if ((*ai)._override > (*ci)._override) {
        // But never mind, keep it anyway.
        attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
      }
      ++ai;
      ++ci;
    }
  }

  while (ai != _on_stages.end()) {
    attrib->_on_stages.insert(attrib->_on_stages.end(), *ai);
    ++ai;
  }

  while (bi != ta->_on_stages.end()) {
    attrib->_on_stages.insert(attrib->_on_stages.end(), *bi);
    ++bi;
  }

  attrib->_next_implicit_sort = _next_implicit_sort + ta->_next_implicit_sort;
  attrib->_sort_seq = UpdateSeq::old();
  attrib->_filtered_seq = UpdateSeq::old();

  return return_new(attrib);
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) TextureAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this needs a
  // bit more thought.  It's hard to imagine that it's even important to
  // compute this properly.
  return other;
}

/**
 * Tells the BamReader how to create objects of type TextureAttrib.
 */
void TextureAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TextureAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  // Write the off_stages information
  dg.add_bool(_off_all_stages);
  dg.add_uint16(get_num_off_stages());
  Stages::const_iterator fi;
  for (fi = _off_stages.begin(); fi != _off_stages.end(); ++fi) {
    TextureStage *stage = (*fi)._stage;
    manager->write_pointer(dg, stage);
  }

  // Write the on_stages information
  dg.add_uint16(get_num_on_stages());
  Stages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    TextureStage *stage = (*si)._stage;
    Texture *tex = (*si)._texture;
    nassertv(tex != nullptr);

    manager->write_pointer(dg, stage);
    manager->write_pointer(dg, tex);
    dg.add_uint16((*si)._implicit_sort);

    if (manager->get_file_minor_ver() >= 23) {
      dg.add_int32((*si)._override);
    }

    if (manager->get_file_minor_ver() >= 36) {
      dg.add_bool((*si)._has_sampler);
      if ((*si)._has_sampler) {
        (*si)._sampler.write_datagram(dg);
      }
    }
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int TextureAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  Stages::iterator ci;
  for (ci = _off_stages.begin(); ci != _off_stages.end(); ++ci) {
    TextureStage *ts = DCAST(TextureStage, p_list[pi++]);
    *ci = StageNode(ts);
  }

  size_t sni = 0;
  while (sni < _on_stages.size()) {
    // Filter the TextureStage through the TextureStagePool.
    PT(TextureStage) ts = DCAST(TextureStage, p_list[pi++]);
    ts = TextureStagePool::get_stage(ts);

    // The Texture pointer filters itself through the TexturePool, so we don't
    // have to do anything special here.
    Texture *tex = DCAST(Texture, p_list[pi++]);

    if (tex != nullptr) {
      StageNode &sn = _on_stages[sni];
      sn._stage = ts;
      sn._texture = tex;
      ++sni;

    } else {
      // If we couldn't load a texture pointer, turn off that particular
      // texture stage.
      _off_stages.push_back(StageNode(ts));
      _on_stages.erase(_on_stages.begin() + sni);
    }
  }
  _on_stages.sort();
  _off_stages.sort();
  _sort_seq = UpdateSeq::old();
  _filtered_seq = UpdateSeq::old();

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type TextureAttrib is encountered in the Bam file.  It should create the
 * TextureAttrib and extract its information from the file.
 */
TypedWritable *TextureAttrib::
make_from_bam(const FactoryParams &params) {
  TextureAttrib *attrib = new TextureAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new TextureAttrib.
 */
void TextureAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // read the _off_stages data.
  _off_all_stages = scan.get_bool();
  int num_off_stages = scan.get_uint16();

  // Push back a NULL pointer for each off TextureStage for now, until we get
  // the actual list of pointers later in complete_pointers().
  int i;
  _off_stages.reserve(num_off_stages);
  for (i = 0; i < num_off_stages; i++) {
    manager->read_pointer(scan);
    _off_stages.push_back(StageNode(nullptr));
  }

  // Read the _on_stages data.
  int num_on_stages = scan.get_uint16();

  // Push back a NULL pointer for each off TextureStage and Texture for now,
  // until we get the actual list of pointers later in complete_pointers().
  _on_stages.reserve(num_on_stages);
  _next_implicit_sort = 0;
  for (i = 0; i < num_on_stages; i++) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    unsigned int implicit_sort;
    if (manager->get_file_minor_ver() >= 15) {
      implicit_sort = scan.get_uint16();
    } else {
      implicit_sort = (unsigned int)i;
    }
    int override = 0;
    if (manager->get_file_minor_ver() >= 23) {
      override = scan.get_int32();
    }

    _next_implicit_sort = std::max(_next_implicit_sort, implicit_sort + 1);
    Stages::iterator si =
      _on_stages.insert_nonunique(StageNode(nullptr, _next_implicit_sort, override));
    ++_next_implicit_sort;

    if (manager->get_file_minor_ver() >= 36) {
      (*si)._has_sampler = scan.get_bool();
      if ((*si)._has_sampler) {
        (*si)._sampler.read_datagram(scan, manager);
      }
    }
  }
}

/**
 * Sorts the list of stages so that they are listed in render order.  Also
 * clears the _filtered map and recalculates the list of fixed-function
 * stages.
 */
void TextureAttrib::
sort_on_stages() {
  typedef pmap<const InternalName *, int> UsedTexcoordIndex;
  UsedTexcoordIndex used_texcoord_index;

  _render_stages.clear();
  _render_ff_stages.clear();

  Stages::iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    StageNode &sn = (*si);
    TextureStage *stage = sn._stage;
    Texture *texture = sn._texture;
    nassertv(stage != nullptr);
    nassertv(texture != nullptr);
    if (stage->is_fixed_function() && texture->get_texture_type() != Texture::TT_2d_texture_array) {
      const InternalName *name = stage->get_texcoord_name();

      // This pair of lines will get the next consecutive texcoord index
      // number if this is the first time we have referenced this particular
      // texcoord name; otherwise, it will return the same index number it
      // returned before.
      UsedTexcoordIndex::iterator ti = used_texcoord_index.insert(UsedTexcoordIndex::value_type(name, (int)used_texcoord_index.size())).first;
      (*si)._ff_tc_index = (*ti).second;

      _render_ff_stages.push_back(&sn);
    } else {
      (*si)._ff_tc_index = -1;
    }

    _render_stages.push_back(&sn);
  }

  sort(_render_stages.begin(), _render_stages.end(), CompareTextureStageSort());
  sort(_render_ff_stages.begin(), _render_ff_stages.end(), CompareTextureStageSort());

  // We'd like to clear the _filtered map, in case the TextureStage priority
  // values have changed as well, but we can't do that here: it's too
  // dangerous.  Clearing _filtered might cause TextureAttribs to be deleted,
  // and hence removed from the map that we might be in the middle of
  // traversing!

  _sort_seq = TextureStage::get_sort_seq();
}
