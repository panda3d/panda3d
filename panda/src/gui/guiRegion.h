// Filename: guiRegion.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIREGION_H__
#define __GUIREGION_H__

#include <pandabase.h>
#include <mouseWatcherRegion.h>
#include <pointerTo.h>

// container for active regions of a GUI

class GuiManager;

class EXPCL_PANDA GuiRegion : public Namable {
private:
  float _left, _right, _bottom, _top;
  PT(MouseWatcherRegion) _region;

  INLINE GuiRegion(void);

  INLINE MouseWatcherRegion* get_region(void) const;

  friend GuiManager;
public:
  INLINE GuiRegion(const string&, float, float, float, float, bool);
  ~GuiRegion(void);

  INLINE void trap_clicks(bool);

  INLINE void set_region(float, float, float, float);
  INLINE LVector4f get_frame(void) const;
};

#include "guiRegion.I"

#endif /* __GUIREGION_H__ */
