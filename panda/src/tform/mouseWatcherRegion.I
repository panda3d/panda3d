// Filename: mouseWatcherRegion.I
// Created by:  drose (13Jul00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherRegion::
MouseWatcherRegion(const string &name, float left, float right,
                   float bottom, float top) :
  Namable(name),
  _frame(left, right, bottom, top)
{
  _sort = 0;
  _active = true;
  _suppress_below = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MouseWatcherRegion::
MouseWatcherRegion(const string &name, const LVecBase4f &frame) :
  Namable(name),
  _frame(frame)
{
  _sort = 0;
  _active = true;
  _suppress_below = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_frame
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_frame(float left, float right, float bottom, float top) {
  set_frame(LVecBase4f(left, right, bottom, top));
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_frame
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_frame(const LVecBase4f &frame) {
  _frame = frame;
  _area = (_frame[1] - _frame[0]) * (_frame[3] - _frame[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_frame
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4f &MouseWatcherRegion::
get_frame() const {
  return _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_area
//       Access: Published
//  Description: Returns the area of the rectangular region.
////////////////////////////////////////////////////////////////////
INLINE float MouseWatcherRegion::
get_area() const {
  return _area;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_sort
//       Access: Published
//  Description: Changes the sorting order of this particular region.
//               The sorting order is used to resolve conflicts in the
//               case of overlapping region; the region with the
//               highest sort value will be preferred, and between
//               regions of the same sort value, the smallest region
//               will be preferred.  The default sorting order, if
//               none is explicitly specified, is 0.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_sort(int sort) {
  _sort = sort;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_sort
//       Access: Published
//  Description: Returns the current sorting order of this region.
//               See set_sort().
////////////////////////////////////////////////////////////////////
INLINE int MouseWatcherRegion::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_active
//       Access: Published
//  Description: Sets whether the region is active or not.  If it is
//               not active, the MouseWatcher will totally ignore it.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_active
//       Access: Published
//  Description: Returns whether the region is active or not.  See
//               set_active().
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherRegion::
get_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::set_suppress_below
//       Access: Published
//  Description: Sets the suppress_below flag.  When this is true, and
//               the region is active, then whenever the mouse is
//               within the region the MouseWatcher will suppress any
//               mouse and keyboard events further down in the data
//               graph.
////////////////////////////////////////////////////////////////////
INLINE void MouseWatcherRegion::
set_suppress_below(bool suppress_below) {
  _suppress_below = suppress_below;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::get_suppress_below
//       Access: Published
//  Description: Returns the suppress_below flag.  See
//               set_suppress_below().
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherRegion::
get_suppress_below() const {
  return _suppress_below;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::Ordering Operator
//       Access: Public
//  Description: Returns true if this region should be preferred over
//               the other region when they overlap, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MouseWatcherRegion::
operator < (const MouseWatcherRegion &other) const {
  if (_sort != other._sort) {
    return _sort > other._sort;
  }
  return _area < other._area;
}
