// Filename: chanlayout.h
// Created by:  cary (04Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CHANLAYOUT_H__
#define __CHANLAYOUT_H__

#include <pandabase.h>

#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "chanviewport.h"


typedef std::vector<ChanViewport> LayoutRegionVec;

class LayoutItem {
private:
  int _x, _y;
  LayoutRegionVec _regions;
public:
  INLINE LayoutItem(void);
  INLINE LayoutItem(int, int);
  INLINE LayoutItem(const LayoutItem&);
  INLINE ~LayoutItem(void);

  INLINE LayoutItem& operator=(const LayoutItem&);
  INLINE void AddRegion(const ChanViewport&);
  INLINE int GetNumRegions(void);
  INLINE const ChanViewport& operator[](int);
};

typedef std::map<std::string, LayoutItem> LayoutType;

extern LayoutType* LayoutDB;

void ResetLayout();
void ParseLayout(istream&);

#include "chanlayout.I"

#endif /* __CHANLAYOUT_H__ */
