// Filename: character.cxx
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "character.h"
#include "characterJoint.h"
#include "computedVertices.h"
#include "config_char.h"

#include "geomNode.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pStatTimer.h"
#include "animControl.h"
#include "clockObject.h"
#include "pStatTimer.h"

TypeHandle Character::_type_handle;

#ifndef CPPPARSER
PStatCollector Character::_anim_pcollector("App:Animation");
#endif

////////////////////////////////////////////////////////////////////
//     Function: Character::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy a Character.
////////////////////////////////////////////////////////////////////
Character::
Character(const Character &copy) :
  PartBundleNode(copy, new CharacterJointBundle(copy.get_bundle()->get_name())),
  _cv(DynamicVertices::deep_copy(copy._cv)),
  _computed_vertices(copy._computed_vertices),
  _parts(copy._parts),
  _char_pcollector(copy._char_pcollector)
{
  // Now make a copy of the joint/slider hierarchy.  We could just use
  // the copy_subgraph feature of the PartBundleNode's copy
  // constructor, but if we do it ourselves we can simultaneously
  // update our _parts list.

  copy_joints(get_bundle(), copy.get_bundle());
}

////////////////////////////////////////////////////////////////////
//     Function: Character::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Character::
Character(const string &name) :
  PartBundleNode(name, new CharacterJointBundle(name)),
  _char_pcollector(_anim_pcollector, name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Character::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Character::
~Character() {
}

////////////////////////////////////////////////////////////////////
//     Function: Character::make_copy
//       Access: Public, Virtual
//  Description: The Character make_copy() function will make a new
//               copy of the Character, with all of its joints copied,
//               and with a new set of dynamic vertex arrays all ready
//               to go, but it will not copy any of the original
//               Character's geometry, so the new Character won't look
//               like much.  Use copy_subgraph() to make a full copy
//               of the Character.
////////////////////////////////////////////////////////////////////
PandaNode *Character::
make_copy() const {
  return new Character(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool Character::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool Character::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool Character::
cull_callback(CullTraverser *, CullTraverserData &) {
  // For now, we update the character during the cull traversal; this
  // prevents us from needlessly updating characters that aren't in
  // the view frustum.  We may need a better way to do this
  // optimization later, to handle characters that might animate
  // themselves in front of the view frustum.
  update_to_now();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::update_to_now
//       Access: Published
//  Description: Advances the character's frame to the current time,
//               and then calls update().  This can be used by show
//               code to force an update of the character's position
//               to the current frame, regardless of whether the
//               character is currently onscreen and animating.
////////////////////////////////////////////////////////////////////
void Character::
update_to_now() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  get_bundle()->advance_time(now);

  if (char_cat.is_spam()) {
    char_cat.spam() << "Animating " << *this << " at time " << now << "\n";
  }

  update();
}

////////////////////////////////////////////////////////////////////
//     Function: Character::update
//       Access: Published
//  Description: Recalculates the Character's joints and vertices for
//               the current frame.  Normally this is performed
//               automatically during the render and need not be
//               called explicitly.
////////////////////////////////////////////////////////////////////
void Character::
update() {
  // Statistics
  PStatTimer timer(_char_pcollector);

  // First, update all the joints and sliders.
  bool any_changed = get_bundle()->update();

  // Now update the vertices, if we need to.  This is likely to be a
  // slow operation.
  if (any_changed || even_animation) {
    if (_computed_vertices != (ComputedVertices *)NULL) {
      _computed_vertices->update(this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::force_update
//       Access: Published
//  Description: Recalculates the character even if we think it
//               doesn't need it.
////////////////////////////////////////////////////////////////////
void Character::
force_update() {
  // Statistics
  PStatTimer timer(_char_pcollector);

  // First, update all the joints and sliders.
  get_bundle()->force_update();

  // Now update the vertices.
  if (_computed_vertices != (ComputedVertices *)NULL) {
    _computed_vertices->update(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_joints
//       Access: Private
//  Description: Recursively walks the joint/slider hierarchy and
//               creates a new copy of the hierarchy.
////////////////////////////////////////////////////////////////////
void Character::
copy_joints(PartGroup *copy, PartGroup *orig) {
  if (copy->get_type() != orig->get_type()) {
    char_cat.warning()
      << "Don't know how to copy " << orig->get_type() << "\n";
  }

  PartGroup::Children::const_iterator ci;
  for (ci = orig->_children.begin(); ci != orig->_children.end(); ++ci) {
    PartGroup *orig_child = (*ci);
    PartGroup *copy_child = orig_child->make_copy();
    copy->_children.push_back(copy_child);
    copy_joints(copy_child, orig_child);
  }

  Parts::iterator pi = find(_parts.begin(), _parts.end(), orig);
  if (pi != _parts.end()) {
    (*pi) = copy;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::r_copy_children
//       Access: Protected, Virtual
//  Description: This is called by r_copy_subgraph(); the copy has
//               already been made of this particular node (and this
//               is the copy); this function's job is to copy all of
//               the children from the original.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of PandaNode
//               that is not also a member of PANDA.DLL *cannot*
//               access this map, and probably should not even
//               override this function.
////////////////////////////////////////////////////////////////////
void Character::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map) {
  // We assume there will be no instancing going on below the
  // Character node.  If there is, too bad; it will get flattened out.

  // We preempt the node's r_copy_children() operation with our own
  // function that keeps track of the old vs. new nodes and also
  // updates any Geoms we find with our new dynamic vertices.

  const Character *from_char;
  DCAST_INTO_V(from_char, from);
  NodeMap node_map;
  r_copy_char(this, from_char, from_char, node_map);
  copy_node_pointers(from_char, node_map);
}


////////////////////////////////////////////////////////////////////
//     Function: Character::r_copy_char
//       Access: Private
//  Description: Recursively walks the scene graph hiernodehy below the
//               Character node, duplicating it while noting the
//               orig:copy node mappings, and also updates any
//               GeomNodes found.
////////////////////////////////////////////////////////////////////
void Character::
r_copy_char(PandaNode *dest, const PandaNode *source,
            const Character *from, Character::NodeMap &node_map) {
  if (source->is_geom_node()) {
    const GeomNode *source_gnode;
    GeomNode *dest_gnode;
    DCAST_INTO_V(source_gnode, source);
    DCAST_INTO_V(dest_gnode, dest);

    dest_gnode->remove_all_geoms();
    int num_geoms = source_gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      Geom *geom = source_gnode->get_geom(i);
      const RenderState *state = source_gnode->get_geom_state(i);
      dest_gnode->add_geom(copy_geom(geom, from), state);
    }
  }

  int num_children = source->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PandaNode *source_child = source->get_child(i);
    int source_sort = source->get_child_sort(i);

    PT(PandaNode) dest_child;
    if (source_child->is_of_type(Character::get_class_type())) {
      // We make a special case for nodes of type Character.  If we
      // encounter one of these, we have a Character under a
      // Character, and the nested Character's copy should be called
      // instead of ours.
      dest_child = source_child->copy_subgraph();

    } else {
      // Otherwise, we assume that make_copy() will make a suitable
      // copy of the node.  This does limit the sorts of things we can
      // have parented to a Character and expect copy_subgraph() to
      // work correctly.  Too bad.
      dest_child = source_child->make_copy();
      r_copy_char(dest_child, source_child, from, node_map);
    }
    dest->add_child(dest_child, source_sort);
    node_map[source_child] = dest_child;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_geom
//       Access: Private
//  Description: Makes a new copy of the Geom with the dynamic vertex
//               arrays replaced to reference this Character instead
//               of the other one.  If no arrays have changed, simply
//               returns the same Geom.
////////////////////////////////////////////////////////////////////
PT(Geom) Character::
copy_geom(Geom *source, const Character *from) {
  GeomBindType bind;
  PTA_ushort index;

  PTA_Vertexf coords;
  PTA_Normalf norms;
  PTA_Colorf colors;
  PTA_TexCoordf texcoords;

  PT(Geom) dest = source;

  source->get_coords(coords, index);
  if ((coords != (void *)NULL) && (coords == (from->_cv._coords))) {
    if (dest == source) {
      dest = source->make_copy();
    }
    dest->set_coords(_cv._coords, index);
  }

  source->get_normals(norms, bind, index);
  if (bind != G_OFF && norms == from->_cv._norms) {
    if (dest == source) {
      dest = source->make_copy();
    }
    dest->set_normals(_cv._norms, bind, index);
  }

  source->get_colors(colors, bind, index);
  if (bind != G_OFF && colors == from->_cv._colors) {
    if (dest == source) {
      dest = source->make_copy();
    }
    dest->set_colors(_cv._colors, bind, index);
  }

  source->get_texcoords(texcoords, bind, index);
  if (bind != G_OFF && texcoords == from->_cv._texcoords) {
    if (dest == source) {
      dest = source->make_copy();
    }
    dest->set_texcoords(_cv._texcoords, bind, index);
  }

  return dest;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_node_pointers
//       Access: Public
//  Description: Creates _net_transform_nodes and _local_transform_nodes
//               as appropriate in each of the Character's joints, as
//               copied from the other Character.
////////////////////////////////////////////////////////////////////
void Character::
copy_node_pointers(const Character *from, const Character::NodeMap &node_map) {
  nassertv(_parts.size() == from->_parts.size());
  for (int i = 0; i < (int)_parts.size(); i++) {
    if (_parts[i]->is_of_type(CharacterJoint::get_class_type())) {
      nassertv(_parts[i] != from->_parts[i]);
      CharacterJoint *source_joint;
      CharacterJoint *dest_joint;
      DCAST_INTO_V(source_joint, from->_parts[i]);
      DCAST_INTO_V(dest_joint, _parts[i]);

      CharacterJoint::NodeList::const_iterator ai;
      for (ai = source_joint->_net_transform_nodes.begin();
           ai != source_joint->_net_transform_nodes.end();
           ++ai) {
        PandaNode *source_node = (*ai);

        NodeMap::const_iterator mi;
        mi = node_map.find(source_node);
        if (mi != node_map.end()) {
          PandaNode *dest_node = (*mi).second;

          // Here's an internal joint that the source Character was
          // animating directly.  We'll animate our corresponding
          // joint the same way.
          dest_joint->add_net_transform(dest_node);
        }
      }

      for (ai = source_joint->_local_transform_nodes.begin();
           ai != source_joint->_local_transform_nodes.end();
           ++ai) {
        PandaNode *source_node = (*ai);

        NodeMap::const_iterator mi;
        mi = node_map.find(source_node);
        if (mi != node_map.end()) {
          PandaNode *dest_node = (*mi).second;

          // Here's an internal joint that the source Character was
          // animating directly.  We'll animate our corresponding
          // joint the same way.
          dest_joint->add_local_transform(dest_node);
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Character::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Character.
////////////////////////////////////////////////////////////////////
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Character::
write_datagram(BamWriter *manager, Datagram &dg) {
  PartBundleNode::write_datagram(manager, dg);
  _cv.write_datagram(manager, dg);
  manager->write_pointer(dg, _computed_vertices);

  dg.add_uint16(_parts.size());
  Parts::const_iterator pi;
  for (pi = _parts.begin(); pi != _parts.end(); pi++) {
    manager->write_pointer(dg, (*pi));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int Character::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PartBundleNode::complete_pointers(p_list, manager);
  _computed_vertices = DCAST(ComputedVertices, p_list[pi++]);

  int num_parts = _parts.size();
  for (int i = 0; i < num_parts; i++) {
    _parts[i] = DCAST(PartGroup, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Character is encountered
//               in the Bam file.  It should create the Character
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *Character::
make_from_bam(const FactoryParams &params) {
  Character *node = new Character("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Character.
////////////////////////////////////////////////////////////////////
void Character::
fillin(DatagramIterator &scan, BamReader *manager) {
  PartBundleNode::fillin(scan, manager);
  _cv.fillin(scan, manager);
  manager->read_pointer(scan);

  // Read the number of parts to expect in the _parts list, and then
  // fill the array up with NULLs.  We'll fill in the actual values in
  // complete_pointers, later.
  int num_parts = scan.get_uint16();
  _parts.clear();
  _parts.reserve(num_parts);
  for (int i = 0; i < num_parts; i++) {
    manager->read_pointer(scan);
    _parts.push_back((PartGroup *)NULL);
  }

#ifdef DO_PSTATS
  // Reinitialize our collector with our name, now that we know it.
  if (has_name()) {
    _char_pcollector =
      PStatCollector(_anim_pcollector, get_name());
  }
#endif
}
