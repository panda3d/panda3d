/* Copyright (c) 1991 Regents of the University of California */

#ifndef lint
static char SCCSid[] = "@(#)resolu.c 2.2 11/28/91 LBL";
#endif

/*
 * Read and write image resolutions.
 */

#include <stdio.h>
#include <stdlib.h>

#include "resolu.h"


char  resolu_buf[RESOLU_BUFLEN];	/* resolution line buffer */

int str2resolu(register RESOLU *, char *);
char *resolu2str(char *, register RESOLU *);

void
fputresolu(int ord, int sl, int ns, FILE *fp)		/* put out picture dimensions */
{
	RESOLU  rs;

	if ((rs.or = ord) & YMAJOR) {
		rs.xr = sl;
		rs.yr = ns;
	} else {
		rs.xr = ns;
		rs.yr = sl;
	}
	fputsresolu(&rs, fp);
}


int
fgetresolu(int *sl, int *ns, FILE *fp)			/* get picture dimensions */
{
	RESOLU  rs;

	if (!fgetsresolu(&rs, fp))
		return(-1);
	if (rs.or & YMAJOR) {
		*sl = rs.xr;
		*ns = rs.yr;
	} else {
		*sl = rs.yr;
		*ns = rs.xr;
	}
	return(rs.or);
}


char *
resolu2str(char *buf, register RESOLU *rp)		/* convert resolution struct to line */
{
	if (rp->or&YMAJOR)
		sprintf(buf, "%cY %d %cX %d\n",
				rp->or&YDECR ? '-' : '+', rp->yr,
				rp->or&XDECR ? '-' : '+', rp->xr);
	else
		sprintf(buf, "%cX %d %cY %d\n",
				rp->or&XDECR ? '-' : '+', rp->xr,
				rp->or&YDECR ? '-' : '+', rp->yr);
	return(buf);
}


int str2resolu(register RESOLU *rp, char *buf)		/* convert resolution line to struct */
{
	register char  *xndx, *yndx;
	register char  *cp;

	if (buf == NULL)
		return(0);
	xndx = yndx = NULL;
	for (cp = buf; *cp; cp++)
		if (*cp == 'X')
			xndx = cp;
		else if (*cp == 'Y')
			yndx = cp;
	if (xndx == NULL || yndx == NULL)
		return(0);
	rp->or = 0;
	if (xndx > yndx) rp->or |= YMAJOR;
	if (xndx[-1] == '-') rp->or |= XDECR;
	if (yndx[-1] == '-') rp->or |= YDECR;
	if ((rp->xr = atoi(xndx+1)) <= 0)
		return(0);
	if ((rp->yr = atoi(yndx+1)) <= 0)
		return(0);
	return(1);
}
