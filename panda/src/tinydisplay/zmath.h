#ifndef __ZMATH__
#define __ZMATH__

/* Matrix & Vertex */

typedef struct {
  PN_stdfloat m[4][4];
} M4;

typedef struct {
  PN_stdfloat m[3][3];
} M3;

typedef struct {
  PN_stdfloat m[3][4];
} M34;

typedef struct {
  PN_stdfloat v[2];
} V2;

typedef struct {
  PN_stdfloat v[3];
} V3;

typedef struct {
  PN_stdfloat v[4];
} V4;

void gl_M4_Id(M4 *a);
int gl_M4_IsId(M4 *a);
void gl_M4_Move(M4 *a,M4 *b);
void gl_MoveV3(V3 *a,V3 *b);
void gl_MulM4V3(V3 *a,M4 *b,V3 *c);
void gl_MulM3V3(V3 *a,M4 *b,V3 *c);

void gl_M4_MulV4(V4 * a,M4 *b,V4 * c);
void gl_M4_InvOrtho(M4 *a,M4 b);
void gl_M4_Inv(M4 *a,M4 *b);
void gl_M4_Mul(M4 *c,M4 *a,M4 *b);
void gl_M4_MulLeft(M4 *c,M4 *a);
void gl_M4_Transpose(M4 *a,M4 *b);
void gl_M4_Rotate(M4 *c,PN_stdfloat t,int u);
int  gl_V3_Norm(V3 *a);

V3 gl_V3_New(PN_stdfloat x,PN_stdfloat y,PN_stdfloat z);
V4 gl_V4_New(PN_stdfloat x,PN_stdfloat y,PN_stdfloat z,PN_stdfloat w);

int gl_Matrix_Inv(PN_stdfloat *r,PN_stdfloat *m,int n);

#endif
