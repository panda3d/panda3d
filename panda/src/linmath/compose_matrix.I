// Filename: compose_matrix.I
// Created by:  drose (21Feb99)
// 
////////////////////////////////////////////////////////////////////

INLINE void
compose_matrix(LMatrix4f &mat,
	       const float components[9],
	       CoordinateSystem cs) {
  LVector3f scale(components[0],
		  components[1],
		  components[2]);
  LVector3f hpr(components[3],
		components[4],
		components[5]);
  LVector3f translate(components[6],
		      components[7],
		      components[8]);
  compose_matrix(mat, scale, hpr, translate, cs);
}

INLINE bool
decompose_matrix(const LMatrix4f &mat,
		 float components[9],
		 CoordinateSystem cs) {
  LVector3f scale, hpr, translate;
  if (!decompose_matrix(mat, scale, hpr, translate, cs)) {
    return false;
  }
  components[0] = scale[0];
  components[1] = scale[1];
  components[2] = scale[2];
  components[3] = hpr[0];
  components[4] = hpr[1];
  components[5] = hpr[2];
  components[6] = translate[0];
  components[7] = translate[1];
  components[8] = translate[2];
  return true;
}

INLINE void
compose_matrix(LMatrix4d &mat,
	       const double components[9],
	       CoordinateSystem cs) {
  LVector3d scale(components[0],
		  components[1],
		  components[2]);
  LVector3d hpr(components[3],
		components[4],
		components[5]);
  LVector3d translate(components[6],
		      components[7],
		      components[8]);
  compose_matrix(mat, scale, hpr, translate, cs);
}

INLINE bool
decompose_matrix(const LMatrix4d &mat,
		 double components[9],
		 CoordinateSystem cs) {
  LVector3d scale, hpr, translate;
  if (!decompose_matrix(mat, scale, hpr, translate, cs)) {
    return false;
  }

  components[0] = scale[0];
  components[1] = scale[1];
  components[2] = scale[2];
  components[3] = hpr[0];
  components[4] = hpr[1];
  components[5] = hpr[2];
  components[6] = translate[0];
  components[7] = translate[1];
  components[8] = translate[2];
  return true;
}
