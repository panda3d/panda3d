// Filename: guiManager.h
// Created by:  cary (25Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIMANAGER_H__
#define __GUIMANAGER_H__

class GuiManager {
private:
  static GuiManager* _singleton;
public:
  INLINE static GuiManager* get_ptr(void);
};

#endif /* __GUIMANAGER_H__ */
