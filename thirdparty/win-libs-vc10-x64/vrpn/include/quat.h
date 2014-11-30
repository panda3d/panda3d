
/*****************************************************************************
 *
    quat.h -  include file for quaternion, vector and matrix routines.  

    
    Overview:
    
      quatlib is a library of routines that implements a grab-bag of 
      useful routines for dealing with quaternions, vectors, and 
      matrices.  See the quatlib man page for an overview.


    Notes:

      - to address the quaternion elements, use the Q_X, Q_Y, Q_Z and Q_W
      #defines from this file.
      
      - to find out which version of the library you're using, do:
       
             % ident  <path>/libquat.a
   
      (this information is in the rcsid string in quat.c)
    
      - see /afs/unc/proj/hmd/src/quat/{quat,vector,matrix}.c 
        for implementation details.


    Conventions:

      - general-purpose quaternion routines start with q_

      - all non-integer values are doubles by default-  the exceptions
         to this are old (non-open-) GL routines which use floats.
      
      - vector routines start with "q_vec"
      
      - matrix routines have the string "matrix" somewhere in their name

      - all matrices are 4x4

      - positive rotation directions are as follows:

         about Z axis: from X axis to Y axis
         about X axis: from Y axis to Z axis
         about Y axis: from Z axis to X axis

      - all angles are specified in radians

      - destination parameter (if any) is always first argument (as in
             Unix string routines)

      - src and dest parameters can always be the same, as long as they 
             are of the same type (copying is done if necessary) 

      - naming conventions for conversion routines:
      
       q_{to,from}_whatever for routines involving quaternions
       q_x_to_y for all others (ie., no "from" is used)


   Revision History (for whole library, not just this file):

   Author            Date      Comments
   ------            --------  ----------------------------
   Rich Holloway     09/10/01  Misc cleanup, deleted PPHIGS support,
                               added q_xyz_quat_xform(), renamed
                               qogl_matrix_mult_fixed() back to
                               qogl_matrix_mult().
   Mark Livingston   01/09/96  Added routines for OpenGL matrices
   Rich Holloway     09/27/93  Added Gary Bishop's matrix to euler rtn
   Rich Holloway     07/16/92  Added q_euler_to_col_matrix(), routines
                               for working with GL matrices, added
                               documentation for euler angle routines
   Erik Erikson/     06/26/92  Added q_xyz_quat_compose
   Stefan Gottschalk/
   Russ Taylor
   
   Rich Holloway     05/13/92  Added Q_NULL_VECTOR, Q_ID_MATRIX
   Jon Leech/        04/29/92  Added CM_ prototypes
   Erik Erikson
   
   Rich Holloway     09/21/90  Made into library, made all matrices 4x4,
                               added matrix routines for 
                               4x4 (standard) or 3x4 (for PPHIGS),
                               changed names of 
                               routines (to avoid name conflicts with
                               non-library routines) by prefixing
                               everything with "q_".
   
   Russ Taylor        1990     Modified q_slerp to pick shortest path
                               between two angles
   
   Warren Robinett   12/89     Added PPHIGS support routines
   
   Ken Shoemake       1985     Initial version
   
   RCS Header:
   $Id: quat.h,v 2.37 2004/07/22 20:54:42 taylorr Exp $
 *
 *****************************************************************************/

/* prevent multiple includes  */
#ifndef Q_INCLUDED
#define Q_INCLUDED


#include <stdio.h>
#include <math.h>

/*****************************************************************************
 *
    #defines
 *
 *****************************************************************************/

/* for accessing the elements of q_type and q_vec_type   */
#define Q_X    0
#define Q_Y    1
#define Q_Z    2
#define Q_W    3

/* tolerance for quaternion operations */
#define  Q_EPSILON   (1e-10)

/* min and max macros   */
#define Q_MAX(x, y)       ( ((x) > (y)) ? (x) : (y) )
#define Q_MIN(x, y)       ( ((x) < (y)) ? (x) : (y) )

#define Q_ABS(x)       ( ((x) > 0 ) ? (x) : (-(x)) )

/* 
 * use local definition of PI for machines that have no def in math.h; this
 *  value stolen from DEC Ultrix 4.1 math.h
 */
#define Q_PI    3.14159265358979323846

#define Q_ID_QUAT   { 0.0, 0.0, 0.0, 1.0 }

