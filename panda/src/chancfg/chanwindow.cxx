// Filename: chanwindow.cxx
// Created by:  cary (06Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "chanwindow.h"
#include "chanparse.h"
#include "chanshare.h"

#include <notify.h>

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

  bool hw_chans = ChanReadNextBool(S);
  bool dvr = ChanReadNextBool(S);
  int hw_chan_offset = ChanReadNextInt(S);
  std::string layout = ChanReadNextWord(S);
  SetupSyms sv;
  {
    ChanCheckScoping(S);
    int i = S.find_first_of(")");
    std::string stmp = S.substr(1, i-1);
    S.erase(0, i+1);
    ChanEatFrontWhite(S);

    while (!stmp.empty()) {
      std::string stmp2 = ChanReadNextWord(stmp);
      sv.push_back(stmp2);
    }
  }
  int X, Y;
  {
    ChanCheckScoping(S);
    int i = S.find_first_of(")");
    std::string stmp = S.substr(1, i-1);
    S.erase(0, i+1);
    ChanEatFrontWhite(S);
    X = ChanReadNextInt(stmp);
    Y = ChanReadNextInt(stmp);
  }
  bool border = ChanReadNextBool(S);
  bool cursor = ChanReadNextBool(S);
  if (!S.empty()) {
    // error, should have consumed all of the data by now
    nout << "error, trailing text in window spec '" << S << "'" << endl;
    S.erase(0, std::string::npos);
  }

  WindowItem W(hw_chans, dvr, hw_chan_offset, layout, sv, X, Y, border,
           cursor);

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
