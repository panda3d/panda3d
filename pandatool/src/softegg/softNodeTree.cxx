// Filename: softNodeTree.cxx
// Created by:  masad (26Sep03)
// 
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2003, Disney Enterprises, Inc.  All rights reserved
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
#include "dcast.h"

#include <SAA.h>

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftNodeTree::
SoftNodeTree() {
  _root = new SoftNodeDesc;
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
    char *eol = strchr( modelNote, '\n' );
    if ( eol != NULL)
      *eol = '\0';
    else
      modelNote[size] = '\0';
    
    cout << "\nmodelNote = " << modelNote << endl;
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
  char *hyphen;
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

  // Get the entire Soft scene.
  int numModels;
  SAA_Elem *models;

  SAA_sceneGetNbModels( &scene, &numModels ); 
  cout << "Scene has " << numModels << " model(s)...\n";
  
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
        cout << "model[" << i << "]" << endl;
        cout << " level " << level << endl;
        cout << " status is " << status << "\n";
        
        //        if (!level) {
          build_node(&scene, &models[i]);
          //        }
      }
    }
  }
# if 0  
  if (all_ok) {
    _root->check_pseudo_joints(false);
  }
#endif  
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
//     Function: SoftNodeTree::clear_egg
//       Access: Public
//  Description: Removes all of the references to generated egg
//               structures from the tree, and prepares the tree for
//               generating new egg structures.
////////////////////////////////////////////////////////////////////
void SoftNodeTree::
clear_egg(EggData *egg_data, EggGroupNode *egg_root, 
          EggGroupNode *skeleton_node) {
  //  _root->clear_egg();
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

  cout << node_desc->_egg_group << endl;
  cout << node_desc->_parent << endl;
  if (node_desc->_egg_group == (EggGroup *)NULL) {
    // We need to make a new group node.
    EggGroup *egg_group;
    
    egg_group = new EggGroup(node_desc->get_name());
    if (node_desc->is_joint()) {
      egg_group->set_group_type(EggGroup::GT_joint);
    }

    if (!node_desc->_parent || node_desc->_parent == _root) {
      // The parent is the root.
      _egg_root->add_child(egg_group);
    } else {
      // The parent is another node.
      EggGroup *parent_egg_group = get_egg_group(node_desc->_parent);
      parent_egg_group->add_child(egg_group);
    }

#if 0
    SoftEggGroupUserData *parent_user_data = NULL;

    if (node_desc->_parent == _root) {
      // The parent is the root.
      _egg_root->add_child(egg_group);

    } else {
      // The parent is another node.
      EggGroup *parent_egg_group = get_egg_group(node_desc->_parent);
      parent_egg_group->add_child(egg_group);

      if (parent_egg_group->has_user_data()) {
        DCAST_INTO_R(parent_user_data, parent_egg_group->get_user_data(), NULL);
      }
    }

    if (node_desc->has_models()) {
      // Check for an object type setting, from Oliver's plug-in.
      MObject dag_object = node_desc->get_dag_path().node();
      string object_type;
      if (get_enum_attribute(dag_object, "eggObjectTypes1", object_type)) {
        egg_group->add_object_type(object_type);
      }
      if (get_enum_attribute(dag_object, "eggObjectTypes2", object_type)) {
        egg_group->add_object_type(object_type);
      }
      if (get_enum_attribute(dag_object, "eggObjectTypes3", object_type)) {
        egg_group->add_object_type(object_type);
      }

      // Is the node flagged to be invisible?  If it is, and is has no
      // other egg flags, it is implicitly tagged "backstage", so it
      // won't get converted.  (But it might be an invisible collision
      // solid, which is why we do this only if it has no other egg
      // flags.)
      bool visible = true;
      get_bool_attribute(dag_object, "visibility", visible);
      if (!visible && egg_group->get_num_object_types() == 0) {
        egg_group->add_object_type("backstage");
      }

      // We treat the object type "billboard" as a special case: we
      // apply this one right away and also flag the group as an
      // instance.
      if (egg_group->has_object_type("billboard")) {    
        egg_group->remove_object_type("billboard");
        egg_group->set_group_type(EggGroup::GT_instance);
        egg_group->set_billboard_type(EggGroup::BT_axis);
        
      } else if (egg_group->has_object_type("billboard-point")) {    
        egg_group->remove_object_type("billboard-point");
        egg_group->set_group_type(EggGroup::GT_instance);
        egg_group->set_billboard_type(EggGroup::BT_point_camera_relative);
      }
      
      // We also treat the object type "dcs" and "model" as a special
      // case, so we can test for these flags later.
      if (egg_group->has_object_type("dcs")) {
        egg_group->remove_object_type("dcs");
        egg_group->set_dcs_type(EggGroup::DC_default);
      }
      if (egg_group->has_object_type("model")) {
        egg_group->remove_object_type("model");
        egg_group->set_model_flag(true);
      }
      
      // And "vertex-color" and "double-sided" have meaning only to
      // this converter.
      SoftEggGroupUserData *user_data;
      if (parent_user_data == (SoftEggGroupUserData *)NULL) {
        user_data = new SoftEggGroupUserData;
      } else {
        // Inherit the flags from above.
        user_data = new SoftEggGroupUserData(*parent_user_data);
      }

      if (egg_group->has_object_type("vertex-color")) {
        egg_group->remove_object_type("vertex-color");
        user_data->_vertex_color = true;
      }
      if (egg_group->has_object_type("double-sided")) {
        egg_group->remove_object_type("double-sided");
        user_data->_double_sided = true;
      }
      egg_group->set_user_data(user_data);
    }
#endif
    //    EggUserData *user_data = new EggUserData;
    //    egg_group->set_user_data(user_data);
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
  
  if (node_desc->_egg_table == (EggTable *)NULL) {
    cout << "creating a new table\n";
    // We need to make a new table node.
    //    nassertr(node_desc->_parent != (SoftNodeDesc *)NULL, NULL);
    
    EggTable *egg_table = new EggTable(node_desc->get_name());
    node_desc->_anim = new EggXfmSAnim("xform", _egg_data->get_coordinate_system());
    node_desc->_anim->set_fps(_fps);
    egg_table->add_child(node_desc->_anim);
    
    //    if (!node_desc->_parent->is_joint()) {
    // The parent is not a joint; put it at the top.
    _skeleton_node->add_child(egg_table);
    /*
    } else {
      // The parent is another joint.
      EggTable *parent_egg_table = get_egg_table(node_desc->_parent);
      parent_egg_table->add_child(egg_table);
    }
    */
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
//     Function: SoftNodeTree::build_node
//       Access: Public
//  Description: Returns a pointer to the node corresponding to the
//               indicated dag_path object, creating it first if
//               necessary.
////////////////////////////////////////////////////////////////////
void SoftNodeTree::
build_node(SAA_Scene *scene, SAA_Elem *model) {
  char *name;
  string node_name;
  int numChildren;
  int thisChild;
  SAA_Elem *children;
  SAA_Boolean isSkeleton = FALSE;

  if (_use_prefix)
    name = GetFullName(scene, model);
  else
    name = GetName(scene, model);

  node_name = name;

  ///////////////////////////////////////////////////////////////////////
  // check to see if this is a branch we don't want to descend - this
  // will prevent creating geometry for animation control structures
  ///////////////////////////////////////////////////////////////////////

  /*
  if ( (strstr(name, "con-") == NULL) && 
       (strstr(name, "con_") == NULL) && 
       (strstr(name, "fly_") == NULL) && 
       (strstr(name, "fly-") == NULL) && 
       (strstr(name, "camRIG") == NULL) &&
       (strstr(name, "bars") == NULL) && 
       // split
       (!_search_prefix || (strstr(name, _search_prefix) != NULL)) )
    {
  */
      SoftNodeDesc *node_desc = r_build_node(NULL, node_name);
      node_desc->set_model(model);
      SAA_modelIsSkeleton( scene, model, &isSkeleton );
      if (isSkeleton || (strstr(node_desc->get_name().c_str(), "joint") != NULL))
        node_desc->set_joint();
      
      SAA_modelGetNbChildren( scene, model, &numChildren );
      cout << " Model " << node_name << " children: " << numChildren << endl;
      
      if ( numChildren ) {
        children = new SAA_Elem[numChildren];
        SAA_modelGetChildren( scene, model, numChildren, children );
        if (!children)
          cout << "Not enough Memory for children...\n";
        
        for ( thisChild = 0; thisChild < numChildren; thisChild++ ) {
          if (_use_prefix)
            node_name = GetFullName(scene, &children[thisChild]);
          else
            node_name = GetName(scene, &children[thisChild]);
          
          cout << " building child " << thisChild << "...";

          SoftNodeDesc *node_child = r_build_node(node_desc, node_name);
          node_child->set_model(&children[thisChild]);
          
          //  if (strstr(name, "joint") != NULL)
          SAA_modelIsSkeleton( scene, &children[thisChild], &isSkeleton );
          if (isSkeleton || (strstr(node_child->get_name().c_str(), "joint") != NULL))
            node_child->set_joint();
        }
      }
      //    }
  return;
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
  // corresponding MayaNodeDesc immediately.
  NodesByName::const_iterator ni = _nodes_by_name.find(name);
  if (ni != _nodes_by_name.end()) {
    cout << (*ni).first << endl;
    node_desc = (*ni).second;
    node_desc->set_parent(parent_node);
    return node_desc;
  }

  // Otherwise, we have to create it.  Do this recursively, so we
  // create each node along the path.
  /*
  if (!parent_node) {
    node_desc = _root;
  }
  else {
  */
    node_desc = new SoftNodeDesc(parent_node, name);
    /*
  }
    */
  cout << " node name : " << name << endl;
  _nodes.push_back(node_desc);

  _nodes_by_name.insert(NodesByName::value_type(name, node_desc));

  return node_desc;
}

//
//
//
