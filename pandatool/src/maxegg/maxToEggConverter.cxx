// Filename: maxToEggConverter.cxx
// Created by Corey Revilla and Ken Strickland (6/22/03)
// from mayaToEggConverter.cxx created by drose (10Nov99)
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


#define MNEG Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM2
#define MNEG_GEOMETRY_GENERATION Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM3

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MaxToEggConverter::
MaxToEggConverter()
{
    reset();
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MaxToEggConverter::
~MaxToEggConverter() 
{
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::reset
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::reset() {
    _transform_type = TT_model;
    _cur_tref = 0;
    _current_frame = 0;
    _textures.clear();
    _egg_data = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::convert
//       Access: Public
//  Description: Fills up the egg_data structure according to the
//               global Max model data.  Returns true if successful,
//               false if there is an error.  If from_selection is
//               true, the converted geometry is based on that which
//               is selected; otherwise, it is the entire Max scene.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::convert(MaxEggOptions *options) {

    _options = options;

    _egg_data = new EggData;
    if (_egg_data->get_coordinate_system() == CS_default) {
        _egg_data->set_coordinate_system(CS_zup_right);
    }
    
    // Figure out the animation parameters.
    
    // Get the start and end frames and the animation frame rate from Max
    
    Interval anim_range = _options->_max_interface->GetAnimRange();
    int start_frame = anim_range.Start()/GetTicksPerFrame();
    int end_frame = anim_range.End()/GetTicksPerFrame();
    
    if (_options->_start_frame < start_frame) _options->_start_frame = start_frame;
    if (_options->_start_frame > end_frame)   _options->_start_frame = end_frame;
    if (_options->_end_frame < start_frame)   _options->_end_frame = start_frame;
    if (_options->_end_frame > end_frame)     _options->_end_frame = end_frame;
    if (_options->_end_frame < _options->_start_frame)  _options->_end_frame = _options->_start_frame;
    
    int frame_inc = 1;
    int output_frame_rate = GetFrameRate();

    bool all_ok = true;

    if (_options->_export_whole_scene) {
        all_ok = _tree.build_complete_hierarchy(_options->_max_interface->GetRootNode(), NULL, 0);
    } else {
        all_ok = _tree.build_complete_hierarchy(_options->_max_interface->GetRootNode(), &_options->_node_list.front(), _options->_node_list.size());
    }

    if (all_ok) {
        switch (_options->_anim_type) {
        case MaxEggOptions::AT_pose:
            // pose: set to a specific frame, then get out the static geometry.
            sprintf(Logger::GetLogString(), "Extracting geometry from frame #%d.",
                    start_frame); 
            Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, Logger::GetLogString() );
            _current_frame = start_frame;
            Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Converting static model." );
            all_ok = convert_hierarchy(_egg_data);
            break;
            
        case MaxEggOptions::AT_model:
            // model: get out an animatable model with joints and vertex
            // membership.
            all_ok = convert_char_model();
            break;
            
        case MaxEggOptions::AT_chan:
            // chan: get out a series of animation tables.
            all_ok = convert_char_chan(start_frame, end_frame, frame_inc,
                                       output_frame_rate);
            break;
            
        case AC_both:
            
            _options->_anim_type = MaxEggOptions::AT_model;
            if (!convert_char_model()) {
                all_ok = false;
            }
            _options->_anim_type = MaxEggOptions::AT_chan;
            if (!convert_char_chan(start_frame, end_frame, frame_inc,
                                   output_frame_rate)) {
                all_ok = false;
            }
            break;
        };
        
        reparent_decals(_egg_data);
    }
    
    if (all_ok) {
        Filename fn = Filename::from_os_specific(_options->_file_name);
        return _egg_data->write_egg(fn);
    } else {
        return false;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::convert_char_model
//       Access: Private
//  Description: Converts the file as an animatable character
//               model, with joints and vertex membership.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
convert_char_model() {
    std::string character_name = "character";
    _current_frame = _options->_start_frame;

    EggGroup *char_node = new EggGroup(character_name);
    _egg_data->add_child(char_node);
    char_node->set_dart_type(EggGroup::DT_default);
    
    return convert_hierarchy(char_node);
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::convert_char_chan
//       Access: Private
//  Description: Converts the animation as a series of tables to apply
//               to the character model, as retrieved earlier via
//               AC_model.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
convert_char_chan(double start_frame, double end_frame, double frame_inc,
                  double output_frame_rate) {
    std::string character_name = "character";

    EggTable *root_table_node = new EggTable();
    _egg_data->add_child(root_table_node);
    EggTable *bundle_node = new EggTable(character_name);
    bundle_node->set_table_type(EggTable::TT_bundle);
    root_table_node->add_child(bundle_node);
    EggTable *skeleton_node = new EggTable("<skeleton>");
    bundle_node->add_child(skeleton_node);

    // Set the frame rate before we start asking for anim tables to be
    // created.
    _tree._fps = output_frame_rate / frame_inc;
    _tree.clear_egg(_egg_data, NULL, skeleton_node);
    
    // Now we can get the animation data by walking through all of the
    // frames, one at a time, and getting the joint angles at each
    // frame.
    
    // This is just a temporary EggGroup to receive the transform for
    // each joint each frame.
    EggGroup* tgroup;
    
    int num_nodes = _tree.get_num_nodes();
    int i;
    
    sprintf(Logger::GetLogString(), 
            "sf %lf ef %lf inc %lf ofr %lf.", 
            start_frame, end_frame, frame_inc, output_frame_rate );
    Logger::Log(MNEG_GEOMETRY_GENERATION, Logger::SAT_LOW_LEVEL, 
                Logger::GetLogString() );
    
    TimeValue frame = start_frame;
    TimeValue frame_stop = end_frame;
    while (frame <= frame_stop) {
        _current_frame = frame;
        sprintf(Logger::GetLogString(), 
                "Current frame: %lf.", 
                _current_frame );
        Logger::Log(MNEG_GEOMETRY_GENERATION, Logger::SAT_LOW_LEVEL, 
                    Logger::GetLogString() );
        
        for (i = 0; i < num_nodes; i++) {
            // Find all joints in the hierarchy
            MaxNodeDesc *node_desc = _tree.get_node(i);
            if (node_desc->is_joint()) {
                tgroup = new EggGroup();
                INode *max_node = node_desc->get_max_node();
                
                if (node_desc->_parent && node_desc->_parent->is_joint()) {
                    // If this joint also has a joint as a parent, the parent's 
                    // transformation has to be divided out of this joint's TM
                    get_joint_transform(max_node, node_desc->_parent->get_max_node(), 
                                        tgroup);
                } else {
                    get_joint_transform(max_node, NULL, tgroup);
                }
                
                EggXfmSAnim *anim = _tree.get_egg_anim(node_desc);
                if (!anim->add_data(tgroup->get_transform3d())) {
                    // *** log an error
                }
                delete tgroup;
            }
        }
        
        frame += frame_inc;
    }
    
    // Now optimize all of the tables we just filled up, for no real
    // good reason, except that it makes the resulting egg file a little
    // easier to read.
    for (i = 0; i < num_nodes; i++) {
        MaxNodeDesc *node_desc = _tree.get_node(i);
        if (node_desc->is_joint()) {
            _tree.get_egg_anim(node_desc)->optimize();
        }
    }
    
    return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::convert_hierarchy
//       Access: Private
//  Description: Generates egg structures for each node in the Max
//               hierarchy.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
convert_hierarchy(EggGroupNode *egg_root) {
    //int num_nodes = _tree.get_num_nodes();
    
    _tree.clear_egg(_egg_data, egg_root, NULL);
    for (int i = 0; i < _tree.get_num_nodes(); i++) {
        if (!process_model_node(_tree.get_node(i))) {
            return false;
        }
    }
    
    return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::process_model_node
//       Access: Private
//  Description: Converts the indicated Max node to the
//               corresponding Egg structure.  Returns true if
//               successful, false if an error was encountered.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
process_model_node(MaxNodeDesc *node_desc) {
  if (!node_desc->has_max_node()) {
    // If the node has no Max equivalent, never mind.
    return true;
  }

  // Skip all nodes that represent joints in the geometry, but aren't 
  // the actual joints themselves
  if (node_desc->is_node_joint()) {
    return true;
  }

  TimeValue time = 0;
  INode *max_node = node_desc->get_max_node();

  ObjectState state;
  state = max_node->EvalWorldState(_current_frame * GetTicksPerFrame());

  if (node_desc->is_joint()) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    // Don't bother with joints unless we're getting an animatable
    // model.
    if (_options->_anim_type == MaxEggOptions::AT_model) { 
        get_joint_transform(max_node, egg_group);
    }
  } else {
    if (state.obj) {
      EggGroup *egg_group = NULL;
      TriObject *myMaxTriObject;
      Mesh max_mesh;
      //Call the correct exporter based on what type of object this is.
      switch( state.obj->SuperClassID() ){
        //A geometric object.
        case GEOMOBJECT_CLASS_ID:
          Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, 
                       "Found a geometric object in the hierarchy!" );
    egg_group = _tree.get_egg_group(node_desc);
          get_transform(max_node, egg_group);
    
    //Try converting this geometric object to a mesh we can use.
          if (!state.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) {
            Logger::Log(MTEC, Logger::SAT_OTHER_ERROR, 
                        "Cannot create geometry from state.obj!");
            // Logger::FunctionExit();
            return false;
          } 
          //Convert our state object to a TriObject.
          myMaxTriObject = (TriObject *) state.obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0 ));
          // *** Want to figure this problem out 
          // If actual conversion was required, then we want to delete this 
          // new mesh later to avoid mem leaks. **BROKEN. doesnt delete

          //Now, get the mesh.
          max_mesh = myMaxTriObject->GetMesh();
          Logger::Log( MTEC, Logger::SAT_LOW_LEVEL, 
                       "TriObject attached and mesh generated!" );

          make_polyset(max_node, &max_mesh, egg_group);

          if (myMaxTriObject != state.obj)
            delete myMaxTriObject;
          break;

        case SHAPE_CLASS_ID:
    if (state.obj->ClassID() == EDITABLE_SURF_CLASS_ID) {
            Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, 
                         "Found a NURB object in the hierarchy!" );
      NURBSSet getSet;
      if (GetNURBSSet(state.obj, time, getSet, TRUE)) {
        NURBSObject *nObj = getSet.GetNURBSObject(0);
        if (nObj->GetType() == kNCVCurve) {
          //It's a CV Curve, process it
          egg_group = _tree.get_egg_group(node_desc);
          get_transform(max_node, egg_group);
          make_nurbs_curve((NURBSCVCurve *)nObj, string(max_node->GetName()),
                           time, egg_group);
        }
      }
    }
    break;

        case CAMERA_CLASS_ID:
          Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, 
                       "Found a camera object in the hierarchy!" );
          break;

        case LIGHT_CLASS_ID:
          Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, 
                       "Found a light object in the hierarchy!" );
          break;

        case HELPER_CLASS_ID:
          Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, 
                       "Found a helper object in the hierarchy!" );
          break;
/*        default:
          char buf[1024];
          sprintf(buf, "Unknown Superclass ID: %x, ClassID: %x,%x", state.obj->SuperClassID(),
            state.obj->ClassID().PartA(), state.obj->ClassID().PartB());
          Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, 
                       buf ); */

      }
    }
  }

  // *** Check if any of these Maya options have a Max equivalent
  /*
  } else if (dag_node.inUnderWorld()) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring underworld node " << path
        << "\n";
    }

  } else if (dag_node.isIntermediateObject()) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring intermediate object " << path
        << "\n";
    }

  } else if (dag_path.hasFn(MFn::kNurbsSurface)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(dag_path, egg_group);
    
    MFnNurbsSurface surface(dag_path, &status);
    if (!status) {
      mayaegg_cat.info()
        << "Error in node " << path
        << ":\n"
        << "  it appears to have a NURBS surface, but does not.\n";
    } else {
      make_nurbs_surface(dag_path, surface, egg_group);
    }

  } else if (dag_path.hasFn(MFn::kNurbsCurve)) {
    // Only convert NurbsCurves if we aren't making an animated model.
    // Animated models, as a general rule, don't want these sorts of
    // things in them.
    if (_options->_anim_type != MaxEggOptions::AT_model) {
      EggGroup *egg_group = _tree.get_egg_group(node_desc);
      get_transform(dag_path, egg_group);
      
      MFnNurbsCurve curve(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << path << ":\n"
          << "  it appears to have a NURBS curve, but does not.\n";
      } else {
        make_nurbs_curve(dag_path, curve, egg_group);
      }
    }
      
  } else if (dag_path.hasFn(MFn::kMesh)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(dag_path, egg_group);

    MFnMesh mesh(dag_path, &status);
    if (!status) {
      mayaegg_cat.info()
        << "Error in node " << path << ":\n"
        << "  it appears to have a polygon mesh, but does not.\n";
    } else {
      make_polyset(dag_path, mesh, egg_group);
    }

  } else if (dag_path.hasFn(MFn::kLocator)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);

    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Locator at " << path << "\n";
    }
    
    // Presumably, the locator's position has some meaning to the
    // end-user, so we will implicitly tag it with the DCS flag so it
    // won't get flattened out.
    if (_options->_anim_type != MaxEggOptions::AT_model) {
      // For now, don't set the DCS flag on locators within
      // character models, since egg-optchar doesn't understand
      // this.  Perhaps there's no reason to ever change this, since
      // locators within character models may not be meaningful.
      egg_group->set_dcs_type(EggGroup::DC_net);
    }
    get_transform(dag_path, egg_group);
    make_locator(dag_path, dag_node, egg_group);

  } else {
    // Just a generic node.
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(dag_path, egg_group);
  }
  */
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::get_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               and applies it to the corresponding Egg node.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
get_transform(INode *max_node, EggGroup *egg_group) {
  if (_options->_anim_type == MaxEggOptions::AT_model) {
    // When we're getting an animated model, we only get transforms
    // for joints.
    return;
  }

  // The 3dsMax-style matrix that contains the pivot matrix...this pivot matrix
  // encapsulates all the scales, rotates, and transforms it takes
  // to "get to" the pivot point.
  Matrix3 pivot;
  //This is the Panda-flava-flav-style matrix we'll be exporting to.
  Point3 row0;
  Point3 row1;
  Point3 row2;
  Point3 row3;

  Logger::FunctionEntry("MaxNodeEggGroup::ApplyTransformFromMaxNodeToEggGroup");
  if ( !egg_group ) {
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_NULL_ERROR, 
                 "Destination EggGroup is null!" );
    Logger::FunctionExit();
    return;
  }

  // *** A special case that I don't think we're supporting in Max
  /*
  // A special case: if the group is a billboard, we center the
  // transform on the rotate pivot and ignore whatever transform might
  // be there.
  if (egg_group->get_billboard_type() != EggGroup::BT_none) {
    MFnTransform transform(transformNode, &status);
    if (!status) {
      status.perror("MFnTransform constructor");
      return;
    }

    MPoint pivot = transform.rotatePivot(MSpace::kObject, &status);
    if (!status) {
      status.perror("Can't get rotate pivot");
      return;
    }

    // We need to convert the pivot to world coordinates.
    // Unfortunately, Maya can only tell it to us in local
    // coordinates.
    MMatrix mat = dag_path.inclusiveMatrix(&status);
    if (!status) {
      status.perror("Can't get coordinate space for pivot");
      return;
    }
    LMatrix4d n2w(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                  mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                  mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                  mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    LPoint3d p3d(pivot[0], pivot[1], pivot[2]);
    p3d = p3d * n2w;

    if (egg_group->get_parent() != (EggGroupNode *)NULL) {
      // Now convert the pivot point into the group's parent's space.
      p3d = p3d * egg_group->get_parent()->get_vertex_frame_inv();
    }

    egg_group->clear_transform();
    egg_group->add_translate(p3d);
    return;
  }
  */

  // *** Make sure this is happening correctly; I'm not sure at the moment
  //     how these flags get specified