#define Q_ID_MATRIX { {1.0, 0.0, 0.0, 0.0}, \
                      {0.0, 1.0, 0.0, 0.0}, \
                      {0.0, 0.0, 1.0, 0.0}, \
                      {0.0, 0.0, 0.0, 1.0} }

#define Q_NULL_VECTOR   { 0.0, 0.0, 0.0 }

/* 
 * degree/radian conversion
 */
#define Q_DEG_TO_RAD(deg)       ( ((deg)*Q_PI)/180.0 )
#define Q_RAD_TO_DEG(rad)       ( (((rad)*180.0)/Q_PI) )


/*****************************************************************************
 *
    typedefs
 *
 *****************************************************************************/

/* basic quaternion type- scalar part is last element in array    */
typedef double q_type[4];

/* basic vector type */
typedef double q_vec_type[3];

/* for row and column matrices   */
typedef double q_matrix_type[4][4];

/* for working with gl or other 4x4 float matrices  */
typedef float  qgl_matrix_type[4][4];

/* for working with OpenGL matrices - these are really just like row matrices
** (i.e. same bits in same order), but the decl is a 1-D array, not 2-D, sigh
*/
typedef double  qogl_matrix_type[16];

/* special transformation type using quaternions and vectors   */
typedef struct  q_xyz_quat_struct {
    q_vec_type xyz;   /* translation */
    q_type     quat;  /* rotation    */
} q_xyz_quat_type;



/*****************************************************************************
 *****************************************************************************
 *
    function declarations
 *
 *****************************************************************************
 *****************************************************************************/

/* On some platforms, we need to specifically tell the compiler
 * that these functions are to have C linkage.  [why not everywhere?]
 */

#if defined(__cplusplus)

#ifdef FLOW
#define EXTERN_QUALIFICATION 
#else
#define EXTERN_QUALIFICATION "C"
#endif /* FLOW */

#define BEGIN_EXTERN_BLOCK extern EXTERN_QUALIFICATION {
#define END_EXTERN_BLOCK }

#else /* __cplusplus */

#define BEGIN_EXTERN_BLOCK 
#define END_EXTERN_BLOCK

#endif /* __cplusplus */



BEGIN_EXTERN_BLOCK

/*****************************************************************************
 *
    strictly quaternion operations
 *
 *****************************************************************************/

/*  prints a quaternion */
void q_print (const q_type quat);

/* make a quaternion given an axis and an angle;  x,y,z is axis of 
 *  rotation;  angle is angle of rotation in radians (see also q_from_two_vecs)
 *
 *  rotation is counter-clockwise when rotation axis vector is 
 *       pointing at you
 *
 * if angle or vector are 0, the identity quaternion is returned.
 */
void q_make (q_type destQuat,
             double x,  double y,  double z,
             double angle);
void q_from_axis_angle(q_type destQuat,
             double x,  double y,  double z,
             double angle);

/* Turn a quaternion into an axis and an angle;  x,y,z is axis of 
 *  rotation;  angle is angle of rotation in radians.
 *
 *  rotation is counter-clockwise when rotation axis vector is 
 *       pointing at you
 *
 *  if the identity quaternion is passed in, the angle will be
 *  zero and the axis will be the Z axis.
 */
void q_to_axis_angle (double *x, double *y, double *z, double *angle,
		      const q_type srcQuat);

/*  copy srcQuat to destQuat    */
void q_copy (q_type destQuat, const q_type srcQuat);

/* normalizes quaternion;  src and dest can be same */
void q_normalize (q_type destQuat, const q_type srcQuat);

/* invert quat;  src and dest can be the same   */
void q_invert (q_type destQuat, const q_type srcQuat);

/*
 * computes quaternion product destQuat = qLeft * qRight.
 *        destQuat can be same as either qLeft or qRight or both.
 */
void q_mult (q_type destQuat, const q_type qLeft, const q_type qRight);

/* conjugate quat; src and dest can be same */
void q_conjugate (q_type destQuat, const q_type srcQuat);

/* take natural log of unit quat; src and dest can be same  */
void q_log (q_type destQuat, const q_type srcQuat);

/* exponentiate quaternion, assuming scalar part 0.  src can be same as dest */
void q_exp (q_type destQuat, const q_type srcQuat);


/*
 * q_slerp: Spherical linear interpolation of unit quaternions.
 *
 *    As t goes from 0 to 1, destQuat goes from startQ to endQuat.
 *      This routine should always return a point along the shorter
 *    of the two paths between the two.  That is why the vector may be
 *    negated in the end.
 *    
 *    src == dest should be ok, although that doesn't seem to make much
 *    sense here.
 */
