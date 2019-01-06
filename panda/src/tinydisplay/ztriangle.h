/*
 * We draw a triangle with various interpolations
 */

{
  ZBufferPoint *t,*pr1,*pr2,*l1,*l2;
  PN_stdfloat fdx1, fdx2, fdy1, fdy2, fz, d1, d2;
  ZPOINT *pz1;
  PIXEL *pp1;
  int part, update_left, update_right;

  int nb_lines, dx1, dy1, tmp, dx2, dy2;

  int error, derror;
  int x1, dxdy_min, dxdy_max;
  /* warning: x2 is multiplied by 2^16 */
  int x2, dx2dy2;

#ifdef INTERP_Z
  int z1 = 0, dzdx = 0, dzdy = 0, dzdl_min = 0, dzdl_max = 0;
#endif
#ifdef INTERP_RGB
  int r1 = 0, drdx = 0, drdy = 0, drdl_min = 0, drdl_max = 0;
  int g1 = 0, dgdx = 0, dgdy = 0, dgdl_min = 0, dgdl_max = 0;
  int b1 = 0, dbdx = 0, dbdy = 0, dbdl_min = 0, dbdl_max = 0;
  int a1 = 0, dadx = 0, dady = 0, dadl_min = 0, dadl_max = 0;
#endif
#ifdef INTERP_ST
  int s1 = 0, dsdx = 0, dsdy = 0, dsdl_min = 0, dsdl_max = 0;
  int t1 = 0, dtdx = 0, dtdy = 0, dtdl_min = 0, dtdl_max = 0;
#endif
#ifdef INTERP_STZ
  PN_stdfloat sz1 = 0, dszdx = 0, dszdy = 0, dszdl_min = 0, dszdl_max = 0;
  PN_stdfloat tz1 = 0, dtzdx = 0, dtzdy = 0, dtzdl_min = 0, dtzdl_max = 0;
#endif
#ifdef INTERP_STZA
  PN_stdfloat sza1 = 0, dszadx = 0, dszady = 0, dszadl_min = 0, dszadl_max = 0;
  PN_stdfloat tza1 = 0, dtzadx = 0, dtzady = 0, dtzadl_min = 0, dtzadl_max = 0;
#endif
#ifdef INTERP_STZB
  PN_stdfloat szb1 = 0, dszbdx = 0, dszbdy = 0, dszbdl_min = 0, dszbdl_max = 0;
  PN_stdfloat tzb1 = 0, dtzbdx = 0, dtzbdy = 0, dtzbdl_min = 0, dtzbdl_max = 0;
#endif
#if defined(INTERP_MIPMAP) && (defined(INTERP_ST) || defined(INTERP_STZ))
  unsigned int mipmap_dx = 0, mipmap_level = 0;
#endif
#if defined(INTERP_MIPMAP) && defined(INTERP_STZA)
  unsigned int mipmap_dxa = 0, mipmap_levela = 0;
#endif
#if defined(INTERP_MIPMAP) && defined(INTERP_STZB)
  unsigned int mipmap_dxb = 0, mipmap_levelb = 0;
#endif

  EARLY_OUT();

  COUNT_PIXELS(PIXEL_COUNT, p0, p1, p2);

  /* we sort the vertex with increasing y */
  if (p1->y < p0->y) {
    t = p0;
    p0 = p1;
    p1 = t;
  }
  if (p2->y < p0->y) {
    t = p2;
    p2 = p1;
    p1 = p0;
    p0 = t;
  } else if (p2->y < p1->y) {
    t = p1;
    p1 = p2;
    p2 = t;
  }

  /* we compute dXdx and dXdy for all interpolated values */
  
  fdx1 = (PN_stdfloat) (p1->x - p0->x);
  fdy1 = (PN_stdfloat) (p1->y - p0->y);

  fdx2 = (PN_stdfloat) (p2->x - p0->x);
  fdy2 = (PN_stdfloat) (p2->y - p0->y);

  fz = fdx1 * fdy2 - fdx2 * fdy1;
  if (fz == 0)
    return;
  fz = 1.0f / fz;

  fdx1 *= fz;
  fdy1 *= fz;
  fdx2 *= fz;
  fdy2 *= fz;

#ifdef INTERP_Z
  d1 = (PN_stdfloat) (p1->z - p0->z);
  d2 = (PN_stdfloat) (p2->z - p0->z);
  dzdx = (int) (fdy2 * d1 - fdy1 * d2);
  dzdy = (int) (fdx1 * d2 - fdx2 * d1);
#endif

#ifdef INTERP_RGB
  d1 = (PN_stdfloat) (p1->r - p0->r);
  d2 = (PN_stdfloat) (p2->r - p0->r);
  drdx = (int) (fdy2 * d1 - fdy1 * d2);
  drdy = (int) (fdx1 * d2 - fdx2 * d1);

  d1 = (PN_stdfloat) (p1->g - p0->g);
  d2 = (PN_stdfloat) (p2->g - p0->g);
  dgdx = (int) (fdy2 * d1 - fdy1 * d2);
  dgdy = (int) (fdx1 * d2 - fdx2 * d1);

  d1 = (PN_stdfloat) (p1->b - p0->b);
  d2 = (PN_stdfloat) (p2->b - p0->b);
  dbdx = (int) (fdy2 * d1 - fdy1 * d2);
  dbdy = (int) (fdx1 * d2 - fdx2 * d1);

  d1 = (PN_stdfloat) (p1->a - p0->a);
  d2 = (PN_stdfloat) (p2->a - p0->a);
  dadx = (int) (fdy2 * d1 - fdy1 * d2);
  dady = (int) (fdx1 * d2 - fdx2 * d1);

#endif
  
#ifdef INTERP_ST
  d1 = (PN_stdfloat) (p1->s - p0->s);
  d2 = (PN_stdfloat) (p2->s - p0->s);
  dsdx = (int) (fdy2 * d1 - fdy1 * d2);
  dsdy = (int) (fdx1 * d2 - fdx2 * d1);
  
  d1 = (PN_stdfloat) (p1->t - p0->t);
  d2 = (PN_stdfloat) (p2->t - p0->t);
  dtdx = (int) (fdy2 * d1 - fdy1 * d2);
  dtdy = (int) (fdx1 * d2 - fdx2 * d1);

  CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);
