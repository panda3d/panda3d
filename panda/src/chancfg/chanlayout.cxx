// Filename: chanlayout.cxx
// Created by:  cary (02Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "chanlayout.h"
#include "chanparse.h"
#include "chanshare.h"
#include "notify.h"

LayoutType* LayoutDB = (LayoutType*)0;

class LayoutParseFunctor : public ChanParseFunctor {
public:
  INLINE LayoutParseFunctor(void) : ChanParseFunctor() {}
  virtual ~LayoutParseFunctor(void);

  virtual void operator()(std::string);
};

LayoutParseFunctor::~LayoutParseFunctor(void) {
  return;
}

typedef pvector<bool> LayoutBoolVec;

void LayoutParseFunctor::operator()(std::string S) {
  std::string sym;

  ChanEatFrontWhite(S);
  sym = ChanReadNextWord(S);
  ChanCheckScoping(S);
  ChanDescope(S);

  int X, Y;

  X = ChanReadNextInt(S);
  Y = ChanReadNextInt(S);

  LayoutItem L(X, Y);
  LayoutBoolVec mask;
  int i=0;
  for (; i<(X*Y); ++i)
    mask.push_back(false);

  float xfrac = 1. / float(X);
  float yfrac = 1. / float(Y);

  while (!S.empty()) {
    ChanCheckScoping(S);
    i = S.find_first_of(")");
    std::string stmp = S.substr(1, i-1);
    S.erase(0, i+1);
    ChanEatFrontWhite(S);

    int m, n, start;

    m = ChanReadNextInt(stmp);
    n = ChanReadNextInt(stmp);
    start = ChanReadNextInt(stmp);

    int x = start % X;
    int y = (start - x) / X;
    bool ok = true;

    for (int j=y; j<y+n; ++j)
      for (int k=x; k<x+m; ++k)
    if (mask[(j*X)+k])
      ok = false;
    else
      mask[(j*X)+k] = true;
    if (ok) {
      ChanViewport l((x*xfrac), ((x+m)*xfrac), (y*yfrac), ((y+n)*yfrac));
      L.AddRegion(l);
    } else {
      nout << "error, region (" << m << " " << n << " " << start
       << ") overlaps another.  It is being skipped." << endl;
    }
  }
  for (i=0; i<(X*Y); ++i)
    if (!mask[i]) {
      // an otherwise unaccounted for sub-region
      int x = i % X;
      int y = (i - x) / X;
      ChanViewport l((x*xfrac), ((x+1)*xfrac), (y*yfrac), ((y+1)*yfrac));
      L.AddRegion(l);
    }

  if (chancfg_cat.is_debug()) {
    chancfg_cat->debug() << "parsed a layout called '" << sym << "':" << endl;
    chancfg_cat->debug() << "  " << L.GetNumRegions() << " regions" << endl;
    for (int q=0; q<L.GetNumRegions(); ++q) {
      const ChanViewport& v = L[q];
      chancfg_cat->debug() << "  region #" << q << ": (" << v.left() << ", "
               << v.right() << ", " << v.bottom() << ", "
               << v.top() << ")" << endl;
    }
  }
  (*LayoutDB)[sym] = L;
}

void ResetLayout() {
  if (LayoutDB != (LayoutType *)NULL) {
    delete LayoutDB;
  }
  LayoutDB = new LayoutType;
}

void ParseLayout(istream& is) {
  LayoutParseFunctor l;
  ChanParse(is, &l);
}
