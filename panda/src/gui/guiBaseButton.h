// Filename: guiBaseButton.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBASEBUTTON_H__
#define __GUIBASEBUTTON_H__

class GuiBaseButton : public GuiRegion, public GuiLabel {
private:
public:
  INLINE GuiBaseButton(float, float, float, float);
};

#include "guiBaseButton.h"

#endif /* __GUIBASEBUTTON_H__ */
