// Filename: chanviewport.h
// Created by:  cary (06Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CHANVIEWPORT_H__
#define __CHANVIEWPORT_H__


class EXPCL_PANDA ChanViewport {
private:
  float _left, _right, _bottom, _top;

  INLINE ChanViewport(void);
public:
  INLINE ChanViewport(float, float, float, float);
  INLINE ChanViewport(const ChanViewport&);
  INLINE ~ChanViewport(void);
  INLINE ChanViewport& operator=(const ChanViewport&);

  INLINE float left(void) const;
  INLINE float right(void) const;
  INLINE float bottom(void) const;
  INLINE float top(void) const;
};

#include <pandabase.h>

#include "chanviewport.I"

#endif /* __CHANVIEWPORT_H__ */