#endif

#ifdef INTERP_STZ
  {
    PN_stdfloat zz;
    zz=(PN_stdfloat) p0->z;
    p0->sz= (PN_stdfloat) p0->s * zz;
    p0->tz= (PN_stdfloat) p0->t * zz;
    zz=(PN_stdfloat) p1->z;
    p1->sz= (PN_stdfloat) p1->s * zz;
    p1->tz= (PN_stdfloat) p1->t * zz;
    zz=(PN_stdfloat) p2->z;
    p2->sz= (PN_stdfloat) p2->s * zz;
    p2->tz= (PN_stdfloat) p2->t * zz;

    d1 = p1->sz - p0->sz;
    d2 = p2->sz - p0->sz;
    dszdx = (fdy2 * d1 - fdy1 * d2);
    dszdy = (fdx1 * d2 - fdx2 * d1);
    
    d1 = p1->tz - p0->tz;
    d2 = p2->tz - p0->tz;
    dtzdx = (fdy2 * d1 - fdy1 * d2);
    dtzdy = (fdx1 * d2 - fdx2 * d1);
  }
#endif

#ifdef INTERP_STZA
  {
    PN_stdfloat zz;
    zz=(PN_stdfloat) p0->z;
    p0->sza= (PN_stdfloat) p0->sa * zz;
    p0->tza= (PN_stdfloat) p0->ta * zz;
    zz=(PN_stdfloat) p1->z;
    p1->sza= (PN_stdfloat) p1->sa * zz;
    p1->tza= (PN_stdfloat) p1->ta * zz;
    zz=(PN_stdfloat) p2->z;
    p2->sza= (PN_stdfloat) p2->sa * zz;
    p2->tza= (PN_stdfloat) p2->ta * zz;

    d1 = p1->sza - p0->sza;
    d2 = p2->sza - p0->sza;
    dszadx = (fdy2 * d1 - fdy1 * d2);
    dszady = (fdx1 * d2 - fdx2 * d1);
    
    d1 = p1->tza - p0->tza;
    d2 = p2->tza - p0->tza;
    dtzadx = (fdy2 * d1 - fdy1 * d2);
    dtzady = (fdx1 * d2 - fdx2 * d1);
  }
#endif

