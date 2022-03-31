/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlToEggConverter.cxx
 * @author drose
 * @date 2004-10-01
 */

#include "vrmlToEggConverter.h"
#include "vrmlAppearance.h"
#include "indexedFaceSet.h"
#include "vrmlNodeType.h"
#include "parse_vrml.h"
#include "vrmlParser.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggData.h"
#include "deg_2_rad.h"

/**
 *
 */
VRMLToEggConverter::
VRMLToEggConverter() {
}

/**
 *
 */
VRMLToEggConverter::
VRMLToEggConverter(const VRMLToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

/**
 *
 */
VRMLToEggConverter::
~VRMLToEggConverter() {
}

/**
 * Allocates and returns a new copy of the converter.
 */
SomethingToEggConverter *VRMLToEggConverter::
make_copy() {
  return new VRMLToEggConverter(*this);
}


/**
 * Returns the English name of the file type this converter supports.
 */
std::string VRMLToEggConverter::
get_name() const {
  return "VRML";
}

/**
 * Returns the common extension of the file type this converter supports.
 */
std::string VRMLToEggConverter::
get_extension() const {
  return "wrl";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz extension), false otherwise.
 */
bool VRMLToEggConverter::
supports_compressed() const {
  return true;
}

/**
 * Handles the reading of the input file and converting it to egg.  Returns
 * true if successful, false otherwise.
 */
bool VRMLToEggConverter::
convert_file(const Filename &filename) {
  clear_error();

  VrmlScene *scene = parse_vrml(filename);
  if (scene == nullptr) {
    return false;
  }

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_yup_right);
  }

  // First, resolve all the DEFUSE references, and count the number of times
  // each node is USEd.
  Nodes nodes;
  VrmlScene::iterator si;
  for (si = scene->begin(); si != scene->end(); ++si) {
    get_all_defs((*si)._node, nodes);
  }

  // Now go through the hierarchy again, and this time actually build the egg
  // structure.
  VrmlScene::const_iterator csi;
  for (csi = scene->begin(); csi != scene->end(); ++csi) {
    vrml_node((*csi)._node, get_egg_data(), LMatrix4d::ident_mat());
  }

  return !had_error();
}

/**
 * Makes a first pass through the VRML hierarchy, identifying all nodes marked
 * with a DEF code, and also counting the times each one is referenced by USE.
 * Later, we'll need this information: if a node is referenced at least once,
 * we need to define it as an instance node.
 */
void VRMLToEggConverter::
get_all_defs(SFNodeRef &vrml, VRMLToEggConverter::Nodes &nodes) {
  Nodes::iterator ni;

  switch (vrml._type) {
  case SFNodeRef::T_def:
    // If this is a node definition, add it to the map.
    nassertv(vrml._name != nullptr);
    nassertv(vrml._p != nullptr);
    /*
      This happens too often to bother yelling about it.
    ni = nodes.find(vrml._name);
    if (ni != nodes.end()) {
      cerr << "Warning: node name " << vrml._name
           << " appears multiple times.\n";
    }
    */
    nodes[vrml._name] = vrml._p;
    break;

  case SFNodeRef::T_use:
    // If it's a reference, resolve it.
    nassertv(vrml._name != nullptr);
    ni = nodes.find(vrml._name);
    if (ni == nodes.end()) {
      std::cerr << "Unknown node reference: " << vrml._name << "\n";
    } else {
      // Increment the use count of the node.
      (*ni).second->_use_count++;

      // Store the pointer itself in the reference, so we don't have to do
      // this again later.
      vrml._p = (*ni).second;
    }
    return;

  default:
    break;
  }

  VrmlNode *node = vrml._p;
  if (node != nullptr) {
    VrmlNode::Fields::iterator fi;
    for (fi = node->_fields.begin(); fi != node->_fields.end(); ++fi) {
      if ((*fi)._type->type == SFNODE) {
        get_all_defs((*fi)._value._sfnode, nodes);
      } else if ((*fi)._type->type == MFNODE) {
        MFArray *children = (*fi)._value._mf;
        MFArray::iterator ci;
        for (ci = children->begin(); ci != children->end(); ++ci) {
          get_all_defs((*ci)._sfnode, nodes);
        }
      }
    }
  }
}

