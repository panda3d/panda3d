// Filename: chanparse.h
// Created by:  cary (02Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CHANPARSE_H__
#define __CHANPARSE_H__

#include <pandabase.h>

#include <string>

class EXPCL_PANDA ChanParseFunctor {
public:
  INLINE ChanParseFunctor() {};
  virtual ~ChanParseFunctor();

  virtual void operator()(std::string) = 0;
};

int ChanMatchingParen(std::string);
void ChanParse(istream&, ChanParseFunctor*);

INLINE void ChanEatFrontWhite(std::string&);
INLINE void ChanCheckScoping(std::string&);
INLINE void ChanDescope(std::string&);
INLINE std::string ChanReadNextWord(std::string&);
INLINE int ChanReadNextInt(std::string&);
INLINE bool ChanReadNextBool(std::string&);
INLINE int ChanReadNextIntB(std::string&);
INLINE float ChanReadNextFloat(std::string&);

#include "chanparse.I"

#endif /* __CHANPARSE_H__ */
