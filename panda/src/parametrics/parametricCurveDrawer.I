// Filename: parametricCurveDrawer.I
// Created by:  drose (04Mar01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::get_max_t
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE float ParametricCurveDrawer::
get_max_t() const {
  if (_curves == (ParametricCurveCollection *)NULL) {
    return 0.0;
  } else {
    return _curves->get_max_t();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParametricCurveDrawer::redraw
//       Access: Public
//  Description: Redraws the curve if it is currently visible.
////////////////////////////////////////////////////////////////////
INLINE void ParametricCurveDrawer::
redraw() {
  if (_drawn) {
    draw();
  }
}
