/* Copyright (c) 1994 Regents of the University of California */

#ifndef lint
static char SCCSid[] = "@(#)header.c 2.4 2/27/94 LBL";
#endif

/*
 *  header.c - routines for reading and writing information headers.
 *
 *	8/19/88
 *
 *  newheader(t,fp)	start new information header identified by string t
 *  isheadid(s)		returns true if s is a header id line
 *  headidval(r,s)	copy header identifier value in s to r
 *  printargs(ac,av,fp) print an argument list to fp, followed by '\n'
 *  isformat(s)		returns true if s is of the form "FORMAT=*"
 *  formatval(r,s)	copy the format value in s to r
 *  fputformat(s,fp)	write "FORMAT=%s" to fp
 *  getheader(fp,f,p)	read header from fp, calling f(s,p) on each line
 *  checkheader(i,p,o)	check header format from i against p and copy to o
 *
 *  To copy header from input to output, use getheader(fin, fputs, fout)
 */

#include  <stdio.h>
#include  <ctype.h>
#include <string.h>

#define	 MAXLINE	512

char  HDRSTR[] = "#?";		/* information header magic number */

char  FMTSTR[] = "FORMAT=";	/* format identifier */


void
newheader(char *s, register FILE *fp)		/* identifying line of information header */
{
	fputs(HDRSTR, fp);
	fputs(s, fp);
	putc('\n', fp);
}


headidval(register char *r, register char *s)			/* get header id (return true if is id) */
{
	register char  *cp = HDRSTR;

	while (*cp) if (*cp++ != *s++) return(0);
	if (r == NULL) return(1);
	while (*s) *r++ = *s++;
	*r = '\0';
	return(1);
}


isheadid(char *s)			/* check to see if line is header id */
{
	return(headidval(NULL, s));
}


void
printargs(int ac, char **av, register FILE *fp)		/* print arguments to a file */
{
	int  quote;

	while (ac-- > 0) {
		if (strchr(*av, ' ') != NULL) {		/* quote it */
			if (strchr(*av, '\'') != NULL)
				quote = '"';
			else
				quote = '\'';
			putc(quote, fp);
			fputs(*av++, fp);
			putc(quote, fp);
		} else
			fputs(*av++, fp);
		putc(' ', fp);
	}
	putc('\n', fp);
}


formatval(register char *r, register char *s)			/* get format value (return true if format) */
{
	register char  *cp = FMTSTR;

	while (*cp) if (*cp++ != *s++) return(0);
	while (isspace(*s)) s++;
	if (!*s) return(0);
	if (r == NULL) return(1);
	while(*s) *r++ = *s++;
	while (isspace(r[-1])) r--;
	*r = '\0';
	return(1);
}


isformat(char *s)			/* is line a format line? */
{
	return(formatval(NULL, s));
}


void
fputformat(char *s, FILE *fp)		/* put out a format value */
{
	fputs(FMTSTR, fp);
	fputs(s, fp);
	putc('\n', fp);
}


int
getheader(FILE *fp, int (*f)(char *, char *), char *p)		/* get header from file */
{
	char  buf[MAXLINE];

	for ( ; ; ) {
		buf[MAXLINE-2] = '\n';
		if (fgets(buf, MAXLINE, fp) == NULL)
			return(-1);
		if (buf[0] == '\n')
			return(0);
#ifdef MSDOS
		if (buf[0] == '\r' && buf[1] == '\n')
			return(0);
#endif
		if (buf[MAXLINE-2] != '\n') {
			ungetc(buf[MAXLINE-2], fp);	/* prevent false end */
			buf[MAXLINE-2] = '\0';
		}
		if (f != NULL)
			(*f)(buf, p);
	}
}


struct check {
	FILE	*fp;
	char	fs[64];
};


static void
mycheck(char *s, register struct check *cp)			/* check a header line for format info. */
{
	if (!formatval(cp->fs, s) && cp->fp != NULL)
		fputs(s, cp->fp);
}


/*
 * Copymatch(pat,str) checks pat for wildcards, and
 * copies str into pat if there is a match (returning true).
 */

#ifdef COPYMATCH
copymatch(char *pat, char *str)
{
	int	docopy = 0;
	register char	*p = pat, *s = str;

	do {
		switch (*p) {
		case '?':			/* match any character */
			if (!*s++)
				return(0);
			docopy++;
			break;
		case '*':			/* match any string */
			while (p[1] == '*') p++;
			do
				if ( (p[1]=='?' || p[1]==*s)
						&& copymatch(p+1,s) ) {
					strcpy(pat, str);
					return(1);
				}
			while (*s++);
			return(0);
		case '\\':			/* literal next */
			p++;
		/* fall through */
		default:			/* normal character */
			if (*p != *s)
				return(0);
			s++;
			break;
		}
	} while (*p++);
	if (docopy)
		strcpy(pat, str);
	return(1);
}
#else
#define copymatch(pat, s)	(!strcmp(pat, s))
#endif


/*
 * Checkheader(fin,fmt,fout) returns a value of 1 if the input format
 * matches the specification in fmt, 0 if no input format was found,
 * and -1 if the input format does not match or there is an
 * error reading the header.  If fmt is empty, then -1 is returned
 * if any input format is found (or there is an error), and 0 otherwise.
 * If fmt contains any '*' or '?' characters, then checkheader
 * does wildcard expansion and copies a matching result into fmt.
 * Be sure that fmt is big enough to hold the match in such cases!
 * The input header (minus any format lines) is copied to fout
 * if fout is not NULL.
 */

checkheader(FILE *fin, char *fmt, FILE *fout)
{
	struct check	cdat;

	cdat.fp = fout;
	cdat.fs[0] = '\0';
	if (getheader(fin, (int (*)(char *, char *))mycheck, (char *)&cdat) < 0)
		return(-1);
	if (cdat.fs[0] != '\0')
		return(copymatch(fmt, cdat.fs) ? 1 : -1);
	return(0);
}
