// Filename: qpnodePathLerps.h
// Created by:  frang (01Jun00)
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

#ifndef qpNODEPATHLERPS_H
#define qpNODEPATHLERPS_H

#include "pandabase.h"

#include "lerpfunctor.h"
#include "qpnodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : PosLerpFunctor
// Description : Class for Lerping between positions in space
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpPosLerpFunctor : public LPoint3fLerpFunctor {
private:
  qpNodePath _node_path;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpPosLerpFunctor(qpNodePath np, LPoint3f start, LPoint3f end)
    : LPoint3fLerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  qpPosLerpFunctor(qpNodePath np, float sx, float sy, float sz, float ex, float ey,
                 float ez) : LPoint3fLerpFunctor(LPoint3f(sx, sy, sz),
                                                 LPoint3f(ex, ey, ez)),
                             _node_path(np), _is_wrt(false) {}
  qpPosLerpFunctor(qpNodePath np, LPoint3f start, LPoint3f end, qpNodePath wrt)
    : LPoint3fLerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  qpPosLerpFunctor(qpNodePath np, float sx, float sy, float sz, float ex, float ey,
                 float ez, qpNodePath wrt)
    : LPoint3fLerpFunctor(LPoint3f(sx, sy, sz), LPoint3f(ex, ey, ez)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  qpPosLerpFunctor(const qpPosLerpFunctor&);
  virtual ~qpPosLerpFunctor(void);
  qpPosLerpFunctor& operator=(const qpPosLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LPoint3fLerpFunctor::init_type();
    register_type(_type_handle, "qpPosLerpFunctor",
                  LPoint3fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};


// evil bad bad evil HPR
////////////////////////////////////////////////////////////////////
//       Class : HprLerpFunctor
// Description : Class for Lerping between orientations in space
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpHprLerpFunctor : public LVecBase3fLerpFunctor {
private:
  qpNodePath _node_path;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpHprLerpFunctor(qpNodePath np, LVecBase3f start, LVecBase3f end)
    : LVecBase3fLerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  qpHprLerpFunctor(qpNodePath np, float sx, float sy, float sz, float ex, float ey,
                 float ez) : LVecBase3fLerpFunctor(LVecBase3f(sx, sy, sz),
                                                  LVecBase3f(ex, ey, ez)),
                             _node_path(np), _is_wrt(false) {}
  qpHprLerpFunctor(qpNodePath np, LVecBase3f start, LVecBase3f end, qpNodePath wrt)
    : LVecBase3fLerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  qpHprLerpFunctor(qpNodePath np, float sx, float sy, float sz, float ex, float ey,
                 float ez, qpNodePath wrt)
    : LVecBase3fLerpFunctor(LVecBase3f(sx, sy, sz), LVecBase3f(ex, ey, ez)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}
  void take_shortest(void);
  void take_longest(void);

public:
  qpHprLerpFunctor(const qpHprLerpFunctor&);
  virtual ~qpHprLerpFunctor(void);
  qpHprLerpFunctor& operator=(const qpHprLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LVecBase3fLerpFunctor::init_type();
    register_type(_type_handle, "qpHprLerpFunctor",
                  LVecBase3fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : qpScaleLerpFunctor
// Description : Class for Lerping between scales
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpScaleLerpFunctor : public LVecBase3fLerpFunctor {
private:
  qpNodePath _node_path;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpScaleLerpFunctor(qpNodePath np, LVecBase3f start, LVecBase3f end)
    : LVecBase3fLerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  qpScaleLerpFunctor(qpNodePath np, float sx, float sy, float sz, float ex,
                   float ey, float ez)
    : LVecBase3fLerpFunctor(LVecBase3f(sx, sy, sz), LVecBase3f(ex, ey, ez)),
      _node_path(np), _is_wrt(false) {}
  qpScaleLerpFunctor(qpNodePath np, LVecBase3f start, LVecBase3f end, qpNodePath wrt)
    : LVecBase3fLerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  qpScaleLerpFunctor(qpNodePath np, float sx, float sy, float sz, float ex,
                   float ey, float ez, qpNodePath wrt)
    : LVecBase3fLerpFunctor(LVecBase3f(sx, sy, sz), LVecBase3f(ex, ey, ez)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  qpScaleLerpFunctor(const qpScaleLerpFunctor&);
  virtual ~qpScaleLerpFunctor(void);
  qpScaleLerpFunctor& operator=(const qpScaleLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LVecBase3fLerpFunctor::init_type();
    register_type(_type_handle, "qpScaleLerpFunctor",
                  LVecBase3fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : qpColorLerpFunctor
// Description : Class for Lerping between colors
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpColorLerpFunctor : public LVecBase4fLerpFunctor {
private:
  qpNodePath _node_path;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpColorLerpFunctor(qpNodePath np, LVecBase4f start, LVecBase4f end)
    : LVecBase4fLerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  qpColorLerpFunctor(qpNodePath np, float sr, float sg, float sb, float sa,
                float er, float eg, float eb, float ea) : LVecBase4fLerpFunctor(LVecBase4f(sr, sg, sb, sa),
                                                 LVecBase4f(er, eg, eb, ea)), _node_path(np), _is_wrt(false) {}
  qpColorLerpFunctor(qpNodePath np, LVecBase4f start, LVecBase4f end, qpNodePath wrt)
    : LVecBase4fLerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  qpColorLerpFunctor(qpNodePath np, float sr, float sg, float sb, float sa, float er, float eg,
                 float eb, float ea, qpNodePath wrt)
    : LVecBase4fLerpFunctor(LVecBase4f(sr, sg, sb, sa), LVecBase4f(er, eg, eb, ea)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  qpColorLerpFunctor(const qpColorLerpFunctor&);
  virtual ~qpColorLerpFunctor(void);
  qpColorLerpFunctor& operator=(const qpColorLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LVecBase4fLerpFunctor::init_type();
    register_type(_type_handle, "qpColorLerpFunctor",
                  LVecBase4fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : qpPosHprLerpFunctor
// Description : Class for Lerping between positions and orientations
//               in space
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpPosHprLerpFunctor : public LerpFunctor {
private:
  qpNodePath _node_path;
  LPoint3f _pstart;
  LPoint3f _pend;
  LPoint3f _pdiff_cache;
  LVecBase3f _hstart;
  LVecBase3f _hend;
  LVecBase3f _hdiff_cache;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpPosHprLerpFunctor(qpNodePath np, LPoint3f pstart, LPoint3f pend,
                    LVecBase3f hstart, LVecBase3f hend)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _is_wrt(false) {}
  qpPosHprLerpFunctor(qpNodePath np, float psx, float psy, float psz, float pex,
                    float pey, float pez, float hsx, float hsy, float hsz,
                    float hex, float hey, float hez)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend - _hstart), _is_wrt(false) {}
  qpPosHprLerpFunctor(qpNodePath np, LPoint3f pstart, LPoint3f pend,
                    LVecBase3f hstart, LVecBase3f hend, qpNodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _is_wrt(true), _wrt_path(wrt) {}
  qpPosHprLerpFunctor(qpNodePath np, float psx, float psy, float psz, float pex,
                    float pey, float pez, float hsx, float hsy, float hsz,
                    float hex, float hey, float hez, qpNodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend - _hstart), _is_wrt(true), _wrt_path(wrt) {}
  void take_shortest(void);
  void take_longest(void);

public:
  qpPosHprLerpFunctor(const qpPosHprLerpFunctor&);
  virtual ~qpPosHprLerpFunctor(void);
  qpPosHprLerpFunctor& operator=(const qpPosHprLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LerpFunctor::init_type();
    register_type(_type_handle, "qpPosHprLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : qpHprScaleLerpFunctor
// Description : Class for Lerping between orientation
//               and scale
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpHprScaleLerpFunctor : public LerpFunctor {
private:
  qpNodePath _node_path;
  LVecBase3f _hstart;
  LVecBase3f _hend;
  LVecBase3f _hdiff_cache;
  LVecBase3f _sstart;
  LVecBase3f _send;
  LVecBase3f _sdiff_cache;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpHprScaleLerpFunctor(qpNodePath np, 
		      LVecBase3f hstart, LVecBase3f hend, LVecBase3f sstart,
		      LVecBase3f send)
    : LerpFunctor(), _node_path(np),
      _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(false) {}
  qpHprScaleLerpFunctor(qpNodePath np, float hsx, float hsy,
                         float hsz, float hex, float hey, float hez, float ssx,
                         float ssy, float ssz, float sex, float sey, float sez)
    : LerpFunctor(), _node_path(np),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(false) {}
  qpHprScaleLerpFunctor(qpNodePath np, 
		      LVecBase3f hstart, LVecBase3f hend, LVecBase3f sstart,
		      LVecBase3f send, qpNodePath wrt)
    : LerpFunctor(), _node_path(np), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(true), _wrt_path(wrt) {}
  qpHprScaleLerpFunctor(qpNodePath np, float hsx, float hsy,
		      float hsz, float hex, float hey, float hez, float ssx,
		      float ssy, float ssz, float sex, float sey, float sez,
		      qpNodePath wrt)
    : LerpFunctor(), _node_path(np),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(true),
      _wrt_path(wrt) {}
  void take_shortest(void);
  void take_longest(void);

public:
  qpHprScaleLerpFunctor(const qpHprScaleLerpFunctor&);
  virtual ~qpHprScaleLerpFunctor(void);
  qpHprScaleLerpFunctor& operator=(const qpHprScaleLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LerpFunctor::init_type();
    register_type(_type_handle, "qpHprScaleLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : qpPosHprScaleLerpFunctor
// Description : Class for Lerping between position, orientation,
//               and scale
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpPosHprScaleLerpFunctor : public LerpFunctor {
private:
  qpNodePath _node_path;
  LPoint3f _pstart;
  LPoint3f _pend;
  LPoint3f _pdiff_cache;
  LVecBase3f _hstart;
  LVecBase3f _hend;
  LVecBase3f _hdiff_cache;
  LVecBase3f _sstart;
  LVecBase3f _send;
  LVecBase3f _sdiff_cache;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpPosHprScaleLerpFunctor(qpNodePath np, LPoint3f pstart, LPoint3f pend,
                         LVecBase3f hstart, LVecBase3f hend, LVecBase3f sstart,
                         LVecBase3f send)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(false) {}
  qpPosHprScaleLerpFunctor(qpNodePath np, float psx, float psy, float psz,
                         float pex, float pey, float pez, float hsx, float hsy,
                         float hsz, float hex, float hey, float hez, float ssx,
                         float ssy, float ssz, float sex, float sey, float sez)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(false) {}
  qpPosHprScaleLerpFunctor(qpNodePath np, LPoint3f pstart, LPoint3f pend,
                         LVecBase3f hstart, LVecBase3f hend, LVecBase3f sstart,
                         LVecBase3f send, qpNodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(true), _wrt_path(wrt) {}
  qpPosHprScaleLerpFunctor(qpNodePath np, float psx, float psy, float psz,
                         float pex, float pey, float pez, float hsx, float hsy,
                         float hsz, float hex, float hey, float hez, float ssx,
                         float ssy, float ssz, float sex, float sey, float sez,
                         qpNodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(true),
      _wrt_path(wrt) {}
  void take_shortest(void);
  void take_longest(void);

public:
  qpPosHprScaleLerpFunctor(const qpPosHprScaleLerpFunctor&);
  virtual ~qpPosHprScaleLerpFunctor(void);
  qpPosHprScaleLerpFunctor& operator=(const qpPosHprScaleLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LerpFunctor::init_type();
    register_type(_type_handle, "qpPosHprScaleLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : qpColorScaleLerpFunctor
// Description : Class for Lerping between color scales
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpColorScaleLerpFunctor : public LVecBase4fLerpFunctor {
private:
  qpNodePath _node_path;
  bool _is_wrt;
  qpNodePath _wrt_path;

PUBLISHED:
  qpColorScaleLerpFunctor(qpNodePath np, LVecBase4f start, LVecBase4f end)
    : LVecBase4fLerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  qpColorScaleLerpFunctor(qpNodePath np, float sr, float sg, float sb, float sa,
                float er, float eg, float eb, float ea) : LVecBase4fLerpFunctor(LVecBase4f(sr, sg, sb, sa),
                                                 LVecBase4f(er, eg, eb, ea)), _node_path(np), _is_wrt(false) {}
  qpColorScaleLerpFunctor(qpNodePath np, LVecBase4f start, LVecBase4f end, qpNodePath wrt)
    : LVecBase4fLerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  qpColorScaleLerpFunctor(qpNodePath np, float sr, float sg, float sb, float sa, float er, float eg,
                 float eb, float ea, qpNodePath wrt)
    : LVecBase4fLerpFunctor(LVecBase4f(sr, sg, sb, sa), LVecBase4f(er, eg, eb, ea)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  qpColorScaleLerpFunctor(const qpColorScaleLerpFunctor&);
  virtual ~qpColorScaleLerpFunctor(void);
  qpColorScaleLerpFunctor& operator=(const qpColorScaleLerpFunctor&);
  virtual void operator()(float);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LVecBase4fLerpFunctor::init_type();
    register_type(_type_handle, "qpColorScaleLerpFunctor",
                  LVecBase4fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#endif /* qpNODEPATHLERPS_H */





