// Filename: softNodeDesc.cxx
// Created by:  masad (03Oct03)
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

#include "softNodeDesc.h"
#include "config_softegg.h"
#include "eggGroup.h"
#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "softToEggConverter.h"
#include "dcast.h"

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

  // set the _parentJoint to Null
  _parentJoint = NULL;

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
  numTexTri = NULL;
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
  // I think it is a mistake to try to delete this.  This was one
  // member of an entire array allocated at once; you can't delete
  // individual elements of an array.

  // Screw cleanup, anyway--we'll just let the array leak.
  /*
  if (_model != (SAA_Elem *)NULL) {
    delete _model;
  }
  */
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
//     Function: SoftNodeDesc::set_parent
//       Access: Public
//  Description: Sometimes, parent is not known at node creation
//               As soon as it is known, set the parent
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
force_set_parent(SoftNodeDesc *parent) {
  if (_parent)
    softegg_cat.spam() << " current parent " << _parent->get_name();

  _parent = parent;
  
  if (_parent)
    softegg_cat.spam() << " new parent " << _parent->get_name() << endl;

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
  //  return _joint_type == JT_joint || _joint_type == JT_pseudo_joint;
  return _joint_type == JT_joint;
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
    softegg_cat.debug() << "matched " << name << " ";
    return false;
  }
  // if name is not search_prefix, look in its parent
  if (strstr(name, search_prefix) == NULL) {
    softegg_cat.debug() << "node " << name << " ";
    if (_parent) 
      return _parent->is_partial(search_prefix);
  }
  // neither name nor its parent is search_prefix
  return true;
}