/*
  switch (_transform_type) {
  case TT_all:
    break;
    
  case TT_model:
    if (!egg_group->get_model_flag() &&
        !egg_group->has_dcs_type()) {
      return;
    }
    break;
    
  case TT_dcs: 
    if (!egg_group->get_dcs_type()) {
      return;
    }
    break;
    
  case TT_none:
  case TT_invalid:
    return;
  }
*/

  // Gets the TM for this node, a matrix which encapsulates all transformations
  // it takes to get to the current node, including parent transformations.
  pivot = max_node->GetNodeTM(_current_frame * GetTicksPerFrame());
  row0 = pivot.GetRow(0);
  row1 = pivot.GetRow(1);
  row2 = pivot.GetRow(2);
  row3 = pivot.GetRow(3);

  LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                row1.x, row1.y, row1.z, 0.0f,
                row2.x, row2.y, row2.z, 0.0f,
                row3.x, row3.y, row3.z, 1.0f );
  // Now here's the tricky part. I believe this command strips out the node
  // "frame" which is the sum of all transformations enacted by the parent of
  // this node. This should reduce to the transformation relative to this 
  // node's parent
  m4d = m4d * egg_group->get_node_frame_inv();
  if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
    egg_group->add_matrix4(m4d);
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_DEBUG_SPAM_LEVEL, 
                 "Non-identity matrix applied to node!" );
  } else {
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_DEBUG_SPAM_LEVEL, 
                 "Resultant matrix too close to identity; no transformation applied!" );
  }
  Logger::FunctionExit();
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::get_object_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               and applies it to the corresponding Egg node.
////////////////////////////////////////////////////////////////////
LMatrix4d MaxToEggConverter::
get_object_transform(INode *max_node) {

  // The 3dsMax-style matrix that contains the pivot matrix...this pivot matrix
  // encapsulates all the scales, rotates, and transforms it takes
  // to "get to" the pivot point.
  Matrix3 pivot;
  //This is the Panda-flava-flav-style matrix we'll be exporting to.
  Point3 row0;
  Point3 row1;
  Point3 row2;
  Point3 row3;

  Logger::FunctionEntry("MaxNodeEggGroup::ApplyTransformFromMaxNodeToEggGroup");

  // Gets the TM for this node, a matrix which encapsulates all transformations
  // it takes to get to the current node, including parent transformations.
  pivot = max_node->GetObjectTM(_current_frame * GetTicksPerFrame());
  row0 = pivot.GetRow(0);
  row1 = pivot.GetRow(1);
  row2 = pivot.GetRow(2);
  row3 = pivot.GetRow(3);

  LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                row1.x, row1.y, row1.z, 0.0f,
                row2.x, row2.y, row2.z, 0.0f,
                row3.x, row3.y, row3.z, 1.0f );
  Logger::FunctionExit();
  return m4d;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::get_joint_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               as appropriate for a joint in an animated character,
