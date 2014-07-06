/* Some simple mathematical functions. Don't look for some logic in
   the function names :-) */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "zmath.h"


/* ******* Gestion des matrices 4x4 ****** */

void gl_M4_Id(M4 *a)
{
  int i,j;
  for(i=0;i<4;i++)
    for(j=0;j<4;j++) 
      if (i==j) a->m[i][j]=1.0; else a->m[i][j]=0.0;
}

int gl_M4_IsId(M4 *a)
{
  int i,j;
  for(i=0;i<4;i++)
    for(j=0;j<4;j++) {
      if (i==j) { 
        if (a->m[i][j] != 1.0) return 0;
      } else if (a->m[i][j] != 0.0) return 0;
    }
  return 1;
}

void gl_M4_Mul(M4 *c,M4 *a,M4 *b)
{
  int i,j,k;
  PN_stdfloat s;
  for(i=0;i<4;i++)
    for(j=0;j<4;j++) {
      s=0.0;
      for(k=0;k<4;k++) s+=a->m[i][k]*b->m[k][j];
      c->m[i][j]=s;
    }
}

/* c=c*a */
void gl_M4_MulLeft(M4 *c,M4 *b)
{
  int i,j,k;
  PN_stdfloat s;
  M4 a;

  /*memcpy(&a, c, 16*sizeof(PN_stdfloat));
   */
  a=*c;

  for(i=0;i<4;i++)
    for(j=0;j<4;j++) {
      s=0.0;
      for(k=0;k<4;k++) s+=a.m[i][k]*b->m[k][j];
      c->m[i][j]=s;
    }
}

void gl_M4_Move(M4 *a,M4 *b)
{
  memcpy(a,b,sizeof(M4));
}

void gl_MoveV3(V3 *a,V3 *b)
{
  memcpy(a,b,sizeof(V3));
}


void gl_MulM4V3(V3 *a,M4 *b,V3 *c)
{
  a->v[0]=b->m[0][0]*c->v[0]+b->m[0][1]*c->v[1]+b->m[0][2]*c->v[2]+b->m[0][3];
  a->v[1]=b->m[1][0]*c->v[0]+b->m[1][1]*c->v[1]+b->m[1][2]*c->v[2]+b->m[1][3];
  a->v[2]=b->m[2][0]*c->v[0]+b->m[2][1]*c->v[1]+b->m[2][2]*c->v[2]+b->m[2][3];
}

void gl_MulM3V3(V3 *a,M4 *b,V3 *c)
{
  a->v[0]=b->m[0][0]*c->v[0]+b->m[0][1]*c->v[1]+b->m[0][2]*c->v[2];
  a->v[1]=b->m[1][0]*c->v[0]+b->m[1][1]*c->v[1]+b->m[1][2]*c->v[2];
  a->v[2]=b->m[2][0]*c->v[0]+b->m[2][1]*c->v[1]+b->m[2][2]*c->v[2];
}

void gl_M4_MulV4(V4 *a,M4 *b,V4 *c)
{
  a->v[0]=b->m[0][0]*c->v[0]+b->m[0][1]*c->v[1]+b->m[0][2]*c->v[2]+b->m[0][3]*c->v[3];
  a->v[1]=b->m[1][0]*c->v[0]+b->m[1][1]*c->v[1]+b->m[1][2]*c->v[2]+b->m[1][3]*c->v[3];
  a->v[2]=b->m[2][0]*c->v[0]+b->m[2][1]*c->v[1]+b->m[2][2]*c->v[2]+b->m[2][3]*c->v[3];
  a->v[3]=b->m[3][0]*c->v[0]+b->m[3][1]*c->v[1]+b->m[3][2]*c->v[2]+b->m[3][3]*c->v[3];
}

/* transposition of a 4x4 matrix */
void gl_M4_Transpose(M4 *a,M4 *b)
{
  a->m[0][0]=b->m[0][0]; 
  a->m[0][1]=b->m[1][0]; 
  a->m[0][2]=b->m[2][0]; 
  a->m[0][3]=b->m[3][0]; 

  a->m[1][0]=b->m[0][1]; 
  a->m[1][1]=b->m[1][1]; 
  a->m[1][2]=b->m[2][1]; 
  a->m[1][3]=b->m[3][1]; 

  a->m[2][0]=b->m[0][2]; 
  a->m[2][1]=b->m[1][2]; 
  a->m[2][2]=b->m[2][2]; 
  a->m[2][3]=b->m[3][2]; 

  a->m[3][0]=b->m[0][3]; 
  a->m[3][1]=b->m[1][3]; 
  a->m[3][2]=b->m[2][3]; 
  a->m[3][3]=b->m[3][3]; 
}

/* inversion of an orthogonal matrix of type Y=M.X+P */ 
void gl_M4_InvOrtho(M4 *a,M4 b)
{
  int i,j;
  PN_stdfloat s;
  for(i=0;i<3;i++)
    for(j=0;j<3;j++) a->m[i][j]=b.m[j][i];
  a->m[3][0]=0.0; a->m[3][1]=0.0; a->m[3][2]=0.0; a->m[3][3]=1.0;
  for(i=0;i<3;i++) {
    s=0;
    for(j=0;j<3;j++) s-=b.m[j][i]*b.m[j][3];
    a->m[i][3]=s;
  }
}

