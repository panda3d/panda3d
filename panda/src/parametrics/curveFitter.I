// Filename: curveFitter.I
// Created by:  drose (04Mar01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::DataPoint::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CurveFitter::DataPoint::
DataPoint() : 
  _t(0.0),
  _xyz(0.0, 0.0, 0.0),
  _hpr(0.0, 0.0, 0.0),
  _tangent(0.0, 0.0, 0.0),
  _hpr_tangent(0.0, 0.0, 0.0) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::DataPoint::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CurveFitter::DataPoint::
output(ostream &out) const {
  out << "Time " << _t << " xyz " << _xyz << " hpr " << _hpr
      << " tan " << _tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::DataPoint::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool CurveFitter::DataPoint::
operator < (const DataPoint &other) const {
  return _t < other._t;
}

