// Filename: nodePathLerps.h
// Created by:  frang (01Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef NODEPATHLERPS_H
#define NODEPATHLERPS_H

#include "pandabase.h"

#include "lerpfunctor.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : PosLerpFunctor
// Description : Class for Lerping between positions in space
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH PosLerpFunctor : public LPoint3LerpFunctor {
private:
  NodePath _node_path;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  PosLerpFunctor(NodePath np, LPoint3 start, LPoint3 end)
    : LPoint3LerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  PosLerpFunctor(NodePath np, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz, PN_stdfloat ex, PN_stdfloat ey,
                 PN_stdfloat ez) : LPoint3LerpFunctor(LPoint3(sx, sy, sz),
                                                 LPoint3(ex, ey, ez)),
                             _node_path(np), _is_wrt(false) {}
  PosLerpFunctor(NodePath np, LPoint3 start, LPoint3 end, NodePath wrt)
    : LPoint3LerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  PosLerpFunctor(NodePath np, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz, PN_stdfloat ex, PN_stdfloat ey,
                 PN_stdfloat ez, NodePath wrt)
    : LPoint3LerpFunctor(LPoint3(sx, sy, sz), LPoint3(ex, ey, ez)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  PosLerpFunctor(const PosLerpFunctor&);
  virtual ~PosLerpFunctor();
  PosLerpFunctor& operator=(const PosLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LPoint3LerpFunctor::init_type();
    register_type(_type_handle, "PosLerpFunctor",
                  LPoint3LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
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
class EXPCL_PANDA_PGRAPH HprLerpFunctor : public LVecBase3LerpFunctor {
private:
  NodePath _node_path;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  HprLerpFunctor(NodePath np, LVecBase3 start, LVecBase3 end)
    : LVecBase3LerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  HprLerpFunctor(NodePath np, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz, PN_stdfloat ex, PN_stdfloat ey,
                 PN_stdfloat ez) : LVecBase3LerpFunctor(LVecBase3(sx, sy, sz),
                                                  LVecBase3(ex, ey, ez)),
                             _node_path(np), _is_wrt(false) {}
  HprLerpFunctor(NodePath np, LVecBase3 start, LVecBase3 end, NodePath wrt)
    : LVecBase3LerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  HprLerpFunctor(NodePath np, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz, PN_stdfloat ex, PN_stdfloat ey,
                 PN_stdfloat ez, NodePath wrt)
    : LVecBase3LerpFunctor(LVecBase3(sx, sy, sz), LVecBase3(ex, ey, ez)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}
  void take_shortest();
  void take_longest();

public:
  HprLerpFunctor(const HprLerpFunctor&);
  virtual ~HprLerpFunctor();
  HprLerpFunctor& operator=(const HprLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LVecBase3LerpFunctor::init_type();
    register_type(_type_handle, "HprLerpFunctor",
                  LVecBase3LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : ScaleLerpFunctor
// Description : Class for Lerping between scales
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ScaleLerpFunctor : public LVecBase3LerpFunctor {
private:
  NodePath _node_path;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  ScaleLerpFunctor(NodePath np, LVecBase3 start, LVecBase3 end)
    : LVecBase3LerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  ScaleLerpFunctor(NodePath np, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz, PN_stdfloat ex,
                   PN_stdfloat ey, PN_stdfloat ez)
    : LVecBase3LerpFunctor(LVecBase3(sx, sy, sz), LVecBase3(ex, ey, ez)),
      _node_path(np), _is_wrt(false) {}
  ScaleLerpFunctor(NodePath np, LVecBase3 start, LVecBase3 end, NodePath wrt)
    : LVecBase3LerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  ScaleLerpFunctor(NodePath np, PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz, PN_stdfloat ex,
                   PN_stdfloat ey, PN_stdfloat ez, NodePath wrt)
    : LVecBase3LerpFunctor(LVecBase3(sx, sy, sz), LVecBase3(ex, ey, ez)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  ScaleLerpFunctor(const ScaleLerpFunctor&);
  virtual ~ScaleLerpFunctor();
  ScaleLerpFunctor& operator=(const ScaleLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LVecBase3LerpFunctor::init_type();
    register_type(_type_handle, "ScaleLerpFunctor",
                  LVecBase3LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorLerpFunctor
// Description : Class for Lerping between colors
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ColorLerpFunctor : public LVecBase4LerpFunctor {
private:
  NodePath _node_path;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  ColorLerpFunctor(NodePath np, LVecBase4 start, LVecBase4 end)
    : LVecBase4LerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  ColorLerpFunctor(NodePath np, PN_stdfloat sr, PN_stdfloat sg, PN_stdfloat sb, PN_stdfloat sa,
                PN_stdfloat er, PN_stdfloat eg, PN_stdfloat eb, PN_stdfloat ea) : LVecBase4LerpFunctor(LVecBase4(sr, sg, sb, sa),
                                                 LVecBase4(er, eg, eb, ea)), _node_path(np), _is_wrt(false) {}
  ColorLerpFunctor(NodePath np, LVecBase4 start, LVecBase4 end, NodePath wrt)
    : LVecBase4LerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  ColorLerpFunctor(NodePath np, PN_stdfloat sr, PN_stdfloat sg, PN_stdfloat sb, PN_stdfloat sa, PN_stdfloat er, PN_stdfloat eg,
                 PN_stdfloat eb, PN_stdfloat ea, NodePath wrt)
    : LVecBase4LerpFunctor(LVecBase4(sr, sg, sb, sa), LVecBase4(er, eg, eb, ea)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  ColorLerpFunctor(const ColorLerpFunctor&);
  virtual ~ColorLerpFunctor();
  ColorLerpFunctor& operator=(const ColorLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LVecBase4LerpFunctor::init_type();
    register_type(_type_handle, "ColorLerpFunctor",
                  LVecBase4LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : PosHprLerpFunctor
// Description : Class for Lerping between positions and orientations
//               in space
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH PosHprLerpFunctor : public LerpFunctor {
private:
  NodePath _node_path;
  LPoint3 _pstart;
  LPoint3 _pend;
  LPoint3 _pdiff_cache;
  LVecBase3 _hstart;
  LVecBase3 _hend;
  LVecBase3 _hdiff_cache;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  PosHprLerpFunctor(NodePath np, LPoint3 pstart, LPoint3 pend,
                    LVecBase3 hstart, LVecBase3 hend)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _is_wrt(false) {}
  PosHprLerpFunctor(NodePath np, PN_stdfloat psx, PN_stdfloat psy, PN_stdfloat psz, PN_stdfloat pex,
                    PN_stdfloat pey, PN_stdfloat pez, PN_stdfloat hsx, PN_stdfloat hsy, PN_stdfloat hsz,
                    PN_stdfloat hex, PN_stdfloat hey, PN_stdfloat hez)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend - _hstart), _is_wrt(false) {}
  PosHprLerpFunctor(NodePath np, LPoint3 pstart, LPoint3 pend,
                    LVecBase3 hstart, LVecBase3 hend, NodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _is_wrt(true), _wrt_path(wrt) {}
  PosHprLerpFunctor(NodePath np, PN_stdfloat psx, PN_stdfloat psy, PN_stdfloat psz, PN_stdfloat pex,
                    PN_stdfloat pey, PN_stdfloat pez, PN_stdfloat hsx, PN_stdfloat hsy, PN_stdfloat hsz,
                    PN_stdfloat hex, PN_stdfloat hey, PN_stdfloat hez, NodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend - _hstart), _is_wrt(true), _wrt_path(wrt) {}
  void take_shortest();
  void take_longest();

public:
  PosHprLerpFunctor(const PosHprLerpFunctor&);
  virtual ~PosHprLerpFunctor();
  PosHprLerpFunctor& operator=(const PosHprLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LerpFunctor::init_type();
    register_type(_type_handle, "PosHprLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : HprScaleLerpFunctor
// Description : Class for Lerping between orientation
//               and scale
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH HprScaleLerpFunctor : public LerpFunctor {
private:
  NodePath _node_path;
  LVecBase3 _hstart;
  LVecBase3 _hend;
  LVecBase3 _hdiff_cache;
  LVecBase3 _sstart;
  LVecBase3 _send;
  LVecBase3 _sdiff_cache;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  HprScaleLerpFunctor(NodePath np, 
                      LVecBase3 hstart, LVecBase3 hend, LVecBase3 sstart,
                      LVecBase3 send)
    : LerpFunctor(), _node_path(np),
      _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(false) {}
  HprScaleLerpFunctor(NodePath np, PN_stdfloat hsx, PN_stdfloat hsy,
                         PN_stdfloat hsz, PN_stdfloat hex, PN_stdfloat hey, PN_stdfloat hez, PN_stdfloat ssx,
                         PN_stdfloat ssy, PN_stdfloat ssz, PN_stdfloat sex, PN_stdfloat sey, PN_stdfloat sez)
    : LerpFunctor(), _node_path(np),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(false) {}
  HprScaleLerpFunctor(NodePath np, 
                      LVecBase3 hstart, LVecBase3 hend, LVecBase3 sstart,
                      LVecBase3 send, NodePath wrt)
    : LerpFunctor(), _node_path(np), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(true), _wrt_path(wrt) {}
  HprScaleLerpFunctor(NodePath np, PN_stdfloat hsx, PN_stdfloat hsy,
                      PN_stdfloat hsz, PN_stdfloat hex, PN_stdfloat hey, PN_stdfloat hez, PN_stdfloat ssx,
                      PN_stdfloat ssy, PN_stdfloat ssz, PN_stdfloat sex, PN_stdfloat sey, PN_stdfloat sez,
                      NodePath wrt)
    : LerpFunctor(), _node_path(np),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(true),
      _wrt_path(wrt) {}
  void take_shortest();
  void take_longest();

public:
  HprScaleLerpFunctor(const HprScaleLerpFunctor&);
  virtual ~HprScaleLerpFunctor();
  HprScaleLerpFunctor& operator=(const HprScaleLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LerpFunctor::init_type();
    register_type(_type_handle, "HprScaleLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : PosHprScaleLerpFunctor
// Description : Class for Lerping between position, orientation,
//               and scale
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH PosHprScaleLerpFunctor : public LerpFunctor {
private:
  NodePath _node_path;
  LPoint3 _pstart;
  LPoint3 _pend;
  LPoint3 _pdiff_cache;
  LVecBase3 _hstart;
  LVecBase3 _hend;
  LVecBase3 _hdiff_cache;
  LVecBase3 _sstart;
  LVecBase3 _send;
  LVecBase3 _sdiff_cache;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  PosHprScaleLerpFunctor(NodePath np, LPoint3 pstart, LPoint3 pend,
                         LVecBase3 hstart, LVecBase3 hend, LVecBase3 sstart,
                         LVecBase3 send)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(false) {}
  PosHprScaleLerpFunctor(NodePath np, PN_stdfloat psx, PN_stdfloat psy, PN_stdfloat psz,
                         PN_stdfloat pex, PN_stdfloat pey, PN_stdfloat pez, PN_stdfloat hsx, PN_stdfloat hsy,
                         PN_stdfloat hsz, PN_stdfloat hex, PN_stdfloat hey, PN_stdfloat hez, PN_stdfloat ssx,
                         PN_stdfloat ssy, PN_stdfloat ssz, PN_stdfloat sex, PN_stdfloat sey, PN_stdfloat sez)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(false) {}
  PosHprScaleLerpFunctor(NodePath np, LPoint3 pstart, LPoint3 pend,
                         LVecBase3 hstart, LVecBase3 hend, LVecBase3 sstart,
                         LVecBase3 send, NodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(pstart), _pend(pend),
      _pdiff_cache(pend-pstart), _hstart(hstart), _hend(hend),
      _hdiff_cache(hend-hstart), _sstart(sstart), _send(send),
      _sdiff_cache(send-sstart), _is_wrt(true), _wrt_path(wrt) {}
  PosHprScaleLerpFunctor(NodePath np, PN_stdfloat psx, PN_stdfloat psy, PN_stdfloat psz,
                         PN_stdfloat pex, PN_stdfloat pey, PN_stdfloat pez, PN_stdfloat hsx, PN_stdfloat hsy,
                         PN_stdfloat hsz, PN_stdfloat hex, PN_stdfloat hey, PN_stdfloat hez, PN_stdfloat ssx,
                         PN_stdfloat ssy, PN_stdfloat ssz, PN_stdfloat sex, PN_stdfloat sey, PN_stdfloat sez,
                         NodePath wrt)
    : LerpFunctor(), _node_path(np), _pstart(psx, psy, psz),
      _pend(pex, pey, pez), _pdiff_cache(_pend-_pstart),
      _hstart(hsx, hsy, hsz), _hend(hex, hey, hez),
      _hdiff_cache(_hend-_hstart), _sstart(ssx, ssy, ssz),
      _send(sex, sey, sez), _sdiff_cache(_send-_sstart), _is_wrt(true),
      _wrt_path(wrt) {}
  void take_shortest();
  void take_longest();

public:
  PosHprScaleLerpFunctor(const PosHprScaleLerpFunctor&);
  virtual ~PosHprScaleLerpFunctor();
  PosHprScaleLerpFunctor& operator=(const PosHprScaleLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LerpFunctor::init_type();
    register_type(_type_handle, "PosHprScaleLerpFunctor",
                  LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorScaleLerpFunctor
// Description : Class for Lerping between color scales
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ColorScaleLerpFunctor : public LVecBase4LerpFunctor {
private:
  NodePath _node_path;
  bool _is_wrt;
  NodePath _wrt_path;

PUBLISHED:
  ColorScaleLerpFunctor(NodePath np, LVecBase4 start, LVecBase4 end)
    : LVecBase4LerpFunctor(start, end), _node_path(np), _is_wrt(false) {}
  ColorScaleLerpFunctor(NodePath np, PN_stdfloat sr, PN_stdfloat sg, PN_stdfloat sb, PN_stdfloat sa,
                PN_stdfloat er, PN_stdfloat eg, PN_stdfloat eb, PN_stdfloat ea) : LVecBase4LerpFunctor(LVecBase4(sr, sg, sb, sa),
                                                 LVecBase4(er, eg, eb, ea)), _node_path(np), _is_wrt(false) {}
  ColorScaleLerpFunctor(NodePath np, LVecBase4 start, LVecBase4 end, NodePath wrt)
    : LVecBase4LerpFunctor(start, end), _node_path(np), _is_wrt(true),
      _wrt_path(wrt) {}
  ColorScaleLerpFunctor(NodePath np, PN_stdfloat sr, PN_stdfloat sg, PN_stdfloat sb, PN_stdfloat sa, PN_stdfloat er, PN_stdfloat eg,
                 PN_stdfloat eb, PN_stdfloat ea, NodePath wrt)
    : LVecBase4LerpFunctor(LVecBase4(sr, sg, sb, sa), LVecBase4(er, eg, eb, ea)),
      _node_path(np), _is_wrt(true), _wrt_path(wrt) {}

public:
  ColorScaleLerpFunctor(const ColorScaleLerpFunctor&);
  virtual ~ColorScaleLerpFunctor();
  ColorScaleLerpFunctor& operator=(const ColorScaleLerpFunctor&);
  virtual void operator()(PN_stdfloat);

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LVecBase4LerpFunctor::init_type();
    register_type(_type_handle, "ColorScaleLerpFunctor",
                  LVecBase4LerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#endif /* NODEPATHLERPS_H */





