/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureAttrib.I
 * @author drose
 * @date 2002-02-21
 */

/**
 * Use TextureAttrib::make() to construct a new TextureAttrib object.
 */
INLINE TextureAttrib::
TextureAttrib() {
  _next_implicit_sort = 0;
  _off_all_stages = false;
  _sort_seq = UpdateSeq::old();
  _filtered_seq = UpdateSeq::old();
}

/**
 * Use TextureAttrib::make() to construct a new TextureAttrib object.  The
 * copy constructor is only defined to facilitate methods like add_on_stage().
 */
INLINE TextureAttrib::
TextureAttrib(const TextureAttrib &copy) :
  _on_stages(copy._on_stages),
  _render_stages(),
  _render_ff_stages(),
  _next_implicit_sort(copy._next_implicit_sort),
  _off_stages(copy._off_stages),
  _off_all_stages(copy._off_all_stages),
  _sort_seq(UpdateSeq::old()),
  _filtered_seq(UpdateSeq::old())
{
}

/**
 * Returns true if the TextureAttrib is an 'off' TextureAttrib, indicating
 * that it should disable texturing.
 *
 * If multitexture is in effect, a TextureAttrib may not be strictly "on" or
 * "off"; therefore, to get a more precise answer to this question, you should
 * consider using has_all_off() or get_num_off_stages() or has_off_stage()
 * instead.
 */
INLINE bool TextureAttrib::
is_off() const {
  return (_on_stages.empty());
}

/**
 * If the TextureAttrib is not an 'off' TextureAttrib, returns the base-level
 * texture that is associated.  Otherwise, return NULL.
 */
INLINE Texture *TextureAttrib::
get_texture() const {
  if (_on_stages.empty()) {
    return nullptr;
  }
  check_sorted();
  return get_on_texture(filter_to_max(1)->get_on_stage(0));
}

/**
 * Returns the number of stages that are turned on by the attribute.
 */
INLINE int TextureAttrib::
get_num_on_stages() const {
  check_sorted();
  return _render_stages.size();
}

/**
 * Returns the nth stage turned on by the attribute, sorted in render order.
 */
INLINE TextureStage *TextureAttrib::
get_on_stage(int n) const {
  nassertr(n >= 0 && n < (int)_render_stages.size(), nullptr);
  return _render_stages[n]->_stage;
}

/**
 * Returns the number of on-stages that are relevant to the classic fixed
 * function pipeline.  This excludes texture stages such as normal maps.
 */
INLINE int TextureAttrib::
get_num_on_ff_stages() const {
  check_sorted();
  return _render_ff_stages.size();
}

/**
 * Returns the nth stage turned on by the attribute, sorted in render order,
 * including only those relevant to the classic fixed function pipeline.  This
 * excludes texture stages such as normal maps.
 */
INLINE TextureStage *TextureAttrib::
get_on_ff_stage(int n) const {
  nassertr(n >= 0 && n < (int)_render_ff_stages.size(), nullptr);
  return _render_ff_stages[n]->_stage;
}

/**
 * For each TextureStage listed in get_on_ff_stage(), this returns a unique
 * index number for the texture coordinate name used by that TextureStage.  It
 * is guaranteed to remain the same index number for each texcoord name (for a
 * given set of TextureStages), even if the texture render order changes.
 */
INLINE int TextureAttrib::
get_ff_tc_index(int n) const {
  nassertr(n >= 0 && n < (int)_render_ff_stages.size(), -1);
  return _render_ff_stages[n]->_ff_tc_index;
}

/**
 * Returns true if the indicated stage is turned on by the attrib, false
 * otherwise.
 */
INLINE bool TextureAttrib::
has_on_stage(TextureStage *stage) const {
  return _on_stages.find(StageNode(stage)) != _on_stages.end();
}

/**
 * Returns the texture associated with the indicated stage, or NULL if no
 * texture is associated.
 */
INLINE Texture *TextureAttrib::
get_on_texture(TextureStage *stage) const {
  Stages::const_iterator si;
  si = _on_stages.find(StageNode(stage));
  if (si != _on_stages.end()) {
    return (*si)._texture;
  }
  return nullptr;
}

/**
 * Returns the sampler associated with the indicated stage, or the one
 * associated with its texture if no custom stage has been specified.  It is
 * an error to call this if the stage does not exist.
 */
