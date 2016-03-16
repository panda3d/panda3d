/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perlinNoise2.I
 * @author drose
 * @date 2005-10-05
 */

/**
 * Randomizes the tables to make a unique noise function.  Uses a default
 * scale (noise frequency), table size, and seed.
 */
INLINE PerlinNoise2::
PerlinNoise2() :
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
INLINE PerlinNoise2::
PerlinNoise2(double sx, double sy,
             int table_size, unsigned long seed) :
  PerlinNoise(table_size, seed)
{
  init_unscaled_xform();
  set_scale(sx, sy);
}

/**
 * Makes an exact copy of the existing PerlinNoise object, including its
 * random seed.
 */
INLINE PerlinNoise2::
PerlinNoise2(const PerlinNoise2 &copy) :
  PerlinNoise(copy),
  _unscaled_xform(copy._unscaled_xform),
  _input_xform(copy._input_xform)
{
}

/**
 * Makes an exact copy of the existing PerlinNoise object, including its
 * random seed.
 */
INLINE void PerlinNoise2::
operator = (const PerlinNoise2 &copy) {
  PerlinNoise::operator = (copy);
  _unscaled_xform = copy._unscaled_xform;
  _input_xform = copy._input_xform;
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise2::
set_scale(double scale) {
  set_scale(scale, scale);
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise2::
set_scale(double x, double y) {
  set_scale(LVecBase2d(x, y));
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise2::
set_scale(const LVecBase2f &value) {
  set_scale(value[0], value[1]);
}

/**
 * Changes the scale (frequency) of the noise.
 */
INLINE void PerlinNoise2::
set_scale(const LVecBase2d &value) {
  _input_xform = LMatrix3d::scale_mat(1.0f / value[0], 1.0f / value[1]) * _unscaled_xform;
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE double PerlinNoise2::
noise(double x, double y) const {
  return noise(LVecBase2d(x, y));
}

/**
 * Returns the noise function of the three inputs.
 */
INLINE float PerlinNoise2::
noise(const LVecBase2f &value) const {
  return (float)noise(value[0], value[1]);
}

/**
 * Returns the noise function of the two inputs.
 */
INLINE double PerlinNoise2::
operator ()(double x, double y) const {
  return noise(x, y);
}

/**
 * Returns the noise function of the two inputs.
 */
INLINE float PerlinNoise2::
operator ()(const LVecBase2f &value) const {
  return noise(value);
}

/**
 * Returns the noise function of the two inputs.
 */
INLINE double PerlinNoise2::
operator ()(const LVecBase2d &value) const {
  return noise(value);
}

/**
 * Returns the dot product of a random gradient vector (determined by the hash
 * code) with the indicated offset vector.
 */
INLINE double PerlinNoise2::
grad(int hash, double x, double y) {
  // Convert low 3 bits of hash code into 8 gradient directions.
  switch (hash & 7) {
    // Four corners.
  case 0: return x + y;
  case 1: return x - y;
  case 2: return -x + y;
  case 3: return -x - y;

    // Four edges.  Here we scale by 1.707 to make all the vectors equal
    // length, and to make their lengths consistent with PerlinNoise3.
  case 4: return 1.707 * x;
  case 5: return 1.707 * y;
  case 6: return -1.707 * x;
  case 7: return -1.707 * y;
  }

  nassertr(false, 0);
  return 0;
}