/**
 * Processes a single VRML node, converting it to egg and adding it to the egg
 * file, if appropriate, or doing whatever else should be done.
 */
void VRMLToEggConverter::
vrml_node(const SFNodeRef &vrml, EggGroupNode *egg,
          const LMatrix4d &net_transform) {
  const VrmlNode *node = vrml._p;
  if (node != nullptr) {
    // Now add it to the egg file at this point.
    if (strcmp(node->_type->getName(), "Group") == 0) {
      vrml_grouping_node(vrml, egg, net_transform,
                         &VRMLToEggConverter::vrml_group);
    } else if (strcmp(node->_type->getName(), "Transform") == 0) {
      vrml_grouping_node(vrml, egg, net_transform,
                         &VRMLToEggConverter::vrml_transform);
    } else if (strcmp(node->_type->getName(), "Shape") == 0) {
      vrml_grouping_node(vrml, egg, net_transform,
                         &VRMLToEggConverter::vrml_shape);
    }
  }
}

/**
 * Begins initial processing of a grouping-type node; that is, any node (like
 * Group, Transform, or Shape) that maps to a <Group> or <Instance> in egg.
 * This create the group and does any instance-munging necessary, then calls
 * the indicated method with the new parameters.
 */
void VRMLToEggConverter::
vrml_grouping_node(const SFNodeRef &vrml, EggGroupNode *egg,
                   const LMatrix4d &net_transform,
                   void (VRMLToEggConverter::*process_func)
                   (const VrmlNode *node, EggGroup *group,
                    const LMatrix4d &net_transform)) {
  const VrmlNode *node = vrml._p;
  nassertv(node != nullptr);
  std::string name;
  if (vrml._name != nullptr) {
    name = vrml._name;
  }

  /*
    The following code fragment was used in the old DWD-style egg
    library.  Currently, the Panda egg library doesn't support
    instance references, so we deal with VRML instances by copying.

  if (vrml._type == SFNodeRef::T_use) {
    // If this is an instancing reference, just add the reference and return;
    // no need for further processing on the node.
    Instances::const_iterator fi = _instances.find(node);
    assert(fi != _instances.end());
    EggInstance *inst = _data.CreateInstance(egg);
    inst->AddGroupRef((*fi).second);
    return;
  }
    */

  PT(EggGroup) group = new EggGroup(name);
  egg->add_child(group);

  LMatrix4d next_transform = net_transform;

  if (node->_use_count > 0) {
    // If this node is referenced one or more times later in the file, we must
    // make it an instance node.
    group->set_group_type(EggGroup::GT_instance);
    next_transform = LMatrix4d::ident_mat();

    // And define the instance for future references.  _instances[node] =
    // group;
  }

  (this->*process_func)(node, group, next_transform);
}


/**
 * Creates an Egg group corresponding to the VRML group.
 */
void VRMLToEggConverter::
vrml_group(const VrmlNode *node, EggGroup *group,
           const LMatrix4d &net_transform) {
  const MFArray *children = node->get_value("children")._mf;
  MFArray::const_iterator ci;
  for (ci = children->begin(); ci != children->end(); ++ci) {
    vrml_node((*ci)._sfnode, group, net_transform);
  }
}

/**
 * Creates an Egg group with a transform corresponding to the VRML group.
 */
