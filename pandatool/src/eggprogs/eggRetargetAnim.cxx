/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRetargetAnim.cxx
 * @author drose
 * @date 2005-05-05
 */

#include "eggRetargetAnim.h"

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
EggRetargetAnim::
EggRetargetAnim() {
  add_path_replace_options();
  add_path_store_options();

  set_program_brief("remove transformations from animation data in .egg files");
  set_program_description
    ("egg-retarget-anim reads a character model and its associated animation "
     "files, and removes the translations and scales from the animation "
     "files, replacing them with the translations and scales from the "
     "rest position of the character model.\n\n"

     "This allows an animation that was generated for a model with one "
     "skeleton to be played successfully on a model with a different "
     "skeleton, provided that both skeletons have the same hierarchy and "
     "differ only in scales and/or translations of the various joints, "
     "and that scales and translations are not part of the per-frame "
     "animations.");

  add_option
    ("r", "file.egg", 0,
     "Read the reference model from the indicated egg file.  All of the "
     "animations will be retargeted to match the indicated file.",
     &EggRetargetAnim::dispatch_filename, nullptr, &_reference_filename);

  add_option
    ("keep", "joint[,joint...]", 0,
     "Preserve the full animation on the named joint(s).  This is especially "
     "appropriate for the root joint.",
     &EggRetargetAnim::dispatch_vector_string_comma, nullptr, &_keep_joints);
}

/**
 *
 */
void EggRetargetAnim::
run() {
  nassertv(_collection != nullptr);
  nassertv(_collection->get_num_eggs() > 0);

  if (_reference_filename.empty()) {
    nout << "No reference filename specified.\n";
    exit(1);
  }

  int num_characters = _collection->get_num_characters();
  if (num_characters != 1) {
    nout << "All animations must have the same character name.\n";
    exit(1);
  }

  // Read in the extra egg file that we use for extracting the references out.
  PT(EggData) reference_egg = read_egg(_reference_filename);
  if (reference_egg == nullptr) {
    nout << "Cannot read " << _reference_filename << "\n";
    exit(1);
  }

  // First, we add it to a separate EggCharacterCollection, so we can figure
  // out its name.
  EggCharacterCollection col;
  if (col.add_egg(reference_egg) < 0) {
    nout << _reference_filename
         << " does not contain a character model or animation reference.\n";
    exit(1);
  }

  if (col.get_num_characters() != 1) {
    nout << "Reference model must contain only one character.\n";
    exit(1);
  }

  std::string ref_name = col.get_character(0)->get_name();

  // Now rename all of the animations to the same name as the reference model,
  // and add the reference animation in to the same collection to match it up
  // joint-for-joint.
  _collection->rename_char(0, ref_name);
  int reference_egg_index = _collection->add_egg(reference_egg);
  nassertv(reference_egg_index > 0);
  nassertv(_collection->get_num_characters() == 1);

  int reference_model = _collection->get_first_model_index(reference_egg_index);
  EggCharacterData *char_data = _collection->get_character(0);
  nout << "Processing " << char_data->get_name() << "\n";

  typedef pset<std::string> Names;
  Names keep_names;

  vector_string::const_iterator si;
  for (si = _keep_joints.begin(); si != _keep_joints.end(); ++si) {
    keep_names.insert(*si);
  }

  EggCharacterDb db;
  EggJointData *root_joint = char_data->get_root_joint();
  retarget_anim(char_data, root_joint, reference_model, keep_names, db);
  root_joint->do_rebuild_all(db);

  write_eggs();
}

/**
 * Recursively replaces the scale and translate information on all of the
 * joints in the char_data hierarchy wiht this from reference_char.
 */
void EggRetargetAnim::
retarget_anim(EggCharacterData *char_data, EggJointData *joint_data,
              int reference_model, const pset<std::string> &keep_names,
              EggCharacterDb &db) {
  if (keep_names.find(joint_data->get_name()) != keep_names.end()) {
    // Don't retarget this joint; keep the translation and scale and whatever.

  } else {
    // Retarget this joint.
    int num_models = joint_data->get_num_models();
    for (int i = 0; i < num_models; i++) {
      if (joint_data->has_model(i)) {
        int num_frames = char_data->get_num_frames(i);

        EggBackPointer *back = joint_data->get_model(i);
        nassertv(back != nullptr);
        EggJointPointer *joint;
        DCAST_INTO_V(joint, back);

        LMatrix4d ref = joint_data->get_frame(reference_model, 0);
        LVecBase3d ref_scale, ref_shear, ref_hpr, ref_translate;
        if (!decompose_matrix(ref, ref_scale, ref_shear, ref_hpr, ref_translate)) {
          nout << "Could not decompose rest frame for "
               << joint_data->get_name() << "\n";
        } else {
          int f;
          for (f = 0; f < num_frames; f++) {
            LMatrix4d mat = joint_data->get_frame(i, f);

            LVecBase3d scale, shear, hpr, translate;
            if (decompose_matrix(mat, scale, shear, hpr, translate)) {
              compose_matrix(mat, ref_scale, ref_shear, hpr, ref_translate);
            } else {
              nout << "Could not decompose matrix for " << joint_data->get_name()
                   << "\n";
            }

            db.set_matrix(joint, EggCharacterDb::TT_rebuild_frame,
                          f, mat);
          }
        }
      }
    }
  }

  int num_children = joint_data->get_num_children();
  for (int i = 0; i < num_children; i++) {
    EggJointData *next_joint_data = joint_data->get_child(i);
    retarget_anim(char_data, next_joint_data, reference_model, keep_names, db);
  }
}


int main(int argc, char *argv[]) {
  EggRetargetAnim prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
