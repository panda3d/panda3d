// Filename: textureAttrib.cxx
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "textureAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "internalName.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "dcast.h"

CPT(RenderAttrib) TextureAttrib::_empty_attrib;
CPT(RenderAttrib) TextureAttrib::_all_off_attrib;
TypeHandle TextureAttrib::_type_handle;
int TextureAttrib::_attrib_slot;


////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make
//       Access: Published, Static
//  Description: Constructs a new TextureAttrib object suitable for
//               rendering the indicated texture onto geometry, using
//               the default TextureStage.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make(Texture *texture) {
  return DCAST(TextureAttrib, make())->add_on_stage(TextureStage::get_default(), texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new TextureAttrib object suitable for
//               rendering untextured geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make_off() {
  return make_all_off();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make
//       Access: Published, Static
//  Description: Constructs a new TextureAttrib object that does
//               nothing.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib
  // forever once we find it the first time, as an optimization.
  if (_empty_attrib == (RenderAttrib *)NULL) {
    _empty_attrib = return_new(new TextureAttrib);
  }

  return _empty_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_all_off
//       Access: Published, Static
//  Description: Constructs a new TextureAttrib object that turns off
//               all stages (and hence disables texturing).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make_all_off() {
  // We make it a special case and store a pointer to the off attrib
  // forever once we find it the first time, as an optimization.
  if (_all_off_attrib == (RenderAttrib *)NULL) {
    TextureAttrib *attrib = new TextureAttrib;
    attrib->_off_all_stages = true;
    _all_off_attrib = return_new(attrib);
  }

  return _all_off_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_default
//       Access: Published, Static
//  Description: Returns a RenderAttrib that corresponds to whatever
//               the standard default properties for render attributes
//               of this type ought to be.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
make_default() {
  return make();
}


////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::find_on_stage
//       Access: Published
//  Description: Returns the index number of the indicated
//               TextureStage within the list of on_stages, or -1 if
//               the indicated stage is not listed.
////////////////////////////////////////////////////////////////////
int TextureAttrib::
find_on_stage(const TextureStage *stage) const {
  for (int n = 0; n < (int)_on_stages.size(); ++n) {
    if (_on_stages[n]._stage == stage) {
      return n;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::add_on_stage
//       Access: Published
//  Description: Returns a new TextureAttrib, just like this one, but
//               with the indicated stage added to the list of stages
//               turned on by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
add_on_stage(TextureStage *stage, Texture *tex) const {
  TextureAttrib *attrib = new TextureAttrib(*this);
  pair<OnTextures::iterator, bool> insert_result = 
    attrib->_on_textures.insert(OnTextures::value_type(stage, tex));
  if (insert_result.second) {
    // If the insert was successful--we have added a new stage that
    // wasn't present before--then add the stage to the linear list
    // also.
    attrib->_on_stages.push_back(OnStageNode(stage, attrib->_next_implicit_sort));
    ++(attrib->_next_implicit_sort);

    // Also ensure it is removed from the off_stages list.
    attrib->_off_stages.erase(stage);

  } else {
    // If the insert was unsuccessful, it means there was already a
    // definition for that stage.  Replace it.
    (*insert_result.first).second = tex;

    // Also update the implicit sort.
    OnStages::iterator si;
    for (si = attrib->_on_stages.begin(); si != attrib->_on_stages.end(); ++si) {
      if ((*si)._stage == stage) {
        (*si)._implicit_sort = attrib->_next_implicit_sort;
        ++(attrib->_next_implicit_sort);
        break;
      }
    }
  }

  // In either case, we now need to re-sort the attrib list.
  attrib->_sort_seq = UpdateSeq::old();
  attrib->_filtered_seq = UpdateSeq::old();

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::remove_on_stage
//       Access: Published
//  Description: Returns a new TextureAttrib, just like this one, but
//               with the indicated stage removed from the list of
//               stages turned on by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
remove_on_stage(TextureStage *stage) const {
  TextureAttrib *attrib = new TextureAttrib(*this);

  OnTextures::iterator ti = attrib->_on_textures.find(stage);
  if (ti != attrib->_on_textures.end()) {
    attrib->_on_textures.erase(ti);
    
    OnStages::iterator si;
    for (si = attrib->_on_stages.begin(); si != attrib->_on_stages.end(); ++si) {
      if ((*si)._stage == stage) {
        attrib->_on_stages.erase(si);
        break;
      }
    }

    attrib->_sort_seq = UpdateSeq::old();
    attrib->_filtered_seq = UpdateSeq::old();
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::add_off_stage
//       Access: Published
//  Description: Returns a new TextureAttrib, just like this one, but
//               with the indicated stage added to the list of stages
//               turned off by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
add_off_stage(TextureStage *stage) const {
  TextureAttrib *attrib = new TextureAttrib(*this);
  if (!_off_all_stages) {
    attrib->_off_stages.insert(stage);

    // Also ensure it is removed from the on_stages list.
    OnTextures::iterator ti = attrib->_on_textures.find(stage);
    if (ti != attrib->_on_textures.end()) {
      attrib->_on_textures.erase(ti);
    
      OnStages::iterator si;
      for (si = attrib->_on_stages.begin(); si != attrib->_on_stages.end(); ++si) {
        if ((*si)._stage == stage) {
          attrib->_on_stages.erase(si);
          break;
        }
      }
      attrib->_sort_seq = UpdateSeq::old();
      attrib->_filtered_seq = UpdateSeq::old();
    }
  }
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::remove_off_stage
//       Access: Published
//  Description: Returns a new TextureAttrib, just like this one, but
//               with the indicated stage removed from the list of
//               stages turned off by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
remove_off_stage(TextureStage *stage) const {
  TextureAttrib *attrib = new TextureAttrib(*this);
  attrib->_off_stages.erase(stage);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::unify_texture_stages
//       Access: Published
//  Description: Returns a new TextureAttrib, just like this one, but
//               with any included TextureAttribs that happen to have
//               the same name as the given object replaced with the
//               object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
unify_texture_stages(TextureStage *stage) const {
  PT(TextureAttrib) attrib = new TextureAttrib;

  attrib->_off_all_stages = _off_all_stages;
  bool any_changed = false;

  OnStages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    TextureStage *this_stage = (*si)._stage;
    Texture *tex = get_on_texture(this_stage);
    nassertr(tex != (Texture *)NULL, this);

    if (this_stage->get_name() == stage->get_name()) {
      this_stage = stage;
      any_changed = true;
    }

    bool inserted = attrib->_on_textures.insert(OnTextures::value_type(this_stage, tex)).second;
    if (inserted) {
      // If the texture was successfully inserted, it was the first
      // appearance of this stage.  Add it to the on_stages list.
      attrib->_on_stages.push_back(OnStageNode(this_stage, (*si)._implicit_sort));
    } else {
      // If the texture was not successfully inserted, it was a
      // duplicate texture stage.  This should only be possible if we
      // have just collapsed a stage.
      nassertr(this_stage == stage, this);
    }
  }

  attrib->_next_implicit_sort = _next_implicit_sort;

  OffStages::const_iterator fsi;
  for (fsi = _off_stages.begin(); fsi != _off_stages.end(); ++fsi) {
    if ((*fsi) != stage && 
        (*fsi)->get_name() == stage->get_name()) {
      attrib->_off_stages.insert(stage);
      any_changed = true;
    } else {
      attrib->_off_stages.insert(*fsi);
    }
  }

  if (!any_changed) {
    return this;
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::filter_to_max
//       Access: Public
//  Description: Returns a new TextureAttrib, very much like this one,
//               but with the number of on_stages reduced to be no
//               more than max_texture_stages.  The number of
//               off_stages in the new TextureAttrib is undefined.
////////////////////////////////////////////////////////////////////
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

  // Harder case: we have to compute it now.  We must choose the n
  // stages with the highest priority in our list of stages.  In the
  // case of equal priority, we prefer the stage with the lower sort.
  check_sorted();

  OnStages priority_stages = _on_stages;

  // This sort function uses the STL function object defined above.
  sort(priority_stages.begin(), priority_stages.end(), 
       CompareTextureStagePriorities());

  // Now lop off all of the stages after the first max_texture_stages.
  priority_stages.erase(priority_stages.begin() + max_texture_stages,
                        priority_stages.end());

  // And create a new attrib reflecting these stages.
  PT(TextureAttrib) attrib = new TextureAttrib;

  OnStages::const_iterator si;
  for (si = priority_stages.begin(); si != priority_stages.end(); ++si) {
    TextureStage *stage = (*si)._stage;
    attrib->_on_textures[stage] = get_on_texture(stage);
  }

  attrib->_on_stages.swap(priority_stages);
  attrib->_next_implicit_sort = _next_implicit_sort;

  CPT(RenderAttrib) new_attrib = return_new(attrib);

  // Finally, record this newly-created attrib in the map for next
  // time.
  CPT(TextureAttrib) tex_attrib = (const TextureAttrib *)new_attrib.p();
  ((TextureAttrib *)this)->_filtered[max_texture_stages] = tex_attrib;
  return tex_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TextureAttrib::
output(ostream &out) const {
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
    OffStages::const_iterator fi;
    for (fi = _off_stages.begin(); fi != _off_stages.end(); ++fi) {
      TextureStage *stage = (*fi);
      out << " " << stage->get_name();
    }

    if (!_on_stages.empty()) {
      out << " on";
    }
  }
    
  OnStages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    TextureStage *stage = (*si)._stage;
    OnTextures::const_iterator ti = _on_textures.find(stage);
    if (ti != _on_textures.end()) {
      Texture *tex = (*ti).second;
      out << " " << stage->get_name() << ":" << tex->get_name();
    } else {
      out << " " << stage->get_name();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool TextureAttrib::
has_cull_callback() const {
  OnTextures::const_iterator nti;
  for (nti = _on_textures.begin(); nti != _on_textures.end(); ++nti) {
    Texture *texture = (*nti).second;
    if (texture->has_cull_callback()) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.
//
//               This is called each time the RenderAttrib is
//               discovered applied to a Geom in the traversal.  It
//               should return true if the Geom is visible, false if
//               it should be omitted.
////////////////////////////////////////////////////////////////////
bool TextureAttrib::
cull_callback(CullTraverser *trav, const CullTraverserData &data) const {
  OnTextures::const_iterator nti;
  for (nti = _on_textures.begin(); nti != _on_textures.end(); ++nti) {
    Texture *texture = (*nti).second;
    if (!texture->cull_callback(trav, data)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TextureAttrib
//               types to return a unique number indicating whether
//               this TextureAttrib is equivalent to the other one.
//
//               This should return 0 if the two TextureAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TextureAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TextureAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TextureAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_off_all_stages != ta->_off_all_stages) {
    return (int)_off_all_stages - (int)ta->_off_all_stages;
  }

  // First, verify that the texture assigned to each stage is the
  // same.
  OnTextures::const_iterator li = _on_textures.begin();
  OnTextures::const_iterator oli = ta->_on_textures.begin();

  while (li != _on_textures.end() && oli != ta->_on_textures.end()) {
    TextureStage *stage = (*li).first;
    TextureStage *other_stage = (*oli).first;

    if (stage != other_stage) {
      return stage < other_stage ? -1 : 1;
    }

    Texture *tex = (*li).second;
    Texture *other_tex = (*oli).second;

    if (tex != other_tex) {
      return tex < other_tex ? -1 : 1;
    }

    ++li;
    ++oli;
  }

  if (li != _on_textures.end()) {
    return 1;
  }
  if (oli != ta->_on_textures.end()) {
    return -1;
  }

  // Then, we also have to check the linear list of texture stages, to
  // ensure primarily that the implicit sort numbers match between the
  // two attribs.  (If they did not, then a later call to
  // texture_stage->set_sort() might make these attribs apply textures
  // differently, even if they are the same now.)
  check_sorted();
  ta->check_sorted();

  nassertr(_on_ptr_stages.size() == _on_stages.size() &&
           ta->_on_ptr_stages.size() == ta->_on_stages.size(), 0)

  OnStages::const_iterator si = _on_ptr_stages.begin();
  OnStages::const_iterator osi = ta->_on_ptr_stages.begin();

  while (si != _on_ptr_stages.end() && osi != ta->_on_ptr_stages.end()) {
    TextureStage *stage = (*si)._stage;
    TextureStage *other_stage = (*osi)._stage;

    if (stage != other_stage) {
      return stage < other_stage ? -1 : 1;
    }

    int implicit_sort = (*si)._implicit_sort;
    int other_implicit_sort = (*osi)._implicit_sort;

    if (implicit_sort != other_implicit_sort) {
      return implicit_sort < other_implicit_sort ? -1 : 1;
    }

    ++si;
    ++osi;
  }

  if (si != _on_ptr_stages.end()) {
    return 1;
  }
  if (osi != ta->_on_ptr_stages.end()) {
    return -1;
  }

  // Finally, ensure that the set of off stages is the same.
  OffStages::const_iterator fi = _off_stages.begin();
  OffStages::const_iterator ofi = ta->_off_stages.begin();

  while (fi != _off_stages.end() && ofi != ta->_off_stages.end()) {
    TextureStage *stage = (*fi);
    TextureStage *other_stage = (*ofi);

    if (stage != other_stage) {
      return stage < other_stage ? -1 : 1;
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

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::compose_impl
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
CPT(RenderAttrib) TextureAttrib::
compose_impl(const RenderAttrib *other) const {
  const TextureAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (ta->_off_all_stages) {
    // If the other type turns off all stages, it doesn't matter what
    // we are.
    return ta;
  }

  // This is a three-way merge between ai, bi, and ci, except that bi
  // and ci should have no intersection and therefore needn't be
  // compared to each other.
  OnTextures::const_iterator ai = _on_textures.begin();
  OnTextures::const_iterator bi = ta->_on_textures.begin();
  OffStages::const_iterator ci = ta->_off_stages.begin();

  // TextureStages that are kept from the original attrib are inserted
  // into a_stages.  Those that are inherited from the secondary
  // attrib are inserted into b_stages.
  pset<TextureStage *> a_stages;
  pset<TextureStage *> b_stages;

  // Create a new TextureAttrib that will hold the result.
  TextureAttrib *attrib = new TextureAttrib;

  while (ai != _on_textures.end() && 
         bi != ta->_on_textures.end() && 
         ci != ta->_off_stages.end()) {
    if ((*ai).first < (*bi).first) {
      if ((*ai).first < (*ci)) {
        // Here is a stage that we have in the original, which is not
        // present in the secondary.
        attrib->_on_textures.insert(attrib->_on_textures.end(), *ai);
        a_stages.insert((*ai).first);
        ++ai;

      } else if ((*ci) < (*ai).first) {
        // Here is a stage that is turned off in the secondary, but
        // was not present in the original.
        ++ci;

      } else { // (*ci) == (*ai).first
        // Here is a stage that is turned off in the secondary, and
        // was present in the original.
        ++ai;
        ++ci;
      }

    } else if ((*bi).first < (*ai).first) {
      // Here is a new stage we have in the secondary, that was not
      // present in the original.
      attrib->_on_textures.insert(attrib->_on_textures.end(), *bi);
      b_stages.insert((*bi).first);
      ++bi;

    } else {  // (*bi).first == (*ai).first
      // Here is a stage we have in both.
      attrib->_on_textures.insert(attrib->_on_textures.end(), *bi);
      b_stages.insert((*bi).first);
      ++ai;
      ++bi;
    }
  }

  while (ai != _on_textures.end() && bi != ta->_on_textures.end()) {
    if ((*ai).first < (*bi).first) {
      // Here is a stage that we have in the original, which is not
      // present in the secondary.
      attrib->_on_textures.insert(attrib->_on_textures.end(), *ai);
      a_stages.insert((*ai).first);
      ++ai;

    } else if ((*bi).first < (*ai).first) {
      // Here is a new stage we have in the secondary, that was not
      // present in the original.
      attrib->_on_textures.insert(attrib->_on_textures.end(), *bi);
      b_stages.insert((*bi).first);
      ++bi;

    } else {
      // Here is a stage we have in both.
      attrib->_on_textures.insert(attrib->_on_textures.end(), *bi);
      b_stages.insert((*bi).first);
      ++ai;
      ++bi;
    }
  }

  while (ai != _on_textures.end() && ci != ta->_off_stages.end()) {
    if ((*ai).first < (*ci)) {
      // Here is a stage that we have in the original, which is not
      // present in the secondary.
      attrib->_on_textures.insert(attrib->_on_textures.end(), *ai);
      a_stages.insert((*ai).first);
      ++ai;
      
    } else if ((*ci) < (*ai).first) {
      // Here is a stage that is turned off in the secondary, but
      // was not present in the original.
      ++ci;
      
    } else { // (*ci) == (*ai).first
      // Here is a stage that is turned off in the secondary, and
      // was present in the original.
      ++ai;
      ++ci;
    }
  }

  while (ai != _on_textures.end()) {
    attrib->_on_textures.insert(attrib->_on_textures.end(), *ai);
    a_stages.insert((*ai).first);
    ++ai;
  }

  while (bi != ta->_on_textures.end()) {
    attrib->_on_textures.insert(attrib->_on_textures.end(), *bi);
    b_stages.insert((*bi).first);
    ++bi;
  }

  // Now we need to build up the linear list.  We must put this in
  // order so that the original stages are first, followed by the
  // secondary stages.  If a texture stage is listed in both, it
  // receives the secondary sort.

  OnStages::const_iterator asi;
  for (asi = _on_stages.begin(); asi != _on_stages.end(); ++asi) {
    TextureStage *stage = (*asi)._stage;

    if (a_stages.find(stage) != a_stages.end()) {
      // This stage came from the original, and thus receives the
      // original sort.
      int implicit_sort = (*asi)._implicit_sort;
      attrib->_on_stages.push_back(OnStageNode(stage, implicit_sort));
    }
  }

  OnStages::const_iterator bsi;
  for (bsi = ta->_on_stages.begin(); bsi != ta->_on_stages.end(); ++bsi) {
    TextureStage *stage = (*bsi)._stage;

    if (b_stages.find(stage) != b_stages.end()) {
      // This stage was inherited from the secondary, and thus
      // receives the secondary sort.
      int implicit_sort = _next_implicit_sort + (*bsi)._implicit_sort;
      attrib->_on_stages.push_back(OnStageNode(stage, implicit_sort));
    }
  }

  attrib->_next_implicit_sort = _next_implicit_sort + ta->_next_implicit_sort;

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TextureAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this
  // needs a bit more thought.  It's hard to imagine that it's even
  // important to compute this properly.
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TextureAttrib.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  // Write the off_stages information
  dg.add_bool(_off_all_stages);
  dg.add_uint16(get_num_off_stages());
  OffStages::const_iterator fi;
  for (fi = _off_stages.begin(); fi != _off_stages.end(); ++fi) {
    TextureStage *stage = (*fi);
    manager->write_pointer(dg, stage);
  }

  // Write the on_stages information
  dg.add_uint16(get_num_on_stages());
  OnStages::const_iterator si;
  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    TextureStage *stage = (*si)._stage;
    Texture *tex = get_on_texture(stage);
    nassertv(tex != (Texture *)NULL);

    manager->write_pointer(dg, stage);
    manager->write_pointer(dg, tex);
    dg.add_uint16((*si)._implicit_sort);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TextureAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  OffStages::iterator ci;
  for (ci = _off_stages.begin(); ci != _off_stages.end(); ++ci) {
    TextureStage *ts = DCAST(TextureStage, p_list[pi++]);
    *ci = ts;
  }

  size_t sn = 0;
  while (sn < _on_stages.size()) {
    TextureStage *ts = DCAST(TextureStage, p_list[pi++]);
    Texture *tex = DCAST(Texture, p_list[pi++]);
    
    if (tex != (Texture *)NULL) {
      _on_textures[ts] = tex;
      _on_stages[sn]._stage = ts;
      ++sn;
      
    } else {
      // If we couldn't load a texture pointer, turn off that
      // particular texture stage.
      _off_stages.push_back(ts);
      _on_stages.erase(_on_stages.begin() + sn);
    }
  }
  _sort_seq = UpdateSeq::old();
  _filtered_seq = UpdateSeq::old();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TextureAttrib is encountered
//               in the Bam file.  It should create the TextureAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TextureAttrib::
make_from_bam(const FactoryParams &params) {
  TextureAttrib *attrib = new TextureAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TextureAttrib.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // read the _off_stages data.
  _off_all_stages = scan.get_bool();
  int num_off_stages = scan.get_uint16();
  
  // Push back a NULL pointer for each off TextureStage for now, until
  // we get the actual list of pointers later in complete_pointers().
  int i;
  _off_stages.reserve(num_off_stages);
  for (i = 0; i < num_off_stages; i++) {
    manager->read_pointer(scan);
    _off_stages.push_back(NULL);
  }

  // Read the _on_stages data.
  int num_on_stages = scan.get_uint16();

  // Push back a NULL pointer for each off TextureStage and Texture
  // for now, until we get the actual list of pointers later in
  // complete_pointers().
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
    _next_implicit_sort = max(_next_implicit_sort, implicit_sort + 1);
    _on_stages.push_back(OnStageNode(NULL, implicit_sort));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::sort_on_stages
//       Access: Private
//  Description: Sorts the list of stages so that they are listed in
//               render order.  Also clears the _filtered map and
//               recalculates the list of fixed-function stages.
////////////////////////////////////////////////////////////////////
void TextureAttrib::
sort_on_stages() {
  // First, we have to build up the tc_index mapping.  We need a
  // unique number for each different texcoord name for the various
  // TextureStages.

  // It's important that this assignment not be based on the whims of
  // render order--it mustn't change arbitrarily--so we must first
  // sort the on_stages list into pointer order for this purpose.
  _on_ptr_stages = _on_stages;
  sort(_on_ptr_stages.begin(), _on_ptr_stages.end(), CompareTextureStagePointer());

  typedef pmap<const InternalName *, int> UsedTexcoordIndex;
  UsedTexcoordIndex used_texcoord_index;

  typedef pmap<const TextureStage *, int> TexcoordMap;
  TexcoordMap tc_map;

  OnStages::const_iterator si;
  for (si = _on_ptr_stages.begin(); si != _on_ptr_stages.end(); ++si) {
    TextureStage *stage = (*si)._stage;
    if (stage->is_fixed_function()) {
      const InternalName *name = stage->get_texcoord_name();

      // This pair of lines will get the next consecutive texcoord index
      // number if this is the first time we have referenced this
      // particular texcoord name; otherwise, it will return the same
      // index number it returned before.
      UsedTexcoordIndex::iterator ti = used_texcoord_index.insert(UsedTexcoordIndex::value_type(name, (int)used_texcoord_index.size())).first;
      int texcoord_index = (*ti).second;

      tc_map[stage] = texcoord_index;
    }
  }

  // Now we can sort the on_stages list into render order.
  sort(_on_stages.begin(), _on_stages.end(), CompareTextureStageSort());

  _on_ff_stages.clear();
  _ff_tc_index.clear();

  for (si = _on_stages.begin(); si != _on_stages.end(); ++si) {
    TextureStage *stage = (*si)._stage;
    if (stage->is_fixed_function()) {
      _on_ff_stages.push_back(*si);
      int texcoord_index = tc_map[stage];
      _ff_tc_index.push_back(texcoord_index);
    }
  }

  // We'd like to clear the _filtered map, in case the priority orders
  // have changed as well, but we can't do that here: too dangerous.
  // Clearing _filtered might cause TextureAttribs to be deleted, and
  // hence removed from the map that we might be in the middle of
  // traversing!

  _sort_seq = TextureStage::get_sort_seq();
}
