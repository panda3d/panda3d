// Filename: chansetup.h
// Created by:  cary (05Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CHANSETUP_H__
#define __CHANSETUP_H__

#include <pandabase.h>

#include <vector_string.h>

#include <map>
#include "chanviewport.h"

typedef vector_string SetupSyms;

class SetupFOV {
public:
  enum FOVType { Invalid, Default, Horizontal, Both };
private:
  FOVType _type;
  float _horiz, _vert;
public:
  INLINE SetupFOV(void);
  INLINE SetupFOV(const SetupFOV&);
  INLINE ~SetupFOV(void);

  INLINE SetupFOV& operator=(const SetupFOV&);

  INLINE void setFOV(void);
  INLINE void setFOV(const float);
  INLINE void setFOV(const float, const float);
  INLINE FOVType getType(void) const;
  INLINE float getHoriz(void) const;
  INLINE float getVert(void) const;
};

class SetupItem {
public:
  enum Orientation { Up, Down, Left, Right };
private:
  bool _recurse;

  SetupSyms _layouts;
  SetupSyms _setups;

  bool _stereo;
  bool _hw_chan;
  int _chan;
  ChanViewport _viewport;
  SetupFOV _fov;
  Orientation _orientation;
public:
  INLINE SetupItem(void);
  INLINE SetupItem(bool);
  INLINE SetupItem(const SetupItem&);
  INLINE ~SetupItem(void);

  INLINE SetupItem& operator=(const SetupItem&);

  INLINE void AddRecurse(std::string&, std::string&);

  INLINE void setState(const bool, const bool, const int, const ChanViewport&,
                       const SetupFOV&, const Orientation&);

  INLINE bool getRecurse(void) const;
  INLINE SetupSyms getLayouts(void) const;
  INLINE SetupSyms getSetups(void) const;
  INLINE bool getStereo(void) const;
  INLINE bool getHWChan(void) const;
  INLINE int getChan(void) const;
  INLINE ChanViewport getViewport(void) const;
  INLINE SetupFOV getFOV(void) const;
  INLINE Orientation getOrientation(void) const;
};

typedef std::map<std::string, SetupItem> SetupType;

extern SetupType* SetupDB;

void ResetSetup();
void ParseSetup(istream&);

#include "chansetup.I"

#endif /* __CHANSETUP_H__ */
