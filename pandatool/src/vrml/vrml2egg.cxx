// Filename: vrml2egg.C
// Created by:  drose (24Jun99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#include "vrml2egg.h"
#include "parse_vrml.h"
#include "vrmlNode.h"
#include "appearance.h"
#include "indexedFaceSet.h"
#include "y.tab.h"

#include <assert.h>
#include <math.h>

static const double pi = 4.0 * atan(1.0);

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::Help
//       Access: Public, Virtual
//  Description: Displays the "what is this program" message, along
//               with the usage message.  Should be overridden in base
//               classes to describe the current program.
////////////////////////////////////////////////////////////////////
void MainProgram::
Help() {
  cerr <<
    "\n"
    "vrml2egg converts VRML 2.0 files (with the .wrl extension) to egg format.\n"
    "A reasonable subset of the VRML standard is supported, including polygons,\n"
    "colors, normals, textures, and hierarchy.\n";

  Usage();
}

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::Usage
//       Access: Public, Virtual
//  Description: Displays the usage message.
////////////////////////////////////////////////////////////////////
void MainProgram::
Usage() {
  cerr << "\nUsage:\n"
       << _commandName << " [opts] input.wrl\n\n"

       << "Options:\n";

  ShowOpts();
  cerr << "\n";
}

    


////////////////////////////////////////////////////////////////////
//     Function: MainProgram::ShowOpts
//       Access: Public, Virtual
//  Description: Displays the valid options.  Should be extended in
//               base classes to show additional options relevant to
//               the current program.
////////////////////////////////////////////////////////////////////
void MainProgram::
ShowOpts() {
  EggBase::ShowOpts();
}

  

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::HandleGetopts
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
boolean MainProgram::
HandleGetopts(char flag, char *optarg, int &optind,
	      int argc, char **argv) {
  bool okflag = true;

  switch (flag) {
  default:
    okflag = EggBase::HandleGetopts(flag, optarg, optind, argc, argv);
  }

  return okflag;
}


