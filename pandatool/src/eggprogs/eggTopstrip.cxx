/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTopstrip.cxx
 * @author drose
 * @date 2001-02-23
 */

#include "eggTopstrip.h"

#include "dcast.h"
#include "eggJointData.h"
#include "eggCharacterCollection.h"
#include "eggCharacterData.h"
#include "eggCharacterDb.h"
#include "eggJointPointer.h"
#include "eggTable.h"
#include "compose_matrix.h"

/**
 *
 */
EggTopstrip::
EggTopstrip() {
  add_path_replace_options();
  add_path_store_options();

  set_program_brief("unapplies animation from a joint in an .egg file");
  set_program_description
    ("egg-topstrip reads a character model and its associated animation "
     "files, and unapplies the animation from one of the top joints.  "
     "This effectively freezes that particular joint, and makes the rest "
     "of the character relative to that joint.\n\n"

     "This is a particularly useful thing to do to generate character "
     "models that can stack one on top of the other in a sensible way.");

  add_option
    ("t", "name", 0,
     "Specify the name of the 'top' joint, from which to draw the "
     "animation channels which will be applied to the entire animation.",
     &EggTopstrip::dispatch_string, nullptr, &_top_joint_name);

  add_option
    ("i", "", 0,
     "Invert the matrix before applying.  This causes a subtractive "
     "effect.  This is the default unless -r is specified.",
     &EggTopstrip::dispatch_true, &_got_invert_transform, &_invert_transform);

  add_option
    ("n", "", 0,
     "Do not invert the matrix before applying.  This causes an "
     "additive effect.",
     &EggTopstrip::dispatch_false, &_got_invert_transform, &_invert_transform);

  add_option
    ("s", "[ijkphrxyz]", 0,
     "Specify the components of the transform that are to be applied.  Use "
     "any combination of the nine token letters: i, j, k represent the "
     "three scale axes; h, p, r represent rotation; and x, y, z represent "
     "translation.  The default is everything: -s ijkphrxyz.",
     &EggTopstrip::dispatch_string, nullptr, &_transform_channels);

  add_option
    ("r", "file.egg", 0,
     "Read the animation channel from the indicated egg file.  If this "
     "is not specified, each egg file will supply its own animation channel.",
     &EggTopstrip::dispatch_filename, nullptr, &_channel_filename);

  _invert_transform = true;
  _transform_channels = "ijkphrxyz";
}

/**
 *
 */
void EggTopstrip::
run() {
  nassertv(_collection != nullptr);
  nassertv(_collection->get_num_eggs() > 0);

  check_transform_channels();

  // Get the number of characters first, in case adding the _channel_egg
  // changes this.
  int num_characters = _collection->get_num_characters();

  // Determine which model and character we'll be pulling the animation
  // channels from.
  int from_model = -1;

  if (!_channel_filename.empty()) {
    // Read in the extra egg file that we use for extracting the channels out.
    PT(EggData) channel_egg = read_egg(_channel_filename);
    if (channel_egg == nullptr) {
      nout << "Cannot read " << _channel_filename << "\n";
      exit(1);
    }
    int channel_egg_index = _collection->add_egg(channel_egg);
    if (channel_egg_index < 0) {
      nout << _channel_filename
           << " does not contain a character model or animation channel.\n";
      exit(1);
    }

    from_model = _collection->get_first_model_index(channel_egg_index);

    if (!_got_invert_transform) {
      // With -r, the default is not to invert the transform.
      _invert_transform = false;
    }
  }

  // Now process each character.
  EggCharacterDb db;

  int ci;
  for (ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    nout << "Processing " << char_data->get_name() << "\n";

    EggJointData *root_joint = char_data->get_root_joint();

    // We'll read the transform to apply from this character, which will be
    // the same character unless -r was specified.
    EggCharacterData *from_char = char_data;
    if (from_model != -1) {
      from_char = _collection->get_character_by_model_index(from_model);
    }

    // Determine which joint we'll use to extract the transform to apply.
    EggJointData *top_joint = nullptr;
    if (_top_joint_name.empty()) {
      // The default top joint name is the alphabetically first joint in the
      // top level.
      if (root_joint->get_num_children() == 0) {
        nout << "Character " << from_char->get_name() << " has no joints.\n";
        exit(1);
      }
      top_joint = root_joint->get_child(0);
    } else {
      top_joint = from_char->find_joint(_top_joint_name);
      if (top_joint == nullptr) {
        nout << "Character " << from_char->get_name()
             << " has no joint named " << _top_joint_name << "\n";
        exit(1);
      }
    }

    // First, transform all the joints.
    int num_children = root_joint->get_num_children();
    for (int i = 0; i < num_children; i++) {
      EggJointData *joint_data = root_joint->get_child(i);
      strip_anim(char_data, joint_data, from_model, from_char, top_joint, db);
    }

    // We also need to transform the vertices for any models involved here.
    int num_models = char_data->get_num_models();
    for (int m = 0; m < num_models; m++) {
      EggNode *node = char_data->get_model_root(m);
      if (!node->is_of_type(EggTable::get_class_type())) {
        strip_anim_vertices(node, char_data->get_model_index(m),
                            from_model, top_joint, db);
      }
    }
  }

  // Now, trigger the actual rebuilding of all the joint data.
  for (ci = 0; ci < num_characters; ci++) {
    EggCharacterData *char_data = _collection->get_character(ci);
    char_data->get_root_joint()->do_rebuild_all(db);
  }

  write_eggs();
}

