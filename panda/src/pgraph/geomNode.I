/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomNode.I
 * @author drose
 * @date 2002-02-23
 */

/**
 * Sets the "preserved" flag.  When this is true, the GeomNode will be left
 * untouched by any flatten operations.
 */
INLINE void GeomNode::
set_preserved(bool value) {
  _preserved = value;
}

/**
 * Returns the "preserved" flag.  When this is true, the GeomNode will be left
 * untouched by any flatten operations.
 */
INLINE bool GeomNode::
get_preserved() const {
  return _preserved;
}

/**
 * Returns the number of geoms in the node.
 */
INLINE int GeomNode::
get_num_geoms() const {
  CDReader cdata(_cycler);
  return cdata->get_geoms()->size();
}


/**
 * Returns the nth geom of the node.  This object should not be modified,
 * since the same object might be shared between multiple different GeomNodes,
 * but see modify_geom().
 */
INLINE CPT(Geom) GeomNode::
get_geom(int n) const {
  CDReader cdata(_cycler);
  CPT(GeomList) geoms = cdata->get_geoms();
  nassertr(n >= 0 && n < (int)geoms->size(), nullptr);
  return (*geoms)[n]._geom.get_read_pointer();
}

/**
 * Returns the nth geom of the node, suitable for modifying it.  If the nth
 * Geom has multiple reference counts to it, reassigns it to an identical copy
 * first, and returns the new copy--this provides a "copy on write" that
 * ensures that the Geom that is returned is unique to this GeomNode and is
 * not shared with any other GeomNodes.
 *
 * Note that if this method is called in a downstream stage (for instance,
 * during cull or draw), then it will propagate the new list of Geoms upstream
 * all the way to pipeline stage 0, which may step on changes that were made
 * independently in pipeline stage 0. Use with caution.
 */
INLINE PT(Geom) GeomNode::
modify_geom(int n) {
  CDWriter cdata(_cycler, true);
  PT(GeomList) geoms = cdata->modify_geoms();
  nassertr(n >= 0 && n < (int)geoms->size(), nullptr);
  mark_internal_bounds_stale();
  return (*geoms)[n]._geom.get_write_pointer();
}

/**
 * Returns the RenderState associated with the nth geom of the node.  This is
 * just the RenderState directly associated with the Geom; the actual state in
 * which the Geom is rendered will also be affected by RenderStates that
 * appear on the scene graph in nodes above this GeomNode.
 */
INLINE const RenderState *GeomNode::
get_geom_state(int n) const {
  CDReader cdata(_cycler);
  CPT(GeomList) geoms = cdata->get_geoms();
  nassertr(n >= 0 && n < (int)geoms->size(), nullptr);
  return (*geoms)[n]._state;
}

/**
 * Changes the RenderState associated with the nth geom of the node.  This is
 * just the RenderState directly associated with the Geom; the actual state in
 * which the Geom is rendered will also be affected by RenderStates that
 * appear on the scene graph in nodes above this GeomNode.
 *
 * Note that if this method is called in a downstream stage (for instance,
 * during cull or draw), then it will propagate the new list of Geoms upstream
 * all the way to pipeline stage 0, which may step on changes that were made
 * independently in pipeline stage 0. Use with caution.
 */
INLINE void GeomNode::
set_geom_state(int n, const RenderState *state) {
  CDWriter cdata(_cycler, true);
  PT(GeomList) geoms = cdata->modify_geoms();
  nassertv(n >= 0 && n < (int)geoms->size());
  (*geoms)[n]._state = state;
}

/**
 * Removes the nth geom from the node.
 */
INLINE void GeomNode::
remove_geom(int n) {
  CDWriter cdata(_cycler);
  PT(GeomList) geoms = cdata->modify_geoms();
  nassertv(n >= 0 && n < (int)geoms->size());

  geoms->erase(geoms->begin() + n);
  mark_internal_bounds_stale();
}

/**
 * Removes all the geoms from the node at once.
 */
INLINE void GeomNode::
remove_all_geoms() {
  CDWriter cdata(_cycler);
  cdata->set_geoms(new GeomList);
  mark_internal_bounds_stale();
}

/**
 * Returns the default into_collide_mask assigned to new GeomNodes.
 */
INLINE CollideMask GeomNode::
get_default_collide_mask() {
  return default_geom_node_collide_mask;
}

