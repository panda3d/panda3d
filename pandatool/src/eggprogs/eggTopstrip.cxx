// Filename: eggTopstrip.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggTopstrip.h"

#include <eggJointData.h>
#include <eggCharacterCollection.h>
#include <eggCharacterData.h>
#include <eggJointPointer.h>
#include <eggTable.h>

////////////////////////////////////////////////////////////////////
//     Function: EggTopstrip::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggTopstrip::
EggTopstrip() {
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
     &EggTopstrip::dispatch_string, NULL, &_top_joint_name);

  add_option
    ("i", "", 0, 
     "Invert the matrix before applying.  This causes a subtractive "
     "effect.  This is the default unless -r is specified.",
     &EggTopstrip::dispatch_true, &_got_invert_transform, &_invert_transform);

  add_option
    ("n", "", 0, 
     "Do not invert the matrix before applying.  This causes an "
     "additive effect.",
     &EggTopstrip::dispatch_true, &_got_invert_transform, &_invert_transform);

  add_option
    ("s", "[ijkphrxyz]", 0, 
     "Specify the components of the transform that are to be applied.  Use "
     "any combination of the nine token letters: i, j, k represent the "
     "three scale axes; h, p, r represent rotation; and x, y, z represent "
     "translation.  The default is everything: -s ijkphrxyz.",
     &EggTopstrip::dispatch_string, NULL, &_transform_channels);

  add_option
    ("r", "file.egg", 0, 
     "Read the animation channel from the indicated egg file.  If this "
     "is not specified, the first egg file named on the command line is "
     "used.",
     &EggTopstrip::dispatch_filename, NULL, &_channel_filename);

  _invert_transform = true;
  _transform_channels = "ijkphrxyz";
}

////////////////////////////////////////////////////////////////////
//     Function: EggTopstrip::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggTopstrip::
run() {
  nassertv(_collection != (EggCharacterCollection *)NULL);
  nassertv(_collection->get_num_eggs() > 0);

  // Get the number of characters first, in case adding the
  // _channel_egg changes this.
  int num_characters = _collection->get_num_characters();

  // Determine which model we'll be pulling the animation channels
  // from.
  int from_model = -1;

  if (!_channel_filename.empty()) {
    // Read in the extra egg file that we use for extracting the
    // channels out.
    PT(EggData) channel_egg = read_egg(_channel_filename);
    if (channel_egg == (EggData *)NULL) {
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
  for (int i = 0; i < num_characters; i++) {
    EggCharacterData *char_data = _collection->get_character(i);

    EggJointData *root_joint = char_data->get_root_joint();

    EggJointData *top_joint = (EggJointData *)NULL;
    if (_top_joint_name.empty()) {
      // The default top joint name is the alphabetically first joint
      // in the top level.
      if (root_joint->get_num_children() == 0) {
	nout << "Character " << char_data->get_name() << " has no joints.\n";
	exit(1);
      }
      top_joint = root_joint->get_child(0);
    } else {
      top_joint = char_data->find_joint(_top_joint_name);
      if (top_joint == (EggJointData *)NULL) {
	nout << "Character " << char_data->get_name()
	     << " has no joint named " << _top_joint_name << "\n";
	exit(1);
      }
    }

    int num_children = root_joint->get_num_children();
    for (int i = 0; i < num_children; i++) {
      EggJointData *joint_data = root_joint->get_child(i);
      strip_anim(joint_data, from_model, top_joint);
    }

    // We also need to transform the vertices for any models involved
    // here.
    int num_models = char_data->get_num_models();
    for (int m = 0; m < num_models; m++) {
      EggNode *node = char_data->get_model_root(m);
      if (!node->is_of_type(EggTable::get_class_type())) {
	strip_anim_vertices(node, m, from_model, top_joint);
      }
    }
  }

  write_eggs();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTopstrip::strip_anim
//       Access: Public
//  Description: Applies the channels from joint _top_joint
//               in model from_model to the joint referenced by
//               joint_data.
////////////////////////////////////////////////////////////////////
void EggTopstrip::
strip_anim(EggJointData *joint_data, int from_model, EggJointData *top_joint) {
  int num_models = joint_data->get_num_models();
  for (int i = 0; i < num_models; i++) {
    EggBackPointer *back = joint_data->get_model(i);
    if (back != (EggBackPointer *)NULL) {
      EggJointPointer *joint;
      DCAST_INTO_V(joint, back);

      cerr << "joint is " << joint->get_type() << "\n";

      int model = (from_model < 0) ? i : from_model;
      EggBackPointer *from_back = top_joint->get_model(model);
      if (from_back == (EggBackPointer *)NULL) {
	nout << "Joint " << top_joint->get_name() << " has no model index "
	     << model << "\n";
	exit(1);
      }
      EggJointPointer *from_joint;
      DCAST_INTO_V(from_joint, from_back);

      int num_into_frames = joint->get_num_frames();
      int num_from_frames = from_joint->get_num_frames();

      int num_frames = max(num_into_frames, num_from_frames);

      for (int f = 0; f < num_frames; f++) {
	LMatrix4d start = joint->get_frame(f % num_into_frames);
	LMatrix4d strip = from_joint->get_frame(f % num_from_frames);

	if (_invert_transform) {
	  strip.invert_in_place();
	}

	cerr << "Applying " << strip << " to " << f << " of " 
	     << joint_data->get_name() << " model " << i << "\n";

	if (f >= num_into_frames) {
	  if (!joint->add_frame(start * strip)) {
	    nout << "Cannot apply multiple frames of animation to a model file.\n"
		 << "In general, be careful when using -r and model files.\n";
	    exit(1);
	  }
	} else {
	  joint->set_frame(f, start * strip);
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTopstrip::strip_anim_vertices
//       Access: Public
//  Description: Applies the channels from joint _top_joint
//               in model from_model to the vertices at egg_node.
////////////////////////////////////////////////////////////////////
void EggTopstrip::
strip_anim_vertices(EggNode *egg_node, int into_model, int from_model, 
		    EggJointData *top_joint) {
  int model = (from_model < 0) ? into_model : from_model;
  EggBackPointer *from_back = top_joint->get_model(model);
  if (from_back == (EggBackPointer *)NULL) {
    nout << "Joint " << top_joint->get_name() << " has no model index "
	 << model << "\n";
    exit(1);
  }

  EggJointPointer *from_joint;
  DCAST_INTO_V(from_joint, from_back);

  LMatrix4d strip = from_joint->get_frame(0);
  if (_invert_transform) {
    strip.invert_in_place();
  }

  cerr << "Applying " << strip << " to vertices.\n";
  egg_node->transform_vertices_only(strip);
}


int main(int argc, char *argv[]) {
  EggTopstrip prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