//               and applies it to the indicated node.  This is
//               different from get_transform() in that it does not
//               respect the _transform_type flag, and it does not
//               consider the relative transforms within the egg file.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
get_joint_transform(INode *max_node, EggGroup *egg_group) {

  // The 3dsMax-style matrix that contains the pivot matrix...this pivot matrix
  // encapsulates all the scales, rotates, and transforms it takes
  // to "get to" the pivot point.
  Matrix3 pivot;
  //This is the Panda-flava-flav-style matrix we'll be exporting to.
  Point3 row0;
  Point3 row1;
  Point3 row2;
  Point3 row3;

  Logger::FunctionEntry("MaxNodeEggGroup::ApplyTransformFromMaxNodeToEggGroup");
  if ( !egg_group ) {
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_NULL_ERROR, 
                 "Destination EggGroup is null!" );
    Logger::FunctionExit();
    return;
  }

  // Gets the TM for this node, a matrix which encapsulates all transformations
  // it takes to get to the current node, including parent transformations.
  pivot = max_node->GetNodeTM(_current_frame * GetTicksPerFrame());
  row0 = pivot.GetRow(0);
  row1 = pivot.GetRow(1);
  row2 = pivot.GetRow(2);
  row3 = pivot.GetRow(3);

  LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                row1.x, row1.y, row1.z, 0.0f,
                row2.x, row2.y, row2.z, 0.0f,
                row3.x, row3.y, row3.z, 1.0f );
  // Now here's the tricky part. I believe this command strips out the node
  // "frame" which is the sum of all transformations enacted by the parent of
  // this node. This should reduce to the transformation relative to this 
  // node's parent
  m4d = m4d * egg_group->get_node_frame_inv();
  if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
    egg_group->add_matrix4(m4d);
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_DEBUG_SPAM_LEVEL, 
                 "Non-identity matrix applied to node!" );
  } else {
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_DEBUG_SPAM_LEVEL, 
                 "Resultant matrix too close to identity; no transformation applied!" );
  }
  Logger::FunctionExit();
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::get_joint_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               as appropriate for a joint in an animated character,
//               and applies it to the indicated node.  This is
//               different from get_transform() in that it does not
//               respect the _transform_type flag, and it does not
//               consider the relative transforms within the egg file.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
get_joint_transform(INode *max_node, INode *parent_node, EggGroup *egg_group) {

  // The 3dsMax-style matrix that contains the pivot matrix...this pivot matrix
  // encapsulates all the scales, rotates, and transforms it takes
  // to "get to" the pivot point.
  Matrix3 pivot;
  Matrix3 parent_pivot;
  //This is the Panda-flava-flav-style matrix we'll be exporting to.
  Point3 row0;
  Point3 row1;
  Point3 row2;
  Point3 row3;

  Logger::FunctionEntry("MaxNodeEggGroup::ApplyTransformFromMaxNodeToEggGroup");
  if ( !egg_group ) {
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_NULL_ERROR, 
                 "Destination EggGroup is null!" );
    Logger::FunctionExit();
    return;
  }

  // Gets the TM for this node, a matrix which encapsulates all transformations
  // it takes to get to the current node, including parent transformations.
  pivot = max_node->GetNodeTM(_current_frame * GetTicksPerFrame());
  row0 = pivot.GetRow(0);
  row1 = pivot.GetRow(1);
  row2 = pivot.GetRow(2);
  row3 = pivot.GetRow(3);

  LMatrix4d m4d(row0.x, row0.y, row0.z, 0.0f,
                row1.x, row1.y, row1.z, 0.0f,
                row2.x, row2.y, row2.z, 0.0f,
                row3.x, row3.y, row3.z, 1.0f );

