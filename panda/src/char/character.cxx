// Filename: character.cxx
// Created by:  drose (23Feb99)
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

#ifdef WIN32_VC
#include "char_headers.h"
#endif

#pragma hdrstop

#ifndef WIN32_VC
#include "character.h"
#include "characterJoint.h"
#include "computedVertices.h"
#include "config_char.h"

#include <geomNode.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#include <pStatTimer.h>
#endif

#include <geomNode.h>
#include <animControl.h>
#include <clockObject.h>
#include <pStatTimer.h>



TypeHandle Character::_type_handle;

#ifndef CPPPARSER
PStatCollector Character::_anim_pcollector("App:Animation");
#endif

////////////////////////////////////////////////////////////////////
//     Function: Character::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy a character.
////////////////////////////////////////////////////////////////////
Character::
Character(const Character &copy) :
  PartBundleNode(copy.get_name(), new CharacterJointBundle(copy.get_bundle()->get_name())),
  _cv(DynamicVertices::deep_copy(copy._cv)),
  _computed_vertices(copy._computed_vertices),
  _parts(copy._parts),
  _char_pcollector(copy._char_pcollector)
{
  // Now make a copy of the joint/slider hierarchy.  We could just use
  // the PartBundleNode's copy constructor, but if we do it ourselves
  // we can simultaneously update our _parts list.

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
//               copy of the character, with all of its joints copied,
//               and with a new set of dynamic vertex arrays all ready
//               to go, but it will not copy any of the original
//               Character's geometry, so the new Character won't look
//               like much.  Use copy_subgraph() to make a full copy
//               of the Character.
////////////////////////////////////////////////////////////////////
Node *Character::
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
//     Function: Character::app_traverse
//       Access: Public, Virtual
//  Description: This is called by the App traversal by virtue of the
//               character node's being present in the scene graph.
////////////////////////////////////////////////////////////////////
void Character::
app_traverse() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  get_bundle()->advance_time(now);

  if (char_cat.is_debug()) {
    char_cat.debug() << "Animating " << *this << " at time " << now << "\n";
  }

  update();
}

