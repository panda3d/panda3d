// Filename: softNodeTree.cxx
// Created by:  masad (26Sep03)
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

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "softNodeTree.h"
#include "softEggGroupUserData.h"
#include "config_softegg.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "eggXfmSAnim.h"
#include "eggData.h"
#include "softToEggConverter.h"
#include "dcast.h"

#include <SAA.h>

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftNodeTree::
SoftNodeTree() {
  _root = new SoftNodeDesc(NULL, "----root");
  _root->fullname = "----root";
  _fps = 0.0;
  _use_prefix = 0;
  _search_prefix = NULL;
  _egg_data = (EggData *)NULL;
  _egg_root = (EggGroupNode *)NULL;
  _skeleton_node = (EggGroupNode *)NULL;
}
////////////////////////////////////////////////////////////////////
//     Function: GetName
//       Access: Public
//  Description: Given an element, return a copy of the element's 
//                 name WITHOUT prefix. 
////////////////////////////////////////////////////////////////////
char *SoftNodeTree::
GetName( SAA_Scene *scene, SAA_Elem *element ) {
  int nameLen;
  char *name;
  
  // get the name
  SAA_elementGetNameLength( scene, element, &nameLen ); 
  name = new char[++nameLen];
  SAA_elementGetName( scene, element, nameLen, name );

  return name;
}

