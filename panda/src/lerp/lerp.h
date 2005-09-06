// Filename: lerp.h
// Created by:  frang (18Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef __LERP_H__
#define __LERP_H__

#include "pandabase.h"

#include "lerpfunctor.h"
#include "lerpblend.h"

#include "typedReferenceCount.h"
#include "eventHandler.h"

class EXPCL_PANDA Lerp : public TypedReferenceCount {
private:
  PT(LerpBlendType) _blend;
  PT(LerpFunctor) _func;
  std::string _event;
  float _startt;
  float _endt;
  float _delta;
  float _t;

PUBLISHED:
  Lerp(LerpFunctor* func, float endt, LerpBlendType* blend);
  Lerp(LerpFunctor* func, float startt, float endt, LerpBlendType* blend);
  Lerp(const Lerp&);
  virtual ~Lerp();
  Lerp& operator=(const Lerp&);
  void step();
  void set_step_size(float);
  float get_step_size() const;
  void set_t(float);
  float get_t() const;
  bool is_done() const;
  LerpFunctor* get_functor() const;
  void set_end_event(const std::string&);
  std::string get_end_event() const;

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Lerp", TypedReferenceCount::get_class_type());
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

class EXPCL_PANDA AutonomousLerp : public TypedReferenceCount {
private:
  PT(LerpBlendType) _blend;
  PT(LerpFunctor) _func;
  EventHandler* _handler;
  std::string _event;
  float _startt;
  float _endt;
  float _t;

  virtual void step();
  static void handle_event(CPT(Event), void*);

PUBLISHED:
  AutonomousLerp(LerpFunctor* func, float endt, LerpBlendType* blend,
                 EventHandler* handler);
  AutonomousLerp(LerpFunctor* func, float startt, float endt,
                 LerpBlendType* blend, EventHandler* handler);
  AutonomousLerp(const AutonomousLerp&);
  virtual ~AutonomousLerp();
  AutonomousLerp& operator=(const AutonomousLerp&);
  void start();
  void stop();
  void resume();
  bool is_done() const;
  LerpFunctor* get_functor() const;
  void set_t(float);
  float get_t() const;
  void set_end_event(const std::string&);
  std::string get_end_event() const;

public:
  // now for typehandle stuff
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AutonomousLerp",
                  TypedReferenceCount::get_class_type());
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

#endif /* __LERP_H__ */