void VRMLToEggConverter::
vrml_transform(const VrmlNode *node, EggGroup *group,
               const LMatrix4d &net_transform) {
  const double *scale = node->get_value("scale")._sfvec;
  const double *rotation = node->get_value("rotation")._sfvec;
  const double *translation = node->get_value("translation")._sfvec;

  const double *center = node->get_value("center")._sfvec;
  const double *o = node->get_value("scaleOrientation")._sfvec;

  LMatrix4d local_transform = LMatrix4d::ident_mat();

  bool any_transform = false;

  if (scale[0] != 1.0 || scale[1] != 1.0 || scale[2] != 1.0) {
    any_transform = true;
    if (center[0] != 0.0 || center[1] != 0.0 || center[2] != 0.0) {
      local_transform *=
        LMatrix4d::translate_mat(-center[0], -center[1], -center[2]);

      if (o[3] != 0.0) {
        local_transform *=
          LMatrix4d::rotate_mat(rad_2_deg(-o[3]), LVector3d(o[0], o[1], o[2]));
        local_transform *=
          LMatrix4d::scale_mat(scale[0], scale[1], scale[2]);
        local_transform *=
          LMatrix4d::rotate_mat(rad_2_deg(o[3]), LVector3d(o[0], o[1], o[2]));

      } else {
        local_transform *=
          LMatrix4d::scale_mat(scale[0], scale[1], scale[2]);
      }
      local_transform *=
        LMatrix4d::translate_mat(center[0], center[1], center[2]);

    } else {
      if (o[3] != 0.0) {
        local_transform *=
          LMatrix4d::rotate_mat(rad_2_deg(-o[3]), LVector3d(o[0], o[1], o[2]));
        local_transform *=
          LMatrix4d::scale_mat(scale[0], scale[1], scale[2]);
        local_transform *=
          LMatrix4d::rotate_mat(rad_2_deg(o[3]), LVector3d(o[0], o[1], o[2]));

      } else {
        local_transform *=
          LMatrix4d::scale_mat(scale[0], scale[1], scale[2]);
      }
    }
  }

  if (rotation[3] != 0.0) {
    any_transform = true;
    if (center[0] != 0.0 || center[1] != 0.0 || center[2] != 0.0) {
      local_transform *=
        LMatrix4d::translate_mat(-center[0], -center[1], -center[2]);
      local_transform *=
        LMatrix4d::rotate_mat(rad_2_deg(rotation[3]),
                              LVector3d(rotation[0], rotation[1], rotation[2]));
      local_transform *=
        LMatrix4d::translate_mat(center[0], center[1], center[2]);

    } else {
      local_transform *=
        LMatrix4d::rotate_mat(rad_2_deg(rotation[3]),
                              LVector3d(rotation[0], rotation[1], rotation[2]));
    }
  }

  if (translation[0] != 0.0 ||
      translation[1] != 0.0 ||
      translation[2] != 0.0) {
    any_transform = true;
    local_transform *=
      LMatrix4d::translate_mat(translation[0], translation[1], translation[2]);
  }

  if (any_transform) {
    group->set_transform3d(local_transform);
  }

  LMatrix4d next_transform = local_transform * net_transform;

  const MFArray *children = node->get_value("children")._mf;
  MFArray::const_iterator ci;
  for (ci = children->begin(); ci != children->end(); ++ci) {
    vrml_node((*ci)._sfnode, group, next_transform);
  }
}

/**
 * Creates an Egg group corresponding a VRML shape.  This will probably
 * contain a vertex pool and a number of polygons.
 */
void VRMLToEggConverter::
vrml_shape(const VrmlNode *node, EggGroup *group,
           const LMatrix4d &net_transform) {
  const VrmlNode *geometry = node->get_value("geometry")._sfnode._p;

  if (geometry != nullptr) {
    VRMLAppearance appearance(node->get_value("appearance")._sfnode._p);

    if (strcmp(geometry->_type->getName(), "IndexedFaceSet") == 0) {
      IndexedFaceSet ifs(geometry, appearance);
      ifs.convert_to_egg(group, net_transform);
    } else {
      std::cerr << "Ignoring " << geometry->_type->getName() << "\n";
    }
  }
}
