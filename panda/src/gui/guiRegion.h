// Filename: guiRegion.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIREGION_H__
#define __GUIREGION_H__

// container for active regions of a GUI

class GuiRegion {
private:
  float _left, _right, _bottom, _top;
public:
  INLINE GuiRegion(float, float, float, float);
};

#include "guiRegion.I"

#endif /* __GUIREGION_H__ */
