// Filename: chansetup.cxx
// Created by:  cary (05Feb99)
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

#include "chansetup.h"
#include "chanparse.h"
#include "chanshare.h"
#include "notify.h"

SetupType* SetupDB = (SetupType*)0;

class SetupParseFunctor : public ChanParseFunctor {
public:
  INLINE SetupParseFunctor(void) : ChanParseFunctor() {}
  virtual ~SetupParseFunctor(void);

  virtual void operator()(std::string);
};

SetupParseFunctor::~SetupParseFunctor(void) {
  return;
}

void SetupParseFunctor::operator()(std::string S) {
  std::string sym;

  ChanEatFrontWhite(S);
  sym = ChanReadNextWord(S);
  ChanCheckScoping(S);
  ChanDescope(S);

  SetupItem s;

  bool stereo = ChanReadNextBool(S);
  //KEH
  //bool hw_chan = ChanReadNextBool(S);
  //int chan_num = ChanReadNextIntB(S);

  ChanCheckScoping(S);
  int i = S.find_first_of(")");
  std::string stmp = S.substr(1, i-2);
  S.erase(0, i+1);
  ChanEatFrontWhite(S);
  ChanViewport v(ChanReadNextFloat(stmp), ChanReadNextFloat(stmp),
         ChanReadNextFloat(stmp), ChanReadNextFloat(stmp));
  if (!stmp.empty()) {
    // error, there shouldn't be anything left after eating the viewport
    nout << "error, tailing text in viewport spec '" << stmp << "'" << endl;
    stmp.erase(0, std::string::npos);
  }
  SetupFOV fov;
  if (S[0] == '#') {
    bool b = ChanReadNextBool(S);
    if (b) {
        // error, #t is not allowed here
        nout << "error, cannot have #t for FOV spec" << endl;
        b = false;
    }
    fov.setFOV();
  } else if (S[0] == '(') {
    i = S.find_first_of(")");
    stmp = S.substr(1, i-2);
    S.erase(0, i+1);
    ChanEatFrontWhite(S);
    fov.setFOV(ChanReadNextFloat(stmp), ChanReadNextFloat(stmp));
    if (!stmp.empty()) {
        // error, there shouldn't be anything left after eating the fov
        nout << "error, trailing text after fov spec '" << stmp << "'" << endl;
        stmp.erase(0, std::string::npos);
    }
  } else {
    fov.setFOV(ChanReadNextFloat(S));
  }
  SetupItem::Orientation orie = SetupItem::Up;
  if (!S.empty()) {
    stmp = ChanReadNextWord(S);
    if (stmp == "up") {
      // nothing really to do
      orie = SetupItem::Up;
    } else if (stmp == "down") {
      orie = SetupItem::Down;
    } else if (stmp == "left") {
      orie = SetupItem::Left;
    } else if (stmp == "right") {
      orie = SetupItem::Right;
    } else {
      // error, not a recognized orientation
      nout << "error, invalid orientation '" << stmp << "'" << endl;
      stmp.erase(0, std::string::npos);
    }
  }
  if (!S.empty()) {
    // error, we should have consumed all the data by now
    nout << "error, trailing text on setup spec '" << S << "'" << endl;
    S.erase(0, std::string::npos);
  }
  //KEH
  //s.setState(stereo, hw_chan, chan_num, v, fov, orie);
  s.setState(stereo, false, -1, v, fov, orie);

  if (chancfg_cat.is_debug()) {
    chancfg_cat->debug() << "parsed a setup called '" << sym << "':" << endl;
    chancfg_cat->debug() << "  is" << (s.getStereo()?" ":" not ")
             << "a stereo setup" << endl;
    chancfg_cat->debug() <<"   is" << (s.getHWChan()?" ":" not ")
             << "a HW channel setup" << endl;
    chancfg_cat->debug() << "  prefered HW channel number: " << s.getChan()
             << endl;
    ChanViewport qv(s.getViewport());
    chancfg_cat->debug() << "  sub viewport: (" << qv.left() << ", "
             << qv.right() << ", " << qv.bottom() << ", "
             << qv.top() << ")" << endl;
    SetupFOV qf(s.getFOV());
    switch (qf.getType()) {
      case SetupFOV::Invalid:
        chancfg_cat->debug() << "  FOV is invalid" << endl;
        break;
      case SetupFOV::Default:
        chancfg_cat->debug() << "  FOV takes defaults" << endl;
        break;
      case SetupFOV::Horizontal:
        chancfg_cat->debug() << "  FOV specifies only horizontal: "
                     << qf.getHoriz() << endl;
        break;
      case SetupFOV::Both:
        chancfg_cat->debug() << "  FOV: " << qf.getHoriz() << " x "
                     << qf.getVert() << endl;
        break;
      default:
        chancfg_cat->debug() << "  FOV is of an unknown type (" << (int)qf.getType()
                 << ")" << endl;
    }
    switch (s.getOrientation()) {
      case SetupItem::Up:
        chancfg_cat->debug() << "  setup is oriented Up" << endl;
        break;
      case SetupItem::Down:
        chancfg_cat->debug() << "  setup is oriented Down" << endl;
        break;
      case SetupItem::Left:
        chancfg_cat->debug() << "  setup is oriented Left" << endl;
        break;
      case SetupItem::Right:
        chancfg_cat->debug() << "  setup is oriented Right" << endl;
        break;
      default:
        chancfg_cat->debug() << "  setup has an unknown orientation ("
                    << (int)s.getOrientation() << ")" << endl;
    }
  }
  (*SetupDB)[sym] = s;
}


void ResetSetup() {
  if (SetupDB != (SetupType *)NULL) {
    delete SetupDB;
  }
  SetupDB = new SetupType;
}

void ParseSetup(istream& is) {
  SetupParseFunctor s;
  ChanParse(is, &s);
}
