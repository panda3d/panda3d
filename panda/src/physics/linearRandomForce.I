// Filename: LinearRandomForce.I
// Created by:  charles (19Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function : random_unit_vector
//       Access : Protected
//  Description : generates a random unit vector
////////////////////////////////////////////////////////////////////
INLINE LVector3f LinearRandomForce::
random_unit_vector(void) {
  float z, r, theta;

  z = 1.0f - (2.0f * bounded_rand());
  r = sqrtf(1.0f - (z * z));
  theta = 2.0f * 3.1415926f * bounded_rand();

  return LVector3f(r * cosf(theta), r * sinf(theta), z);
}

