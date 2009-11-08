// Filename: arToolKit.h
// Created by: jyelon (01Nov2007)
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

#ifndef ARTOOLKIT_H
#define ARTOOLKIT_H

#include "pandabase.h"

#ifdef HAVE_ARTOOLKIT

#include "nodePath.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : ARToolKit
// Description : ARToolKit is a software library for building
//               Augmented Reality (AR) applications. These are
//               applications that involve the overlay of virtual
//               imagery on the real world.  It was developed by
//               Dr. Hirokazu Kato.  Its ongoing development is
//               being supported by the Human Interface Technology
//               Laboratory (HIT Lab) at the University of
//               Washington, HIT Lab NZ at the University of
//               Canterbury, New Zealand, and ARToolworks, Inc,
//               Seattle.  It is available under a GPL license.
//               It is also possible to negotiate other licenses
//               with the copyright holders.
//
//               This class is a wrapper around the ARToolKit
//               library.
////////////////////////////////////////////////////////////////////
class EXPCL_VISION ARToolKit {
  
PUBLISHED:
  static ARToolKit *make(NodePath camera, const Filename &paramfile, double markersize);
  ~ARToolKit();
  
  INLINE void set_threshold(double n);
  void attach_pattern(const Filename &pattern, NodePath path);
  void detach_patterns();
  void analyze(Texture *tex, bool do_flip_texture = true);
  
private:
  static int get_pattern(const Filename &pattern);
  ARToolKit();
  void cleanup();
  
  typedef pmap<Filename, int> PatternTable;
  static PatternTable _pattern_table;
  
  typedef pmap<int, NodePath> Controls;
  Controls _controls;
  
  NodePath _camera;
  void *_camera_param;
  double _threshold;
  double _marker_size;
  double _prev_conv[3][4];
  bool _have_prev_conv;
};

#include "arToolKit.I"

#endif // HAVE_ARTOOLKIT
#endif // ARTOOLKIT_H
