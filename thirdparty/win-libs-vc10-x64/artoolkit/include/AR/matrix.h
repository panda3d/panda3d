/*  --------------------------------------------------------------------------
*   Copyright (C) 2004 Hitlab NZ.
*   The distribution policy is describe on the Copyright.txt furnish 
*    with this library.
*   -------------------------------------------------------------------------*/
/**
*  \file matrix.h
*  \brief ARToolkit algebric mathematics subroutines.
*
*  This package include matrix, vector manipulation routine. In complement
*  to must classical routines (inversion, innerproduct), it includes a PCA (Principal)
*  Component Analysis) routine.
*  For the structure of the matrix see ARMat.
*   \remark 
*
*   History :
*
*  \author Hirokazu Kato kato@sys.im.hiroshima-cu.ac.jp
*  \version 
*  \date 
*
**/
/*  --------------------------------------------------------------------------
*   History : 
*   Rev		Date		Who		Changes
*
*----------------------------------------------------------------------------*/

#ifndef AR_MATRIX_H
#define AR_MATRIX_H
#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//	Public includes.
// ============================================================================

#include <math.h>
#include <AR/config.h>

// ============================================================================
//	Public types and defines.
// ============================================================================

/* === matrix definition ===

  <---- clm --->
  [ 10  20  30 ] ^
  [ 20  10  15 ] |
  [ 12  23  13 ] row
  [ 20  10  15 ] |
  [ 13  14  15 ] v

=========================== */

/** \struct ARMat
* \brief matrix structure.
* 
* Defined the structure of the matrix type based on a dynamic allocation.
* The matrix format is :<br>
*  <---- clm ---><br>
*  [ 10  20  30 ] ^<br>
*  [ 20  10  15 ] |<br>
*  [ 12  23  13 ] row<br>
*  [ 20  10  15 ] |<br>
*  [ 13  14  15 ] v<br>
* 
* \param m content of matrix 
* \param row number of lines in matrix
* \param clm number of column in matrix
*/
typedef struct {
	double *m;
	int row;
	int clm;
} ARMat;

/** \struct ARVec
* \brief vector structure.
*
* The vector format is :<br>
*  <---- clm ---><br>
*  [ 10  20  30 ]<br>
* Defined the structure of the vector type based on a dynamic allocation.
* \param m content of vector
* \param clm number of column in matrix
*/
typedef struct {
        double *v;
        int    clm;
} ARVec;

/** \def ARELEM0(mat,r,c)
* \brief macro function that give direct access to an element (0 origin)
*
* 
*/
/* 0 origin */
#define ARELEM0(mat,r,c) ((mat)->m[(r)*((mat)->clm)+(c)])

/** \def ARELEM1(mat,row,clm)
* \brief macro function that give direct access to an element (1 origin)
*
* 
*/
/* 1 origin */
#define ARELEM1(mat,row,clm) ARELEM0(mat,row-1,clm-1)

// ============================================================================
//	Public globals.
// ============================================================================

// ============================================================================
//	Public functions.
// ============================================================================

/** \fn ARMat *arMatrixAlloc(int row, int clm)
* \brief creates a new matrix.
*
* Allocate and initialize a new matrix structure.
* XXXBK initializing ?? to 0 m ??
* \param row number of line
* \param clm number of column
* \return the matrix structure, NULL if allocation is impossible
*/
ARMat  *arMatrixAlloc(int row, int clm);

/** \fn int arMatrixFree(ARMat *m)
* \brief deletes a matrix.
*
* Delete a matrix structure (deallocate used memory).
* \param m matrix to delete
* \return 0
*/
int    arMatrixFree(ARMat *m);

/** \fn int arMatrixDup(ARMat *dest, ARMat *source)
* \brief copy a matrix
*
* copy one matrix to another. The two ARMat must
* be allocated.
* \param dest the destination matrix of the copy
* \param source the original matrix source
* \return 0 if success, -1 if error (matrix with different size)
*/
int    arMatrixDup(ARMat *dest, ARMat *source);

/** \fn ARMat *arMatrixAllocDup(ARMat *source)
* \brief dumps a new matrix
*
* Allocates and recopy the original source matrix. 
* \param source the source matrix to copy
* \return the matrix if success, NULL if error
*/
ARMat  *arMatrixAllocDup(ARMat *source);

/** \fn int arMatrixUnit(ARMat *unit)
* \brief Creates a unit matrix.
*
* Transforms the source parameter matrix to
* a unit matrix (all values are modified).
* the unit matrix needs to be allocated.
* \param unit the matrix to transform
* \return 0 if success, -1 if error
*/
int    arMatrixUnit(ARMat *unit);

/** \fn int arMatrixAllocUnit(int dim)
* \brief Creates a unit matrix.
*
* Allocates and initializes a matrix to a 
* an identity matrix.
* \param dim dimensions of the unit matrix (square)
* \return the matrix allocated if success, NULL if error
*/
ARMat  *arMatrixAllocUnit(int dim);

/** \fn int arMatrixMul(ARMat *dest, ARMat *a, ARMat *b)
* \brief Multiply two matrix
*
* Multiply two matrix and copy the result in another
* the product is this one : dest = a * b. The destination
* matrix must be allocated. Matrix a and b need to have
* the same size (the source matrix is unmodified).
* \param dest final matrix product
* \param a first matrix
* \param b second matrix
* \return 0 if success, -1 if error (multiplication impossible, or destination matrix have not comptabile size)
*/
int    arMatrixMul(ARMat *dest, ARMat *a, ARMat *b);

