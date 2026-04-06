/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeContactGeom.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE LVecBase3f OdeContactGeom::
get_pos() const {
  return LVecBase3f(_contact_geom.pos[0],
                    _contact_geom.pos[1],
                    _contact_geom.pos[2]);
}

INLINE LVecBase3f OdeContactGeom::
get_normal() const {
  return LVecBase3f(_contact_geom.normal[0],
                    _contact_geom.normal[1],
                    _contact_geom.normal[2]);
}

INLINE dReal OdeContactGeom::
get_depth() const {
  return _contact_geom.depth;
}

INLINE OdeGeom OdeContactGeom::
get_g1() const {
  return OdeGeom(_contact_geom.g1);
}

INLINE OdeGeom OdeContactGeom::
get_g2() const {
  return OdeGeom(_contact_geom.g2);
}

INLINE int OdeContactGeom::
get_side1() const {
  return _contact_geom.side1;
}

INLINE int OdeContactGeom::
get_side2() const {
  return _contact_geom.side2;
}

INLINE void OdeContactGeom::
set_pos(const LVecBase3f &pos){
  _contact_geom.pos[0] = pos[0];
  _contact_geom.pos[1] = pos[1];
  _contact_geom.pos[2] = pos[2];
}

INLINE void OdeContactGeom::
set_normal(const LVecBase3f &normal){
  _contact_geom.normal[0] = normal[0];
  _contact_geom.normal[1] = normal[1];
  _contact_geom.normal[2] = normal[2];
}

INLINE void OdeContactGeom::
set_depth(const dReal depth){
  _contact_geom.depth = depth;
}

INLINE void OdeContactGeom::
set_g1(const OdeGeom &geom){
  _contact_geom.g1 = geom._id;
}

INLINE void OdeContactGeom::
set_g2(const OdeGeom &geom){
  _contact_geom.g2 = geom._id;
}