////////////////////////////////////////////////////////////////////
//     Function: MainProgram::HandleArgs
//       Access: Public
//  Description: Called by EggBase::CommandLine() to do the right
//               thing with the arguments after the switches.
////////////////////////////////////////////////////////////////////
boolean MainProgram::
HandleArgs(int &argc, char **&argv) {
  if (argc != 2) {
    cerr << "VRML file name required.\n";
    return false;
  }

  _filename = argv[1];

  return EggBase::HandleArgs(argc, argv);
}

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::vrml_group
//       Access: Public
//  Description: Creates an Egg group corresponding to the VRML group.
////////////////////////////////////////////////////////////////////
void MainProgram::
vrml_group(const VrmlNode *node, EggGroup *group,
	   const pfMatrix &net_transform) {
  const MFArray *children = node->get_value("children")._mf;
  MFArray::const_iterator ci;
  for (ci = children->begin(); ci != children->end(); ++ci) {
    vrml_node((*ci)._sfnode, group, net_transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::vrml_transform
//       Access: Public
//  Description: Creates an Egg group with a transform corresponding
//               to the VRML group.
////////////////////////////////////////////////////////////////////
void MainProgram::
vrml_transform(const VrmlNode *node, EggGroup *group,
	       const pfMatrix &net_transform) {
  const double *scale = node->get_value("scale")._sfvec;
  const double *rotation = node->get_value("rotation")._sfvec;
  const double *translation = node->get_value("translation")._sfvec;

  const double *center = node->get_value("center")._sfvec;
  const double *o = node->get_value("scaleOrientation")._sfvec;

  pfMatrix local_transform;
  local_transform.makeIdent();

  boolean any_transform = false;

  if (scale[0] != 1.0 || scale[1] != 1.0 || scale[2] != 1.0) {
    any_transform = true;
    if (center[0] != 0.0 || center[1] != 0.0 || center[2] != 0.0) {
      local_transform.postTrans(local_transform, 
				-center[0], -center[1], -center[2]);
      if (o[3] != 0.0) {
	local_transform.postRot(local_transform,
				-o[3] * 180.0 / pi, o[0], o[1], o[2]);
	local_transform.postScale(local_transform,
				  scale[0], scale[1], scale[2]);
	local_transform.postRot(local_transform,
				o[3] * 180.0 / pi, o[0], o[1], o[2]);
      } else {
	local_transform.postScale(local_transform, 
				  scale[0], scale[1], scale[2]);
      }
      local_transform.postTrans(local_transform, 
				center[0], center[1], center[2]);
    } else {
      if (o[3] != 0.0) {
	local_transform.postRot(local_transform,
				-o[3] * 180.0 / pi, o[0], o[1], o[2]);
	local_transform.postScale(local_transform,
				  scale[0], scale[1], scale[2]);
	local_transform.postRot(local_transform,
				o[3] * 180.0 / pi, o[0], o[1], o[2]);
      } else {
	local_transform.postScale(local_transform, 
				  scale[0], scale[1], scale[2]);
      }
    }      
  }

  if (rotation[3] != 0.0) {
    any_transform = true;
    if (center[0] != 0.0 || center[1] != 0.0 || center[2] != 0.0) {
      local_transform.postTrans(local_transform, 
				-center[0], -center[1], -center[2]);
      local_transform.postRot(local_transform,
			      rotation[3] * 180.0 / pi,
			      rotation[0], rotation[1], rotation[2]);
      local_transform.postTrans(local_transform, 
				center[0], center[1], center[2]);

    } else {
      local_transform.postRot(local_transform,
			      rotation[3] * 180.0 / pi,
			      rotation[0], rotation[1], rotation[2]);
    }
  }

  if (translation[0] != 0.0 ||
      translation[1] != 0.0 ||
      translation[2] != 0.0) {
    any_transform = true;
    local_transform.postTrans(local_transform, 
			      translation[0],
			      translation[1],
			      translation[2]);
  }

  if (any_transform) {
    group->transform = local_transform;
    group->flags |= EF_TRANSFORM;
  }

  pfMatrix next_transform = local_transform * net_transform;

  const MFArray *children = node->get_value("children")._mf;
  MFArray::const_iterator ci;
  for (ci = children->begin(); ci != children->end(); ++ci) {
    vrml_node((*ci)._sfnode, group, next_transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::vrml_shape
//       Access: Public
//  Description: Creates an Egg group corresponding a VRML shape.
//               This will probably contain a vertex pool and a number
//               of polygons.
////////////////////////////////////////////////////////////////////
void MainProgram::
vrml_shape(const VrmlNode *node, EggGroup *group,
	   const pfMatrix &net_transform) {
  const VrmlNode *geometry = node->get_value("geometry")._sfnode._p;

  double transparency = 0.0;

  if (geometry != NULL) {
    Appearance appearance
      (node->get_value("appearance")._sfnode._p, _data);

    if (strcmp(geometry->_type->getName(), "IndexedFaceSet") == 0) {
      IndexedFaceSet ifs(geometry, appearance, _data);
      ifs.convert_to_egg(group, net_transform);
    } else {
      cerr << "Ignoring " << geometry->_type->getName() << "\n";
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MainProgram::vrml_grouping_node
//       Access: Public
//  Description: Begins initial processing of a grouping-type node;
//               that is, any node (like Group, Transform, or Shape)
//               that maps to a <Group> or <Instance> in egg.  This
//               create the group and does any instance-munging
//               necessary, then calls the indicated method with the
//               new parameters.
////////////////////////////////////////////////////////////////////
void MainProgram::
vrml_grouping_node(const SFNodeRef &vrml, EggGroup *egg,
		   const pfMatrix &net_transform,
		   void (MainProgram::*process_func)
		   (const VrmlNode *node, EggGroup *group,
		    const pfMatrix &net_transform)) {
  const VrmlNode *node = vrml._p;
  assert(node != NULL);
  const char *name = vrml._name;

  if (vrml._type == SFNodeRef::T_use) {
    // If this is an instancing reference, just add the reference and
    // return; no need for further processing on the node.
    Instances::const_iterator fi = _instances.find(node);
    assert(fi != _instances.end());
    EggInstance *inst = _data.CreateInstance(egg);
    inst->AddGroupRef((*fi).second);
    return;
  }

  EggGroup *group;
  pfMatrix next_transform;

  if (node->_use_count > 0) {
    // If this node is referenced one or more times later in the file,
    // we must make it an instance node.
    group = _data.CreateInstance(egg, name);
    next_transform.makeIdent();

    // And define the instance for future references.
    _instances[node] = group;

  } else {
    group = _data.CreateGroup(egg, name);
    next_transform = net_transform;
  }

  (this->*process_func)(node, group, next_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::vrml_node
//       Access: Public
//  Description: Processes a single VRML node, converting it to egg
//               and adding it to the egg file, if appropriate, or
//               doing whatever else should be done.
////////////////////////////////////////////////////////////////////
void MainProgram::
vrml_node(const SFNodeRef &vrml, EggGroup *egg, 
	  const pfMatrix &net_transform) {
  const VrmlNode *node = vrml._p;
  if (node != NULL) {
    // Now add it to the egg file at this point.
    if (strcmp(node->_type->getName(), "Group") == 0) {
      vrml_grouping_node(vrml, egg, net_transform, &vrml_group);
    } else if (strcmp(node->_type->getName(), "Transform") == 0) {
      vrml_grouping_node(vrml, egg, net_transform, &vrml_transform);
    } else if (strcmp(node->_type->getName(), "Shape") == 0) {
      vrml_grouping_node(vrml, egg, net_transform, &vrml_shape);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MainProgram::get_all_defs
//       Access: Public
//  Description: Makes a first pass through the VRML hierarchy,
//               identifying all nodes marked with a DEF code, and
//               also counting the times each one is referenced by
//               USE.  Later, we'll need this information: if a node
//               is referenced at least once, we need to define it as
//               an instance node.
////////////////////////////////////////////////////////////////////
void MainProgram::
get_all_defs(SFNodeRef &vrml, MainProgram::Nodes &nodes) {
  Nodes::iterator ni;

  switch (vrml._type) {
  case SFNodeRef::T_def:
    // If this is a node definition, add it to the map.
    assert(vrml._name != NULL);
    assert(vrml._p != NULL);
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
    assert(vrml._name != NULL);
    ni = nodes.find(vrml._name);
    if (ni == nodes.end()) {
      cerr << "Unknown node reference: " << vrml._name << "\n";
    } else {
      // Increment the use count of the node.
      (*ni).second->_use_count++;

      // Store the pointer itself in the reference, so we don't have
      // to do this again later.
      vrml._p = (*ni).second;
    }
    return;
  }

  VrmlNode *node = vrml._p;
  if (node != NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: MainProgram::DoIt
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void MainProgram::
DoIt() {
  VrmlScene *scene = parse_vrml(_filename.c_str());
  if (scene != NULL) {
    // First, resolve all the DEF/USE references, and count the number
    // of times each node is USEd.
    Nodes nodes;
    VrmlScene::iterator si;
    for (si = scene->begin(); si != scene->end(); ++si) {
      get_all_defs((*si)._node, nodes);
    }

    // Now go through the hierarchy again, and this time actually
    // build the egg structure.
    pfMatrix ident;
    ident.makeIdent();

    VrmlScene::const_iterator csi;
    for (csi = scene->begin(); csi != scene->end(); ++csi) {
      vrml_node((*csi)._node, &_data.root_group, ident);
    }

    _data.UniquifyNames();
    Output() << _data << "\n";
  }
}


int 
main(int argc, char *argv[]) {
  pfInitArenas();

  MainProgram prog;

  if (prog.CommandLine(argc, argv)) {
    prog.DoIt();
    return 0;
  }
  return 1;
}
