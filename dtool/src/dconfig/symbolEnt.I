// Filename: symbolEnt.I
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////


INLINE SymbolEnt& SymbolEnt::operator=(const SymbolEnt& c) {
  _src = c._src;
  _val = c._val;
  _srctok = c._srctok;
  _state = c._state;
  return *this;
}

INLINE bool SymbolEnt::
operator == (const SymbolEnt &) const {
  return true;
}

INLINE bool SymbolEnt::
operator != (const SymbolEnt &) const {
  return false;
}

INLINE bool SymbolEnt::
operator < (const SymbolEnt &) const {
  return false;
}

INLINE ConfigString SymbolEnt::Src(void) const {
  return _src;
}

INLINE ConfigString SymbolEnt::Val(void) const {
  return _val;
}

INLINE SymbolEnt::SymbolEntSrc SymbolEnt::SrcTok(void) const {
  return _srctok;
}

INLINE bool SymbolEnt::State(void) const {
  return _state;
}