////////////////////////////////////////////////////////////////////
//     Function: GetFullName
//       Access: Public
//  Description: Given an element, return a copy of the element's 
//                 name complete with prefix. 
////////////////////////////////////////////////////////////////////
char *SoftNodeTree::
GetFullName( SAA_Scene *scene, SAA_Elem *element )
{
  int nameLen, prefixLen;
  char *name, *prefix;

  // get the name length
  SAA_elementGetNameLength( scene, element, &nameLen );
  // get the prefix length
  SAA_elementGetPrefixLength( scene, element, &prefixLen );
  // allocate the array to hold name
  name = new char[++nameLen];
  // allocate the array to hold prefix and length + hyphen
  prefix = new char[++prefixLen + nameLen + 4];
  // get the name
  SAA_elementGetName( scene, element, nameLen, name );
  // get the prefix
  SAA_elementGetPrefix( scene, element, prefixLen, prefix );
  // add 'em together
  strcat(prefix, "-");
  strcat(prefix, name);

  // return string
  return prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: GetModelNoteInfo
//       Access: Public
//  Description: Given an element, return a string containing the
//               contents of its MODEL NOTE entry 
////////////////////////////////////////////////////////////////////
char *SoftNodeTree::
GetModelNoteInfo( SAA_Scene *scene, SAA_Elem *model ) {
  int size;
  char *modelNote = NULL;
  SAA_Boolean bigEndian;

  SAA_elementGetUserDataSize( scene, model, "MNOT", &size );

  if ( size != 0 ) {
    // allocate modelNote string
    modelNote = new char[size + 1];
    
    // get ModelNote data from this model
    SAA_elementGetUserData( scene, model, "MNOT", size,
                            &bigEndian, (void *)modelNote );
    
    //strip off newline, if present
    char *eol = (char *)memchr( modelNote, '\n', size );
    if ( eol != NULL)
      *eol = '\0';
    else
      modelNote[size] = '\0';

    softegg_cat.spam() << "\nmodelNote = " << modelNote << endl;
  }
  
  return modelNote;
}

////////////////////////////////////////////////////////////////////
//     Function: GetRootName
//       Access: Public
//  Description: Given a string, return a copy of the string up to
//                 the first occurence of '-'. 
////////////////////////////////////////////////////////////////////
char *SoftNodeTree::
GetRootName( const char *name ) {
  const char *hyphen;
  char *root;
  int len;
  
  hyphen = strchr( name, '-' );
  len = hyphen-name;
  
  if ( (hyphen != NULL) && len ) {
    root = new char[len+1];
    strncpy( root, name, len );
    root[len] = '\0';
  }
  else {
    root = new char[strlen(name)+1];
    strcpy( root, name );
  }
  return( root );
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::build_complete_hierarchy
//       Access: Public
//  Description: Walks through the complete Soft hierarchy and builds
//               up the corresponding tree.
////////////////////////////////////////////////////////////////////
bool SoftNodeTree::
build_complete_hierarchy(SAA_Scene &scene, SAA_Database &database) {
  SI_Error status;
  SoftNodeDesc *node;

  // Get the entire Soft scene.
  int numModels;
  SAA_Elem *models;

  SAA_sceneGetNbModels( &scene, &numModels ); 
  softegg_cat.spam() << "Scene has " << numModels << " model(s)...\n";
  
  // This while loop walks through the entire Soft hierarchy, one
  // node at a time. 
  bool all_ok = true;
  if ( numModels ) {
    // allocate array of models
    models = (SAA_Elem *) new SAA_Elem[numModels];
    if ( models != NULL ) {
      if ((status = SAA_sceneGetModels( &scene, numModels, models )) != SI_SUCCESS) {
        return false;
      }
      for ( int i = 0; i < numModels; i++ ) {
        int level;
        status = SAA_elementGetHierarchyLevel( &scene, &models[i], &level );
        softegg_cat.spam() << "model[" << i << "]" << endl;
        softegg_cat.spam() << " level " << level << endl;
        softegg_cat.spam() << " status is " << status << "\n";

        node = build_node(&scene, &models[i]);
        if (!level && node)
          node->set_parent(_root);
      }
    }
  }

  softegg_cat.spam() << "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj\n";

  // check the nodes that are junk for animation/artist control purposes
  _root->check_junk(false);

  softegg_cat.spam() << "jpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjpjp\n";

  // check the nodes that are parent of ancestors of a joint
  _root->check_joint_parent();

  softegg_cat.spam() << "pppppppppppppppppppppppppppppppppppppppppppppppppppppppp\n";

  // check the nodes that are pseudo joints
  _root->check_pseudo_joints(false);

  softegg_cat.spam() << "========================================================\n";

  // find _parentJoint for each node
  _root->set_parentJoint(&scene, NULL);

  return all_ok;
}
#if 0
////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::build_selected_hierarchy
//       Access: Public
//  Description: Walks through the selected subset of the Soft
//               hierarchy (or the complete hierarchy, if nothing is
//               selected) and builds up the corresponding tree.
////////////////////////////////////////////////////////////////////
bool SoftNodeTree::
build_selected_hierarchy(char *scene_name) {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  // Get only the selected geometry.
  MSelectionList selection;
  status = MGlobal::getActiveSelectionList(selection);
  if (!status) {
    status.perror("MGlobal::getActiveSelectionList");
    return false;
  }
  
  // Get the selected geometry only if the selection is nonempty;
  // otherwise, get the whole scene anyway.
  if (selection.isEmpty()) {
    softegg_cat.info()
      << "Selection list is empty.\n";
    return build_complete_hierarchy();
  }

  bool all_ok = true;
  unsigned int length = selection.length();
  for (unsigned int i = 0; i < length; i++) {
    MDagPath root_path;
    status = selection.getDagPath(i, root_path);
    if (!status) {
      status.perror("MSelectionList::getDagPath");
    } else {
      // Now traverse through the selected dag path and all nested
      // dag paths.
      dag_iterator.reset(root_path);
      while (!dag_iterator.isDone()) {
        MDagPath dag_path;
        status = dag_iterator.getPath(dag_path);
        if (!status) {
          status.perror("MItDag::getPath");
        } else {
          build_node(dag_path);
        }
        
        dag_iterator.next();
      }
    }
  }

  if (all_ok) {
    _root->check_pseudo_joints(false);
  }

  return all_ok;
}
#endif
////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::get_num_nodes
//       Access: Public
//  Description: Returns the total number of nodes in the hierarchy,
//               not counting the root node.
////////////////////////////////////////////////////////////////////
int SoftNodeTree::
get_num_nodes() const {
  return _nodes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::get_node
//       Access: Public
//  Description: Returns the nth node in the hierarchy, in an
//               arbitrary ordering.
////////////////////////////////////////////////////////////////////
SoftNodeDesc *SoftNodeTree::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), NULL);
  return _nodes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::get_node
//       Access: Public
//  Description: Returns the node named 'name' in the hierarchy, in
//               an arbitrary ordering.
////////////////////////////////////////////////////////////////////
SoftNodeDesc *SoftNodeTree::
get_node(string name) const {
  NodesByName::const_iterator ni = _nodes_by_name.find(name);
  if (ni != _nodes_by_name.end())
    return (*ni).second;
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::clear_egg
//       Access: Public
//  Description: Removes all of the references to generated egg
//               structures from the tree, and prepares the tree for
//               generating new egg structures.
////////////////////////////////////////////////////////////////////
void SoftNodeTree::
clear_egg(EggData *egg_data, EggGroupNode *egg_root, 
          EggGroupNode *skeleton_node) {
  _root->clear_egg();
  _egg_data = egg_data;
  _egg_root = egg_root;
  _skeleton_node = skeleton_node;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::get_egg_group
//       Access: Public
//  Description: Returns the EggGroupNode corresponding to the group
//               or joint for the indicated node.  Creates the group
//               node if it has not already been created.
////////////////////////////////////////////////////////////////////
EggGroup *SoftNodeTree::
get_egg_group(SoftNodeDesc *node_desc) {
  nassertr(_egg_root != (EggGroupNode *)NULL, NULL);

  // lets print some relationship
  softegg_cat.spam() << " group " << node_desc->get_name() << "(" << node_desc->_egg_group << ")";
  if (node_desc->_parent)
    softegg_cat.spam() << " parent " << node_desc->_parent->get_name() << "(" << node_desc->_parent << ")";
  else
    softegg_cat.spam() << " parent " << node_desc->_parent;
  softegg_cat.spam() << endl;

  if (node_desc->_egg_group == (EggGroup *)NULL) {
    // We need to make a new group node.
    EggGroup *egg_group;
    
    egg_group = new EggGroup(node_desc->get_name());
    if (node_desc->is_joint()) {
      egg_group->set_group_type(EggGroup::GT_joint);
    }

    if (stec.flatten || (!node_desc->_parentJoint || node_desc->_parentJoint == _root)) {
      // The parent is the root.
      softegg_cat.spam() << "came hereeeee\n";
      _egg_root->add_child(egg_group);
    } else {
      // The parent is another node.
      EggGroup *parent_egg_group = get_egg_group(node_desc->_parentJoint);
      parent_egg_group->add_child(egg_group);
    }

    node_desc->_egg_group = egg_group;
  }
  
  return node_desc->_egg_group;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::get_egg_table
//       Access: Public
//  Description: Returns the EggTable corresponding to the joint
//               for the indicated node.  Creates the table node if it
//               has not already been created.
////////////////////////////////////////////////////////////////////
EggTable *SoftNodeTree::
get_egg_table(SoftNodeDesc *node_desc) {
  nassertr(_skeleton_node != (EggGroupNode *)NULL, NULL);
  nassertr(node_desc->is_joint(), NULL);
  
  // lets print some relationship
  softegg_cat.spam() << " group " << node_desc->get_name() << "(" << node_desc->_egg_group << ")";
  if (node_desc->_parent)
    softegg_cat.spam() << " parent " << node_desc->_parent->get_name() << "(" << node_desc->_parent << ")";
  else
    softegg_cat.spam() << " parent " << node_desc->_parent;
  softegg_cat.spam() << endl;

  if (node_desc->_egg_table == (EggTable *)NULL) {
    softegg_cat.spam() << "creating a new table\n";
    // We need to make a new table node.
    //    nassertr(node_desc->_parent != (SoftNodeDesc *)NULL, NULL);
    
    EggTable *egg_table = new EggTable(node_desc->get_name());
    node_desc->_anim = new EggXfmSAnim("xform", _egg_data->get_coordinate_system());
    node_desc->_anim->set_fps(_fps);
    egg_table->add_child(node_desc->_anim);
    
    if (stec.flatten || (!node_desc->_parentJoint || node_desc->_parentJoint == _root)) {
      //    if (!node_desc->_parent->is_joint()) {
      // The parent is not a joint; put it at the top.
      _skeleton_node->add_child(egg_table);
    } else {
      // The parent is another joint.
      EggTable *parent_egg_table = get_egg_table(node_desc->_parentJoint);
      parent_egg_table->add_child(egg_table);
    }

    node_desc->_egg_table = egg_table;
  }
  
  return node_desc->_egg_table;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::get_egg_anim
//       Access: Public
//  Description: Returns the anim table corresponding to the joint
//               for the indicated node.  Creates the table node if it
//               has not already been created.
////////////////////////////////////////////////////////////////////
EggXfmSAnim *SoftNodeTree::
get_egg_anim(SoftNodeDesc *node_desc) {
  get_egg_table(node_desc);
  return node_desc->_anim;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::handle_null
//       Access: Public
//  Description: Sets joint information for MNILL node
////////////////////////////////////////////////////////////////////
void SoftNodeTree::
handle_null(SAA_Scene *scene, SoftNodeDesc *node_desc, const char *node_name) {
  const char *name = node_name;
  SAA_AlgorithmType    algo;
  SAA_Elem *model = node_desc->get_model();
  
  SAA_modelGetAlgorithm( scene, model, &algo );
  softegg_cat.spam() << " null algorithm: " << algo << endl;
  
  if ( algo == SAA_ALG_INV_KIN ) {
    //    MakeJoint( &scene, lastJoint, lastAnim,  model, name );
    node_desc->set_joint();
    softegg_cat.spam() << " encountered IK root: " << name << endl;
  }
  else if ( algo == SAA_ALG_INV_KIN_LEAF ) {
    //    MakeJoint( &scene, lastJoint, lastAnim, model, name );
    node_desc->set_joint();
    softegg_cat.spam() << " encountered IK leaf: " << name << endl;
  }
  else if ( algo == SAA_ALG_STANDARD ) {
    SAA_Boolean isSkeleton = FALSE;
    softegg_cat.spam() << " encountered Standard null: " << name << endl;

    SAA_modelIsSkeleton( scene, model, &isSkeleton );

    // check to see if this NULL is used as a skeleton
    // or is animated via constraint only ( these nodes are
    // tagged by the animator with the keyword "joint"
    // somewhere in the nodes name)
    if ( isSkeleton || (strstr( name, "joint" ) != NULL) ) {
      //      MakeJoint( &scene, lastJoint, lastAnim, model, name );
      node_desc->set_joint();
      softegg_cat.spam() << " animating Standard null!!!\n";
      softegg_cat.spam() << "isSkeleton: " << isSkeleton << endl;
    }
  }
  else
    softegg_cat.spam() << " encountered some other NULL: " << algo << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::build_node
//       Access: Public
//  Description: Returns a pointer to the node corresponding to the
//               indicated dag_path object, creating it first if
//               necessary.
////////////////////////////////////////////////////////////////////
SoftNodeDesc *SoftNodeTree::
build_node(SAA_Scene *scene, SAA_Elem *model) {
  char *name, *fullname;
  string node_name;
  int numChildren;
  int thisChild;
  SAA_Elem *children;
  SAA_ModelType type;
  SAA_Boolean isSkeleton = FALSE;

  fullname = GetFullName(scene, model);
  if (_use_prefix)
    name = fullname;
  else
    name = GetName(scene, model);

  node_name = name;

  SoftNodeDesc *node_desc = r_build_node(NULL, node_name);

  node_desc->fullname = fullname;
  node_desc->set_model(model);
  SAA_modelIsSkeleton( scene, model, &isSkeleton );

  // find out what type of node we're dealing with
  SAA_modelGetType( scene, node_desc->get_model(), &type );
  
  if (type == SAA_MJNT || isSkeleton || (strstr(node_desc->get_name().c_str(), "joint") != NULL))
    node_desc->set_joint();
  
  // treat the MNILL differently, because it needs to detect and set some joints
  if (type == SAA_MNILL)
    handle_null(scene, node_desc, name);

  if (node_desc->is_joint())
    softegg_cat.spam() << "type: " << type << " isSkeleton: " << isSkeleton << endl;

  // get to the children
  SAA_modelGetNbChildren( scene, model, &numChildren );
  softegg_cat.spam() << " Model " << node_name << " children: " << numChildren << endl;
  
  if ( numChildren ) {
    children = new SAA_Elem[numChildren];
    SAA_modelGetChildren( scene, model, numChildren, children );
    if (!children)
      softegg_cat.info() << "Not enough Memory for children...\n";
    
    for ( thisChild = 0; thisChild < numChildren; thisChild++ ) {
      fullname = GetFullName(scene, &children[thisChild]);
      if (_use_prefix)
        node_name = fullname;
      else
        node_name = GetName(scene, &children[thisChild]);
      
      softegg_cat.spam() << " building child " << thisChild << "...";
      
      SoftNodeDesc *node_child = r_build_node(node_desc, node_name);

      node_child->fullname = fullname;
      node_child->set_model(&children[thisChild]);
      SAA_modelIsSkeleton( scene, &children[thisChild], &isSkeleton );

      // find out what type of node we're dealing with
      SAA_modelGetType( scene, node_child->get_model(), &type );
      
      if (type == SAA_MJNT || isSkeleton || (strstr(node_child->get_name().c_str(), "joint") != NULL))
        node_child->set_joint();

      // treat the MNILL differently, because it needs to detect and set some joints
      if (type == SAA_MNILL)
        handle_null(scene, node_child, node_name.c_str());

      if (node_child->is_joint())
        softegg_cat.spam() << "type: " << type << " isSkeleton: " << isSkeleton << endl;
    }
  }
  return node_desc;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::r_build_node
//       Access: Private
//  Description: The recursive implementation of build_node().
////////////////////////////////////////////////////////////////////
SoftNodeDesc *SoftNodeTree::
r_build_node(SoftNodeDesc *parent_node, const string &name) {
  SoftNodeDesc *node_desc;

  // If we have already encountered this pathname, return the
  // corresponding SoftNodeDesc immediately.
  NodesByName::const_iterator ni = _nodes_by_name.find(name);
  if (ni != _nodes_by_name.end()) {
    softegg_cat.spam() << "  already built node " << (*ni).first;
    node_desc = (*ni).second;
    node_desc->set_parent(parent_node);
    return node_desc;
  }

  // Otherwise, we have to create it.  Do this recursively, so we
  // create each node along the path.
  node_desc = new SoftNodeDesc(parent_node, name);

  softegg_cat.spam() << " node name : " << name << endl;
  _nodes.push_back(node_desc);

  _nodes_by_name.insert(NodesByName::value_type(name, node_desc));

  return node_desc;
}

//
//
//
