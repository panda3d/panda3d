// Filename: eggOptchar.cxx
// Created by:  drose (18Jul03)
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

#include "eggOptchar.h"

#include "eggOptcharUserData.h"
#include "dcast.h"
#include "eggJointData.h"
#include "eggSliderData.h"
#include "eggCharacterCollection.h"
#include "eggCharacterData.h"
#include "eggJointPointer.h"
#include "eggTable.h"
#include "compose_matrix.h"

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggOptchar::
EggOptchar() {
  add_path_replace_options();
  add_path_store_options();

  set_program_description
    ("egg-optchar performs basic optimizations of a character model "
     "and its associated animations, primarily by analyzing the "
     "animation tables and removing unneeded joints and/or morphs.  "
     "It can also perform basic restructuring operations on the "
     "character hierarchy.");

  add_option
    ("ls", "", 0,
     "List the joint hierarchy instead of performing any operations.",
     &EggOptchar::dispatch_none, &_list_hierarchy);
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggOptchar::
run() {
  int num_characters = _collection->get_num_characters();

  int ci;
  for (ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);

    nout << "Processing " << char_data->get_name() << "\n";
    analyze_joints(char_data->get_root_joint());
    analyze_sliders(char_data);
  }

  if (_list_hierarchy) {
    for (ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      nout << "Character: " << char_data->get_name() << "\n";
      list_joints(char_data->get_root_joint(), 0);
      list_scalars(char_data);
    }

  } else {
    // Now, trigger the actual rebuilding of all the joint data.
    for (ci = 0; ci < num_characters; ci++) {
      EggCharacterData *char_data = _collection->get_character(ci);
      char_data->get_root_joint()->do_rebuild();
    }
    
    write_eggs();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggOptchar::
handle_args(ProgramBase::Args &args) {
  if (_list_hierarchy) {
    _read_only = true;
  }

  return EggCharacterFilter::handle_args(args);
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::analyze_joints
//       Access: Private
//  Description: Recursively walks the joint hierarchy for a
//               particular character, indentifying properties of each
//               joint.
////////////////////////////////////////////////////////////////////
void EggOptchar::
analyze_joints(EggJointData *joint_data) {
  PT(EggOptcharUserData) user_data = new EggOptcharUserData;
  joint_data->set_user_data(user_data);

  // Analyze the table of matrices for this joint, checking to see if
  // they're all the same across all frames, or if any of them are
  // different; also look for empty joints (that control no vertices).
  int num_mats = 0;
  bool different_mat = false;
  bool has_vertices = false;

  int num_models = joint_data->get_num_models();
  for (int i = 0; i < num_models; i++) {
    if (joint_data->has_model(i)) {
      EggBackPointer *model = joint_data->get_model(i);
      if (model->has_vertices()) {
        has_vertices = true;
      }

      int num_frames = joint_data->get_num_frames(i);

      int f;
      for (f = 0; f < num_frames && !different_mat; f++) {
        LMatrix4d mat = joint_data->get_frame(i, f);
        num_mats++;
        if (num_mats == 1) {
          // This is the first matrix.
          user_data->_static_mat = mat;

        } else {
          // This is a second or later matrix.
          if (!mat.almost_equal(user_data->_static_mat)) {
            // It's different than the first one.
            different_mat = true;
          }
        }
      }
    }
  }

  if (!different_mat) {
    // All the mats are the same for this joint.
    user_data->_flags |= EggOptcharUserData::F_static;

    if (num_mats == 0 || 
        user_data->_static_mat.almost_equal(LMatrix4d::ident_mat())) {
      // It's not only static, but it's the identity matrix.
      user_data->_flags |= EggOptcharUserData::F_identity;
    }
  }

  if (!has_vertices) {
    // There are no vertices in this joint.
    user_data->_flags |= EggOptcharUserData::F_empty;
  }

  int num_children = joint_data->get_num_children();
  for (int i = 0; i < num_children; i++) {
    analyze_joints(joint_data->get_child(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::analyze_sliders
//       Access: Private
//  Description: Linearly walks the slider list for a particular
//               character, indentifying properties of each slider.
////////////////////////////////////////////////////////////////////
void EggOptchar::
analyze_sliders(EggCharacterData *char_data) {
  int num_sliders = char_data->get_num_sliders();
  for (int si = 0; si < num_sliders; si++) {
    EggSliderData *slider_data = char_data->get_slider(si);

    PT(EggOptcharUserData) user_data = new EggOptcharUserData;
    slider_data->set_user_data(user_data);

    // Analyze the table of values for this slider, checking to see if
    // they're all the same across all frames, or if any of them are
    // different; also look for empty sliders (that control no vertices).
    int num_values = 0;
    bool different_value = false;
    bool has_vertices = false;

    int num_models = slider_data->get_num_models();
    for (int i = 0; i < num_models; i++) {
      if (slider_data->has_model(i)) {
        EggBackPointer *model = slider_data->get_model(i);
        if (model->has_vertices()) {
          has_vertices = true;
        }
        
        int num_frames = slider_data->get_num_frames(i);
        
        int f;
        for (f = 0; f < num_frames && !different_value; f++) {
          double value = slider_data->get_frame(i, f);
          num_values++;
          if (num_values == 1) {
            // This is the first value.
            user_data->_static_value = value;
            
          } else {
            // This is a second or later value.
            if (!IS_NEARLY_EQUAL(value, user_data->_static_value)) {
              // It's different than the first one.
              different_value = true;
            }
          }
        }
      }
    }
    
    if (!different_value) {
      // All the values are the same for this slider.
      user_data->_flags |= EggOptcharUserData::F_static;
      
      if (num_values == 0 || IS_NEARLY_EQUAL(user_data->_static_value, 0.0)) {
        // It's not only static, but it's the identity value.
        user_data->_flags |= EggOptcharUserData::F_identity;
      }
    }
    
    if (!has_vertices) {
      // There are no vertices in this slider.
      user_data->_flags |= EggOptcharUserData::F_empty;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::list_joints
//       Access: Private
//  Description: Outputs a list of the joint hierarchy.
////////////////////////////////////////////////////////////////////
void EggOptchar::
list_joints(EggJointData *joint_data, int indent_level) {
  // Don't list the root joint, which is artificially created when the
  // character is loaded.  Instead, list each child as it is
  // encountered.

  int num_children = joint_data->get_num_children();
  for (int i = 0; i < num_children; i++) {
    EggJointData *child_data = joint_data->get_child(i);
    describe_component(child_data, indent_level);

    list_joints(child_data, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::list_scalars
//       Access: Private
//  Description: Outputs a list of the scalars.
////////////////////////////////////////////////////////////////////
void EggOptchar::
list_scalars(EggCharacterData *char_data) {
  int num_sliders = char_data->get_num_sliders();
  for (int si = 0; si < num_sliders; si++) {
    EggSliderData *slider_data = char_data->get_slider(si);
    describe_component(slider_data, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggOptchar::describe_component
//       Access: Private
//  Description: Describes one particular slider or joint.
////////////////////////////////////////////////////////////////////
void EggOptchar::
describe_component(EggComponentData *comp_data, int indent_level) {
  indent(nout, indent_level)
    << comp_data->get_name();
  
  EggOptcharUserData *user_data = 
    DCAST(EggOptcharUserData, comp_data->get_user_data());
  if (user_data->is_identity()) {
    nout << " (identity)";
  } else if (user_data->is_static()) {
    nout << " (static)";
  }
  if (user_data->is_empty()) {
    nout << " (empty)";
  }
  nout << "\n";
}


int main(int argc, char *argv[]) {
  EggOptchar prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
