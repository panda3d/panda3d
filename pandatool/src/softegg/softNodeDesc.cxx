// Filename: softNodeDesc.cxx
// Created by:  masad (03Oct03)
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

#include "softNodeDesc.h"

TypeHandle SoftNodeDesc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftNodeDesc::
SoftNodeDesc(SoftNodeDesc *parent, const string &name) :
  Namable(name),
  _parent(parent)
{
  _model = (SAA_Elem *)NULL;
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;
  _joint_type = JT_none;

  // Add ourselves to our parent.
  if (_parent != (SoftNodeDesc *)NULL) {
    softegg_cat.spam() << "parent name " << _parent->get_name();
    _parent->_children.push_back(this);
  }

  fullname = NULL;

  numTexLoc = 0;
  numTexGlb = 0;

  uScale = NULL; 
  vScale = NULL;
  uOffset = NULL;
  vOffset = NULL;
  
  valid;
  uv_swap;
  //  SAA_Boolean visible;
  textures = NULL;
  materials = NULL;
  triangles = NULL;
  gtype = SAA_GEOM_ORIGINAL;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftNodeDesc::
~SoftNodeDesc() {
  if (_model != (SAA_Elem *)NULL) {
    delete _model;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::set_model
//       Access: Public
//  Description: Indicates an associated between the SoftNodeDesc and
//               some SAA_Elem instance.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
set_model(SAA_Elem *model) {
  _model = model;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::set_parent
//       Access: Public
//  Description: Sometimes, parent is not known at node creation
//               As soon as it is known, set the parent
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
set_parent(SoftNodeDesc *parent) {
  if (_parent) {
    softegg_cat.spam() << endl;
    /*
    softegg_cat.spam() << " expected _parent to be null!?\n";
    if (_parent == parent)
      softegg_cat.spam() << " parent already set\n";
    else {
      softegg_cat.spam() << " current parent " << _parent->get_name() << " new parent " 
           << parent << endl;
    }
    */
    return;
  }
  _parent = parent;
  softegg_cat.spam() << " set parent to " << _parent->get_name() << endl;

  // Add ourselves to our parent.
  _parent->_children.push_back(this);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::has_model
//       Access: Public
//  Description: Returns true if a Soft dag path has been associated
//               with this node, false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
has_model() const {
  return (_model != (SAA_Elem *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::get_model
//       Access: Public
//  Description: Returns the SAA_Elem * associated with this node.  It
//               is an error to call this unless has_model()
//               returned true.
////////////////////////////////////////////////////////////////////
SAA_Elem *SoftNodeDesc::
get_model() const {
  nassertr(_model != (SAA_Elem *)NULL, _model);
  return _model;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::is_joint
//       Access: Private
//  Description: Returns true if the node should be treated as a joint
//               by the converter.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
is_joint() const {
  return _joint_type == JT_joint || _joint_type == JT_pseudo_joint;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::is_junk
//       Access: Private
//  Description: Returns true if the node should be treated as a junk
//               by the converter.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
is_junk() const {
  return _joint_type == JT_junk;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::set_joint
//       Access: Private
//  Description: sets the _joint_type to JT_joint
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
set_joint() {
  _joint_type = JT_joint;
}
////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::is_joint_parent
//       Access: Private
//  Description: Returns true if the node is the parent or ancestor of
//               a joint.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
is_joint_parent() const {
  return _joint_type == JT_joint_parent;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::clear_egg
//       Access: Private
//  Description: Recursively clears the egg pointers from this node
//               and all children.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
clear_egg() {
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    SoftNodeDesc *child = (*ci);
    child->clear_egg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::mark_joint_parent
//       Access: Private
//  Description: Indicates that this node has at least one child that
//               is a joint or a pseudo-joint.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
mark_joint_parent() {
  if (_joint_type == JT_none) {
    _joint_type = JT_joint_parent;
    softegg_cat.spam() << " marked parent " << get_name();
  }
  else
    softegg_cat.spam() << " ?parent " << get_name() << " joint type " << _joint_type;
  
  if (_parent != (SoftNodeDesc *)NULL) {
    _parent->mark_joint_parent();
  }
  softegg_cat.spam() << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::check_joint_parent
//       Access: Private
//  Description: Walks the hierarchy, if a node is joint, make
//               sure all its parents are marked JT_joint_parent
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
check_joint_parent() {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    SoftNodeDesc *child = (*ci);
    if (child->is_joint()) {
      softegg_cat.spam() << "child " << child->get_name();
      mark_joint_parent();
    }
    child->check_joint_parent();
  }
}

///////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::check_junk
//       Access: Public
//  Description: check to see if this is a branch we don't want to 
//               descend - this will prevent creating geometry for 
//               animation control structures
///////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
check_junk(bool parent_junk) {
  const char *name = get_name().c_str();

  if (parent_junk) {
    _joint_type = JT_junk;
    softegg_cat.spam() << "junk node " << get_name() << endl;
  }
  if ( (strstr(name, "con-") != NULL) || 
       (strstr(name, "con_") != NULL) || 
       (strstr(name, "fly_") != NULL) || 
       (strstr(name, "fly-") != NULL) || 
       (strstr(name, "camRIG") != NULL) ||
       (strstr(name, "cam_rig") != NULL) ||
       (strstr(name, "bars") != NULL) )
    {
      _joint_type = JT_junk;
      softegg_cat.spam() << "junk node " << get_name() << endl;
      parent_junk = true;
      Children::const_iterator ci;
      for (ci = _children.begin(); ci != _children.end(); ++ci) {
        SoftNodeDesc *child = (*ci);
        softegg_cat.spam() << child->get_name() << ",";
      }
      softegg_cat.spam() << endl;
    }
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    SoftNodeDesc *child = (*ci);
    child->check_junk(parent_junk);
  }
}

///////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::is_partial
//       Access: Public
//  Description: check to see if this is a selected branch we want to 
//               descend - this will prevent creating geometry for 
//               other parts
///////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
is_partial(char *search_prefix) {
  const char *name = fullname;

  // if no search prefix then return false
  if (!search_prefix)
    return false;
  // if name is search_prefix, return false
  if (strstr(name, search_prefix) != NULL) {
    cout << "matched " << name << " ";
    return false;
  }
  // if name is not search_prefix, look in its parent
  if (strstr(name, search_prefix) == NULL) {
    cout << "node " << name << " ";
    if (_parent) 
      return _parent->is_partial(search_prefix);
  }
  // neither name nor its parent is search_prefix
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::check_pseudo_joints
//       Access: Private
//  Description: Walks the hierarchy, looking for non-joint nodes that
//               are both children and parents of a joint.  These
//               nodes are deemed to be pseudo joints, since the
//               converter must treat them as joints.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
check_pseudo_joints(bool joint_above) {
  if (_joint_type == JT_joint_parent && joint_above) {
    // This is one such node: it is the parent of a joint
    // (JT_joint_parent is set), and it is the child of a joint
    // (joint_above is set).
    _joint_type = JT_pseudo_joint;
    softegg_cat.spam() << "pseudo " << get_name() << " case1\n";
  }

  if (_joint_type == JT_joint) {
    // If this node is itself a joint, then joint_above is true for
    // all child nodes.
    joint_above = true;
  }

  // Don't bother traversing further if _joint_type is none or junk, since
  // that means this node has no joint children.
  if (_joint_type != JT_none && _joint_type != JT_junk) {

    bool any_joints = false;
    Children::const_iterator ci;
    for (ci = _children.begin(); ci != _children.end(); ++ci) {
      SoftNodeDesc *child = (*ci);
      child->check_pseudo_joints(joint_above);
      if (child->is_joint()) {
        softegg_cat.spam() << get_name() << " any_joint true by " << child->get_name() << endl;
        any_joints = true;
      }
    }

    // If any children qualify as joints, then any sibling nodes that
    // are parents of joints are also elevated to joints.
    if (any_joints) {
      bool all_joints = true;
      for (ci = _children.begin(); ci != _children.end(); ++ci) {
        SoftNodeDesc *child = (*ci);
        if (child->_joint_type == JT_joint_parent) {
          child->_joint_type = JT_pseudo_joint;
          softegg_cat.spam() << "pseudo " << child->get_name() << " case2 by parent " << get_name() << "\n";
        } else if (child->_joint_type == JT_none || child->_joint_type == JT_junk) {
          all_joints = false;
        }
      }

      if (all_joints || any_joints) {
        // Finally, if all children or at least one is a joint, then we are too.
        if (_joint_type == JT_joint_parent) {
          _joint_type = JT_pseudo_joint;
          softegg_cat.spam() << "pseudo " << get_name() << " case3\n";
        }
      }
    }
  }
  else
    softegg_cat.spam() << "found null joint " << get_name() << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftToEggConverter::get_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Soft node,
//               and applies it to the corresponding Egg node.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
get_transform(SAA_Scene *scene, EggGroup *egg_group, bool global) {
  // Get the model's matrix
  int scale_joint = 0;

  /*
  if ( strstr( _parent->get_name().c_str(), "scale" ) != NULL ) {
    scale_joint = 1;
    softegg_cat.spam() << "scale joint flag set!\n";
  }
  */

  if (!global && _parent->is_joint() && !stec.flatten && !scale_joint) {

    SAA_modelGetMatrix( scene, get_model(), SAA_COORDSYS_LOCAL,  matrix );
    softegg_cat.spam() << get_name() << " using local matrix :parent ";

  } else {

    SAA_modelGetMatrix( scene, get_model(), SAA_COORDSYS_GLOBAL,  matrix );
    softegg_cat.spam() << get_name() << " using global matrix :parent ";

  }

  softegg_cat.spam() << _parent->get_name() << endl;

  softegg_cat.spam() << "model matrix = " << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << " " << matrix[0][3] << "\n";
  softegg_cat.spam() << "model matrix = " << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << " " << matrix[1][3] << "\n";
  softegg_cat.spam() << "model matrix = " << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << " " << matrix[2][3] << "\n";
  softegg_cat.spam() << "model matrix = " << matrix[3][0] << " " << matrix[3][1] << " " << matrix[3][2] << " " << matrix[3][3] << "\n";

  if (!global && is_joint()) {
    LMatrix4d m4d(matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
                  matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
                  matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
                  matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
    if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
      egg_group->set_transform(m4d);
      softegg_cat.spam() << "set transform in egg_group\n";
    }
  }
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::get_joint_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Soft node,
//               as appropriate for a joint in an animated character,
//               and applies it to the indicated node.  This is
//               different from get_transform() in that it does not
//               respect the _transform_type flag, and it does not
//               consider the relative transforms within the egg file.
//               more added functionality: now fills in components of
//               anim (EffXfmSAnim) class (masad).
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
get_joint_transform(SAA_Scene *scene,  EggGroup *egg_group, EggXfmSAnim *anim, bool global) {
  //  SI_Error result;
  SAA_Elem *skeletonPart = _model;
  const char *name = get_name().c_str();

  if ( skeletonPart != NULL ) {
    float i,j,k;
    float h,p,r;
    float x,y,z;
    int scale_joint = 0;
    
    /*
    if ( strstr( _parent->get_name().c_str(), "scale" ) != NULL ) {
      scale_joint = 1;    
      softegg_cat.spam() << "scale joint flag set!\n";
    }
    */

    softegg_cat.spam() << "\n\nanimating child " << name << endl;

    if (_parent->is_joint() && !stec.flatten && !scale_joint ) {
      softegg_cat.spam() << "using local matrix\n";

      //get SAA orientation
      SAA_modelGetRotation( scene, skeletonPart, SAA_COORDSYS_LOCAL, 
                            &p, &h, &r );

      //get SAA translation
      SAA_modelGetTranslation( scene, skeletonPart, SAA_COORDSYS_LOCAL, 
                               &x, &y, &z );
      
      //get SAA scaling
      SAA_modelGetScaling( scene, skeletonPart, SAA_COORDSYS_LOCAL, 
                           &i, &j, &k );
    } else {
      softegg_cat.spam() << " using global matrix\n";

      //get SAA orientation
      SAA_modelGetRotation( scene, skeletonPart, SAA_COORDSYS_GLOBAL, 
                            &p, &h, &r );

      //get SAA translation
      SAA_modelGetTranslation( scene, skeletonPart, SAA_COORDSYS_GLOBAL, 
                               &x, &y, &z );

      //get SAA scaling
      SAA_modelGetScaling( scene, skeletonPart, SAA_COORDSYS_GLOBAL, 
                           &i, &j, &k );
    }
    
    softegg_cat.spam() << "\nanim data: " << i << " " << j << " " << k << endl;
    softegg_cat.spam() << "\t" << p << " " << h << " " << r << endl;
    softegg_cat.spam() << "\t" << x << " " << y << " " << z << endl;

    // make sure the ordering is correct
    anim->set_order(anim->get_standard_order());

    // Add each component by their names
    anim->add_component_data("i", i);
    anim->add_component_data("j", j);
    anim->add_component_data("k", k);
    anim->add_component_data("p", p);
    anim->add_component_data("h", h);
    anim->add_component_data("r", r);
    anim->add_component_data("x", x);
    anim->add_component_data("y", y);
    anim->add_component_data("z", z);
  }
  else {
    softegg_cat.spam() << "Cannot build anim table - no skeleton\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::load_poly_model
//       Access: Private
//  Description: Converts the indicated Soft polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
load_poly_model(SAA_Scene *scene, SAA_ModelType type) {
  SI_Error result;
  const char *name = get_name().c_str();
  
  int i;
  int id = 0;

  // if making a pose - get deformed geometry
  if ( stec.make_pose )
    gtype = SAA_GEOM_DEFORMED;
        
  // If the model is a PATCH in soft, set its step before tesselating
  else if ( type == SAA_MPTCH )
    SAA_patchSetStep( scene, _model, stec.nurbs_step, stec.nurbs_step );
  
  // Get the number of triangles    
  result = SAA_modelGetNbTriangles( scene, _model, gtype, id, &numTri);
  softegg_cat.spam() << "triangles: " << numTri << "\n";
  
  if ( result != SI_SUCCESS ) {
    softegg_cat.spam() << "Error: couldn't get number of triangles!\n";
    softegg_cat.spam() << "\tbailing on model: " << name << "\n";
    return;    
  }
  
  // check to see if surface is also skeleton...
  SAA_Boolean isSkeleton = FALSE;
  
  SAA_modelIsSkeleton( scene, _model, &isSkeleton );
  
  // check to see if this surface is used as a skeleton
  // or is animated via constraint only ( these nodes are
  // tagged by the animator with the keyword "joint"
  // somewhere in the nodes name)
  softegg_cat.spam() << "is Skeleton? " << isSkeleton << "\n";
  
  /*************************************************************************************/
  
  // model is not a null and has no triangles!
  if ( !numTri ) {
    softegg_cat.spam() << "no triangles!\n";
  }
  else {
    // allocate array of triangles
    triangles = (SAA_SubElem *) new SAA_SubElem[numTri];
    if (!triangles) {
      softegg_cat.spam() << "Not enough Memory for triangles...\n";
      exit(1);
    }
    // triangulate model and read the triangles into array
    SAA_modelGetTriangles( scene, _model, gtype, id, numTri, triangles );
    softegg_cat.spam() << "got triangles\n";
    
    /***********************************************************************************/
    
    // allocate array of materials (Asad: it gives a warning if try to get one triangle
    //                                    at a time...investigate later
    // read each triangle's material into array  
    materials = (SAA_Elem*) new SAA_Elem[numTri];
    SAA_triangleGetMaterials( scene, _model, numTri, triangles, materials );
    if (!materials) {
      softegg_cat.spam() << "Not enough Memory for materials...\n";
      exit(1);
    }
    softegg_cat.spam() << "got materials\n";
    
    /***********************************************************************************/
    
    // allocate array of textures per triangle
    int *numTexTri = new int[numTri];
    const void *relinfo;
    
    // find out how many local textures per triangle
    for (i = 0; i < numTri; i++) {    
      result = SAA_materialRelationGetT2DLocNbElements( scene, &materials[i], FALSE, 
                                                        &relinfo, &numTexTri[i] );
      // polytex    
      if ( result == SI_SUCCESS )
        numTexLoc += numTexTri[i];
    }
    
    // don't need this anymore...
    //free( numTexTri ); 
    
    // get local textures if present
    if ( numTexLoc ) {
      softegg_cat.spam() << "numTexLoc = " << numTexLoc << endl;
      
      // allocate arrays of texture info
      uScale = new float[numTri];
      vScale = new float[numTri];
      uOffset = new float[numTri];
      vOffset = new float[numTri];
      texNameArray = new char *[numTri];
      
      // ASSUME only one texture per material
      textures = new SAA_Elem[numTri];
      
      for ( i = 0; i < numTri; i++ ) {
        // and read all referenced local textures into array
        SAA_materialRelationGetT2DLocElements( scene, &materials[i],
                                               TEX_PER_MAT , &textures[i] );
        // initialize the array value
        texNameArray[i] = NULL;
        
        // check to see if texture is present
        result = SAA_elementIsValid( scene, &textures[i], &valid );
        
        if ( result != SI_SUCCESS )
          softegg_cat.spam() << "SAA_elementIsValid failed!!!!\n";
        
        // texture present - get the name and uv info 
        if ( valid ) {
          // according to drose, we don't need to convert .pic files to .rgb,
          // panda can now read the .pic files.
          texNameArray[i] = stec.GetTextureName(scene, &textures[i]);
          
          softegg_cat.spam() << " tritex[" << i << "] named: " << texNameArray[i] << endl;
          
          SAA_texture2DGetUVSwap( scene, &textures[i], &uv_swap );
          
          if ( uv_swap == TRUE )
            softegg_cat.spam() << " swapping u and v...\n" ;
          
          SAA_texture2DGetUScale( scene, &textures[i], &uScale[i] );
          SAA_texture2DGetVScale( scene, &textures[i], &vScale[i] );
          SAA_texture2DGetUOffset( scene, &textures[i], &uOffset[i] );
          SAA_texture2DGetVOffset( scene, &textures[i], &vOffset[i] );
          
          softegg_cat.spam() << "tritex[" << i << "] uScale: " << uScale[i] << " vScale: " << vScale[i] << endl;
          softegg_cat.spam() << " uOffset: " << uOffset[i] << " vOffset: " << vOffset[i] << endl;
          
          SAA_texture2DGetRepeats( scene, &textures[i], &uRepeat, &vRepeat );
          softegg_cat.spam() << "uRepeat = " << uRepeat << ", vRepeat = " << vRepeat << endl;
        }
        else {
          softegg_cat.spam() << "Invalid texture...\n";
          softegg_cat.spam() << " tritex[" << i << "] named: (null)\n";
        }
      }
    }
    else { // if no local textures, try to get global textures
      SAA_modelRelationGetT2DGlbNbElements( scene, _model,
                                            FALSE, &relinfo, &numTexGlb );
      if ( numTexGlb ) {
        // ASSUME only one texture per model
        textures = new SAA_Elem;
        // get the referenced texture
        SAA_modelRelationGetT2DGlbElements( scene, _model, 
                                            TEX_PER_MAT, textures ); 
        softegg_cat.spam() << "numTexGlb = " << numTexGlb << endl;
        // check to see if texture is present
        SAA_elementIsValid( scene, textures, &valid );
        if ( valid ) {  // texture present - get the name and uv info 
          SAA_texture2DGetUVSwap( scene, textures, &uv_swap );
          
          if ( uv_swap == TRUE )
            softegg_cat.spam() << " swapping u and v...\n";
          
          // according to drose, we don't need to convert .pic files to .rgb,
          // panda can now read the .pic files.
          texNameArray = new char *[1];
          *texNameArray = stec.GetTextureName(scene, textures);
          
          softegg_cat.spam() << " global tex named: " << *texNameArray << endl;
          
          // allocate arrays of texture info
          uScale = new float;
          vScale = new float;
          uOffset = new float;
          vOffset = new float;
          
          SAA_texture2DGetUScale( scene, textures, uScale );
          SAA_texture2DGetVScale( scene, textures, vScale );
          SAA_texture2DGetUOffset( scene, textures, uOffset );
          SAA_texture2DGetVOffset( scene, textures, vOffset );
          
          softegg_cat.spam() << " global tex uScale: " << *uScale << " vScale: " << *vScale << endl;
          softegg_cat.spam() << "            uOffset: " << *uOffset << " vOffset: " << *vOffset << endl;
          
          SAA_texture2DGetRepeats(  scene, textures, &uRepeat, &vRepeat );
          softegg_cat.spam() << "uRepeat = " << uRepeat << ", vRepeat = " << vRepeat << endl;
        }
        else {
          softegg_cat.spam() << "Invalid Texture...\n";
        }
      }
    }
  }
  softegg_cat.spam() << "got textures" << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::load_nurbs_model
//       Access: Private
//  Description: Converts the indicated Soft polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
load_nurbs_model(SAA_Scene *scene, SAA_ModelType type) {
  SI_Error result;
  const char *name = get_name().c_str();
  
  // if making a pose - get deformed geometry
  if ( stec.make_pose )
    gtype = SAA_GEOM_DEFORMED;
        
  // If the model is a NURBS in soft, set its step before tesselating
  if ( type == SAA_MNSRF )
    SAA_nurbsSurfaceSetStep( scene, _model, stec.nurbs_step, stec.nurbs_step );
  
  // get the materials
  /***********************************************************************************/
  const void *relinfo;

  SAA_modelRelationGetMatNbElements( scene, get_model(), FALSE, &relinfo,
                                     &numNurbMats );

  softegg_cat.spam() << "nurbs surf has " << numNurbMats << " materials\n";

  if ( numNurbMats ) {
    materials = new SAA_Elem[numNurbMats];
    if (!materials) {
      softegg_cat.spam() << "Out Of Memory on allocating materials\n";
      exit(1);
    }
    
    SAA_modelRelationGetMatElements( scene, get_model(), relinfo, 
                                     numNurbMats, materials ); 
    
    softegg_cat.spam() << "got materials\n";

    // get the textures
    /***********************************************************************************/
    numNurbTexLoc = 0;
    numNurbTexGlb = 0;
    
    // find out how many local textures per NURBS surface
    // ASSUME it only has one material
    SAA_materialRelationGetT2DLocNbElements( scene, &materials[0], FALSE, &relinfo, &numNurbTexLoc );
    
    // if present, get local textures
    if ( numNurbTexLoc ) {
      softegg_cat.spam() << name << " had " << numNurbTexLoc << " local tex\n";
      nassertv(numNurbTexLoc == 1);
      
      textures = new SAA_Elem[numNurbTexLoc];
      
      // get the referenced texture
      SAA_materialRelationGetT2DLocElements( scene, &materials[0], TEX_PER_MAT, &textures[0] );
      
    }
    // if no locals, try to get globals
    else {
      SAA_modelRelationGetT2DGlbNbElements( scene, get_model(), FALSE, &relinfo, &numNurbTexGlb );
      
      if ( numNurbTexGlb ) {
        softegg_cat.spam() << name << " had " << numNurbTexGlb << " global tex\n";
        nassertv(numNurbTexGlb == 1);
        
        textures = new SAA_Elem[numNurbTexGlb];
        
        // get the referenced texture
        SAA_modelRelationGetT2DGlbElements( scene, get_model(), TEX_PER_MAT, &textures[0] );
      }
    }
    
    if ( numNurbTexLoc || numNurbTexGlb) {
      
      // allocate the texture name array
      texNameArray = new char *[1];
      // allocate arrays of texture info
      uScale = new float;
      vScale = new float;
      uOffset = new float;
      vOffset = new float;
      
      // check to see if texture is present
      result = SAA_elementIsValid( scene, &textures[0], &valid );
      
      if ( result != SI_SUCCESS )
        softegg_cat.spam() << "SAA_elementIsValid failed!!!!\n";
      
      // texture present - get the name and uv info 
      if ( valid ) {
        // according to drose, we don't need to convert .pic files to .rgb,
        // panda can now read the .pic files.
        texNameArray[0] = stec.GetTextureName(scene, &textures[0]);
        
        softegg_cat.spam() << " tritex[0] named: " << texNameArray[0] << endl;
        
        SAA_texture2DGetUVSwap( scene, &textures[0], &uv_swap );
        
        if ( uv_swap == TRUE )
          softegg_cat.spam() << " swapping u and v...\n" ;
        
        SAA_texture2DGetUScale( scene, &textures[0], &uScale[0] );
        SAA_texture2DGetVScale( scene, &textures[0], &vScale[0] );
        SAA_texture2DGetUOffset( scene, &textures[0], &uOffset[0] );
        SAA_texture2DGetVOffset( scene, &textures[0], &vOffset[0] );
        
        softegg_cat.spam() << "tritex[0] uScale: " << uScale[0] << " vScale: " << vScale[0] << endl;
        softegg_cat.spam() << " uOffset: " << uOffset[0] << " vOffset: " << vOffset[0] << endl;
        
        SAA_texture2DGetRepeats( scene, &textures[0], &uRepeat, &vRepeat );
        softegg_cat.spam() << "uRepeat = " << uRepeat << ", vRepeat = " << vRepeat << endl;
      }
      else {
        softegg_cat.spam() << "Invalid texture...\n";
        softegg_cat.spam() << " tritex[0] named: (null)\n";
      }
    }
    
    softegg_cat.spam() << "got textures\n";
  }
}

//
//
//