void q_slerp (q_type destQuat, const q_type startQuat, const q_type endQuat, double t);

/*****************************************************************************
 *  
    q_from_euler - converts 3 euler angles (in radians) to a quaternion
     
   Assumes roll is rotation about X, pitch
   is rotation about Y, yaw is about Z.  Assumes order of 
   yaw, pitch, roll applied as follows:
       
       p' = roll( pitch( yaw(p) ) )

      See comments for q_euler_to_col_matrix for more on this.
 *
 *****************************************************************************/
void q_from_euler (q_type destQuat, double yaw, double pitch, double roll);

/* converts quat to euler angles (yaw, pitch, roll).  see
 * q_col_matrix_to_euler() for conventions.
 */
void q_to_euler(q_vec_type yawPitchRoll, const q_type q);

/*****************************************************************************
 *
    mixed quaternion operations:  conversions to and from vectors & matrices
 *
 *****************************************************************************/

/* destVec = q * vec * q(inverse);  vec can be same storage as destVec  */
void q_xform (q_vec_type destVec, const q_type q, const q_vec_type vec);

/* quat/vector conversion  */
/* create a quaternion from two vectors that rotates v1 to v2 
 *   about an axis perpendicular to both
 */
void q_from_two_vecs (q_type destQuat, const q_vec_type v1, const q_vec_type v2);

/* simple conversion */
void q_from_vec (q_type destQuat, const q_vec_type srcVec);
void q_to_vec (q_vec_type destVec, const q_type srcQuat);

/* quaternion/4x4 matrix conversions   */
void q_from_row_matrix (q_type destQuat, const q_matrix_type matrix);
void q_from_col_matrix (q_type destQuat, const q_matrix_type matrix);
void q_to_row_matrix (q_matrix_type destMatrix, const q_type srcQuat);
void q_to_col_matrix (q_matrix_type destMatrix, const q_type srcQuat);

/* quat/ogl conversion */
void q_from_ogl_matrix (q_type destQuat, const qogl_matrix_type matrix);
void q_to_ogl_matrix (qogl_matrix_type matrix, const q_type srcQuat);


/*****************************************************************************
 *
    strictly vector operations
 *
 *****************************************************************************/

/* prints a vector to stdout  */
void q_vec_print (const q_vec_type vec);

/* compatibility w/ old  */
#define q_set_vec   q_vec_set

/* sets vector equal to 3 values given */
void q_vec_set (q_vec_type vec, double x, double y, double z);

/* copies srcVec to destVec */
void q_vec_copy (q_vec_type destVec, const q_vec_type srcVec);

/* adds two vectors */
void q_vec_add (q_vec_type destVec, const q_vec_type aVec, const q_vec_type bVec);

/* destVec = v1 - v2 (v1, v2, destVec need not be distinct storage) */
void q_vec_subtract (q_vec_type destVec, const q_vec_type v1, const q_vec_type v2);

/* returns value of dot product of v1 and v2 */
double q_vec_dot_product (const q_vec_type v1, const q_vec_type v2);

/* scale a vector  (src and dest need not be distinct) */
void q_vec_scale (q_vec_type destVec, double scaleFactor, const q_vec_type srcVec);


/* negate a vector to point in the opposite direction */
void q_vec_invert (q_vec_type destVec, const q_vec_type srcVec);

/*  normalize a vector  (destVec and srcVec may be the same) */
void q_vec_normalize (q_vec_type destVec, const q_vec_type srcVec);

/* returns magnitude of vector   */
double q_vec_magnitude (const q_vec_type vec);

/*  returns distance between two points/vectors */
double q_vec_distance (const q_vec_type vec1, const q_vec_type vec2);

/* computes cross product of two vectors:  destVec = aVec X bVec
 *    destVec same as aVec or bVec ok */
void q_vec_cross_product (q_vec_type destVec,
                          const q_vec_type aVec, const q_vec_type bVec);


/*****************************************************************************
 *
    strictly matrix operations
 *
 *****************************************************************************/

/* q_matrix_copy - copies srcMatrix to destMatrix (both matrices are 4x4)   */
void q_matrix_copy (q_matrix_type destMatrix, const q_matrix_type srcMatrix);

void qogl_matrix_copy (qogl_matrix_type dest, const qogl_matrix_type src);

/* does a 4x4 matrix multiply (the input matrices are 4x4) and
 *            puts the result in a 4x4 matrix.  src == dest ok.
 */