/**
 * Increments the count for the indicated InternalName.
 */
INLINE void GeomNode::
count_name(GeomNode::NameCount &name_count, const InternalName *name) {
  std::pair<NameCount::iterator, bool> result =
    name_count.insert(NameCount::value_type(name, 1));
  if (!result.second) {
    (*result.first).second++;
  }
}

/**
 * Returns the count for the indicated InternalName.
 */
INLINE int GeomNode::
get_name_count(const GeomNode::NameCount &name_count, const InternalName *name) {
  NameCount::const_iterator ni;
  ni = name_count.find(name);
  if (ni != name_count.end()) {
    return (*ni).second;
  }
  return 0;
}

/**
 * Returns an object that can be used to walk through the list of geoms of the
 * node.  When you intend to visit multiple geoms, using this is slightly
 * faster than calling get_geom() directly on the GeomNode, since this object
 * avoids reopening the PipelineCycler each time.
 *
 * This object also protects you from self-modifying loops (e.g.  adding or
 * removing geoms during traversal), since a virtual copy of the geoms is made
 * ahead of time.  The virtual copy is fast--it is a form of copy-on-write, so
 * the list is not actually copied unless it is modified during the traversal.
 */
INLINE GeomNode::Geoms GeomNode::
get_geoms(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return Geoms(cdata);
}

/**
 *
 */
INLINE GeomNode::GeomEntry::
GeomEntry(Geom *geom, const RenderState *state) :
  _geom(geom),
  _state(state)
{
}

/**
 *
 */
INLINE GeomNode::CData::
CData() :
  _geoms(new GeomNode::GeomList)
{
}

/**
 * Returns a read-only pointer to the _geoms list.
 */
INLINE CPT(GeomNode::GeomList) GeomNode::CData::
get_geoms() const {
  return _geoms.get_read_pointer();
}

/**
 * Returns a modifiable, unique pointer to the _geoms list.
 */
INLINE PT(GeomNode::GeomList) GeomNode::CData::
modify_geoms() {
  return _geoms.get_write_pointer();
}

/**
 * Replaces the _geoms list with a new list.
 */
INLINE void GeomNode::CData::
set_geoms(GeomNode::GeomList *geoms) {
  _geoms = geoms;
}

/**
 *
 */
INLINE GeomNode::Geoms::
Geoms() {
}

/**
 *
 */
INLINE GeomNode::Geoms::
Geoms(const GeomNode::CData *cdata) :
  _geoms(cdata->get_geoms())
{
}

/**
 *
 */
INLINE GeomNode::Geoms::
Geoms(const GeomNode::Geoms &copy) :
  _geoms(copy._geoms)
{
}

/**
 *
 */
INLINE void GeomNode::Geoms::
operator = (const GeomNode::Geoms &copy) {
  _geoms = copy._geoms;
}

/**
 *
 */
INLINE GeomNode::Geoms::
Geoms(GeomNode::Geoms &&from) noexcept :
  _geoms(std::move(from._geoms))
{
}

/**
 *
 */
INLINE void GeomNode::Geoms::
operator = (GeomNode::Geoms &&from) noexcept {
  _geoms = std::move(from._geoms);
}

/**
 * Returns the number of geoms of the node.
 */
INLINE int GeomNode::Geoms::
get_num_geoms() const {
  nassertr(!_geoms.is_null(), 0);
  return _geoms->size();
}

/**
 * Returns the nth geom of the node.  This object should not be modified,
 * since the same object might be shared between multiple different GeomNodes.
 */
INLINE CPT(Geom) GeomNode::Geoms::
get_geom(int n) const {
  nassertr(!_geoms.is_null(), nullptr);
  nassertr(n >= 0 && n < (int)_geoms->size(), nullptr);
  return (*_geoms)[n]._geom.get_read_pointer();
}

/**
 * Returns the RenderState associated with the nth geom of the node.  This is
 * just the RenderState directly associated with the Geom; the actual state in
 * which the Geom is rendered will also be affected by RenderStates that
 * appear on the scene graph in nodes above this GeomNode.
 */
INLINE const RenderState *GeomNode::Geoms::
get_geom_state(int n) const {
  nassertr(!_geoms.is_null(), nullptr);
  nassertr(n >= 0 && n < (int)_geoms->size(), nullptr);
  return (*_geoms)[n]._state;
}