/** \fn ARMat *arMatrixAllocMul(ARMat *a, ARMat *b)
* \brief Multiply two matrix with memory allocation.
* 
* multiply two matrix and copy the result in a new
* allocate matrix (the source matrix is unmodified).
* the product is this one : dest = a * b
*
* \param a first matrix
* \param b second matrix
* \return the allocated matrix if success, NULL if error
*/
ARMat  *arMatrixAllocMul(ARMat *a, ARMat *b);

/** \fn int  arMatrixTrans(ARMat *dest, ARMat *source)
* \brief transposes a matrix.
*
* Transposes a matrix. The destination matrix
* must be allocated (the source matrix is unmodified).
* \param dest the destination matrix of the copy
* \param source the source matrix
* \return 0 if success, -1 if error (source and destination matrix have different size)
*/
int    arMatrixTrans(ARMat *dest, ARMat *source);

/** \fn ARMat *arMatrixAllocTrans(ARMat *source)
* \brief transposes a matrix with allocation.
*
* transposes a matrix and copy the result in a new
* allocate matrix (the source matrix is unmodified).
* \param source the matrix to transpose
* \return the allocated matrix if success, NULL if error (creation or transposition impossible)
*/
ARMat  *arMatrixAllocTrans(ARMat *source);

/** \fn int arMatrixInv(ARMat *dest, ARMat *source)
* \brief inverse a matrix.
*
* inverse a matrix and copy the result in a new
* one (the source matrix is unmodified). the destination
* matrix must be allocated. the source matrix need to be a 
* square matrix.
* \param dest result matrix of the inverse operation
* \param source source matrix
* \return 0 if success, -1 if error (not square matrix)
*/
int    arMatrixInv(ARMat *dest, ARMat *source);

/** \fn int arMatrixSelfInv(ARMat *m)
* \brief inverses a matrix.
*
* Inverses a matrix and copy the result in
* the same structure. 
* \param m the matrix to inverse
* \return 0 if success, -1 if error
*/
int    arMatrixSelfInv(ARMat *m);

/** \fn int arMatrixAllocInv(ARMat *source)
* \brief inverses a matrix.
*
* Inverses a matrix and copy the result in
* in a new allocated structure.
* \param source the matrix to inverse
* \return the inversed matrix if success, NULL if error
*/
ARMat  *arMatrixAllocInv(ARMat *source);

/** \fn int arMatrixDet(ARMat *m)
* \brief compute determinant of a matrix.
*
* Compute the determinant of a matrix.
* \param m matrix source
* \return the computed determinant
*/
double arMatrixDet(ARMat *m);

/** \fn int arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean )
* \brief compute the PCA of a matrix.
*
* Compute the Principal Component Analysis (PCA) of a matrix.
* \param input source matrix
* \param evec eigen vector computed
* \param ev eigen value computed
* \param mean mean computed
* \return 0 if success to compute, -1 otherwise
*/
int    arMatrixPCA( ARMat *input, ARMat *evec, ARVec *ev, ARVec *mean );

/** \fn int arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev )
* \brief compute the PCA of a matrix.
*
* Compute the Principal Component Analysis (PCA) of a matrix.
* \param input source matrix
* \param evec result matrix
* \param ev egein value computed
* \return 0 if success to compute, -1 otherwise
*/
int    arMatrixPCA2( ARMat *input, ARMat *evec, ARVec *ev );

/** \fn int arMatrixDisp(ARMat *m)
* \brief display content of a matrix.
*
* Display in current console, the content of
* the matrix. The display is done line by line.
* \param m
* \return 0
*/
int    arMatrixDisp(ARMat *m);

/** \fn ARVec *arVecAlloc( int clm )
* \brief creates a new vector.
*
* Allocates and initializes new vector structure.
* \param clm dimension of vector
* \return the allocated vector, NULL if error (impossible allocation)
*/
ARVec  *arVecAlloc( int clm );

/** \fn int arVecFree( ARVec *v )
* \brief delete a vector.
*
* Delete a vector structure (deallocate used memory).
* \param v the vector to delete
* \return 0
*/
int    arVecFree( ARVec *v );

/** \fn int arVecDisp( ARVec *v )
* \brief display a vector.
*
* Display element of a vector.
* \param v the vector to display
* \return 0
*/
int    arVecDisp( ARVec *v );

/** \fn double arVecHousehold( ARVec *x )
* \brief XXXBK
*
* XXXBK: for QR decomposition ?? (can't success to find french translation of this term)
* \param x XXXBK
* \return XXXBK
*/
double arVecHousehold( ARVec *x );

/** \fn double arVecInnerproduct( ARVec *x, ARVec *y )
* \brief Computes the inner product of 2 vectors.
*
* computes the inner product of the two argument vectors.
* the operation done is  a=x.y (and a is return)
* \param x first vector source
* \param y second vector source 
* \return the computed innerproduct
*/
double arVecInnerproduct( ARVec *x, ARVec *y );

/** \fn int arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e )
* \brief XXXBK
*
* XXXBK
* \param a XXXBK
* \param d XXXBK
* \param e XXXBK
* \return XXXBK
*/
int    arVecTridiagonalize( ARMat *a, ARVec *d, ARVec *e );


#ifdef __cplusplus
}
#endif
#endif

