// Filename: guiBehavior.h
// Created by:  cary (07Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBEHAVIOR_H__
#define __GUIBEHAVIOR_H__

#include "guiItem.h"

claas EXPCL_PANDA GuiBehavior : public GuiItem {
protected:
  EventHandler* _eh;
PUBLISHED:
  GuiBehavior(const string&);
  virtual ~GuiBehavior(void);

  virtual void manage(GuiManager*, EventHandler&) = 0;
  virtual void unmanage(void) = 0;

  virtual void start_behavior(void) = 0;
  virtual void stop_behavior(void) = 0;
};

#endif /* __GUIBEHAVIOR_H__ */