INLINE const SamplerState &TextureAttrib::
get_on_sampler(TextureStage *stage) const {
  Stages::const_iterator si;
  si = _on_stages.find(StageNode(stage));
  nassertr_always(si != _on_stages.end(), SamplerState::get_default());

  return si->_has_sampler ? si->_sampler
                          : si->_texture->get_default_sampler();
}

/**
 * Returns the override value associated with the indicated stage.
 */
INLINE int TextureAttrib::
get_on_stage_override(TextureStage *stage) const {
  Stages::const_iterator si;
  si = _on_stages.find(StageNode(stage));
  if (si != _on_stages.end()) {
    return (*si)._override;
  }
  nassert_raise("Specified TextureStage not included in attrib");
  return 0;
}

/**
 * Returns the number of stages that are turned off by the attribute.
 */
INLINE int TextureAttrib::
get_num_off_stages() const {
  return _off_stages.size();
}

/**
 * Returns the nth stage turned off by the attribute, sorted in arbitrary
 * (pointer) order.
 */
INLINE TextureStage *TextureAttrib::
get_off_stage(int n) const {
  nassertr(n >= 0 && n < (int)_off_stages.size(), nullptr);
  return _off_stages[n]._stage;
}

/**
 * Returns true if the indicated stage is turned off by the attrib, false
 * otherwise.
 */
INLINE bool TextureAttrib::
has_off_stage(TextureStage *stage) const {
  return _off_stages.find(StageNode(stage)) != _off_stages.end() ||
    (_off_all_stages && !has_on_stage(stage));
}

/**
 * Returns true if this attrib turns off all stages (although it may also turn
 * some on).
 */
INLINE bool TextureAttrib::
has_all_off() const {
  return _off_all_stages;
}

/**
 * Returns true if this is an identity attrib: it does not change the set of
 * stages in use.
 */
INLINE bool TextureAttrib::
is_identity() const {
  return _on_stages.empty() && _off_stages.empty() && !_off_all_stages;
}

/**
 * Confirms whether the _on_stages list is still sorted.  It will become
 * unsorted if someone calls TextureStage::set_sort().
 *
 * If the list requires sorting, transparently sorts it before returning.
 */
INLINE void TextureAttrib::
check_sorted() const {
  if (_sort_seq != TextureStage::get_sort_seq()) {
    ((TextureAttrib *)this)->sort_on_stages();
  }
}

/**
 *
 */
INLINE TextureAttrib::StageNode::
StageNode(const TextureStage *stage, unsigned int implicit_sort, int override) :
  // Yeah, we cast away the constness here.  Just too much trouble to deal
  // with it properly.
  _stage((TextureStage *)stage),
  _implicit_sort(implicit_sort),
  _override(override),
  _has_sampler(false)
{
}

/**
 * This STL function object is used to sort a list of texture stages in
 * reverse order by priority, and within priority, within order by sort.
 */
INLINE bool TextureAttrib::CompareTextureStagePriorities::
operator () (const TextureAttrib::StageNode *a,
             const TextureAttrib::StageNode *b) const {
  if (a->_stage->get_priority() != b->_stage->get_priority()) {
    return a->_stage->get_priority() > b->_stage->get_priority();
  }
  if (a->_stage->get_sort() != b->_stage->get_sort()) {
    return a->_stage->get_sort() < b->_stage->get_sort();
  }
  return a->_implicit_sort < b->_implicit_sort;
}

/**
 * This STL function object is used to sort a list of texture stages in order
 * by sort.
 */
INLINE bool TextureAttrib::CompareTextureStageSort::
operator () (const TextureAttrib::StageNode *a,
             const TextureAttrib::StageNode *b) const {
  if (a->_stage->get_sort() != b->_stage->get_sort()) {
    return a->_stage->get_sort() < b->_stage->get_sort();
  }
  return a->_implicit_sort < b->_implicit_sort;
}

/**
 * This STL function object is used to sort a list of texture stages in order
 * by pointer.
 */
INLINE bool TextureAttrib::CompareTextureStagePointer::
operator () (const TextureAttrib::StageNode &a,
             const TextureAttrib::StageNode &b) const {
  return a._stage < b._stage;
}