///////////////////////////////////////////////////////////////////////
//     Function: SoftNodeTree::set_parentJoint
//       Access: Public
//  Description: Go through the ancestors and figure out who is the 
//               immediate _parentJoint of this node
///////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
set_parentJoint(SAA_Scene *scene, SoftNodeDesc *lastJoint) {
  if (is_junk())
    return;
  //set its parent joint to the lastJoint
  _parentJoint = lastJoint;
  softegg_cat.spam() << get_name() << ": parent joint set to :" << lastJoint;
  if (lastJoint)
    softegg_cat.spam() << "(" << lastJoint->get_name() << ")";
  softegg_cat.spam() << endl;

  // is this node a joint?
  SAA_Boolean isSkeleton = false;
  if (has_model())
    SAA_modelIsSkeleton( scene, get_model(), &isSkeleton );
  
  // if  already a joint or name has "joint" in it
  const char *name = get_name().c_str();
  if (is_joint() || isSkeleton || strstr(name, "joint") != NULL) {
    lastJoint = this;
  }
  if ( _parentJoint && strstr( _parentJoint->get_name().c_str(), "scale" ) != NULL ) {
    // make sure _parentJoint didn't have the name "joint" in it
    if (strstr(_parentJoint->get_name().c_str(), "joint") == NULL) {
      _parentJoint = NULL;
      //    _parentJoint = lastJoint = NULL;
      softegg_cat.spam() << "scale joint flag set!\n";
    }
  }

  // look in the children
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    SoftNodeDesc *child = (*ci);
    child->set_parentJoint(scene, lastJoint);
  }
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
    softegg_cat.debug() << "pseudo " << get_name() << " case1\n";
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
          softegg_cat.debug() << "pseudo " << child->get_name() << " case2 by parent " << get_name() << "\n";
        } else if (child->_joint_type == JT_none || child->_joint_type == JT_junk) {
          all_joints = false;
        }
      }

      if (all_joints || any_joints) {
        // Finally, if all children or at least one is a joint, then we are too.
        if (_joint_type == JT_joint_parent) {
          _joint_type = JT_pseudo_joint;
          softegg_cat.debug() << "pseudo " << get_name() << " case3\n";
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

  if (!global && _parentJoint && !stec.flatten && !scale_joint) {

    SAA_modelGetMatrix( scene, get_model(), SAA_COORDSYS_LOCAL,  matrix );
    softegg_cat.debug() << get_name() << " using local matrix :parent ";

  } else {

    SAA_modelGetMatrix( scene, get_model(), SAA_COORDSYS_GLOBAL,  matrix );
    softegg_cat.debug() << get_name() << " using global matrix :parent ";

  }

  if (_parentJoint && !stec.flatten)
    softegg_cat.debug() << _parentJoint->get_name() << endl;
  else
    softegg_cat.debug() << _parentJoint << endl;
    

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
      egg_group->set_transform3d(m4d);
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
    PN_stdfloat i,j,k;
    PN_stdfloat h,p,r;
    PN_stdfloat x,y,z;
    int scale_joint = 0;

    softegg_cat.spam() << "\n\nanimating child " << name << endl;

    if (_parentJoint && !stec.flatten && !scale_joint ) {
      softegg_cat.debug() << "using local matrix\n";

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
      softegg_cat.debug() << " using global matrix\n";

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

    // Encode the component multiplication ordering in the egg file.
    // SoftImage always uses this order, regardless of the setting of
    // temp-hpr-fix.
    anim->set_order("sphrt");

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
    softegg_cat.debug() << "Cannot build anim table - no skeleton\n";
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
    softegg_cat.debug() << "\tbailing on model: " << name << "\n";
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
      softegg_cat.info() << "Not enough Memory for triangles...\n";
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
      softegg_cat.info() << "Not enough Memory for materials...\n";
      exit(1);
    }
    softegg_cat.spam() << "got materials\n";
    
    /***********************************************************************************/
    
    // allocate array of textures per triangle
    numTexTri = new int[numTri];
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
      uScale = new PN_stdfloat[numTri];
      vScale = new PN_stdfloat[numTri];
      uOffset = new PN_stdfloat[numTri];
      vOffset = new PN_stdfloat[numTri];
      texNameArray = new char *[numTri];
      uRepeat = new int[numTri];
      vRepeat = new int[numTri];
      
      // ASSUME only one texture per material
      textures = new SAA_Elem[numTri];
      
      for ( i = 0; i < numTri; i++ ) {
        // and read all referenced local textures into array
        SAA_materialRelationGetT2DLocElements( scene, &materials[i],
                                               TEX_PER_MAT , &textures[i] );

        // initialize the array value
        texNameArray[i] = NULL;
        // initialize the repeats
        uRepeat[i] = vRepeat[i] = 0;
        
        // see if this triangle has texture info
        if (numTexTri[i] == 0)
          continue;

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
          
          SAA_texture2DGetRepeats( scene, &textures[i], &uRepeat[i], &vRepeat[i] );
          softegg_cat.spam() << "uRepeat = " << uRepeat[i] << ", vRepeat = " << vRepeat[i] << endl;
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

          uRepeat = new int;
          vRepeat = new int;
          
          softegg_cat.spam() << " global tex named: " << *texNameArray << endl;
          
          // allocate arrays of texture info
          uScale = new PN_stdfloat;
          vScale = new PN_stdfloat;
          uOffset = new PN_stdfloat;
          vOffset = new PN_stdfloat;
          
          SAA_texture2DGetUScale( scene, textures, uScale );
          SAA_texture2DGetVScale( scene, textures, vScale );
          SAA_texture2DGetUOffset( scene, textures, uOffset );
          SAA_texture2DGetVOffset( scene, textures, vOffset );
          
          softegg_cat.spam() << " global tex uScale: " << *uScale << " vScale: " << *vScale << endl;
          softegg_cat.spam() << "            uOffset: " << *uOffset << " vOffset: " << *vOffset << endl;
          
          SAA_texture2DGetRepeats(  scene, textures, uRepeat, vRepeat );
          softegg_cat.spam() << "uRepeat = " << *uRepeat << ", vRepeat = " << *vRepeat << endl;
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
      softegg_cat.info() << "Out Of Memory on allocating materials\n";
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
      uScale = new PN_stdfloat;
      vScale = new PN_stdfloat;
      uOffset = new PN_stdfloat;
      vOffset = new PN_stdfloat;
      uRepeat = new int;
      vRepeat = new int;
      
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
        
        SAA_texture2DGetUScale( scene, &textures[0], uScale );
        SAA_texture2DGetVScale( scene, &textures[0], vScale );
        SAA_texture2DGetUOffset( scene, &textures[0], uOffset );
        SAA_texture2DGetVOffset( scene, &textures[0], vOffset );
        
        softegg_cat.spam() << "tritex[0] uScale: " << *uScale << " vScale: " << *vScale << endl;
        softegg_cat.spam() << " uOffset: " << *uOffset << " vOffset: " << *vOffset << endl;
        
        SAA_texture2DGetRepeats( scene, &textures[0], uRepeat, vRepeat );
        softegg_cat.spam() << "uRepeat = " << *uRepeat << ", vRepeat = " << *vRepeat << endl;
      }
      else {
        softegg_cat.spam() << "Invalid texture...\n";
        softegg_cat.spam() << " tritex[0] named: (null)\n";
      }
    }
    
    softegg_cat.spam() << "got textures\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: find_shape_vert
//       Access: Public
//  Description: given a vertex, find its corresponding shape vertex
//               and return its index.
////////////////////////////////////////////////////////////////////
int SoftNodeDesc::
find_shape_vert(LPoint3d p3d, SAA_DVector *vertices, int numVert) {
  int i, found = 0;

  for (i = 0; i < numVert && !found ; i++) {
    if ((p3d[0] == vertices[i].x) && 
        (p3d[1] == vertices[i].y) && 
        (p3d[2] == vertices[i].z)) {
      found = 1;
      softegg_cat.spam() << "found shape vert at index " << i << endl;
    }
  }

  if (!found )
    i = -1;
  else
    i--;

  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: make_vertex_offsets
//       Access: Public 
//  Description: Given a scene, a model , the vertices of its original
//               shape and its name find the difference between the 
//               geometry of its key shapes and the models original 
//               geometry and add morph vertices to the egg data to 
//               reflect these changes.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
make_vertex_offsets(int numShapes) {
  int i, j;
  int offset;
  int numCV;
  char tableName[_MAX_PATH];
  SAA_DVector *shapeVerts = NULL;
  SAA_DVector *uniqueVerts = NULL;
  SAA_Elem *model = get_model();
  SAA_Scene *scene = &stec.scene;

  EggVertexPool *vpool = NULL;
  string vpool_name = get_name() + ".verts";
  EggNode *t = stec._tree.get_egg_root()->find_child(vpool_name);
  if (t)
    DCAST_INTO_V(vpool, t);

  int numOrigVert = (int) vpool->size();
  EggVertexPool::iterator vi;

  if ((type == SAA_MNSRF) && stec.make_nurbs)
    SAA_nurbsSurfaceSetStep( scene, model, stec.nurbs_step, stec.nurbs_step );

  SAA_modelGetNbVertices( scene, model, &numCV );

  // get the shape verts
  uniqueVerts = new SAA_DVector[numCV];
  SAA_modelGetVertices( scene, model, SAA_GEOM_ORIGINAL, 0,
                        numCV, uniqueVerts );

  softegg_cat.spam() << numCV << " CV's\n";

  for ( i = 0; i < numCV; i++ )
    // convert vertices to global
    _VCT_X_MAT( uniqueVerts[i], uniqueVerts[i], matrix);
    softegg_cat.spam() << "uniqueVerts[" << i << "] = " << uniqueVerts[i].x << " " << uniqueVerts[i].y
         << " " << uniqueVerts[i].z << " " << uniqueVerts[i].w << endl;

  // iterate through for each key shape (except original)
  for ( i = 1; i < numShapes; i++ ) {
    
    sprintf(tableName, "%s.%d", get_name().c_str(), i);

    softegg_cat.spam() << "\nMaking geometry offsets for " << tableName << "...\n";

    if ((type == SAA_MNSRF) && stec.make_nurbs)
      softegg_cat.spam() << "calculating NURBS morphs...\n";
    else 
      softegg_cat.spam() << "calculating triangle morphs...\n";
    
    // get the shape verts
    shapeVerts = new SAA_DVector[numCV];
    SAA_modelGetVertices( scene, model, SAA_GEOM_SHAPE, i+1, numCV, shapeVerts );

    for ( j=0; j < numCV; j++ ) {
      // convert vertices to global
      _VCT_X_MAT( shapeVerts[j], shapeVerts[j], matrix);
    
      softegg_cat.spam() << "shapeVerts[" << j << "] = " << shapeVerts[j].x << " " 
           << shapeVerts[j].y << " " << shapeVerts[j].z << endl;
    }
    softegg_cat.spam() << endl;

    // for every original vertex, compare to the corresponding
    // key shape vertex and see if a vertex offset is needed 
    j = 0;
    for (vi = vpool->begin(); vi != vpool->end(); ++vi, ++j) {

      double dx, dy, dz;
      EggVertex *vert = (*vi);
      LPoint3d p3d = vert->get_pos3();
      
      softegg_cat.spam() << "oVert[" << j << "] = " <<  p3d[0] << " " <<  p3d[1] << " " <<  p3d[2] << endl;
      if ((type == SAA_MNSRF) && stec.make_nurbs) {
        dx = shapeVerts[j].x - p3d[0]; 
        dy = shapeVerts[j].y - p3d[1]; 
        dz = shapeVerts[j].z - p3d[2]; 

        softegg_cat.spam() << "global shapeVerts[" << j << "] = " << shapeVerts[j].x << " "
             << shapeVerts[j].y << " " << shapeVerts[j].z << " " << shapeVerts[j].w << endl;
      }
      else {
        // we need to map from original vertices
        // to triangle shape vertices here
        offset = find_shape_vert(p3d, uniqueVerts, numCV);

        dx = shapeVerts[offset].x - p3d[0]; 
        dy = shapeVerts[offset].y - p3d[1]; 
        dz = shapeVerts[offset].z - p3d[2]; 

        softegg_cat.spam() << "global shapeVerts[" << offset << "] = " << shapeVerts[offset].x << " "
             << shapeVerts[offset].y << " " << shapeVerts[offset].z << endl;
      }

      softegg_cat.spam() << j << ": dx = " << dx << ", dy = " << dy << ", dz = " << dz << endl;

      // if change isn't negligible, make a morph vertex entry 
      double total = fabs(dx)+fabs(dy)+fabs(dz);
      if ( total > 0.00001 ) {
        if ( vpool != NULL ) {
          // create offset
          LVector3d p(dx, dy, dz);
          EggMorphVertex *dxyz = new EggMorphVertex(tableName, p);
          // add the offset to the vertex
          vert->_dxyzs.insert(*dxyz);
        }
        else
          softegg_cat.spam() << "Error: couldn't find vertex pool " << vpool_name << endl; 
                
      } // if total
    } //for j
  } //for i
}

////////////////////////////////////////////////////////////////////
//     Function: make_morph_table
//       Access: Public 
//  Description: Given a scene, a model, a name and a frame time,
//               determine what type of shape interpolation is
//               used and call the appropriate function to extract
//               the shape weight info for this frame...
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
make_morph_table(  PN_stdfloat time ) {
  int numShapes;
  SAA_Elem *model = NULL;
  SAA_AnimInterpType type;
  SAA_Scene *scene = &stec.scene;
  
  if (has_model())
    model = get_model();
  else 
    return;

  // Get the number of key shapes
  SAA_modelGetNbShapes( scene, model, &numShapes );

  if ( numShapes <= 0 ) {
    return;
  }

  stec.has_morph = true;

  softegg_cat.spam() << "make_morph_table: " << get_name() << " : num shapes: " << numShapes << endl;

  SAA_modelGetShapeInterpolation( scene, model, &type );

  if ( type == SAA_ANIM_LINEAR || type == SAA_ANIM_CARDINAL ) {
    softegg_cat.spam() << "linear morph" << endl;
    make_linear_morph_table( numShapes, time );
  }
  else {    // must be weighted...
    // check first for expressions
    softegg_cat.spam() << "expression morph" << endl;
    make_expression_morph_table( numShapes, time );
  }
}

////////////////////////////////////////////////////////////////////
//     Function: make_linear_morph_table
//       Access: Public 
//  Description: Given a scene, a model, its name, and the time,
//               get the shape fcurve for the model and determine
//               the shape weights for the given time and use them
//               to populate the morph table.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
make_linear_morph_table(int numShapes, PN_stdfloat time) {    
  int i;
  PN_stdfloat curveVal;
  char tableName[_MAX_PATH];
  SAA_Elem fcurve;
  //SAnimTable *thisTable;
  EggSAnimData *anim;
  SAA_Elem *model = get_model();
  SAA_Scene *scene = &stec.scene;

  softegg_cat.spam() << "linear interp, getting fcurve\n";

  SAA_modelFcurveGetShape( scene, model, &fcurve );

  SAA_fcurveEval( scene, &fcurve, time, &curveVal );    
    
  softegg_cat.spam() << "at time " << time << ", fcurve for " << get_name() << " = " << curveVal << endl;

  PN_stdfloat nextVal = 0.0f;

  // populate morph table values for this frame
  for ( i = 1; i < numShapes; i++ ) {
    // derive table name from the model name
    sprintf(tableName, "%s.%d", get_name().c_str(), i);

    softegg_cat.spam() << "Linear: looking for table '" << tableName << "'\n";

    //find the morph table associated with this key shape
    anim = stec.find_morph_table(tableName);

    if ( anim != NULL ) {
      if ( i == (int)curveVal ) {
        if ( curveVal - i == 0 ) {
          anim->add_data(1.0f ); 
          softegg_cat.spam() << "adding element 1.0f\n";
        }
        else {
          anim->add_data(1.0f - (curveVal - i));
          nextVal = curveVal - i;
          softegg_cat.spam() << "adding element " << 1.0f - (curveVal - i) << endl;
        }
      }
      else {
        if ( nextVal ) {
          anim->add_data(nextVal );
          nextVal = 0.0f;
          softegg_cat.spam() << "adding element " << nextVal << endl;
        }
        else {
          anim->add_data(0.0f);
          softegg_cat.spam() << "adding element 0.0f\n";
        }
      }
      
      softegg_cat.spam() <<" to '" << tableName << "'\n";
    }
    else
      softegg_cat.spam() << i << " : Couldn't find table '" << tableName << "'\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: make_weighted_morph_table
//       Access: Public 
//  Description: Given a scene, a model, a list of all models in the
//               scene, the number of models in the scece, the number 
//               of key shapes for this model, the name of the model
//               and the current time, determine what method of
//               controlling the shape weights is used and call the
//               appropriate routine.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
make_weighted_morph_table(int numShapes, PN_stdfloat time) {
  PN_stdfloat curveVal;
  SI_Error result;
  char tableName[_MAX_PATH];
  SAA_Elem *weightCurves;
  //SAnimTable *thisTable;
  EggSAnimData *anim;
  SAA_Elem *model = get_model();
  SAA_Scene *scene = &stec.scene;

  // allocate array of weight curves (one for each shape)
  weightCurves = new SAA_Elem[numShapes]; 

  result = SAA_modelFcurveGetShapeWeights(scene, model, numShapes, weightCurves);

  if ( result == SI_SUCCESS ) {
    for ( int i = 1; i < numShapes; i++ ) {
      SAA_fcurveEval( scene, &weightCurves[i], time, &curveVal );    

      // make sure soft gave us a reasonable number
      //if (!isNum(curveVal))
      //curveVal = 0.0f;
      
      softegg_cat.spam() << "at time " << time << ", weightCurve[" << i << "] for " << get_name() << " = " << curveVal << endl;
      
      // derive table name from the model name
      sprintf(tableName, "%s.%d", get_name().c_str(), i);
      
      // find and populate shape table
      softegg_cat.spam() << "Weight: looking for table '" << tableName << "'\n";
      
      //find the morph table associated with this key shape
      anim = stec.find_morph_table(tableName);
      
      if ( anim != NULL ) {    
        anim->add_data(curveVal); 
        softegg_cat.spam() << "adding element " << curveVal << endl;
      }
      else
        softegg_cat.spam() << i << " : Couldn't find table '" << tableName << "'\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: make_expression_morph_table
//       Access: Public 
//  Description: Given a scene, a model and its number of key shapes
//               generate a morph table describing transitions btwn
//               the key shapes by evaluating the positions of the
//               controlling sliders. 
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
make_expression_morph_table(int numShapes, PN_stdfloat time)
{    
  //int j;
  int numExp;
  char *track;
  //PN_stdfloat expVal;
  //PN_stdfloat sliderVal;
  //char *tableName;
  //char *sliderName;
  //SAnimTable *thisTable;
  SAA_Elem *expressions;
  SI_Error result;

  SAA_Elem *model = get_model();
  SAA_Scene *scene = &stec.scene;

  // populate morph table values for this frame

  // compose track name
  track = NULL;

  // find how many expressions for this shape
  SAA_elementGetNbExpressions( scene, model, track, FALSE, &numExp );

  softegg_cat.spam() << get_name() << " has " << numExp << " RHS expressions\n";

  if ( numExp ) {
    // get the expressions for this shape
    expressions = new SAA_Elem[numExp];
    softegg_cat.spam() << "getting " << numExp << " RHS expressions...\n";

    result = SAA_elementGetExpressions( scene, model, track, FALSE,
                                        numExp, expressions );
    /*
    if ( !result ) {
      for ( j = 1; j < numExp; j++ ) {
        if ( verbose >= 2 )
                {
                // debug see what we got
                int numvars;
        
                SAA_expressionGetNbVars( scene, &expressions[j], &numvars );

                int *varnamelen;
                int *varstrlen;
                int  expstrlen;

                varnamelen = (int *)malloc(sizeof(int)*numvars);
                varstrlen = (int *)malloc(sizeof(int)*numvars);

                SAA_expressionGetStringLengths( scene, &expressions[j],
                    numvars, varnamelen, varstrlen, &expstrlen );    

                int *varnamesizes;    
                int *varstrsizes;

                varnamesizes = (int *)malloc(sizeof(int)*numvars);
                varstrsizes = (int *)malloc(sizeof(int)*numvars);

                for ( int k = 0; k < numvars; k++ )
                {
                    varnamesizes[k] = varnamelen[k] + 1;
                    varstrsizes[k] = varstrlen[k] + 1;
                }
    
                int expstrsize = expstrlen + 1;

                char **varnames;
                char **varstrs;

                varnames = (char **)malloc(sizeof(char *)*numvars);
                varstrs = (char **)malloc(sizeof(char *)*numvars);

                for ( k = 0; k < numvars; k++ )
                {
                    varnames[k] = (char *)malloc(sizeof(char)*
                        varnamesizes[k]);

                    varstrs[k] = (char *)malloc(sizeof(char)*
                        varstrsizes[k]);
                }
        
                char *expstr = (char *)malloc(sizeof(char)* expstrsize );    

                SAA_expressionGetStrings( scene, &expressions[j], numvars,
                    varnamesizes, varstrsizes, expstrsize, varnames,
                    varstrs, expstr );
                
                if ( verbose >= 2 )
                {
                    fprintf( outStream, "expression = '%s'\n", expstr );
                    fprintf( outStream, "has %d variables\n", numvars );
                }
                } //if verbose
                
                if ( verbose >= 2 )
                    fprintf( outStream, "evaling expression...\n" );

                SAA_expressionEval( scene, &expressions[j], time, &expVal ); 

                if ( verbose >= 2 )
                    fprintf( outStream, "time %f: exp val %f\n", 
                        time, expVal );

                // derive table name from the model name
                tableName = MakeTableName( name, j );

                if ( verbose >= 2 )
                    fprintf( outStream, "Exp: looking for table '%s'\n", 
                        tableName );

                //find the morph table associated with this key shape
                anim = (SAnimTable *)
                    (morphRoot->FindDescendent( tableName ));

                if ( anim != NULL )
                {    
                    anim->AddElement( expVal ); 
                    if ( verbose >= 1 )    
                        fprintf( outStream, "%d: adding element %f to %s\n",
                            j, expVal, tableName );
                    fflush( outStream );
                }
                else
                {
                    fprintf( outStream, "%d: Couldn't find table '%s'", j, 
                            tableName ); 

                    fprintf( outStream, " for value %f\n", expVal );
                }
            }
        }
        else
            fprintf( outStream, "couldn't get expressions!!!\n" );
    */
  }
  else {
    softegg_cat.spam() << "weighted morph" << endl;
    // no expression, use weight curves
    make_weighted_morph_table(numShapes, time );
  }
}

//
//
//
