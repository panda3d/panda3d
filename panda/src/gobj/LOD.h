// Filename: LOD.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef LOD_H
#define LOD_H

#include "pandabase.h"

#include "luse.h"
#include "typedReferenceCount.h"

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : LODSwitch
// Description : Defines a switching region for an LOD.  An object
//               will be visible when it is closer than "in" units,
//               but further than "out" units from the camera.
//
//               The sense of in vs. out distances is as if the object
//               were coming towards you from far away: it switches
//               "in" at the far distance, and switches "out" at the
//               close distance.  Thus, "in" should be larger than
//               "out".
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LODSwitch {
public:
  INLINE LODSwitch();
  INLINE LODSwitch(float in, float out);
  INLINE LODSwitch(const LODSwitch &copy);
  INLINE void operator = (const LODSwitch &copy);

  INLINE void get_range(float &in, float &out) const;
  INLINE float get_in() const;
  INLINE float get_out() const;
  INLINE void set_range(float in, float out);
  INLINE bool in_range(float dist_squared) const;

  INLINE void rescale(float factor_squared);

  // We must declare these operators to allow VC++ to explicitly
  // export pvector<LODSwitch>, below.  They don't do anything useful.
  INLINE bool operator == (const LODSwitch &other) const;
  INLINE bool operator != (const LODSwitch &other) const;
  INLINE bool operator < (const LODSwitch &other) const;

  INLINE void write_datagram(Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source);

protected:
  float _in;
  float _out;
};

#define EXPCL EXPCL_PANDA
#define EXPTP EXPTP_PANDA
#define TYPE LODSwitch
#define NAME LODSwitchVector

#include "vector_src.h"

////////////////////////////////////////////////////////////////////
//       Class : LOD
// Description : Computes whether a level-of-detail should be rendered
//               or not based on distance from the rendering camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LOD {
public:
  LOD();
  LOD(const LOD &copy);
  ~LOD();

  void xform(const LMatrix4f &mat);

  int compute_child(const LPoint3f &cam_pos,
                    const LPoint3f &center) const;

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

PUBLISHED:
  static void set_stress_factor(float stress_factor);
  static float get_stress_factor();

public:
  LPoint3f _center;
  LODSwitchVector _switch_vector;

private:
  static float _stress_factor;
};

INLINE ostream &operator << (ostream &out, const LOD &lod) {
  lod.output(out);
  return out;
}

#include "LOD.I"

#endif