void q_matrix_mult (q_matrix_type resultMatrix,
                    const q_matrix_type leftMatrix,
                    const q_matrix_type rightMatrix);

// for backward compatibility
#define qogl_matrix_mult_fixed qogl_matrix_mult

/*
 * Computes result=left*right
 * Used to be called qogl_matrix_mult_fixed because the old version
 * did not compute the correct result. 
 */
void qogl_matrix_mult (qogl_matrix_type result,
                       const qogl_matrix_type left,
                       const qogl_matrix_type right);


/*****************************************************************************
 *
   q_euler_to_col_matrix - euler angles should be in radians
      computed assuming the order of rotation is: yaw, pitch, roll.
   
    This means the following:
    
      p' = roll( pitch( yaw(p) ) )
    
    or

      p' = Mr * Mp * My * p

    Yaw is rotation about Z axis, pitch is rotation about Y axis, and roll
    is rotation about X axis.  In terms of these axes, then, the process is:
    
      p' = Mx * My * Mz * p
 
    where Mx = the standard Foley and van Dam column matrix for rotation
    about the X axis, and similarly for Y and Z.
    
    Thus the calling sequence in terms of X, Y, Z is:
    
      q_euler_to_col_matrix(destMatrix, zRot, yRot, xRot);
 *
 *****************************************************************************/
void q_euler_to_col_matrix (q_matrix_type destMatrix,
                            double yaw, double pitch, double roll);

/*****************************************************************************
 *
    q_col_matrix_to_euler- convert a column matrix to euler angles    
 
    input:
      - vector to hold euler angles
      - src column matrix
    
    output:
      - euler angles in radians in the range -pi to pi;
       vec[0] = yaw, vec[1] = pitch, vec[2] = roll
       yaw is rotation about Z axis, pitch is about Y, roll -> X rot.
    
    notes:
      - written by Gary Bishop
 *
 *****************************************************************************/
void q_col_matrix_to_euler (q_vec_type angles, const q_matrix_type colMatrix);

/* prints 4x4 matrix */
void q_print_matrix (const q_matrix_type matrix);

void qogl_print_matrix (const qogl_matrix_type);


/*****************************************************************************
 *
    xyz_quat routines
 *
 *****************************************************************************/

/* invert a vector/quaternion transformation pair   */
void q_xyz_quat_invert (q_xyz_quat_type *destPtr, const q_xyz_quat_type *srcPtr);


/* converts a row matrix to an xyz_quat   */
void q_row_matrix_to_xyz_quat (q_xyz_quat_type * xyzQuatPtr,
                               const q_matrix_type     rowMatrix);

/* convert an xyz_quat to a row matrix */
void q_xyz_quat_to_row_matrix (q_matrix_type     rowMatrix,
                               const q_xyz_quat_type * xyzQuatPtr);

void q_ogl_matrix_to_xyz_quat (q_xyz_quat_type  * xyzQuatPtr,
                               const qogl_matrix_type   matrix);

void q_xyz_quat_to_ogl_matrix (qogl_matrix_type  matrix,
                               const q_xyz_quat_type  * xyzQuatPtr);

/* compose q_xyz_quat_vecs to form a third. */
/* C_from_A_ptr may be = to either C_from_B_ptr or B_from_A_ptr (or both) */
void q_xyz_quat_compose (q_xyz_quat_type * C_from_A_ptr,
                         const q_xyz_quat_type * C_from_B_ptr,
                         const q_xyz_quat_type * B_from_A_ptr);

void q_xyz_quat_xform(q_vec_type dest, const q_xyz_quat_type *xf, const q_vec_type src);

/*****************************************************************************
 *
    GL support
 *
 *****************************************************************************/

/* convert from quat to GL 4x4 float row matrix */
void qgl_to_matrix (qgl_matrix_type destMatrix, const q_type srcQuat);


/* qgl_from_matrix- Convert GL 4x4 row-major rotation matrix to 
 * unit quaternion.
 *    - same as q_from_row_matrix, except basic type is float, not double
 */
void qgl_from_matrix (q_type destQuat, const qgl_matrix_type srcMatrix);

/* print gl-style matrix    */
void qgl_print_matrix (const qgl_matrix_type matrix);
                          
END_EXTERN_BLOCK

#undef BEGIN_EXTERN_BLOCK
#undef END_EXTERN_BLOCK
#undef EXTERN_QUALIFICATION

#endif /* Q_INCLUDED */