if (parent_node) {
  parent_pivot = parent_node->GetNodeTM(_current_frame * GetTicksPerFrame());
//  parent_pivot.Invert();
  row0 = parent_pivot.GetRow(0);
  row1 = parent_pivot.GetRow(1);
  row2 = parent_pivot.GetRow(2);
  row3 = parent_pivot.GetRow(3);

  LMatrix4d pi_m4d(row0.x, row0.y, row0.z, 0.0f,
                row1.x, row1.y, row1.z, 0.0f,
                row2.x, row2.y, row2.z, 0.0f,
                row3.x, row3.y, row3.z, 1.0f );

  // Now here's the tricky part. I believe this command strips out the node
  // "frame" which is the sum of all transformations enacted by the parent of
  // this node. This should reduce to the transformation relative to this 
  // node's parent
  pi_m4d.invert_in_place();
  m4d = m4d * pi_m4d;
}
  if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
    egg_group->add_matrix4(m4d);
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_DEBUG_SPAM_LEVEL, 
                 "Non-identity matrix applied to node!" );
  } else {
    Logger::Log( MNEG_GEOMETRY_GENERATION, Logger::SAT_DEBUG_SPAM_LEVEL, 
                 "Resultant matrix too close to identity; no transformation applied!" );
  }
  Logger::FunctionExit();
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::make_nurbs_curve
//       Access: Private
//  Description: Converts the indicated Maya NURBS curve (a standalone
//               curve, not a trim curve) to a corresponding egg
//               structure and attaches it to the indicated egg group.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
make_nurbs_curve(NURBSCVCurve *curve, const string &name,
                 TimeValue time, EggGroup *egg_group) 
{
  int degree = curve->GetOrder();
  int cvs = curve->GetNumCVs();
  int knots = curve->GetNumKnots();
  int i;

  if (knots != cvs + degree) {
    char buf[1024];
    sprintf(buf, "NURBS knots count incorrect. Order %d, CVs %d, knots %d", degree, cvs, knots);
    Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, buf);
    return false;
  }

  string vpool_name = name + ".cvs";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsCurve *egg_curve = new EggNurbsCurve(name);
  egg_group->add_child(egg_curve);
  egg_curve->setup(degree, knots);

  for (i = 0; i < knots; i++)
    egg_curve->set_knot(i, curve->GetKnot(i));

  LMatrix4d vertex_frame_inv = egg_group->get_vertex_frame_inv();

  for (i = 0; i < cvs; i++) {
    NURBSControlVertex *cv = curve->GetCV(i);
    if (!cv) {
      char buf[1024];
      sprintf(buf, "Error getting CV %d", i);
      Logger::Log( MTEC, Logger::SAT_HIGH_LEVEL, buf);
      return false;
    } else {
      EggVertex vert;
      LPoint4d p4d(0, 0, 0, 1.0);
      cv->GetPosition(time, p4d[0], p4d[1], p4d[2]);
      p4d = p4d * vertex_frame_inv;
      vert.set_pos(p4d);
      egg_curve->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::make_polyset
//       Access: Private
//  Description: Converts the indicated Maya polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
make_polyset(INode *max_node, Mesh *mesh,
             EggGroup *egg_group, Shader *default_shader) {
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Entered make_poly_set." );

  // *** I think this needs to have a plugin written to support
  /*
  MObject mesh_object = mesh.object();
  bool double_sided = false;
  get_bool_attribute(mesh_object, "doubleSided", double_sided);
  */

  mesh->buildNormals();

  if (mesh->getNumFaces() == 0) {
    Logger::Log(MNEG_GEOMETRY_GENERATION, Logger::SAT_MEDIUM_LEVEL, 
                "Ignoring empty mesh ");
    return;
  }

  string vpool_name = string(max_node->GetName()) + ".verts";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  // One way to convert the mesh would be to first get out all the
  // vertices in the mesh and add them into the vpool, then when we
  // traverse the polygons we would only have to index them into the
  // vpool according to their Maya vertex index.

  // Unfortunately, since Maya may store multiple normals and/or
  // colors for each vertex according to which polygon it is in, that
  // approach won't necessarily work.  In egg, those split-property
  // vertices have to become separate vertices.  So instead of adding
  // all the vertices up front, we'll start with an empty vpool, and
  // add vertices to it on the fly.

  // *** Need to find out if I even need to deal with shaders
  /*
  MObjectArray shaders;
  MIntArray poly_shader_indices;

  status = mesh.getConnectedShaders(dag_path.instanceNumber(),
                                    shaders, poly_shader_indices);
  if (!status) {
    status.perror("MFnMesh::getConnectedShaders");
  }
  */

  // We will need to transform all vertices from world coordinate
  // space into the vertex space appropriate to this node.  Usually,
  // this is the same thing as world coordinate space, and this matrix
  // will be identity; but if the node is under an instance
  // (particularly, for instance, a billboard) then the vertex space
  // will be different from world space.
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Before obtaining transform." );
 LMatrix4d vertex_frame = get_object_transform(max_node) * 
                           egg_group->get_vertex_frame_inv();
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "After obtaining transform." );


  // *** Not quite sure how this vertex color flag is handled.  Check on later
  /*
  // Save this modeling flag for the vertex color check later (see the
  // comment below).
  bool egg_vertex_color = false;
  if (egg_group->has_user_data(MayaEggGroupUserData::get_class_type())) {
    egg_vertex_color = 
      DCAST(MayaEggGroupUserData, egg_group->get_user_data())->_vertex_color;
  }
  */

  for ( int iFace=0; iFace < mesh->getNumFaces(); iFace++ ) {
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Getting face." );
   EggPolygon *egg_poly = new EggPolygon;
    egg_group->add_child(egg_poly);

    egg_poly->set_bface_flag(_options->_double_sided);

    Face face = mesh->faces[iFace];

    // *** Once again skipping shaders until I determine if I need them
    /*
    // Determine the shader for this particular polygon.
    MayaShader *shader = NULL;
    int index = pi.index();
    nassertv(index >= 0 && index < (int)poly_shader_indices.length());
    int shader_index = poly_shader_indices[index];
    if (shader_index != -1) {
      nassertv(shader_index >= 0 && shader_index < (int)shaders.length());
      MObject engine = shaders[shader_index];
      shader =
        _shaders.find_shader_for_shading_engine(engine);

    } else if (default_shader != (MayaShader *)NULL) {
      shader = default_shader;
    }

    const MayaShaderColorDef &color_def = shader->_color;
    */

    // Should we extract the color from the vertices?  Normally, in
    // Maya a texture completely replaces the vertex color, so we
    // should ignore the vertex color if we have a texture. 

    // However, this is an inconvenient property of Maya; sometimes we
    // really do want both vertex color and texture applied to the
    // same object.  To allow this, we define the special egg flag
    // "vertex-color", which when set indicates that we should
    // respect the vertex color anyway.

    // *** Ignoring vertex colors for now
    bool ignore_vertex_color = true;
    /*
    bool ignore_vertex_color = false;
    if (shader != (MayaShader *)NULL) {
      ignore_vertex_color = color_def._has_texture && !egg_vertex_color;
    }
    */

    // *** More shader stuff to ignore
    /*
    LPoint3d centroid(0.0, 0.0, 0.0);

    if (shader != (MayaShader *)NULL && color_def.has_projection()) {
      // If the shader has a projection, we may need to compute the
      // polygon's centroid to avoid seams at the edges.
      for (i = 0; i < num_verts; i++) {
        MPoint p = pi.point(i, MSpace::kWorld);
        LPoint3d p3d(p[0], p[1], p[2]);
        p3d = p3d * vertex_frame_inv;
        centroid += p3d;
      }
      centroid /= (double)num_verts;
    }
    */

   // Get the vertices for the polygon.
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Before getting vertices." );

        for ( int iVertex=0; iVertex < 3; iVertex++ ) {
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Getting vertex." );

                EggVertex vert;

      // Get the vertex position
      Point3 vertex = mesh->getVert(face.v[iVertex]);
      LPoint3d p3d(vertex.x, vertex.y, vertex.z);
      p3d = p3d * vertex_frame;
      vert.set_pos(p3d);
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "After getting vertex pos before getting normal." );


      // Get the vertex normal
      Point3 normal = get_max_vertex_normal(mesh, iFace, iVertex);
      LVector3d n3d(normal.x, normal.y, normal.z);
      // *** Not quite sure if this transform should be applied, but it may 
      //     explain why normals were weird previously
      n3d = n3d * vertex_frame;
      vert.set_normal(n3d);
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "After getting normal." );

      // *** More shader stuff to ignore for now
      /*
      if (shader != (MayaShader *)NULL && color_def.has_projection()) {
        // If the shader has a projection, use it instead of the
        // polygon's built-in UV's.
        vert.set_uv(color_def.project_uv(p3d, centroid));

      } else if (pi.hasUVs()) {
        // Get the UV's from the polygon.
        float2 uvs;
        status = pi.getUV(i, uvs);
        if (!status) {
          status.perror("MItMeshPolygon::getUV");
        } else {
          vert.set_uv(TexCoordd(uvs[0], uvs[1]));
        }
      }
      */

      // Get the UVs for this vertex
      if (mesh->getNumTVerts()) {
        UVVert vertTexCoord = mesh->getTVert(mesh->tvFace[iFace].t[iVertex]);
        vert.set_uv( TexCoordd(vertTexCoord.x, vertTexCoord.y));
        sprintf(Logger::GetLogString(), 
                "Got tex vertex %d of %d from local tex data.", 
                iVertex, mesh->getNumTVerts() );
        Logger::Log(MNEG_GEOMETRY_GENERATION, Logger::SAT_LOW_LEVEL, 
                    Logger::GetLogString() );
      }
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "after getting TVerts." );


      // *** Leaving out vertex colors for now
      /*
      if (pi.hasColor() && !ignore_vertex_color) {
        MColor c;
        status = pi.getColor(c, i);
        if (!status) {
          status.perror("MItMeshPolygon::getColor");
        } else {
          vert.set_color(Colorf(c.r, c.g, c.b, 1.0));
        }
      }
      */

      vert.set_external_index(face.v[iVertex]);

      egg_poly->add_vertex(vpool->create_unique_vertex(vert));
    }

  Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Fixing windings" );
  //Max uses normals, not winding, to determine which way a 
  //polygon faces. Make sure the winding and that normal agree
  EggVertex *verts[3];
  LPoint3d points[3];

  for (int i = 0; i < 3; i++) {
    verts[i] = egg_poly->get_vertex(i);
    points[i] = verts[i]->get_pos3();
  }

  LVector3d realNorm = ((points[1] - points[0]).cross( 
                         points[2] - points[0]));
  Point3 maxNormTemp = mesh->getFaceNormal(iFace);
  LVector3d maxNorm = (LVector3d(maxNormTemp.x, maxNormTemp.y, maxNormTemp.z) *
                       vertex_frame);

  if (realNorm.dot(maxNorm) < 0.0) {
    egg_poly->set_vertex(0, verts[2]);
    egg_poly->set_vertex(2, verts[0]);
  }

        // *** More shader stuff to ignore
        /*
    // Now apply the shader.
    if (shader != (MayaShader *)NULL) {
      set_shader_attributes(*egg_poly, *shader);
    }
        */
        Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Before set_material_attributes" );

    set_material_attributes(*egg_poly, max_node->GetMtl(), &face);
  }
   
  // Now that we've added all the polygons (and created all the
  // vertices), go back through the vertex pool and set up the
  // appropriate joint membership for each of the vertices.

  if (_options->_anim_type == MaxEggOptions::AT_model) {
      get_vertex_weights(max_node, vpool);
  }

}

