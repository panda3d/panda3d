// Filename: zSpinParticle.I
// Created by:  charles (16Aug00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : set_thetas
// Description : public
////////////////////////////////////////////////////////////////////
INLINE void ZSpinParticle::
set_thetas(float cur, float target) {
  _initial_theta = cur;
  _cur_theta = cur;
  _target_theta = target;

  float diff = _target_theta - _initial_theta;

  // figure out which way to spin.
  if (diff < 0) {
    if (fabs(diff) >= 180)
      _positive_increment = true;
    else
      _positive_increment = false;
  }
  else {
    if (fabs(diff) >= 180)
      _positive_increment = false;
    else
      _positive_increment = true;
  }
}

