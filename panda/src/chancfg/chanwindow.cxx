// Filename: chanwindow.cxx
// Created by:  cary (06Feb99)
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

#include "chanwindow.h"
#include "chanparse.h"
#include "chanshare.h"

#include "notify.h"

WindowType* WindowDB = (WindowType*)0;

class WindowParseFunctor : public ChanParseFunctor {
public:
  INLINE WindowParseFunctor(void) : ChanParseFunctor() {}
  virtual ~WindowParseFunctor(void);

  virtual void operator()(std::string);
};

WindowParseFunctor::~WindowParseFunctor(void) {
  return;
}

void WindowParseFunctor::operator()(std::string S) {
  std::string sym;

  ChanEatFrontWhite(S);
  sym = ChanReadNextWord(S);
  ChanCheckScoping(S);
  ChanDescope(S);

  //KEH temp
  //bool hw_chans = ChanReadNextBool(S);
  //bool dvr = ChanReadNextBool(S);
  //int hw_chan_offset = ChanReadNextInt(S);
  std::string layout = ChanReadNextWord(S);
  SetupSyms sv;
  PTA(int) cameraGroup;
  int group_index=0;
  {
    ChanCheckScoping(S);
    int i=ChanMatchingParen(S);
    std::string setupSubstring = S.substr(1,i-2);
    S.erase(0,i);
    ChanEatFrontWhite(setupSubstring);

    while (!setupSubstring.empty()) {

      ChanCheckScoping(setupSubstring);
      std::string groupSubstring = setupSubstring;
      //get the group substring
      int iend = setupSubstring.find_first_of(")");
      groupSubstring = setupSubstring.substr(1,iend-1);
      //then erase that portion from the setup string
      setupSubstring.erase(0,iend+1);
      ChanEatFrontWhite(setupSubstring);

      while (!groupSubstring.empty()) {

        int region_number = ChanReadNextInt(groupSubstring);
        if(region_number<0) {
          nout <<"Trying to read region index, which should be >= 0, ";
          nout << "but got "<<region_number<<"."<<endl;
          nout << "Ignoring this entry."<<endl;
          ChanReadNextWord(groupSubstring); //slurp it up, but ignore it.
        } else {
           //0 based.  We don't know yet how much room we actually need
           //as the information is in the un-analyzed layout.  cameraGroup
           //and sv are handled the same in this manner.
           while((int)sv.size() < region_number+1) {
             sv.push_back("");
             cameraGroup.push_back(0);
           }
           std::string stmp2 = ChanReadNextWord(groupSubstring);
           sv[region_number]=stmp2;
           cameraGroup[region_number]=group_index;
        }
      }
      group_index++;
    }
  }
  int X, Y;
  {
    ChanEatFrontWhite(S);
    ChanCheckScoping(S);
    int i = S.find_first_of(")");
    std::string screenSubstring = S.substr(1, i-1);
    S.erase(0, i+1);
    ChanEatFrontWhite(S);
    X = ChanReadNextInt(screenSubstring);
    Y = ChanReadNextInt(screenSubstring);
  }
  bool border = ChanReadNextBool(S);
  bool cursor = ChanReadNextBool(S);
  
  //KEH 
  bool hw_chans = false;
  int hw_chan_offset = 0;
  bool dvr = false;
  if (!S.empty()) {
    hw_chan_offset = ChanReadNextInt(S);
    hw_chans = true;
  }
  if (!S.empty()) {
    // error, should have consumed all of the data by now
    nout << "error, trailing text in window spec '" << S << "'" << endl;
    S.erase(0, std::string::npos);
  }

  WindowItem W(hw_chans, dvr, hw_chan_offset, layout, sv, X, Y, border,
         cursor, cameraGroup);

  if (chancfg_cat.is_debug()) {
    chancfg_cat->debug() << "parsed a window called '" << sym << "':" << endl;
    chancfg_cat->debug() << "  do" << (W.getHWChans()?" ":" not ")
             << "use HW channels" << endl;
    chancfg_cat->debug() << "  do" << (W.getDVR()?" ":" not ") << "use DVR"
             << endl;
    chancfg_cat->debug() << "  do" << (W.getBorder()?" ":" not ")
             << "have a border" << endl;
    chancfg_cat->debug() << "  do" << (W.getCursor()?" ":" not ")
             << "have a cursor" << endl;
    chancfg_cat->debug() << "  HW channel offset: " << W.getChanOffset()
             << endl;
    chancfg_cat->debug() << "  prefered window size: (" << W.getSizeX()
             << ", " << W.getSizeY() << ")" << endl;
    chancfg_cat->debug() << "  layout: '" << W.getLayout() << "'" << endl;
    chancfg_cat->debug() << "  setups:" << endl;
    SetupSyms q(W.getSetups());
    for (SetupSyms::iterator r=q.begin(); r!=q.end(); ++r)
      chancfg_cat->debug() << "    '" << *r << "'" << endl;
  }
  (*WindowDB)[sym] = W;
}

void ResetWindow() {
  if (WindowDB != (WindowType *)NULL) {
    delete WindowDB;
  }
  WindowDB = new WindowType;
}

void ParseWindow(istream& is) {
  WindowParseFunctor w;
  ChanParse(is, &w);
}
