// Filename: geomTransformer.I
// Created by:  drose (23May00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceVertices::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceVertices::
operator < (const GeomTransformer::SourceVertices &other) const {
  if (_coords != other._coords) {
    return _coords < other._coords;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceNormals::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceNormals::
operator < (const GeomTransformer::SourceNormals &other) const {
  if (_norms != other._norms) {
    return _norms < other._norms;
  }
  return (_mat.compare_to(other._mat) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::SourceTexCoords::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GeomTransformer::SourceTexCoords::
operator < (const GeomTransformer::SourceTexCoords &other) const {
  if (_texcoords != other._texcoords) {
    return _texcoords < other._texcoords;
  }
  return (_mat.compare_to(other._mat) < 0);
}