/**
 * Checks the _transform_channels string to ensure that it contains only the
 * expected nine letters, or a subset.
 */
void EggTopstrip::
check_transform_channels() {
  static std::string expected = "ijkphrxyz";
  static const int num_channels = 9;
  bool has_each[num_channels];
  memset(has_each, 0, num_channels * sizeof(bool));

  for (size_t p = 0; p < _transform_channels.size(); p++) {
    int i = expected.find(_transform_channels[p]);
    if (i == (int)std::string::npos) {
      nout << "Invalid letter for -s: " << _transform_channels[p] << "\n";
      exit(1);
    }
    nassertv(i < num_channels);
    has_each[i] = true;
  }

  _transform_channels = "";
  for (int i = 0; i < num_channels; i++) {
    if (has_each[i]) {
      _transform_channels += expected[i];
    }
  }

  if (_transform_channels.empty()) {
    nout << "No transform specified for -s.\n";
    exit(1);
  }
}


/**
 * Applies the channels from joint _top_joint in model from_model to the joint
 * referenced by joint_data.
 */
void EggTopstrip::
strip_anim(EggCharacterData *char_data, EggJointData *joint_data,
           int from_model, EggCharacterData *from_char,
           EggJointData *top_joint, EggCharacterDb &db) {
  int num_models = joint_data->get_num_models();
  for (int i = 0; i < num_models; i++) {
    int model = (from_model < 0) ? i : from_model;
    if (joint_data->has_model(i)) {
      if (!top_joint->has_model(model)) {
        nout << "Warning: Joint " << top_joint->get_name()
             << " is not defined in all models.\n";
        return;
      }

      int num_into_frames = char_data->get_num_frames(i);
      int num_from_frames = from_char->get_num_frames(model);

      int num_frames = std::max(num_into_frames, num_from_frames);

      EggBackPointer *back = joint_data->get_model(i);
      nassertv(back != nullptr);
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);

      // Compute and apply the new transforms.

      int f;
      for (f = 0; f < num_frames; f++) {
        LMatrix4d into = joint_data->get_frame(i, f % num_into_frames);
        LMatrix4d from = top_joint->get_net_frame(model, f % num_from_frames, db);

        adjust_transform(from);

        db.set_matrix(joint, EggCharacterDb::TT_rebuild_frame,
                      f, into * from);
      }
    }
  }
}

/**
 * Applies the channels from joint _top_joint in model from_model to the
 * vertices at egg_node.
 */
void EggTopstrip::
strip_anim_vertices(EggNode *egg_node, int into_model, int from_model,
                    EggJointData *top_joint, EggCharacterDb &db) {
  int model = (from_model < 0) ? into_model : from_model;
  if (!top_joint->has_model(model)) {
    nout << "Warning: Joint " << top_joint->get_name()
         << " is not defined in all models.\n";
    return;
  }

  LMatrix4d from = top_joint->get_net_frame(model, 0, db);
  adjust_transform(from);

  egg_node->transform_vertices_only(from);
}


/**
 * Adjust the transform extracted from the "top" joint according to the -s and
 * -i/-n options, prior to applying it to the skeleton.
 */
void EggTopstrip::
adjust_transform(LMatrix4d &mat) const {
  if (_transform_channels.length() != 9) {
    // Decompose and recompose the matrix, so we can eliminate the parts the
    // user doesn't want.

    LVecBase3d scale, hpr, translate;
    bool result = decompose_matrix(mat, scale, hpr, translate, _coordinate_system);
    if (!result) {
      nout << "Warning: skew transform in animation.\n";
    } else {
      LVecBase3d new_scale(1.0, 1.0, 1.0);
      LVecBase3d new_hpr(0.0, 0.0, 0.0);
      LVecBase3d new_translate(0.0, 0.0, 0.0);

      for (size_t i = 0; i < _transform_channels.size(); i++) {
        switch (_transform_channels[i]) {
        case 'i':
          new_scale[0] = scale[0];
          break;
        case 'j':
          new_scale[1] = scale[1];
          break;
        case 'k':
          new_scale[2] = scale[2];
          break;

        case 'h':
          new_hpr[0] = hpr[0];
          break;
        case 'p':
          new_hpr[1] = hpr[1];
          break;
        case 'r':
          new_hpr[2] = hpr[2];
          break;

        case 'x':
          new_translate[0] = translate[0];
          break;
        case 'y':
          new_translate[1] = translate[1];
          break;
        case 'z':
          new_translate[2] = translate[2];
          break;
        }
      }

      compose_matrix(mat, new_scale, new_hpr, new_translate, _coordinate_system);
    }
  }
  if (_invert_transform) {
    mat.invert_in_place();
  }
}


int main(int argc, char *argv[]) {
  EggTopstrip prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
