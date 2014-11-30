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

#ifndef _ODE_MASS_H_
#define _ODE_MASS_H_

#include <ode/common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dMass;
typedef struct dMass dMass;

/**
 * Check if a mass structure has valid value.
 * The function check if the mass and innertia matrix are positive definits
 *
 * @param m A mass structure to check
 *
 * @return 1 if both codition are met
 */
ODE_API int dMassCheck(const dMass *m);

ODE_API void dMassSetZero (dMass *);

ODE_API void dMassSetParameters (dMass *, dReal themass,
			 dReal cgx, dReal cgy, dReal cgz,
			 dReal I11, dReal I22, dReal I33,
			 dReal I12, dReal I13, dReal I23);

ODE_API void dMassSetSphere (dMass *, dReal density, dReal radius);
ODE_API void dMassSetSphereTotal (dMass *, dReal total_mass, dReal radius);

ODE_API void dMassSetCapsule (dMass *, dReal density, int direction,
		  	dReal radius, dReal length);
ODE_API void dMassSetCapsuleTotal (dMass *, dReal total_mass, int direction,
			dReal radius, dReal length);

ODE_API void dMassSetCylinder (dMass *, dReal density, int direction,
		       dReal radius, dReal length);
ODE_API void dMassSetCylinderTotal (dMass *, dReal total_mass, int direction,
			    dReal radius, dReal length);

ODE_API void dMassSetBox (dMass *, dReal density,
		  dReal lx, dReal ly, dReal lz);
ODE_API void dMassSetBoxTotal (dMass *, dReal total_mass,
		       dReal lx, dReal ly, dReal lz);

ODE_API void dMassSetTrimesh (dMass *, dReal density, dGeomID g);

ODE_API void dMassSetTrimeshTotal (dMass *m, dReal total_mass, dGeomID g);

ODE_API void dMassAdjust (dMass *, dReal newmass);

ODE_API void dMassTranslate (dMass *, dReal x, dReal y, dReal z);

ODE_API void dMassRotate (dMass *, const dMatrix3 R);

ODE_API void dMassAdd (dMass *a, const dMass *b);


// Backwards compatible API
ODE_API ODE_API_DEPRECATED void dMassSetCappedCylinder(dMass *a, dReal b, int c, dReal d, dReal e);
ODE_API ODE_API_DEPRECATED void dMassSetCappedCylinderTotal(dMass *a, dReal b, int c, dReal d, dReal e);


struct dMass {
  dReal mass;
  dVector3 c;
  dMatrix3 I;

#ifdef __cplusplus
  dMass()
    { dMassSetZero (this); }
  void setZero()
    { dMassSetZero (this); }
  void setParameters (dReal themass, dReal cgx, dReal cgy, dReal cgz,
		      dReal I11, dReal I22, dReal I33,
		      dReal I12, dReal I13, dReal I23)
    { dMassSetParameters (this,themass,cgx,cgy,cgz,I11,I22,I33,I12,I13,I23); }

  void setSphere (dReal density, dReal radius)
    { dMassSetSphere (this,density,radius); }
  void setSphereTotal (dReal total, dReal radius)
    { dMassSetSphereTotal (this,total,radius); }

  void setCapsule (dReal density, int direction, dReal radius, dReal length)
    { dMassSetCapsule (this,density,direction,radius,length); }
  void setCapsuleTotal (dReal total, int direction, dReal radius, dReal length)
    { dMassSetCapsule (this,total,direction,radius,length); }

  void setCylinder(dReal density, int direction, dReal radius, dReal length)
    { dMassSetCylinder (this,density,direction,radius,length); }
  void setCylinderTotal(dReal total, int direction, dReal radius, dReal length)
    { dMassSetCylinderTotal (this,total,direction,radius,length); }

  void setBox (dReal density, dReal lx, dReal ly, dReal lz)
    { dMassSetBox (this,density,lx,ly,lz); }
  void setBoxTotal (dReal total, dReal lx, dReal ly, dReal lz)
    { dMassSetBoxTotal (this,total,lx,ly,lz); }

  void setTrimesh(dReal density, dGeomID g)
    { dMassSetTrimesh (this, density, g); }
  void setTrimeshTotal(dReal total, dGeomID g)
    { dMassSetTrimeshTotal (this, total, g); }

  void adjust (dReal newmass)
    { dMassAdjust (this,newmass); }
  void translate (dReal x, dReal y, dReal z)
    { dMassTranslate (this,x,y,z); }
  void rotate (const dMatrix3 R)
    { dMassRotate (this,R); }
  void add (const dMass *b)
    { dMassAdd (this,b); }
#endif
};


#ifdef __cplusplus
}
#endif

#endif
