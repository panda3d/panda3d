// Filename: eggToBam.h
// Created by:  drose (28Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGTOBAM_H
#define EGGTOBAM_H

#include <pandatoolbase.h>

#include <eggToSomething.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggToBam
// Description : 
////////////////////////////////////////////////////////////////////
class EggToBam : public EggToSomething {
public:
  EggToBam();

  void run();

  bool _keep_paths;
  bool _has_egg_flatten;
  int _egg_flatten;
  bool _ls;
  bool _has_compression_quality;
  int _compression_quality;
  bool _compression_off;
};

#endif
