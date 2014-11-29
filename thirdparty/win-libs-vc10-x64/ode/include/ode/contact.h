/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#ifndef _ODE_CONTACT_H_
#define _ODE_CONTACT_H_

#include <ode/common.h>

#ifdef __cplusplus
extern "C" {
#endif


enum {
  dContactMu2		= 0x001,
  dContactFDir1		= 0x002,
  dContactBounce	= 0x004,
  dContactSoftERP	= 0x008,
  dContactSoftCFM	= 0x010,
  dContactMotion1	= 0x020,
  dContactMotion2	= 0x040,
  dContactMotionN	= 0x080,
  dContactSlip1		= 0x100,
  dContactSlip2		= 0x200,

  dContactApprox0	= 0x0000,
  dContactApprox1_1	= 0x1000,
  dContactApprox1_2	= 0x2000,
  dContactApprox1	= 0x3000
};


typedef struct dSurfaceParameters {
  /* must always be defined */
  int mode;
  dReal mu;

  /* only defined if the corresponding flag is set in mode */
  dReal mu2;
  dReal bounce;
  dReal bounce_vel;
  dReal soft_erp;
  dReal soft_cfm;
  dReal motion1,motion2,motionN;
  dReal slip1,slip2;
} dSurfaceParameters;


/**
 * @brief Describe the contact point between two geoms.
 *
 * If two bodies touch, or if a body touches a static feature in its 
 * environment, the contact is represented by one or more "contact 
 * points", described by dContactGeom.
 *
 * The convention is that if body 1 is moved along the normal vector by 
 * a distance depth (or equivalently if body 2 is moved the same distance 
 * in the opposite direction) then the contact depth will be reduced to 
 * zero. This means that the normal vector points "in" to body 1.
 *
 * @ingroup collide
 */
typedef struct dContactGeom {
  dVector3 pos;          ///< contact position
  dVector3 normal;       ///< normal vector
  dReal depth;           ///< penetration depth
  dGeomID g1,g2;         ///< the colliding geoms
  int side1,side2;       ///< (to be documented)
} dContactGeom;


/* contact info used by contact joint */

typedef struct dContact {
  dSurfaceParameters surface;
  dContactGeom geom;
  dVector3 fdir1;
} dContact;


#ifdef __cplusplus
}
#endif

#endif
