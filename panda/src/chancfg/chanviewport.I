// Filename: chanviewport.I
// Created by:  cary (06Feb99)
// 
////////////////////////////////////////////////////////////////////

#include <algorithm>

INLINE ChanViewport::ChanViewport(void) {}

INLINE ChanViewport::ChanViewport(float l, float r, float b, float t) :
  _left(l), _right(r), _bottom(b), _top(t) {
  if (_left > _right)
    swap(_left, _right);
  if (_bottom > _top)
    swap(_bottom, _top);
}

INLINE ChanViewport::ChanViewport(const ChanViewport& c) : _left(c._left),
                                                           _right(c._right),
                                                           _bottom(c._bottom),
                                                           _top(c._top) {}

INLINE ChanViewport::~ChanViewport(void) {}

INLINE ChanViewport& ChanViewport::operator=(const ChanViewport& c) {
  _left = c._left;
  _right = c._right;
  _bottom = c._bottom;
  _top = c._top;
  return *this;
}

INLINE float ChanViewport::left(void) const { return _left; }

INLINE float ChanViewport::right(void) const { return _right; }

INLINE float ChanViewport::bottom(void) const { return _bottom; }

INLINE float ChanViewport::top(void) const { return _top; }
