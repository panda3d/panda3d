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

#ifndef _ODE_COMMON_H_
#define _ODE_COMMON_H_
#include <ode/odeconfig.h>
#include <ode/error.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif


#define PURE_INLINE static __inline


/* configuration stuff */

/* constants */

/* pi and 1/sqrt(2) are defined here if necessary because they don't get
 * defined in <math.h> on some platforms (like MS-Windows)
 */

#ifndef M_PI
#define M_PI REAL(3.1415926535897932384626433832795029)
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2 REAL(0.7071067811865475244008443621048490)
#endif


/* debugging:
 *   IASSERT  is an internal assertion, i.e. a consistency check. if it fails
 *            we want to know where.
 *   UASSERT  is a user assertion, i.e. if it fails a nice error message
 *            should be printed for the user.
 *   AASSERT  is an arguments assertion, i.e. if it fails "bad argument(s)"
 *            is printed.
 *   DEBUGMSG just prints out a message
 */

#  if defined(__STDC__) && __STDC_VERSION__ >= 199901L
#    define __FUNCTION__ __func__
#  endif
#ifndef dNODEBUG
#  ifdef __GNUC__
#    define dIASSERT(a) { if (!(a)) { dDebug (d_ERR_IASSERT, \
      "assertion \"" #a "\" failed in %s() [%s:%u]",__FUNCTION__,__FILE__,__LINE__); } }
#    define dUASSERT(a,msg) { if (!(a)) { dDebug (d_ERR_UASSERT, \
      msg " in %s()", __FUNCTION__); } }
#    define dDEBUGMSG(msg) { dMessage (d_ERR_UASSERT,				\
  msg " in %s() [%s:%u]", __FUNCTION__,__FILE__,__LINE__); }
#  else // not __GNUC__
#    define dIASSERT(a) { if (!(a)) { dDebug (d_ERR_IASSERT, \
      "assertion \"" #a "\" failed in %s:%u",__FILE__,__LINE__); } }
#    define dUASSERT(a,msg) { if (!(a)) { dDebug (d_ERR_UASSERT, \
      msg " (%s:%u)", __FILE__,__LINE__); } }
#    define dDEBUGMSG(msg) { dMessage (d_ERR_UASSERT, \
      msg " (%s:%u)", __FILE__,__LINE__); }
#  endif
#  define dIVERIFY(a) dIASSERT(a)
#else
#  define dIASSERT(a) ((void)0)
#  define dUASSERT(a,msg) ((void)0)
#  define dDEBUGMSG(msg) ((void)0)
#  define dIVERIFY(a) ((void)(a))
#endif

#  ifdef __GNUC__
#    define dICHECK(a) { if (!(a)) { dDebug (d_ERR_IASSERT, \
      "assertion \"" #a "\" failed in %s() [%s:%u]",__FUNCTION__,__FILE__,__LINE__); *(int *)0 = 0; } }
#  else // not __GNUC__
#    define dICHECK(a) { if (!(a)) { dDebug (d_ERR_IASSERT, \
      "assertion \"" #a "\" failed in %s:%u",__FILE__,__LINE__); *(int *)0 = 0; } }
#  endif

// Argument assert is a special case of user assert
#define dAASSERT(a) dUASSERT(a,"Bad argument(s)")

/* floating point data type, vector, matrix and quaternion types */

#if defined(dSINGLE)
typedef float dReal;
#ifdef dDOUBLE
#error You can only #define dSINGLE or dDOUBLE, not both.
#endif // dDOUBLE
#elif defined(dDOUBLE)
typedef double dReal;
#else
#error You must #define dSINGLE or dDOUBLE
#endif

// Detect if we've got both trimesh engines enabled.
#if dTRIMESH_ENABLED
#if dTRIMESH_OPCODE && dTRIMESH_GIMPACT
#error You can only #define dTRIMESH_OPCODE or dTRIMESH_GIMPACT, not both.
#endif
#endif // dTRIMESH_ENABLED

// Define a type for indices, either 16 or 32 bit, based on build option
// TODO: Currently GIMPACT only supports 32 bit indices.
#if dTRIMESH_16BIT_INDICES
#if dTRIMESH_GIMPACT
typedef uint32 dTriIndex;
#else // dTRIMESH_GIMPACT
typedef uint16 dTriIndex;
#endif // dTRIMESH_GIMPACT
#else // dTRIMESH_16BIT_INDICES
typedef uint32 dTriIndex;
#endif // dTRIMESH_16BIT_INDICES

/* round an integer up to a multiple of 4, except that 0 and 1 are unmodified
 * (used to compute matrix leading dimensions)
 */
#define dPAD(a) (((a) > 1) ? ((((a)-1)|3)+1) : (a))

