// Filename: odeSurfaceParameters.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODESURFACEPARAMETERS_H
#define ODESURFACEPARAMETERS_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"

class OdeContact;

////////////////////////////////////////////////////////////////////
//       Class : OdeSurfaceParameters
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeSurfaceParameters : public TypedObject {
  friend class OdeContact;

protected:
  OdeSurfaceParameters(const dSurfaceParameters &surface_parameters);

PUBLISHED:
  enum ModeFlags { MF_contact_mu2      = 0x001,
                   MF_contactFDir1     = 0x002,
                   MF_contactBounce    = 0x004,
                   MF_contactSoftERP   = 0x008,
                   MF_contactSoftCFM   = 0x010,
                   MF_contactMotion1   = 0x020,
                   MF_contactMotion2   = 0x040,
                   MF_contactSlip1     = 0x080,
                   MF_contactSlip2     = 0x100,

                   MF_contactApprox0   = 0x0000,
                   MF_contactApprox1_1 = 0x1000,
                   MF_contactApprox1_2 = 0x2000,
                   MF_contactApprox1   = 0x3000 };
  
  /*
  // Interrogate doesn't seem to handle this so well
  enum ModeFlags { MF_contact_mu2      = dContactMu2,
                   MF_contactFDir1     = dContactFDir1,
                   MF_contactBounce    = dContactBounce,
                   MF_contactSoftERP   = dContactSoftERP,
                   MF_contactSoftCFM   = dContactSoftCFM,
                   MF_contactMotion1   = dContactMotion1,
                   MF_contactMotion2   = dContactMotion2,
                   MF_contactSlip1     = dContactSlip1,
                   MF_contactSlip2     = dContactSlip2,
                   // MF_contactApprox0   = dContactApprox0,
                   MF_contactApprox1_1 = dContactApprox1_1,
                   MF_contactApprox1_2 = dContactApprox1_2,
                   MF_contactApprox1   = dContactApprox1 };
  */

  OdeSurfaceParameters(int mode = 0, dReal mu = 0);
  virtual ~OdeSurfaceParameters();

  INLINE void set_mode(int mode);
  INLINE void set_mu(dReal mu);
  INLINE void set_mu2(dReal mu2);
  INLINE void set_bounce(dReal bounce);
  INLINE void set_bounce_vel(dReal bounce_vel);
  INLINE void set_soft_erp(dReal soft_erp);
  INLINE void set_soft_cfm(dReal soft_cfm);
  INLINE void set_motion1(dReal motion);
  INLINE void set_motion2(dReal motion);
  INLINE void set_slip1(dReal slip);
  INLINE void set_slip2(dReal slip);

  INLINE int get_mode() const;
  INLINE dReal get_mu() const;
  INLINE dReal get_mu2() const;
  INLINE dReal get_bounce() const;
  INLINE dReal get_bounce_vel() const;
  INLINE dReal get_soft_erp() const;
  INLINE dReal get_soft_cfm() const;
  INLINE dReal get_motion1() const;
  INLINE dReal get_motion2() const;
  INLINE dReal get_slip1() const;
  INLINE dReal get_slip2() const;

public:
  const dSurfaceParameters *get_surface_parameters_ptr() const;

private:
  void operator = (const OdeSurfaceParameters &copy);
  dSurfaceParameters _surface_parameters;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "OdeSurfaceParameters",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeSurfaceParameters.I"

#endif
