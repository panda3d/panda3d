// Filename: guiSign.h
// Created by:  cary (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUISIGN_H__
#define __GUISIGN_H__

#include "guiItem.h"
#include "guiLabel.h"
#include "guiManager.h"

class EXPCL_PANDA GuiSign : public GuiItem {
private:
  GuiLabel* _sign;

  INLINE GuiSign(void);
  virtual void recompute_frame(void);
public:
  GuiSign(const string&, GuiLabel*);
  ~GuiSign(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);

  virtual void output(ostream&) const;
};

#include "guiSign.I"

#endif /* __GUISIGN_H__ */