/* these types are mainly just used in headers */
typedef dReal dVector3[4];
typedef dReal dVector4[4];
typedef dReal dMatrix3[4*3];
typedef dReal dMatrix4[4*4];
typedef dReal dMatrix6[8*6];
typedef dReal dQuaternion[4];


/* precision dependent scalar math functions */

#if defined(dSINGLE)

#define REAL(x) (x ## f)					/* form a constant */
#define dRecip(x) ((1.0f/(x)))				/* reciprocal */
#define dSqrt(x) (sqrtf(x))			/* square root */
#define dRecipSqrt(x) ((1.0f/sqrtf(x)))		/* reciprocal square root */
#define dSin(x) (sinf(x))				/* sine */
#define dCos(x) (cosf(x))				/* cosine */
#define dFabs(x) (fabsf(x))			/* absolute value */
#define dAtan2(y,x) (atan2f(y,x))		/* arc tangent with 2 args */
#define dFMod(a,b) (fmodf(a,b))		/* modulo */
#define dFloor(x) floorf(x)			/* floor */
#define dCeil(x) ceilf(x)			/* floor */
#define dCopySign(a,b) ((dReal)copysignf(a,b)) /* copy value sign */
#define dNextAfter(x, y) nextafterf(x, y) /* next value after */

#if defined(_ODE__NEXTAFTERF_REQUIRED)
float _nextafterf(float x, float y);
#endif

#ifdef HAVE___ISNANF
#define dIsNan(x) (__isnanf(x))
#elif defined(HAVE__ISNANF)
#define dIsNan(x) (_isnanf(x))
#elif defined(HAVE_ISNANF)
#define dIsNan(x) (isnanf(x))
#else
  /*
     fall back to _isnan which is the VC way,
     this may seem redundant since we already checked
     for _isnan before, but if isnan is detected by
     configure but is not found during compilation
     we should always make sure we check for __isnanf,
     _isnanf and isnanf in that order before falling
     back to a default
  */
#define dIsNan(x) (_isnan(x))
#endif

#elif defined(dDOUBLE)

#define REAL(x) (x)
#define dRecip(x) (1.0/(x))
#define dSqrt(x) sqrt(x)
#define dRecipSqrt(x) (1.0/sqrt(x))
#define dSin(x) sin(x)
#define dCos(x) cos(x)
#define dFabs(x) fabs(x)
#define dAtan2(y,x) atan2((y),(x))
#define dFMod(a,b) (fmod((a),(b)))
#define dFloor(x) floor(x)
#define dCeil(x) ceil(x)
#define dCopySign(a,b) (copysign((a),(b)))
#define dNextAfter(x, y) nextafter(x, y)

#undef _ODE__NEXTAFTERF_REQUIRED

#ifdef HAVE___ISNAN
#define dIsNan(x) (__isnan(x))
#elif defined(HAVE__ISNAN)
#define dIsNan(x) (_isnan(x))
#elif defined(HAVE_ISNAN)
#define dIsNan(x) (isnan(x))
#else
#define dIsNan(x) (_isnan(x))
#endif

#else
#error You must #define dSINGLE or dDOUBLE
#endif

/* internal object types (all prefixed with `dx') */

struct dxWorld;		/* dynamics world */
struct dxSpace;		/* collision space */
struct dxBody;		/* rigid body (dynamics object) */
struct dxGeom;		/* geometry (collision object) */
struct dxJoint;
struct dxJointNode;
struct dxJointGroup;
struct dxWorldProcessThreadingManager;

typedef struct dxWorld *dWorldID;
typedef struct dxSpace *dSpaceID;
typedef struct dxBody *dBodyID;
typedef struct dxGeom *dGeomID;
typedef struct dxJoint *dJointID;
typedef struct dxJointGroup *dJointGroupID;
typedef struct dxWorldProcessThreadingManager *dWorldStepThreadingManagerID;

/* error numbers */

enum {
  d_ERR_UNKNOWN = 0,		/* unknown error */
  d_ERR_IASSERT,		/* internal assertion failed */
  d_ERR_UASSERT,		/* user assertion failed */
  d_ERR_LCP			/* user assertion failed */
};


/* joint type numbers */

typedef enum {
  dJointTypeNone = 0,		/* or "unknown" */
  dJointTypeBall,
  dJointTypeHinge,
  dJointTypeSlider,
  dJointTypeContact,
  dJointTypeUniversal,
  dJointTypeHinge2,
  dJointTypeFixed,
  dJointTypeNull,
  dJointTypeAMotor,
  dJointTypeLMotor,
  dJointTypePlane2D,
  dJointTypePR,
  dJointTypePU,
  dJointTypePiston
} dJointType;


/* an alternative way of setting joint parameters, using joint parameter
 * structures and member constants. we don't actually do this yet.
 */