#ifdef INTERP_STZB
  {
    PN_stdfloat zz;
    zz=(PN_stdfloat) p0->z;
    p0->szb= (PN_stdfloat) p0->sb * zz;
    p0->tzb= (PN_stdfloat) p0->tb * zz;
    zz=(PN_stdfloat) p1->z;
    p1->szb= (PN_stdfloat) p1->sb * zz;
    p1->tzb= (PN_stdfloat) p1->tb * zz;
    zz=(PN_stdfloat) p2->z;
    p2->szb= (PN_stdfloat) p2->sb * zz;
    p2->tzb= (PN_stdfloat) p2->tb * zz;

    d1 = p1->szb - p0->szb;
    d2 = p2->szb - p0->szb;
    dszbdx = (fdy2 * d1 - fdy1 * d2);
    dszbdy = (fdx1 * d2 - fdx2 * d1);
    
    d1 = p1->tzb - p0->tzb;
    d2 = p2->tzb - p0->tzb;
    dtzbdx = (fdy2 * d1 - fdy1 * d2);
    dtzbdy = (fdx1 * d2 - fdx2 * d1);
  }
#endif

  /* screen coordinates */

  pp1 = (PIXEL *) ((char *) zb->pbuf + zb->linesize * p0->y);
  pz1 = zb->zbuf + p0->y * zb->xsize;

  DRAW_INIT();

  for(part=0;part<2;part++) {
    if (part == 0) {
      if (fz > 0) {
        update_left=1;
        update_right=1;
        l1=p0;
        l2=p2;
        pr1=p0;
        pr2=p1;
      } else {
        update_left=1;
        update_right=1;
        l1=p0;
        l2=p1;
        pr1=p0;
        pr2=p2;
      }
      nb_lines = p1->y - p0->y;
    } else {
      /* second part */
      if (fz > 0) {
        update_left=0;
        update_right=1;
        pr1=p1;
        pr2=p2;
      } else {
        update_left=1;
        update_right=0;
        l1=p1; 
        l2=p2;
      }
      nb_lines = p2->y - p1->y + 1;
    }

    /* compute the values for the left edge */

    if (update_left) {
      dy1 = l2->y - l1->y;
      dx1 = l2->x - l1->x;
      if (dy1 > 0) 
        tmp = (dx1 << 16) / dy1;
      else
        tmp = 0;
      x1 = l1->x;
      error = 0;
      derror = tmp & 0x0000ffff;
      dxdy_min = tmp >> 16;
      dxdy_max = dxdy_min + 1;
      
#ifdef INTERP_Z
      z1=l1->z;
      dzdl_min=(dzdy + dzdx * dxdy_min); 
      dzdl_max=dzdl_min + dzdx;
#endif
#ifdef INTERP_RGB
      r1=l1->r;
      drdl_min=(drdy + drdx * dxdy_min);
      drdl_max=drdl_min + drdx;
      
      g1=l1->g;
      dgdl_min=(dgdy + dgdx * dxdy_min);
      dgdl_max=dgdl_min + dgdx;
      
      b1=l1->b;
      dbdl_min=(dbdy + dbdx * dxdy_min);
      dbdl_max=dbdl_min + dbdx;
      
      a1=l1->a;
      dadl_min=(dady + dadx * dxdy_min);
      dadl_max=dadl_min + dadx;
#endif
#ifdef INTERP_ST
      s1=l1->s;
      dsdl_min=(dsdy + dsdx * dxdy_min);
      dsdl_max=dsdl_min + dsdx;
      
      t1=l1->t;
      dtdl_min=(dtdy + dtdx * dxdy_min);
      dtdl_max=dtdl_min + dtdx;
#endif
#ifdef INTERP_STZ
      sz1=l1->sz;
      dszdl_min=(dszdy + dszdx * dxdy_min);
      dszdl_max=dszdl_min + dszdx;
      
      tz1=l1->tz;
      dtzdl_min=(dtzdy + dtzdx * dxdy_min);
      dtzdl_max=dtzdl_min + dtzdx;
#endif
#ifdef INTERP_STZA
      sza1=l1->sza;
      dszadl_min=(dszady + dszadx * dxdy_min);
      dszadl_max=dszadl_min + dszadx;
      
      tza1=l1->tza;
      dtzadl_min=(dtzady + dtzadx * dxdy_min);
      dtzadl_max=dtzadl_min + dtzadx;
#endif
#ifdef INTERP_STZB
      szb1=l1->szb;
      dszbdl_min=(dszbdy + dszbdx * dxdy_min);
      dszbdl_max=dszbdl_min + dszbdx;
      
      tzb1=l1->tzb;
      dtzbdl_min=(dtzbdy + dtzbdx * dxdy_min);
      dtzbdl_max=dtzbdl_min + dtzbdx;
#endif
    }

    /* compute values for the right edge */

    if (update_right) {
      dx2 = (pr2->x - pr1->x);
      dy2 = (pr2->y - pr1->y);
      if (dy2>0) 
        dx2dy2 = ( dx2 << 16) / dy2;
      else
        dx2dy2 = 0;
      x2 = pr1->x << 16;
    }

    /* we draw all the scan line of the part */

    while (nb_lines>0) {
      nb_lines--;
#ifndef DRAW_LINE
      /* generic draw line */
      {
        PIXEL *pp;
        int n;
#ifdef INTERP_Z
        ZPOINT *pz;
        unsigned int z,zz;
#endif
#ifdef INTERP_RGB
        unsigned int or1,og1,ob1,oa1;
#endif
#ifdef INTERP_ST
        unsigned int s,t;
#endif
#ifdef INTERP_STZ
        PN_stdfloat sz,tz;
#endif
#ifdef INTERP_STZA
        PN_stdfloat sza,tza;
#endif
#ifdef INTERP_STZB
        PN_stdfloat szb,tzb;
#endif

        n=(x2 >> 16) - x1;
        pp=(PIXEL *)((char *)pp1 + x1 * PSZB);
#ifdef INTERP_Z
        pz=pz1+x1;
        z=z1;
#endif
#ifdef INTERP_RGB
        or1 = r1;
        og1 = g1;
        ob1 = b1;
        oa1 = a1;
#endif
#ifdef INTERP_ST
        s=s1;
        t=t1;
#endif
#ifdef INTERP_STZ
        sz=sz1;
        tz=tz1;
#endif
#ifdef INTERP_STZA
        sza=sza1;
        tza=tza1;
#endif
#ifdef INTERP_STZB
        szb=szb1;
        tzb=tzb1;
#endif
        while (n>=3) {
          PUT_PIXEL(0);
          PUT_PIXEL(1);
          PUT_PIXEL(2);
          PUT_PIXEL(3);
#ifdef INTERP_Z
          pz+=4;
#endif
          pp=(PIXEL *)((char *)pp + 4 * PSZB);
          n-=4;
        }
        while (n>=0) {
          PUT_PIXEL(0);
#ifdef INTERP_Z
          pz+=1;
#endif
          pp=(PIXEL *)((char *)pp + PSZB);
          n-=1;
        }
      }
#else
      DRAW_LINE();
#endif
      
      /* left edge */
      error+=derror;
      if (error > 0) {
        error-=0x10000;
        x1+=dxdy_max;
#ifdef INTERP_Z
        z1+=dzdl_max;
#endif      
#ifdef INTERP_RGB
        r1+=drdl_max;
        g1+=dgdl_max;
        b1+=dbdl_max;
        a1+=dadl_max;
#endif
#ifdef INTERP_ST
        s1+=dsdl_max;
        t1+=dtdl_max;
#endif
#ifdef INTERP_STZ
        sz1+=dszdl_max;
        tz1+=dtzdl_max;
#endif
#ifdef INTERP_STZA
        sza1+=dszadl_max;
        tza1+=dtzadl_max;
#endif
#ifdef INTERP_STZB
        szb1+=dszbdl_max;
        tzb1+=dtzbdl_max;
#endif
      } else {
        x1+=dxdy_min;
#ifdef INTERP_Z
        z1+=dzdl_min;
#endif      
#ifdef INTERP_RGB
        r1+=drdl_min;
        g1+=dgdl_min;
        b1+=dbdl_min;
        a1+=dadl_min;
#endif
#ifdef INTERP_ST
        s1+=dsdl_min;
        t1+=dtdl_min;
#endif
#ifdef INTERP_STZ
        sz1+=dszdl_min;
        tz1+=dtzdl_min;
#endif
#ifdef INTERP_STZA
        sza1+=dszadl_min;
        tza1+=dtzadl_min;
#endif
#ifdef INTERP_STZB
        szb1+=dszbdl_min;
        tzb1+=dtzbdl_min;
#endif
      } 
      
      /* right edge */
      x2+=dx2dy2;

      /* screen coordinates */
      pp1=(PIXEL *)((char *)pp1 + zb->linesize);
      pz1+=zb->xsize;
    }
  }
}

#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef INTERP_STZA
#undef INTERP_STZB

#undef EARLY_OUT
#undef EARLY_OUT_FZ
#undef DRAW_INIT
#undef DRAW_LINE  
#undef PUT_PIXEL
#undef PIXEL_COUNT