// *** I don't know if there is a Max equivalent to this.  I will implement 
//     this if I find one
/*
////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::make_locator
//       Access: Private
//  Description: Locators are used in Maya to indicate a particular
//               position in space to the user or the modeler.  We
//               represent that in egg with an ordinary Group node,
//               which we transform by the locator's position, so that
//               the indicated point becomes the origin at this node
//               and below.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
             EggGroup *egg_group) {
  MStatus status;

  unsigned int num_children = dag_node.childCount();
  MObject locator;
  bool found_locator = false;
  for (unsigned int ci = 0; ci < num_children && !found_locator; ci++) {
    locator = dag_node.child(ci);
    found_locator = (locator.apiType() == MFn::kLocator);
  }

  if (!found_locator) {
    mayaegg_cat.error()
      << "Couldn't find locator within locator node " 
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  LPoint3d p3d;
  if (!get_vec3d_attribute(locator, "localPosition", p3d)) {
    mayaegg_cat.error()
      << "Couldn't get position of locator " 
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  // We need to convert the position to world coordinates.  For some
  // reason, Maya can only tell it to us in local coordinates.
  MMatrix mat = dag_path.inclusiveMatrix(&status);
  if (!status) {
    status.perror("Can't get coordinate space for locator");
    return;
  }
  LMatrix4d n2w(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
  p3d = p3d * n2w;

  // Now convert the locator point into the group's space.
  p3d = p3d * egg_group->get_node_frame_inv();

  egg_group->add_translate(p3d);
}
*/