/*
typedef struct dLimot {
  int mode;
  dReal lostop, histop;
  dReal vel, fmax;
  dReal fudge_factor;
  dReal bounce, soft;
  dReal suspension_erp, suspension_cfm;
} dLimot;

enum {
  dLimotLoStop		= 0x0001,
  dLimotHiStop		= 0x0002,
  dLimotVel		= 0x0004,
  dLimotFMax		= 0x0008,
  dLimotFudgeFactor	= 0x0010,
  dLimotBounce		= 0x0020,
  dLimotSoft		= 0x0040
};
*/


/* standard joint parameter names. why are these here? - because we don't want
 * to include all the joint function definitions in joint.cpp. hmmmm.
 * MSVC complains if we call D_ALL_PARAM_NAMES_X with a blank second argument,
 * which is why we have the D_ALL_PARAM_NAMES macro as well. please copy and
 * paste between these two.
 */

#define D_ALL_PARAM_NAMES(start) \
  /* parameters for limits and motors */ \
  dParamLoStop = start, \
  dParamHiStop, \
  dParamVel, \
  dParamFMax, \
  dParamFudgeFactor, \
  dParamBounce, \
  dParamCFM, \
  dParamStopERP, \
  dParamStopCFM, \
  /* parameters for suspension */ \
  dParamSuspensionERP, \
  dParamSuspensionCFM, \
  dParamERP, \

  //////////////////////////////////////////////////////////////////////////////
  /// \enum  D_ALL_PARAM_NAMES_X
  ///
  /// \var dParamGroup This is the starting value of the different group
  ///                  (i.e. dParamGroup1, dParamGroup2, dParamGroup3)
  ///                  It also helps in the use of parameter
  ///                  (dParamGroup2 | dParamFMax) == dParamFMax2
  //////////////////////////////////////////////////////////////////////////////
#define D_ALL_PARAM_NAMES_X(start,x) \
  dParamGroup ## x = start, \
  /* parameters for limits and motors */ \
  dParamLoStop ## x = start, \
  dParamHiStop ## x, \
  dParamVel ## x, \
  dParamFMax ## x, \
  dParamFudgeFactor ## x, \
  dParamBounce ## x, \
  dParamCFM ## x, \
  dParamStopERP ## x, \
  dParamStopCFM ## x, \
  /* parameters for suspension */ \
  dParamSuspensionERP ## x, \
  dParamSuspensionCFM ## x, \
  dParamERP ## x,

enum {
  D_ALL_PARAM_NAMES(0)
  dParamsInGroup,     ///< Number of parameter in a group
  D_ALL_PARAM_NAMES_X(0x000,1)
  D_ALL_PARAM_NAMES_X(0x100,2)
  D_ALL_PARAM_NAMES_X(0x200,3)

  /* add a multiple of this constant to the basic parameter numbers to get
   * the parameters for the second, third etc axes.
   */
  dParamGroup=0x100
};


/* angular motor mode numbers */

enum {
  dAMotorUser = 0,
  dAMotorEuler = 1
};


/* joint force feedback information */

typedef struct dJointFeedback {
  dVector3 f1;		/* force applied to body 1 */
  dVector3 t1;		/* torque applied to body 1 */
  dVector3 f2;		/* force applied to body 2 */
  dVector3 t2;		/* torque applied to body 2 */
} dJointFeedback;


/* private functions that must be implemented by the collision library:
 * (1) indicate that a geom has moved, (2) get the next geom in a body list.
 * these functions are called whenever the position of geoms connected to a
 * body have changed, e.g. with dBodySetPosition(), dBodySetRotation(), or
 * when the ODE step function updates the body state.
 */

void dGeomMoved (dGeomID);
dGeomID dGeomGetBodyNext (dGeomID);

/**
 * dGetConfiguration returns the specific ODE build configuration as
 * a string of tokens. The string can be parsed in a similar way to
 * the OpenGL extension mechanism, the naming convention should be
 * familiar too. The following extensions are reported:
 *
 * ODE
 * ODE_single_precision
 * ODE_double_precision
 * ODE_EXT_no_debug
 * ODE_EXT_trimesh
 * ODE_EXT_opcode
 * ODE_EXT_gimpact
 * ODE_EXT_malloc_not_alloca
 * ODE_EXT_gyroscopic
 * ODE_OPC_16bit_indices
 * ODE_OPC_new_collider
*/
ODE_API const char* dGetConfiguration (void);

/**
 * Helper to check for a token in the ODE configuration string.
 * Caution, this function is case sensitive.
 *
 * @param token A configuration token, see dGetConfiguration for details
 *
 * @return 1 if exact token is present, 0 if not present
 */
ODE_API int dCheckConfiguration( const char* token );

#ifdef __cplusplus
}
#endif

#endif
