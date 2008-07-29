// Filename: mayaNodeDesc.cxx
// Created by:  drose (06Jun03)
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

#include "mayaNodeDesc.h"
#include "mayaNodeTree.h"
#include "mayaBlendDesc.h"
#include "mayaToEggConverter.h"
#include "maya_funcs.h"
#include "eggGroup.h"
#include "config_mayaegg.h"

#include "pre_maya_include.h"
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnMesh.h>
#include "post_maya_include.h"

TypeHandle MayaNodeDesc::_type_handle;

// This is a list of the names of Maya connections that count as a
// transform.
static const char *transform_connections[] = {
  "translate",
  "translateX",
  "translateY",
  "translateZ",
  "rotate",
  "rotateX",
  "rotateY",
  "rotateZ",
};
static const int num_transform_connections = sizeof(transform_connections) / sizeof(const char *);

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaNodeDesc::
MayaNodeDesc(MayaNodeTree *tree, MayaNodeDesc *parent, const string &name) :
  Namable(name),
  _tree(tree),
  _parent(parent)
{
  _dag_path = (MDagPath *)NULL;
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;
  _joint_type = JT_none;
  _is_lod = false;
  _tagged = false;
  _joint_tagged = false;

  // Add ourselves to our parent.
  if (_parent != (MayaNodeDesc *)NULL) {
    _parent->_children.push_back(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaNodeDesc::
~MayaNodeDesc() {
  if (_dag_path != (MDagPath *)NULL) {
    delete _dag_path;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::from_dag_path
//       Access: Public
//  Description: Indicates an association between the MayaNodeDesc and
//               some Maya instance.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
from_dag_path(const MDagPath &dag_path, MayaToEggConverter *converter) {
  MStatus status;

  if (_dag_path == (MDagPath *)NULL) {
    _dag_path = new MDagPath(dag_path);

    string name;
    MFnDagNode dag_node(dag_path, &status);
    if (!status) {
      status.perror("MFnDagNode constructor");
    } else {
      name = dag_node.name().asChar();
    }

    if (_dag_path->hasFn(MFn::kJoint) || converter->force_joint(name)) {
      // This node is a joint, or the user specifically asked to treat
      // it like a joint.
      _joint_type = JT_joint;
      if (_parent != (MayaNodeDesc *)NULL) {
        _parent->mark_joint_parent();
      }

    } else {
      // The node is not a joint, but maybe its transform is
      // controlled by connected inputs.  If so, we should treat it
      // like a joint.
      bool transform_connected = false;

      MStatus status;
      MObject node = dag_path.node(&status);
      if (status) {
        for (int i = 0; 
             i < num_transform_connections && !transform_connected;
             i++) {
          if (is_connected(node, transform_connections[i])) {
            transform_connected = true;
          }
        }
      }
      
      if (transform_connected) {
        _joint_type = JT_joint;
        if (_parent != (MayaNodeDesc *)NULL) {
          _parent->mark_joint_parent();
        }
      }
    }

    if (dag_path.hasFn(MFn::kNurbsSurface)) {
      MFnNurbsSurface surface(dag_path, &status);
      if (status) {
        check_blend_shapes(surface, "create");
      }
    } else if (dag_path.hasFn(MFn::kMesh)) {
      MFnMesh mesh(dag_path, &status);
      if (status) {
        check_blend_shapes(mesh, "inMesh");
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::has_dag_path
//       Access: Public
//  Description: Returns true if a Maya dag path has been associated
//               with this node, false otherwise.
////////////////////////////////////////////////////////////////////
bool MayaNodeDesc::
has_dag_path() const {
  return (_dag_path != (MDagPath *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::get_dag_path
//       Access: Public
//  Description: Returns the dag path associated with this node.  It
//               is an error to call this unless has_dag_path()
//               returned true.
////////////////////////////////////////////////////////////////////
const MDagPath &MayaNodeDesc::
get_dag_path() const {
  nassertr(_dag_path != (MDagPath *)NULL, *_dag_path);
  return *_dag_path;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::get_num_blend_descs
//       Access: Public
//  Description: Returns the number of unique MayaBlendDesc objects
//               (and hence the number of morph sliders) that affect
//               the geometry in this node.
////////////////////////////////////////////////////////////////////
int MayaNodeDesc::
get_num_blend_descs() const {
  return _blend_descs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::get_blend_desc
//       Access: Public
//  Description: Returns the nth MayaBlendDesc object that affects the
//               geometry in this node.
////////////////////////////////////////////////////////////////////
MayaBlendDesc *MayaNodeDesc::
get_blend_desc(int n) const {
  nassertr(n >= 0 && n < (int)_blend_descs.size(), NULL);
  return _blend_descs[n];
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::is_joint
//       Access: Public
//  Description: Returns true if the node should be treated as a joint
//               by the converter.
////////////////////////////////////////////////////////////////////
bool MayaNodeDesc::
is_joint() const {
  //return _joint_type == JT_joint || _joint_type == JT_pseudo_joint;
  return _joint_tagged && (_joint_type == JT_joint || _joint_type == JT_pseudo_joint);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::is_joint_parent
//       Access: Public
//  Description: Returns true if the node is the parent or ancestor of
//               a joint.
////////////////////////////////////////////////////////////////////
bool MayaNodeDesc::
is_joint_parent() const {
  return _joint_type == JT_joint_parent;
  //return _joint_tagged && (_joint_type == JT_joint_parent);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::is_joint_tagged
//       Access: Public
//  Description: Returns true if the node has been joint_tagged to be
//               converted, false otherwise.
////////////////////////////////////////////////////////////////////
bool MayaNodeDesc::
is_joint_tagged() const {
  return _joint_tagged;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::tag_joint
//       Access: Private
//  Description: Tags this node for conversion, but does not tag child
//               nodes.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
tag_joint() {
  _joint_tagged = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::tag_joint_recursively
//       Access: Private
//  Description: Tags this node and all descendant nodes for
//               conversion.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
tag_joint_recursively() {
  _joint_tagged = true;
  //mayaegg_cat.info() << "tjr: " << get_name() << endl;
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    MayaNodeDesc *child = (*ci);
    child->tag_joint_recursively();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::is_tagged
//       Access: Public
//  Description: Returns true if the node has been tagged to be
//               converted, false otherwise.
////////////////////////////////////////////////////////////////////
bool MayaNodeDesc::
is_tagged() const {
  return _tagged;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::tag
//       Access: Private
//  Description: Tags this node for conversion, but does not tag child
//               nodes.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
tag() {
  _tagged = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::untag
//       Access: Private
//  Description: Un-tags this node for conversion, but does not tag child
//               nodes.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
untag() {
  _tagged = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::tag_recursively
//       Access: Private
//  Description: Tags this node and all descendant nodes for
//               conversion.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
tag_recursively() {
  _tagged = true;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    MayaNodeDesc *child = (*ci);
    child->tag_recursively();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::untag_recursively
//       Access: Private
//  Description: Un-tags this node and all descendant nodes for
//               conversion.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
untag_recursively() {
  _tagged = false;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    MayaNodeDesc *child = (*ci);
    child->untag_recursively();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::has_object_type
//       Access: Public
//  Description: Returns true if this node or any of its parent
//               has_object_type of object_type.
////////////////////////////////////////////////////////////////////
bool MayaNodeDesc::
has_object_type(string object_type) const {
  bool ret = false;
  if ((_egg_group != (EggGroup*) NULL) 
      && _egg_group->has_object_type(object_type)) {
    return true;
  }
  if (_parent != (MayaNodeDesc *)NULL) {
    ret |= _parent->has_object_type(object_type);
  }
  return ret;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::clear_egg
//       Access: Private
//  Description: Recursively clears the egg pointers from this node
//               and all children.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
clear_egg() {
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    MayaNodeDesc *child = (*ci);
    child->clear_egg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::mark_joint_parent
//       Access: Private
//  Description: Indicates that this node has at least one child that
//               is a joint or a pseudo-joint.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
mark_joint_parent() {
  if (_joint_type == JT_none) {
    _joint_type = JT_joint_parent;
    if (_parent != (MayaNodeDesc *)NULL) {
      _parent->mark_joint_parent();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::check_pseudo_joints
//       Access: Private
//  Description: Walks the hierarchy, looking for non-joint nodes that
//               are both children and parents of a joint.  These
//               nodes are deemed to be pseudo joints, since the
//               converter must treat them as joints.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
check_pseudo_joints(bool joint_above) {
  static PN_uint32 space_count = 0;
  string space;
  for (PN_uint32 idx=0; idx<space_count; ++idx) {
    space.append(" ");
  }
  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam() << "cpj:" << space << get_name() << " joint_type: " << _joint_type << endl;
  }
  if (_joint_type == JT_joint_parent && joint_above) {
    // This is one such node: it is the parent of a joint
    // (JT_joint_parent is set), and it is the child of a joint
    // (joint_above is set).
    _joint_type = JT_pseudo_joint;
  }

  if (_joint_type == JT_joint) {
    // If this node is itself a joint, then joint_above is true for
    // all child nodes.
    joint_above = true;
  }

  // Don't bother traversing further if _joint_type is none, since
  // that means this node has no joint children.
  if (_joint_type != JT_none) {

    bool any_joints = false;
    Children::const_iterator ci;
    for (ci = _children.begin(); ci != _children.end(); ++ci) {
      MayaNodeDesc *child = (*ci);
      if (mayaegg_cat.is_spam()) {
        ++space_count;
      }
      child->check_pseudo_joints(joint_above);
      //if (child->is_joint()) {
      if (child->_joint_type == JT_joint || child->_joint_type == JT_pseudo_joint) {
        any_joints = true;
      }
    }

    // If any children qualify as joints, then any sibling nodes that
    // are parents of joints are also elevated to joints.
    if (any_joints) {
      bool all_joints = true;
      for (ci = _children.begin(); ci != _children.end(); ++ci) {
        MayaNodeDesc *child = (*ci);
        MStatus status;
        MFnDagNode dag_node(child->get_dag_path(), &status);
        if (!status) {
          status.perror("MFnDagNode constructor");
        }
        string type_name = dag_node.typeName().asChar();
        if (child->_joint_type == JT_joint_parent) {
          child->_joint_type = JT_pseudo_joint;
        } else if (child->_joint_type == JT_none) {
          if (mayaegg_cat.is_spam()) {
            mayaegg_cat.spam() << "cpj: " << space << "jt_none for " << child->get_name() << endl;
          }
          if (type_name.find("transform") == string::npos) {
            if (mayaegg_cat.is_spam()) {
              mayaegg_cat.spam() << "cpj: " << space << "all_joints false for " << get_name() << endl;
            }
            all_joints = false;
          }
        }
      }

      if (all_joints) {
        // Finally, if all children are joints, then we are too.
        if (_joint_type == JT_joint_parent) {
          if (!get_name().empty()) { // make sure parent of root is not a joint
            _joint_type = JT_pseudo_joint;
          }
        }
      }
    }
  }
  if (mayaegg_cat.is_spam()) {
    if (space_count > 0)
      --space_count;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::check_blend_shapes
//       Access: Private
//  Description: Looks for blend shapes on a NURBS surface or polygon
//               mesh and records any blend shapes found.  This is
//               similar to MayaToEggConverter::get_vertex_weights(),
//               which checks for membership of vertices to joints;
//               Maya stores the blend shape table in the same place.
//               See the comments in get_vertex_weights() for a more
//               in-depth description of the iteration process here.
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
check_blend_shapes(const MFnDagNode &node, const string &attrib_name) {
  MStatus status;

  MObject attr = node.attribute(attrib_name.c_str()); 
  
  MPlug history(node.object(), attr); 
  MItDependencyGraph it(history, MFn::kDependencyNode, 
                        MItDependencyGraph::kUpstream, 
                        MItDependencyGraph::kDepthFirst, 
                        MItDependencyGraph::kNodeLevel);

  while (!it.isDone()) {
    MObject c_node = it.thisNode(); 

    if (c_node.hasFn(MFn::kBlendShape)) {
      MFnBlendShapeDeformer blends(c_node, &status);
      if (!status) {
        status.perror("MFnBlendShapeDeformer constructor");

      } else {
        // Check if the slider is a "parallel blender", which is a
        // construct created by Maya for Maya's internal purposes
        // only.  We don't want to fiddle with the parallel blenders.
        MPlug plug = blends.findPlug("pb");
        bool is_parallel_blender;
        status = plug.getValue(is_parallel_blender);
        if (!status) {
          status.perror("Could not get value of pb plug.");
          is_parallel_blender = false;
        }

        if (is_parallel_blender || 
            _tree->ignore_slider(blends.name().asChar())) {
          _tree->report_ignored_slider(blends.name().asChar());

        } else {
          MObjectArray base_objects;
          status = blends.getBaseObjects(base_objects);
          if (!status) {
            status.perror("MFnBlendShapeDeformer::getBaseObjects");
          } else {
            for (unsigned int oi = 0; oi < base_objects.length(); oi++) {
              MObject base_object = base_objects[oi];
              
              MIntArray index_list;
              status = blends.weightIndexList(index_list);
              if (!status) {
                status.perror("MFnBlendShapeDeformer::weightIndexList");
              } else {
                for (unsigned int i = 0; i < index_list.length(); i++) {
                  int wi = index_list[i];
                  PT(MayaBlendDesc) blend_desc = new MayaBlendDesc(blends, wi);
                  blend_desc = _tree->add_blend_desc(blend_desc);
                  _blend_descs.push_back(blend_desc);
                }
              }
            }
          }
        }
      }
    }

    it.next();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaNodeDesc::check_lods
//       Access: Private
//  Description: Walks through the hierarchy again and checks for LOD
//               specifications.  Any such specifications found are
//               recorded on the child nodes of the lodGroups
//               themselves: the nodes that actually switch in and
//               out.  (This is the way they are recorded in an egg
//               file.)
////////////////////////////////////////////////////////////////////
void MayaNodeDesc::
check_lods() {
  // Walk through the children first.  This makes it easier in the
  // below (we only have to return in the event of an error).
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    MayaNodeDesc *child = (*ci);
    child->check_lods();
  }

  // Now consider whether this node is an lodGroup.
  if (_dag_path != (MDagPath *)NULL &&
      _dag_path->hasFn(MFn::kLodGroup)) {
    // This node is a parent lodGroup; its children, therefore, are
    // LOD's.
    MStatus status;
    MFnDagNode dag_node(*_dag_path, &status);
    if (!status) {
      status.perror("Couldn't get node from dag path for lodGroup");
      return;
    }

    MPlug plug = dag_node.findPlug("threshold", &status);
    if (!status) {
      status.perror("Couldn't get threshold attributes on lodGroup");
      return;
    }

    // There ought to be the one fewer elements in the array than
    // there are children of the node.
    unsigned int num_elements = plug.numElements();
    unsigned int num_children = _children.size();
    if (num_elements + 1 != num_children) {
      mayaegg_cat.warning()
        << "Node " << get_name() << " has " << num_elements
        << " LOD entries, but " << num_children << " children.\n";
    }

    // Should we also consider cameraMatrix, to transform the LOD's
    // origin?  It's not clear precisely what this transform matrix
    // means in Maya, so we'll wait until we have a sample file that
    // demonstrates its use.

    double switch_out = 0.0;
    unsigned int i = 0;
    while (i < num_elements && i < num_children) {
      MPlug element = plug.elementByLogicalIndex(i);
      MayaNodeDesc *child = _children[i];

      double switch_in;
      status = element.getValue(switch_in);
      if (!status) {
        status.perror("Couldn't get double value from threshold.");
        return;
      }

      child->_is_lod = true;
      child->_switch_in = switch_in;
      child->_switch_out = switch_out;

      switch_out = switch_in;
      ++i;
    }

    while (i < num_children) {
      // Also set the last child(ren).  Maya wants this to switch in
      // at infinity, but Panda doesn't have such a concept; we'll
      // settle for four times the switch_out distance.
      MayaNodeDesc *child = _children[i];
      child->_is_lod = true;
      child->_switch_in = switch_out * 4.0;
      child->_switch_out = switch_out;
      
      ++i;
    }
  }
}
