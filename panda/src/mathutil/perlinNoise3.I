/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perlinNoise3.I
 * @author drose
 * @date 2005-10-05
 */

/**
 * Randomizes the tables to make a unique noise function.  Uses a default
 * scale (noise frequency), table size, and seed.
 */
INLINE PerlinNoise3::
PerlinNoise3() :
  PerlinNoise(256, 0)
{
  init_unscaled_xform();
  _input_xform = _unscaled_xform;
}

/**
 * Randomizes the tables to make a unique noise function.
 *
 * If seed is nonzero, it is used to define the tables; if it is zero a random
 * seed is generated.
 */
INLINE PerlinNoise3::
PerlinNoise3(double sx, double sy, double sz,
             int table_size, unsigned long seed) :
  PerlinNoise(table_size, seed)
{
  init_unscaled_xform();
  set_scale(sx, sy, sz);
}

/**
 * Makes an exact copy of the existing PerlinNoise object, including its
 * random seed.
 */
INLINE PerlinNoise3::
PerlinNoise3(const PerlinNoise3 &copy) :
  PerlinNoise(copy),
  _unscaled_xform(copy._unscaled_xform),
  _input_xform(copy._input_xform)
{
}

/**
 * Makes an exact copy of the existing PerlinNoise object, including its
 * random seed.
 */
INLINE void PerlinNoise3::
operator = (const PerlinNoise3 &copy) {
  PerlinNoise::operator = (copy);
  _unscaled_xform = copy._unscaled_xform;
  _input_xform = copy._input_xform;
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise3::
set_scale(double scale) {
  set_scale(scale, scale, scale);
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise3::
set_scale(double x, double y, double z) {
  set_scale(LVecBase3d(x, y, z));
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise3::
set_scale(const LVecBase3f &value) {
  set_scale(value[0], value[1], value[2]);
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise3::
set_scale(const LVecBase3d &value) {
  _input_xform = LMatrix4d::scale_mat(1.0f / value[0], 1.0f / value[1], 1.0f / value[2]) * _unscaled_xform;
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double PerlinNoise3::
noise(double x, double y, double z) const {
  return noise(LVecBase3d(x, y, z));
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE float PerlinNoise3::
noise(const LVecBase3f &value) const {
  return (float)noise(value[0], value[1], value[2]);
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double PerlinNoise3::
operator ()(double x, double y, double z) const {
  return noise(x, y, z);
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE float PerlinNoise3::
operator ()(const LVecBase3f &value) const {
  return noise(value);
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double PerlinNoise3::
operator ()(const LVecBase3d &value) const {
  return noise(value);
}

/**
 * Returns the dot product of a random gradient vector (determined by the hash
 * code) with the indicated offset vector.
 */
INLINE double PerlinNoise3::
grad(int hash, double x, double y, double z) {
  // Convert low 4 bits of hash code into 12 gradient directions.
  /*
    This is Perlin's reference code, but the switch statement below is
    slightly faster and produces exactly the same results.
  int h = hash & 15;
  double u = (h < 8) ? x : y;
  double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
  return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
  */

  switch (hash & 15) {
  case 0: return x + y;
  case 1: return -x + y;
  case 2: return x - y;
  case 3: return -x - y;

  case 4: return x + z;
  case 5: return -x + z;
  case 6: return x - z;
  case 7: return -x - z;

  case 8: return y + z;
  case 9: return -y + z;
  case 10: return y - z;
  case 11: return -y - z;

  case 12: return x + y;
  case 13: return -y + z;
  case 14: return -x + y;
  case 15: return -y - z;
  }

  nassertr(false, 0);
  return 0;
}