////////////////////////////////////////////////////////////////////
//     Function: Character::update
//       Access: Public
//  Description: Recalculates the character's joints and vertices for
//               the current frame.  Normally this is performed
//               automatically during the render and need not be
//               called explicitly.
////////////////////////////////////////////////////////////////////
void Character::
update() {
  // Statistics
  PStatTimer timer(_char_pcollector);

  // First, update all the joints and sliders.
  get_bundle()->update();

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
//     Function: Character::r_copy_subgraph
//       Access: Private, Virtual
//  Description: This is a virtual function inherited from Node.  It's
//               called when a copy_subgraph() operation reaches the
//               Character node.  In the case of a Character, it's
//               overridden to do the all right things to copy the
//               dynamic geometry to the new Character.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of Node that is
//               not also a member of PANDA.DLL *cannot* access this
//               map.
////////////////////////////////////////////////////////////////////
Node *Character::
r_copy_subgraph(TypeHandle graph_type, Node::InstanceMap &) const {
  Node *copy = make_copy();
  nassertr(copy != (Node *)NULL, NULL);
  if (copy->get_type() != get_type()) {
    graph_cat.warning()
      << "Don't know how to copy nodes of type " << get_type() << "\n";
  }

  // We assume there will be no instancing going on below the
  // Character node.  If there is, too bad; it will get flattened out.

  // Now we preempt the node's r_copy_subgraph() operation with our
  // own function that keeps track of the old vs. new arcs and also
  // updates any Geoms we find with our new dynamic vertices.

  Character *char_copy;
  DCAST_INTO_R(char_copy, copy, NULL);
  ArcMap arc_map;
  char_copy->r_copy_char(char_copy, this, graph_type, this, arc_map);
  char_copy->copy_arc_pointers(this, arc_map);

  return copy;
}


////////////////////////////////////////////////////////////////////
//     Function: Character::r_copy_char
//       Access: Private
//  Description: Recursively walks the scene graph hierarchy below the
//               Character node, duplicating it while noting the
//               orig:copy arc mappings, and also updates any
//               GeomNodes found.
////////////////////////////////////////////////////////////////////
void Character::
r_copy_char(Node *dest, const Node *source, TypeHandle graph_type,
            const Character *from, Character::ArcMap &arc_map) {
  if (source->is_of_type(GeomNode::get_class_type())) {
    const GeomNode *source_gnode;
    GeomNode *dest_gnode;
    DCAST_INTO_V(source_gnode, source);
    DCAST_INTO_V(dest_gnode, dest);

    dest_gnode->clear();
    int num_geoms = source_gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      dDrawable *d = source_gnode->get_geom(i);
      if (d->is_of_type(Geom::get_class_type())) {
        dest_gnode->add_geom(copy_geom(DCAST(Geom, d), from));
      } else {
        dest_gnode->add_geom(d);
      }
    }
  }

  int num_children = source->get_num_children(graph_type);
  for (int i = 0; i < num_children; i++) {
    NodeRelation *source_arc = source->get_child(graph_type, i);
    const Node *source_child = source_arc->get_child();
    nassertv(source_child != (Node *)NULL);

    Node *dest_child;
    if (source_child->is_of_type(Character::get_class_type())) {
      // We make a special case for nodes of type Character.  If we
      // encounter one of these, we have a character under a
      // character, and the nested character's copy should be called
      // instead of ours.
      dest_child = source_child->copy_subgraph(graph_type);

    } else {
      // Otherwise, we assume that make_copy() will make a suitable
      // copy of the node.  This does limit the sorts of things we can
      // have parented to a Character and expect copy_subgraph() to
      // work correctly.  Too bad.
      dest_child = source_child->make_copy();
      r_copy_char(dest_child, source_child, graph_type, from, arc_map);
    }

    NodeRelation *dest_arc =
      NodeRelation::create_typed_arc(graph_type, dest, dest_child);
    nassertv(dest_arc != (NodeRelation *)NULL);
    nassertv(dest_arc->is_exact_type(graph_type));

    dest_arc->copy_transitions_from(source_arc);
    arc_map[source_arc] = dest_arc;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_geom
//       Access: Private
//  Description: Makes a new copy of the Geom with the dynamic vertex
//               arrays replaced to reference this character instead
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

  source->get_coords(coords, bind, index);
  if (bind != G_OFF && coords == from->_cv._coords) {
    if (dest == source) {
      dest = source->make_copy();
    }
    dest->set_coords(_cv._coords, bind, index);
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
//     Function: Character::copy_arc_pointers
//       Access: Public
//  Description: Creates _net_transform_arcs and _local_transform_arcs
//               as appropriate in each of the character's joints, as
//               copied from the other character.
////////////////////////////////////////////////////////////////////
void Character::
copy_arc_pointers(const Character *from, const Character::ArcMap &arc_map) {
  nassertv(_parts.size() == from->_parts.size());
  for (int i = 0; i < (int)_parts.size(); i++) {
    if (_parts[i]->is_of_type(CharacterJoint::get_class_type())) {
      nassertv(_parts[i] != from->_parts[i]);
      CharacterJoint *source_joint;
      CharacterJoint *dest_joint;
      DCAST_INTO_V(source_joint, from->_parts[i]);
      DCAST_INTO_V(dest_joint, _parts[i]);

      CharacterJoint::ArcList::const_iterator ai;
      for (ai = source_joint->_net_transform_arcs.begin();
           ai != source_joint->_net_transform_arcs.end();
           ++ai) {
        NodeRelation *source_arc = (*ai);

        ArcMap::const_iterator mi;
        mi = arc_map.find(source_arc);
        if (mi != arc_map.end()) {
          NodeRelation *dest_arc = (*mi).second;

          // Here's an internal joint that the source Character was
          // animating directly.  We'll animate our corresponding
          // joint the same way.
          dest_joint->add_net_transform(dest_arc);
        }
      }

      for (ai = source_joint->_local_transform_arcs.begin();
           ai != source_joint->_local_transform_arcs.end();
           ++ai) {
        NodeRelation *source_arc = (*ai);

        ArcMap::const_iterator mi;
        mi = arc_map.find(source_arc);
        if (mi != arc_map.end()) {
          NodeRelation *dest_arc = (*mi).second;

          // Here's an internal joint that the source Character was
          // animating directly.  We'll animate our corresponding
          // joint the same way.
          dest_joint->add_local_transform(dest_arc);
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Character::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Character::
write_datagram(BamWriter *manager, Datagram &me)
{
  PartBundleNode::write_datagram(manager, me);
  _cv.write_datagram(manager, me);
  manager->write_pointer(me, _computed_vertices);

  me.add_uint16(_parts.size());
  Parts::const_iterator pi;
  for (pi = _parts.begin(); pi != _parts.end(); pi++) {
    manager->write_pointer(me, (*pi));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Character::
fillin(DatagramIterator& scan, BamReader* manager)
{
  PartBundleNode::fillin(scan, manager);
  _cv.fillin(scan, manager);
  manager->read_pointer(scan, this);

  // Read the number of parts to expect in the _parts list, and then
  // fill the array up with NULLs.  We'll fill in the actual values in
  // complete_pointers, later.
  int num_parts = scan.get_uint16();
  _parts.clear();
  _parts.reserve(num_parts);
  for (int i = 0; i < num_parts; i++) {
    manager->read_pointer(scan, this);
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

////////////////////////////////////////////////////////////////////
//     Function: Character::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointers to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int Character::
complete_pointers(vector_typedWritable &plist, BamReader* manager)
{
  int start = PartBundleNode::complete_pointers(plist, manager);
  _computed_vertices = DCAST(ComputedVertices, plist[start]);
  start++;

  int num_parts = _parts.size();
  for (int i = 0; i < num_parts; i++) {
    _parts[i] = DCAST(PartGroup, plist[start + i]);
  }
  start += num_parts;

  return start;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::make_Character
//       Access: Protected
//  Description: Factory method to generate a Character object
////////////////////////////////////////////////////////////////////
TypedWritable* Character::
make_Character(const FactoryParams &params)
{
  Character *me = new Character;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a Character object
////////////////////////////////////////////////////////////////////
void Character::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_Character);
}




