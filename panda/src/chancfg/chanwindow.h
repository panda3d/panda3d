// Filename: chanwindow.h
// Created by:  cary (06Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CHANWINDOW_H__
#define __CHANWINDOW_H__

#include <pandabase.h>

#include <map>
#include <vector>
#include <string>
#include "chansetup.h"

class WindowItem {
private:
  bool _hw_chans, _dvr, _border, _cursor;
  int _chan_offset, _sizeX, _sizeY;
  std::string _layout;
  SetupSyms _setups;
public:
  INLINE WindowItem(void);
  INLINE WindowItem(const bool, const bool, const int, const std::string&,
                    const SetupSyms&, const int, const int, const bool,
                    const bool);
  INLINE WindowItem(const WindowItem&);
  INLINE ~WindowItem(void);

  INLINE WindowItem& operator=(const WindowItem&);

  INLINE bool getHWChans(void) const;
  INLINE bool getDVR(void) const;
  INLINE bool getBorder(void) const;
  INLINE bool getCursor(void) const;
  INLINE int getChanOffset(void) const;
  INLINE int getSizeX(void) const;
  INLINE int getSizeY(void) const;
  INLINE std::string getLayout(void) const;
  INLINE SetupSyms getSetups(void) const;
};

typedef std::map<std::string, WindowItem> WindowType;

extern WindowType* WindowDB;

void ResetWindow();
void ParseWindow(istream&);

#include "chanwindow.I"

#endif /* __CHANWINDOW_H__ */