Point3 MaxToEggConverter::get_max_vertex_normal(Mesh *mesh, int faceNo, int vertNo)
{
  Face f = mesh->faces[faceNo];
  DWORD smGroup = f.smGroup;
  int vert = f.getVert(vertNo);
  RVertex *rv = mesh->getRVertPtr(vert);
  
  int numNormals;
  Point3 vertexNormal;

  // Is normal specified
  // SPCIFIED is not currently used, but may be used in future versions.
  if (rv->rFlags & SPECIFIED_NORMAL) {
    vertexNormal = rv->rn.getNormal();
  }
  // If normal is not specified it's only available if the face belongs
  // to a smoothing group
  else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) {
    // If there is only one vertex is found in the rn member.
    if (numNormals == 1) {
      vertexNormal = rv->rn.getNormal();
    }
    else {
      // If two or more vertices are there you need to step through them
      // and find the vertex with the same smoothing group as the current face.
      // You will find multiple normals in the ern member.
      for (int i = 0; i < numNormals; i++) {
        if (rv->ern[i].getSmGroup() & smGroup) {
          vertexNormal = rv->ern[i].getNormal();
        }
      }
    }
  }
  else {
    // Get the normal from the Face if no smoothing groups are there
    vertexNormal = mesh->getFaceNormal(faceNo);
  }
  
  return vertexNormal;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::get_vertex_weights
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
get_vertex_weights(INode *max_node, EggVertexPool *vpool) {
  //Try to get the weights out of a physique if one exists
  Modifier *mod = FindSkinModifier(max_node, PHYSIQUE_CLASSID);
  EggVertexPool::iterator vi;

  if (mod) {
    // create a physique export interface
    IPhysiqueExport *pPhysiqueExport = (IPhysiqueExport *)mod->GetInterface(I_PHYINTERFACE);
    if (pPhysiqueExport) {
      // create a context export interface
      IPhyContextExport *pContextExport = 
        (IPhyContextExport *)pPhysiqueExport->GetContextInterface(max_node);
      if (pContextExport) {
        // set the flags in the context export interface
        pContextExport->ConvertToRigid(TRUE);
        pContextExport->AllowBlending(TRUE);
  
        for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
          EggVertex *vert = (*vi);
          int max_vi = vert->get_external_index();

          // get the vertex export interface
          IPhyVertexExport *pVertexExport = 
            (IPhyVertexExport *)pContextExport->GetVertexInterface(max_vi);
          if (pVertexExport) {
            int vertexType = pVertexExport->GetVertexType();

            // handle the specific vertex type
            if(vertexType == RIGID_TYPE) {
              // typecast to rigid vertex
              IPhyRigidVertex *pTypeVertex = (IPhyRigidVertex *)pVertexExport;
              INode *bone_node = pTypeVertex->GetNode();
              MaxNodeDesc *joint_node_desc = _tree.find_joint(bone_node);
              if (joint_node_desc){
                EggGroup *joint = _tree.get_egg_group(joint_node_desc);
                if (joint != (EggGroup *)NULL)
                  joint->ref_vertex(vert, 1.0f);
                else
                  Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, bone_node->GetName() );
              }
            }
            else if(vertexType == RIGID_BLENDED_TYPE) {
              // typecast to blended vertex
              IPhyBlendedRigidVertex *pTypeVertex = (IPhyBlendedRigidVertex *)pVertexExport;

              for (int ji = 0; ji < pTypeVertex->GetNumberNodes(); ++ji) {
                float weight = pTypeVertex->GetWeight(ji);
                if (weight > 0.0f) {
                  INode *bone_node = pTypeVertex->GetNode(ji);
                  MaxNodeDesc *joint_node_desc = _tree.find_joint(bone_node);
                  if (joint_node_desc){
                    EggGroup *joint = _tree.get_egg_group(joint_node_desc);
                    if (joint != (EggGroup *)NULL)
                      joint->ref_vertex(vert, weight);
                    else
                      Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, bone_node->GetName() );
                  }
                }
              }
            }
            //Release the vertex interface
            pContextExport->ReleaseVertexInterface(pVertexExport);
          }
        }
        //Release the context interface
        pPhysiqueExport->ReleaseContextInterface(pContextExport);
      }
      //Release the physique export interface
      mod->ReleaseInterface(I_PHYINTERFACE, pPhysiqueExport);
    }
  }
  else {
    //No physique, try to find a skin
    mod = FindSkinModifier(max_node, SKIN_CLASSID);
    if (mod) {
      ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);
      if (skin) {
        ISkinContextData *skinMC = skin->GetContextInterface(max_node);
        if (skinMC) {
          for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
            EggVertex *vert = (*vi);
            int max_vi = vert->get_external_index();
  
            for (int ji = 0; ji < skinMC->GetNumAssignedBones(max_vi); ++ji) {
              float weight = skinMC->GetBoneWeight(max_vi, ji);
              if (weight > 0.0f) {
                INode *bone_node = skin->GetBone(skinMC->GetAssignedBone(max_vi, ji));
                MaxNodeDesc *joint_node_desc = _tree.find_joint(bone_node);
                if (joint_node_desc){
                  EggGroup *joint = _tree.get_egg_group(joint_node_desc);
                  if (joint != (EggGroup *)NULL) {
                    joint->ref_vertex(vert, weight);
                  } else {
                    Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, bone_node->GetName() );
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

// *** More ignored shader stuff.  I am replacing this with 
//     set_material_attributes for now
/*
////////////////////////////////////////////////////////////////////
//     Function: MayaShader::set_shader_attributes
//       Access: Private
//  Description: Applies the known shader attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader) {
  // In Maya, a polygon is either textured or colored.  The texture,
  // if present, replaces the color.
  const MayaShaderColorDef &color_def = shader._color;
  const MayaShaderColorDef &trans_def = shader._transparency;
  if (color_def._has_texture || trans_def._has_texture) {
    EggTexture tex(shader.get_name(), "");

    if (color_def._has_texture) {
      // If we have a texture on color, apply it as the filename.
      Filename filename = Filename::from_os_specific(color_def._texture);
      Filename fullpath,outpath;
      _options->_path_replace->full_convert_path(filename, get_texture_path(),
                                                  fullpath, outpath);
      tex.set_filename(outpath);
      tex.set_fullpath(fullpath);
      apply_texture_properties(tex, color_def);

      // If we also have a texture on transparency, apply it as the
      // alpha filename.
      if (trans_def._has_texture) {
        if (color_def._wrap_u != trans_def._wrap_u ||
            color_def._wrap_u != trans_def._wrap_u) {
          mayaegg_cat.warning()
            << "Shader " << shader.get_name()
            << " has contradictory wrap modes on color and texture.\n";
        }
          
        if (!compare_texture_properties(tex, trans_def)) {
          // Only report each broken shader once.
          static pset<string> bad_shaders;
          if (bad_shaders.insert(shader.get_name()).second) {
            mayaegg_cat.error()
              << "Color and transparency texture properties differ on shader "
              << shader.get_name() << "\n";
          }
        }
        tex.set_format(EggTexture::F_rgba);
          
        // We should try to be smarter about whether the transparency
        // value is connected to the texture's alpha channel or to its
        // grayscale channel.  However, I'm not sure how to detect
        // this at the moment; rather than spending days trying to
        // figure out, for now I'll just assume that if the same
        // texture image is used for both color and transparency, then
        // the artist meant to use the alpha channel for transparency.
        if (trans_def._texture == color_def._texture) {
          // That means that we don't need to do anything special: use
          // all the channels of the texture.

        } else {
          // Otherwise, pull the alpha channel from the other image
          // file.  Ideally, we should figure out which channel from
          // the other image supplies alpha (and specify this via
          // set_alpha_file_channel()), but for now we assume it comes
          // from the grayscale data.
          filename = Filename::from_os_specific(trans_def._texture);
          _path_replace->full_convert_path(filename, get_texture_path(),
                                           fullpath, outpath);
          tex.set_alpha_filename(outpath);
          tex.set_alpha_fullpath(fullpath);
        }

      } else {
        // If there is no transparency texture specified, we don't
        // have any transparency, so tell the egg format to ignore any
        // alpha channel that might be on the color texture.
        tex.set_format(EggTexture::F_rgb);
      }

    } else {  // trans_def._has_texture
      // We have a texture on transparency only.  Apply it as the
      // primary filename, and set the format accordingly.
      Filename filename = Filename::from_os_specific(trans_def._texture);
      Filename fullpath, outpath;
      _path_replace->full_convert_path(filename, get_texture_path(),
                                       fullpath, outpath);
      tex.set_filename(outpath);
      tex.set_fullpath(fullpath);
      tex.set_format(EggTexture::F_alpha);
      apply_texture_properties(tex, trans_def);
    }
  
    EggTexture *new_tex =
      _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);
    
    primitive.set_texture(new_tex);

  }

  // Also apply an overall color to the primitive.
  Colorf rgba = shader.get_rgba();

  // The existence of a texture on either color channel completely
  // replaces the corresponding flat color.
  if (color_def._has_texture) {
    rgba[0] = 1.0f;
    rgba[1] = 1.0f;
    rgba[2] = 1.0f;
  }
  if (trans_def._has_texture) {
    rgba[3] = 1.0f;
  }

  // But the color gain always gets applied.
  rgba[0] *= color_def._color_gain[0];
  rgba[1] *= color_def._color_gain[1];
  rgba[2] *= color_def._color_gain[2];
  rgba[3] *= color_def._color_gain[3];

  primitive.set_color(rgba);
}
*/

/*
////////////////////////////////////////////////////////////////////
//     Function: MayaShader::set_material_attributes
//       Access: Private
//  Description: Applies the known shader attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
set_material_attributes(EggPrimitive &primitive, Mtl *maxMaterial, Face *face) {
  Bitmap *maxBitmap;
  BitmapTex *maxBitmapTex;
//  Mtl *maxMaterial;
  StdMat *maxStandardMaterial;
  Texmap *maxTexmap;
  EggTexture *myEggTexture = null;
  string outString;
  string outHandle;
  bool has_diffuse_texture = false;
  bool has_trans_texture = false;
  
  Point3 diffuseColor = Point3(1, 1, 1);

  Logger::FunctionEntry( "MaxToEggConverter::CreateEggTextureFromINode" );

  //First, get the material data associated with this node.
//  maxMaterial = max_node->GetMtl();
  if ( !maxMaterial ) {
    Logger::Log(MTEC, Logger::SAT_NULL_ERROR, "maxMaterial is null!");
    Logger::FunctionExit();
    return;
  }

  //Now, determine wether it's a standard or multi material
  if ( maxMaterial->ClassID() == Class_ID(DMTL_CLASS_ID, 0 )) {
      
    // Access the Diffuse map and see if it's a Bitmap texture
    maxStandardMaterial = (StdMat *)maxMaterial;
    maxTexmap = maxMaterial->GetSubTexmap(ID_DI);
    //Determine whether this texture is a bitmap.
    if (maxTexmap && (maxTexmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))) {
     ostringstream name_strm;
      name_strm << "Tex" << ++_cur_tref;
      EggTexture tex(name_strm.str(), "");

      // It is! 
      has_diffuse_texture = true;

      maxBitmapTex = (BitmapTex *) maxTexmap;
      Filename filename = Filename::from_os_specific(maxBitmapTex->GetMapName());
      Filename fullpath, outpath;
      _path_replace->full_convert_path(filename, get_texture_path(),
                                       fullpath, outpath);
      tex.set_filename(outpath);
      tex.set_fullpath(fullpath);
      apply_texture_properties(tex, maxStandardMaterial);
      // *** Must add stuff here for looking for transparencies
      maxBitmap = maxBitmapTex->GetBitmap(0);
      //Query some parameters of the bitmap to get the format option.
      if ( maxBitmap && maxBitmap->HasAlpha() ) {
        has_trans_texture = true;
        tex.set_format(EggTexture::F_rgba);
      } else {
        tex.set_format(EggTexture::F_rgb);
      }
      EggTexture *new_tex =
        _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);
    
      primitive.set_texture(new_tex);
    }

    // Also apply an overall color to the primitive.
    Colorf rgba(1.0f, 1.0f, 1.0f, 1.0f);

    // The existence of a texture on either color channel completely
    // replaces the corresponding flat color.
    if (!has_diffuse_texture) {
      // Get the default diffuse color of the material without the texture map
      diffuseColor = Point3(maxMaterial->GetDiffuse());
      rgba[0] = diffuseColor.x;
      rgba[1] = diffuseColor.y;
      rgba[2] = diffuseColor.z;
    }
    if (!has_trans_texture) {
      // *** Figure out how to actually get the opacity here
      rgba[3] = 1.0f;
    }

    // *** May need color gain, but I don't know what it is
    /*
    // But the color gain always gets applied.
    rgba[0] *= color_def._color_gain[0];
    rgba[1] *= color_def._color_gain[1];
    rgba[2] *= color_def._color_gain[2];
    rgba[3] *= color_def._color_gain[3];
    */
/*
    primitive.set_color(rgba);

  } else if ( maxMaterial->ClassID() == Class_ID(MULTI_CLASS_ID, 0 )) {
        // It's a multi-material.  Find the submaterial for this face.
    // and call set_material_attributes again on the submaterial.
        MtlID matID = face->getMatID();
        if (matID < maxMaterial->NumSubMtls()) {
          set_material_attributes(primitive, maxMaterial->GetSubMtl(matID), face);
        } else {
                sprintf(Logger::GetLogString(),
                            "SubMaterial ID %d is greater than the total submaterial for this material",
                                matID);
          Logger::Log(MTEC, Logger::SAT_NULL_ERROR, "maxMaterial is null!");
        }
  } else {
    // It's non-standard material. At the moment, let's just 
    // return
    Logger::FunctionExit();
    return;
  }

  Logger::FunctionExit();
}
*/

////////////////////////////////////////////////////////////////////
//     Function: MaxToEggConverter::set_material_attributes
//       Access: Private
//  Description: Applies the known material attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
set_material_attributes(EggPrimitive &primitive, Mtl *maxMaterial, Face *face) {
  Bitmap *diffuseBitmap;
  BitmapTex *diffuseBitmapTex;
  BitmapTex *transBitmapTex;
//  Mtl *maxMaterial;
  StdMat *maxStandardMaterial;
  Texmap *diffuseTexmap;
  Texmap *transTexmap;
  EggTexture *myEggTexture = null;
  string outString;
  string outHandle;
  bool has_diffuse_texture = false;
  bool has_trans_texture = false;
  
  Point3 diffuseColor = Point3(1, 1, 1);

  Logger::FunctionEntry( "MaxToEggConverter::CreateEggTextureFromINode" );

  //First, get the material data associated with this node.
//  maxMaterial = max_node->GetMtl();
  if ( !maxMaterial ) {
    Logger::Log(MTEC, Logger::SAT_NULL_ERROR, "maxMaterial is null!");
    Logger::FunctionExit();
    return;
  }

  //Now, determine wether it's a standard or multi material
  if ( maxMaterial->ClassID() == Class_ID(DMTL_CLASS_ID, 0 )) {
    // *** Eventuall we should probably deal with multi-materials
     
    maxStandardMaterial = (StdMat *)maxMaterial;

        // Access the Diffuse map and see if it's a Bitmap texture
    diffuseTexmap = maxMaterial->GetSubTexmap(ID_DI);
        if (diffuseTexmap && (diffuseTexmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))) {
      has_diffuse_texture = true;
          diffuseBitmapTex = (BitmapTex *) diffuseTexmap;
        }

    // Access the Opacity map and see if it's a Bitmap texture
        transTexmap = maxMaterial->GetSubTexmap(ID_OP);
        if (transTexmap && (transTexmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0))) {
      has_trans_texture = true;
          transBitmapTex = (BitmapTex *) transTexmap;
        }

    if (has_diffuse_texture || has_trans_texture) {
      ostringstream name_strm;
      name_strm << "Tex" << ++_cur_tref;
      EggTexture tex(name_strm.str(), "");
      
      if (has_diffuse_texture) {
        // It is! 
        Filename filename = Filename::from_os_specific(diffuseBitmapTex->GetMapName());
        Filename fullpath, outpath;
        _options->_path_replace->full_convert_path(filename, get_texture_path(),
                                                   fullpath, outpath);
        tex.set_filename(outpath);
        tex.set_fullpath(fullpath);
        apply_texture_properties(tex, maxStandardMaterial);
        // *** Must add stuff here for looking for transparencies
        diffuseBitmap = diffuseBitmapTex->GetBitmap(0);
        //Query some parameters of the bitmap to get the format option.
        if ( has_trans_texture ) {
          tex.set_format(EggTexture::F_rgba);
          if (stricmp(diffuseBitmapTex->GetMapName(),
                      transBitmapTex->GetMapName()) == 0) {
            // nothing more needs to be done
          } else {
            filename = Filename::from_os_specific(transBitmapTex->GetMapName());
            _options->_path_replace->full_convert_path(filename, get_texture_path(),
                                                       fullpath, outpath);
            tex.set_alpha_filename(outpath);
            tex.set_alpha_fullpath(fullpath);
          }
        } else {
          if ( diffuseBitmap && diffuseBitmap->HasAlpha()) {
            tex.set_format(EggTexture::F_rgba);
          } else {
            tex.set_format(EggTexture::F_rgb);
          }
        }
      } else {
        // We have a texture on transparency only.  Apply it as the
        // primary filename, and set the format accordingly.
        Filename filename = Filename::from_os_specific(transBitmapTex->GetMapName());
        Filename fullpath, outpath;
        _options->_path_replace->full_convert_path(filename, get_texture_path(),
                                                   fullpath, outpath);
        tex.set_filename(outpath);
        tex.set_fullpath(fullpath);
        tex.set_format(EggTexture::F_alpha);
        apply_texture_properties(tex, maxStandardMaterial);
      }
      EggTexture *new_tex =
        _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);
      
      primitive.set_texture(new_tex);
    }
    
    // Also apply an overall color to the primitive.
    Colorf rgba(1.0f, 1.0f, 1.0f, 1.0f);

    // The existence of a texture on either color channel completely
    // replaces the corresponding flat color.
    if (!has_diffuse_texture) {
      // Get the default diffuse color of the material without the texture map
      diffuseColor = Point3(maxMaterial->GetDiffuse());
      rgba[0] = diffuseColor.x;
      rgba[1] = diffuseColor.y;
      rgba[2] = diffuseColor.z;
    }
    if (!has_trans_texture) {
      // *** Figure out how to actually get the opacity here
      rgba[3] = maxStandardMaterial->GetOpacity(_current_frame * GetTicksPerFrame());
    }

    // *** May need color gain, but I don't know what it is
    /*
    // But the color gain always gets applied.
    rgba[0] *= color_def._color_gain[0];
    rgba[1] *= color_def._color_gain[1];
    rgba[2] *= color_def._color_gain[2];
    rgba[3] *= color_def._color_gain[3];
    */

    primitive.set_color(rgba);
    
  } else if ( maxMaterial->ClassID() == Class_ID(MULTI_CLASS_ID, 0 )) {
        // It's a multi-material.  Find the submaterial for this face.
    // and call set_material_attributes again on the submaterial.
        MtlID matID = face->getMatID();
        if (matID < maxMaterial->NumSubMtls()) {
          set_material_attributes(primitive, maxMaterial->GetSubMtl(matID), face);
        } else {
                sprintf(Logger::GetLogString(),
                            "SubMaterial ID %d is greater than the total submaterial for this material",
                                matID);
          Logger::Log(MTEC, Logger::SAT_NULL_ERROR, "maxMaterial is null!");
        }
  } else {
        // It's another non-standard material. At the moment, let's just 
    // return
    Logger::FunctionExit();
    return;
  }

  Logger::FunctionExit();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::apply_texture_properties
//       Access: Private
//  Description: Applies all the appropriate texture properties to the
//               EggTexture object, including wrap modes and texture
//               matrix.
////////////////////////////////////////////////////////////////////
void MaxToEggConverter::
apply_texture_properties(EggTexture &tex, StdMat *maxMaterial) {
  // Let's mipmap all textures by default.
  tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
  tex.set_magfilter(EggTexture::FT_linear);

  
  // *** Need to figure out how to get the wrap options from Max
  EggTexture::WrapMode wrap_u = EggTexture::WM_repeat;
  EggTexture::WrapMode wrap_v = EggTexture::WM_repeat;
  
  /*
  EggTexture::WrapMode wrap_u = color_def._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = color_def._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  */

  tex.set_wrap_u(wrap_u);
  tex.set_wrap_v(wrap_v);
  
  // *** I may need to find this too
  /*
  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(LMatrix3d::ident_mat())) {
    tex.set_transform(mat);
  }
  */
}

// *** I don't think I need this right now
/*
////////////////////////////////////////////////////////////////////
//     Function: MayaShader::compare_texture_properties
//       Access: Private
//  Description: Compares the texture properties already on the
//               texture (presumably set by a previous call to
//               apply_texture_properties()) and returns false if they
//               differ from that specified by the indicated color_def
//               object, or true if they match.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
compare_texture_properties(EggTexture &tex, 
                           const MayaShaderColorDef &color_def) {
  bool okflag = true;

  EggTexture::WrapMode wrap_u = color_def._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = color_def._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  
  if (wrap_u != tex.determine_wrap_u()) {
    // Choose the more general of the two.
    if (wrap_u == EggTexture::WM_repeat) {
      tex.set_wrap_u(wrap_u);
    }
    okflag = false;
  }
  if (wrap_v != tex.determine_wrap_v()) {
    if (wrap_v == EggTexture::WM_repeat) {
      tex.set_wrap_v(wrap_v);
    }
    okflag = false;
  }
  
  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(tex.get_transform())) {
    okflag = false;
  }

  return okflag;
}
*/


////////////////////////////////////////////////////////////////////
//     Function: MayaShader::reparent_decals
//       Access: Private
//  Description: Recursively walks the egg hierarchy, reparenting
//               "decal" type nodes below their corresponding
//               "decalbase" type nodes, and setting the flags.
//
//               Returns true on success, false if some nodes were
//               incorrect.
////////////////////////////////////////////////////////////////////
bool MaxToEggConverter::
reparent_decals(EggGroupNode *egg_parent) {
  bool okflag = true;

  // First, walk through all children of this node, looking for the
  // one decal base, if any.
  EggGroup *decal_base = (EggGroup *)NULL;
  pvector<EggGroup *> decal_children;

  EggGroupNode::iterator ci;
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = (EggGroup *) child;
      if (child_group->has_object_type("decalbase")) {
        if (decal_base != (EggNode *)NULL) {
          // error
          okflag = false;
        }
        child_group->remove_object_type("decalbase");
        decal_base = child_group;

      } else if (child_group->has_object_type("decal")) {
        child_group->remove_object_type("decal");
        decal_children.push_back(child_group);
      }
    }
  }

  if (decal_base == (EggGroup *)NULL) {
    if (!decal_children.empty()) {
      // warning
    }

  } else {
    if (decal_children.empty()) {
      // warning

    } else {
      // All the decal children get moved to be a child of decal base.
      // This usually will not affect the vertex positions, but it
      // could if the decal base has a transform and the decal child
      // is an instance node.  So don't do that.
      pvector<EggGroup *>::iterator di;
      for (di = decal_children.begin(); di != decal_children.end(); ++di) {
        EggGroup *child_group = (*di);
        decal_base->add_child(child_group);
      }

      // Also set the decal state on the base.
      decal_base->set_decal_flag(true);
    }
  }

  // Now recurse on each of the child nodes.
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *child_group = (EggGroupNode *) child;
      if (!reparent_decals(child_group)) {
        okflag = false;
      }
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::string_transform_type
//       Access: Public, Static
//  Description: Returns the TransformType value corresponding to the
//               indicated string, or TT_invalid.
////////////////////////////////////////////////////////////////////
MaxToEggConverter::TransformType MaxToEggConverter::
string_transform_type(const string &arg) {
  if (strcmp(arg.c_str(), "all") == 0) {
    return TT_all;
  } else if (strcmp(arg.c_str(), "model") == 0) {
    return TT_model;
  } else if (strcmp(arg.c_str(), "dcs") == 0) {
    return TT_dcs;
  } else if (strcmp(arg.c_str(), "none") == 0) {
    return TT_none;
  } else {
    return TT_invalid;
  }
}

Modifier* MaxToEggConverter::FindSkinModifier (INode* node, const Class_ID &type)
{
  // Get object from node. Abort if no object.
  Object* pObj = node->GetObjectRef();
  if (!pObj) return NULL;

  // Is derived object ?
  while (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
  {
    // Yes -> Cast.
    IDerivedObject* pDerObj = static_cast<IDerivedObject*>(pObj);

    // Iterate over all entries of the modifier stack.
    for (int stackId = 0; stackId < pDerObj->NumModifiers(); ++stackId)
    {
      // Get current modifier.
      Modifier* mod = pDerObj->GetModifier(stackId);

      // Is this what we are looking for?
                        if (mod->ClassID() == type )
                                return mod;
                }

    // continue with next derived object
    pObj = pDerObj->GetObjRef();
  }

  // Not found.
  return NULL;
}
