// Filename: guiRollover.h
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIROLLOVER_H__
#define __GUIROLLOVER_H__

#include "guiItem.h"
#include "guiRegion.h"
#include "guiLabel.h"
#include "guiManager.h"

class EXPCL_PANDA GuiRollover : public GuiItem {
private:
  GuiLabel* _off;
  GuiLabel* _on;
  GuiRegion* _rgn;

  bool _state;

  INLINE GuiRollover(void);
  virtual void recompute_frame(void);
public:
  GuiRollover(const string&, GuiLabel*, GuiLabel*);
  virtual ~GuiRollover(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);
  INLINE void enter(void);
  INLINE void exit(void);

  INLINE bool is_over(void) const;

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);

  virtual void output(ostream&) const;
};

#include "guiRollover.I"

#endif /* __GUIROLLOVER_H__ */
