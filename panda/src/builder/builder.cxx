// Filename: builder.cxx
// Created by:  drose (09Sep97)
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

#include "builderFuncs.h"
#include "builderMisc.h"
#include "notify.h"
#include "pmap.h"
#include "builder.h"
#include "pandaNode.h"
#include "geomNode.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: Builder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Builder::
Builder() {
  _bi = _buckets.end();
}

////////////////////////////////////////////////////////////////////
//     Function: Builder::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Builder::
~Builder() {
  // Free all the buckets we allocated.  We allocated 'em, we free
  // 'em.
  Buckets::iterator bi;
  for (bi = _buckets.begin();
       bi != _buckets.end();
       ++bi) {
    BuilderBucket *bucket = (*bi).get_bucket();
    delete bucket;
  }
}


// We use the NodeMap class to build up a map of Nodes to GeomNodes.
// There may be several buckets that point to the same Node; these
// should all be given the same GeomNode, when possible.

// However, if two buckets have different sets of scene graph
// properties--in particular, the _hidden member is different--they
// must be given separate GeomNodes.

// Furthermore, it's possible to name each bucket.  If two buckets
// with the same Node pointer have different names, then they should
// be given two different GeomNodes.

class NodeMap : public Namable {
public:
  NodeMap(PandaNode *node, const BuilderBucket *bucket)
    : _node(node), _bucket(bucket) { }

  bool operator < (const NodeMap &other) const {
    if (_node != other._node) {
      return _node < other._node;
    }
    if (_bucket->_hidden != other._bucket->_hidden) {
      return _bucket->_hidden < other._bucket->_hidden;
    }
    if (_bucket->get_name() != other._bucket->get_name()) {
      return _bucket->get_name() < other._bucket->get_name();
    }
    return 0;
  }

  PandaNode *_node;

  // Although a bucket pointer is stored here in the NodeMap class,
  // you should not use it except to extract the name and/or the
  // _trans member.  Remember, this bucket pointer stands for any of
  // possibly several bucket pointers, all different, except that they
  // share the same name.
  const BuilderBucket *_bucket;
};



////////////////////////////////////////////////////////////////////
//     Function: Builder::build
//       Access: Public
//  Description: Creates Geoms for all the primitives added to all
//               buckets, and adds them where appropriate to place
//               them in the scene graph under their respective
//               parents, and/or returns a single GeomNode that
//               contains all geometry whose bucket did not reference
//               a particular scene graph node to parent them to.
//
//               If a bucket's _node pointer was a GeomNode, the
//               geometry will be added directly to that node.  If the
//               _node pointer was any other kind of node, a GeomNode
//               will be created and parented to that node, and its
//               name will be the name of the bucket.  In this case,
//               the name of the bucket can also be used to
//               differentiate nodes: if two buckets reference the
//               same node, but have different names, then two
//               different GeomNodes are created, one with each name.
////////////////////////////////////////////////////////////////////
GeomNode *Builder::
build(const string &default_name) {
  typedef pmap<NodeMap, GeomNode *> GeomNodeMap;
  GeomNodeMap geom_nodes;

  // First, build all the Geoms and create GeomNodes for them.  Each
  // unique Node gets its own GeomNode.  If the Node is itself a
  // GeomNode, that GeomNode is used directly.
  Buckets::iterator i;
  for (i = _buckets.begin();
       i != _buckets.end();
       ++i) {
    BuilderBucket *bucket = (*i).get_bucket();
    PandaNode *node = bucket->_node;
    GeomNode *geom_node = NULL;

    if (node != (PandaNode *)NULL && 
        node->is_of_type(GeomNode::get_class_type())) {
      // The node is a GeomNode.  In this case, we simply use that
      // node.  We can't separate them out by name in this case; we'll
      // just assign to it the first nonempty name we encounter.
      geom_node = DCAST(GeomNode, node);

      // Since the caller already created this GeomNode and passed it
      // in, we'll leave it up to the caller to name the node and set
      // up the state transitions leading into it.

    } else {
      // The node is not a GeomNode, so look it up in the map.
      GeomNodeMap::iterator f = geom_nodes.find(NodeMap(node, bucket));
      if (f != geom_nodes.end()) {
        geom_node = (*f).second;

      } else {
        // No such node/name combination.  Create a new one.
        geom_node = bucket->make_geom_node();
        if (geom_node != NULL) {
          geom_nodes[NodeMap(node, bucket)] = geom_node;
        }
      }
    }

    if (geom_node != NULL) {
      (*i).build(geom_node);
    }
  }

  // Now go through and parent the geom_nodes under their respective
  // group nodes.  Save out the geom_node associated with a NULL Node;
  // this one is returned from this function.

  GeomNode *base_geom_node = NULL;

  GeomNodeMap::iterator gi;

  for (gi = geom_nodes.begin();
       gi != geom_nodes.end();
       ++gi) {
    const NodeMap &nm = (*gi).first;
    GeomNode *geom_node = (*gi).second;

    PandaNode *node = nm._node;
    const string &name = nm._bucket->get_name();

    // Assign the name to the geom, if it doesn't have one already.
    if (!geom_node->has_name()) {
      if (!name.empty()) {
        geom_node->set_name(name);

      } else if (!default_name.empty()) {
        geom_node->set_name(default_name);
      }
    }

    // Only reparent the geom_node if it has no parent already.
    int num_parents = geom_node->get_num_parents();
    if (num_parents == 0) {
      if (geom_node->get_num_geoms() == 0) {
        // If there was nothing added, never mind.
        delete geom_node;

      } else if (node==NULL) {
        nassertr(base_geom_node == NULL, NULL);
        if (!nm._bucket->_hidden) {
          base_geom_node = geom_node;
        }

      } else {
        if (nm._bucket->_hidden) {
          // If the contents of the bucket are supposed to be hidden,
          // add the GeomNode as a stashed child.
          node->add_stashed(geom_node);
        } else {
          // Otherwise, in the normal case, the GeomNode is a normal,
          // visible child of its parent.
          node->add_child(geom_node);
        }
      }
    }
  }

  return base_geom_node;
}


////////////////////////////////////////////////////////////////////
//     Function: Builder::add_bucket
//       Access: Protected
//  Description: Adds a new BuilderBucket just like the given one to
//               the set of all used BuilderBuckets, and makes it the
//               current bucket.  Future primitives will be added to
//               this bucket.
////////////////////////////////////////////////////////////////////
void Builder::
add_bucket(const BuilderBucket &bucket) {
  // Optimization: maybe it's the same bucket we used last time.
  if (_bi != _buckets.end() &&
      (*_bi) == BuilderBucketNode((BuilderBucket *)&bucket)) {
    return;
  }

  // Nope.  Look again.
  _bi = _buckets.find((BuilderBucket *)&bucket);
  if (_bi == _buckets.end()) {
    BuilderBucket *new_bucket = bucket.make_copy();
    _bi = _buckets.insert(new_bucket).first;
  }
}

