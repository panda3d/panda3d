// Filename: eggTopstrip.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGTOPSTRIP_H
#define EGGTOPSTRIP_H

#include <pandatoolbase.h>

#include <eggCharacterFilter.h>
#include <luse.h>

#include <vector>

class EggJointData;
class EggJointPointer;

////////////////////////////////////////////////////////////////////
// 	 Class : EggTopstrip
// Description : Reads a character model and/or animations and strips
//               out the animation from one of the top joints from the
//               entire character.  Particularly useful for generating
//               stackable character models from separately-extracted
//               characters.
////////////////////////////////////////////////////////////////////
class EggTopstrip : public EggCharacterFilter {
public:
  EggTopstrip();

  void run();
  void check_transform_channels();

  void strip_anim(EggJointData *joint_data, int from_model,
		  EggJointData *top_joint);
  void strip_anim_vertices(EggNode *egg_node, int into_model,
			   int from_model, EggJointData *top_joint);

  void adjust_transform(LMatrix4d &mat) const;


  string _top_joint_name;
  bool _got_invert_transform;
  bool _invert_transform;
  string _transform_channels;
  Filename _channel_filename;
};

#endif

