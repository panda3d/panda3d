// Filename: guiRollover.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIROLLOVER_H__
#define __GUIROLLOVER_H__

#include "guiRegion.h"
#include "guiLabel.h"
#include "guiManager.h"

#include <eventHandler.h>

class EXPCL_PANDA GuiRollover : public Namable {
private:
  GuiLabel* _off;
  GuiLabel* _on;
  GuiRegion* _rgn;

  bool _state;
  bool _added_hooks;
  GuiManager* _mgr;

  float _scale;
  LVector3f _pos;

  INLINE GuiRollover(void);
  void recompute_frame(void);
public:
  GuiRollover(const string&, GuiLabel*, GuiLabel*);
  ~GuiRollover(void);

  void manage(GuiManager*, EventHandler&);
  void unmanage(void);
  INLINE void enter(void);
  INLINE void exit(void);

  INLINE bool is_over(void) const;

  INLINE void set_scale(float);
  INLINE void set_pos(const LVector3f&);

  INLINE float get_scale(void) const;
  INLINE LVector3f get_pos(void) const;
};

#include "guiRollover.I"

#endif /* __GUIROLLOVER_H__ */