/* Inversion of a general nxn matrix.
   Note : m is destroyed */

int Matrix_Inv(PN_stdfloat *r,PN_stdfloat *m,int n)
{
  int i,j,k,l;
  PN_stdfloat max,tmp,t;

  /* identite dans r */
  for(i=0;i<n*n;i++) r[i]=0;
  for(i=0;i<n;i++) r[i*n+i]=1;

  for(j=0;j<n;j++) {

    /* recherche du nombre de plus grand module sur la colonne j */
    max=m[j*n+j];
    k=j;
    for(i=j+1;i<n;i++)
      if (fabs(m[i*n+j])>fabs(max)) {
        k=i;
        max=m[i*n+j];
      }

    /* non intersible matrix */
    if (max==0) return 1;


    /* permutation des lignes j et k */
    if (k!=j) {
      for(i=0;i<n;i++) {
        tmp=m[j*n+i];
        m[j*n+i]=m[k*n+i];
        m[k*n+i]=tmp;

        tmp=r[j*n+i];
        r[j*n+i]=r[k*n+i];
        r[k*n+i]=tmp;
      }
    }

    /* multiplication de la ligne j par 1/max */
    max=1/max;
    for(i=0;i<n;i++) {
      m[j*n+i]*=max;
      r[j*n+i]*=max;
    }


    for(l=0;l<n;l++) if (l!=j) {
        t=m[l*n+j];
        for(i=0;i<n;i++) {
          m[l*n+i]-=m[j*n+i]*t;
          r[l*n+i]-=r[j*n+i]*t;
        }
      }
  }

  return 0;
}


/* inversion of a 4x4 matrix */

void gl_M4_Inv(M4 *a,M4 *b)
{
  M4 tmp;
  memcpy(&tmp, b, 16*sizeof(PN_stdfloat));
  /*tmp=*b;*/
  Matrix_Inv(&a->m[0][0],&tmp.m[0][0],4);
}

void gl_M4_Rotate(M4 *a,PN_stdfloat t,int u)
{
  PN_stdfloat s,c;
  int v,w;
  if ((v=u+1)>2) v=0;
  if ((w=v+1)>2) w=0;
  s=sin(t);
  c=cos(t);
  gl_M4_Id(a);
  a->m[v][v]=c;  a->m[v][w]=-s;
  a->m[w][v]=s;  a->m[w][w]=c;
}


/* inverse of a 3x3 matrix */
void gl_M3_Inv(M3 *a,M3 *m)
{
  PN_stdfloat det;

  det = m->m[0][0]*m->m[1][1]*m->m[2][2]-m->m[0][0]*m->m[1][2]*m->m[2][1]-
    m->m[1][0]*m->m[0][1]*m->m[2][2]+m->m[1][0]*m->m[0][2]*m->m[2][1]+
    m->m[2][0]*m->m[0][1]*m->m[1][2]-m->m[2][0]*m->m[0][2]*m->m[1][1];

  a->m[0][0] = (m->m[1][1]*m->m[2][2]-m->m[1][2]*m->m[2][1])/det;
  a->m[0][1] = -(m->m[0][1]*m->m[2][2]-m->m[0][2]*m->m[2][1])/det;
  a->m[0][2] = -(-m->m[0][1]*m->m[1][2]+m->m[0][2]*m->m[1][1])/det;

  a->m[1][0] = -(m->m[1][0]*m->m[2][2]-m->m[1][2]*m->m[2][0])/det;
  a->m[1][1] = (m->m[0][0]*m->m[2][2]-m->m[0][2]*m->m[2][0])/det;
  a->m[1][2] = -(m->m[0][0]*m->m[1][2]-m->m[0][2]*m->m[1][0])/det;

  a->m[2][0] = (m->m[1][0]*m->m[2][1]-m->m[1][1]*m->m[2][0])/det;
  a->m[2][1] = -(m->m[0][0]*m->m[2][1]-m->m[0][1]*m->m[2][0])/det;
  a->m[2][2] = (m->m[0][0]*m->m[1][1]-m->m[0][1]*m->m[1][0])/det;
}


/* vector arithmetic */

int gl_V3_Norm(V3 *a)
{
  PN_stdfloat n;
  n=sqrt(a->v[0]*a->v[0]+a->v[1]*a->v[1]+a->v[2]*a->v[2]);
  if (n==0) return 1;
  a->v[0]/=n;
  a->v[1]/=n;
  a->v[2]/=n;
  return 0;
}

V3 gl_V3_New(PN_stdfloat x,PN_stdfloat y,PN_stdfloat z)
{
  V3 a;
  a.v[0]=x;
  a.v[1]=y;
  a.v[2]=z;
  return a;
}

V4 gl_V4_New(PN_stdfloat x,PN_stdfloat y,PN_stdfloat z,PN_stdfloat w)
{
  V4 a;
  a.v[0]=x;
  a.v[1]=y;
  a.v[2]=z;
  a.v[3]=w;
  return a;
}
