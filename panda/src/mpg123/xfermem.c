/*
 *   xfermem.c
 *
 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
 *   Sun Apr  6 02:26:26 MET DST 1997
 *
 *   See xfermem.h for documentation/description.
 */

#ifndef NOXFERMEM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#endif

#ifdef AIX
#include <sys/select.h>
#endif

#include "mpg123.h"
#define HAVE_INCLUDED_MPG123_H

#ifndef USE_MMAP
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

extern int errno;

#if defined (USE_MMAP) && defined(MAP_ANONYMOUS) && !defined(MAP_ANON)
#define MAP_ANON MAP_ANONYMOUS
#endif

void xfermem_init (txfermem **xf, int bufsize, int msize, int skipbuf)
{
	int regsize = bufsize + msize + skipbuf + sizeof(txfermem);
	extern int preload;

#ifdef USE_MMAP
#  ifdef MAP_ANON
	if ((*xf = (txfermem *) mmap(0, regsize, PROT_READ | PROT_WRITE,
			MAP_ANON | MAP_SHARED, -1, 0)) == (txfermem *) -1) {
		perror ("mmap()");
		exit (1);
	}
#  else
	int devzero;
	if ((devzero = open("/dev/zero", O_RDWR, 0)) == -1) {
		perror ("open(/dev/zero)");
		exit (1);
	}
	if ((*xf = (txfermem *) mmap(0, regsize, PROT_READ | PROT_WRITE,
			MAP_SHARED, devzero, 0)) == (txfermem *) -1) {
		perror ("mmap()");
		exit (1);
	}
	close (devzero);
#  endif
#else
	struct shmid_ds shmemds;
	int shmemid;
	if ((shmemid = shmget(IPC_PRIVATE, regsize, IPC_CREAT | 0600)) == -1) {
		perror ("shmget()");
		exit (1);
	}
	if ((*xf = (txfermem *) shmat(shmemid, 0, 0)) == (txfermem *) -1) {
		perror ("shmat()");
		shmctl (shmemid, IPC_RMID, &shmemds);
		exit (1);
	}
	if (shmctl(shmemid, IPC_RMID, &shmemds) == -1) {
		perror ("shmctl()");
		xfermem_done (*xf);
		exit (1);
	}
#endif
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, (*xf)->fd) < 0) {
		perror ("socketpair()");
		xfermem_done (*xf);
		exit (1);
	}
	(*xf)->freeindex = (*xf)->readindex = 0;
	(*xf)->wakeme[0] = (*xf)->wakeme[1] = FALSE;
	(*xf)->data = ((byte *) *xf) + sizeof(txfermem) + msize;
	(*xf)->metadata = ((byte *) *xf) + sizeof(txfermem);
	(*xf)->size = bufsize;
	(*xf)->metasize = msize + skipbuf;
	preload = bufsize>>3;
}

void xfermem_done (txfermem *xf)
{
	if(!xf)
		return;
#ifdef USE_MMAP
	munmap ((caddr_t) xf, xf->size + xf->metasize + sizeof(txfermem));
#else
	if (shmdt((void *) xf) == -1) {
		perror ("shmdt()");
		exit (1);
	}
#endif
}

void xfermem_init_writer (txfermem *xf)
{
	if(xf)
		close (xf->fd[XF_READER]);
}

void xfermem_init_reader (txfermem *xf)
{
	if(xf)
		close (xf->fd[XF_WRITER]);
}

int xfermem_get_freespace (txfermem *xf)
{
	int freeindex, readindex;

	if(!xf)
		return 0;

	if ((freeindex = xf->freeindex) < 0
			|| (readindex = xf->readindex) < 0)
		return (0);
	if (readindex > freeindex)
		return ((readindex - freeindex) - 1);
	else
		return ((xf->size - (freeindex - readindex)) - 1);
}

int xfermem_get_usedspace (txfermem *xf)
{
	int freeindex, readindex;

	if(!xf)
		return 0;

	if ((freeindex = xf->freeindex) < 0
			|| (readindex = xf->readindex) < 0)
		return (0);
	if (freeindex >= readindex)
		return (freeindex - readindex);
	else
		return (xf->size - (readindex - freeindex));
}

int xfermem_getcmd (int fd, int block)
{
	fd_set selfds;
	byte cmd;

	for (;;) {
		struct timeval selto = {0, 0};

		FD_ZERO (&selfds);
		FD_SET (fd, &selfds);
		/* #ifdef HPUX */ /* seems to trigger performance problems? strange */
#if 0
		switch (select(FD_SETSIZE, (int *) &selfds, NULL, NULL, block ? NULL : &selto)) {
#else
		switch (select(FD_SETSIZE, &selfds, NULL, NULL, block ? NULL : &selto)) {
#endif
			case 0:
				if (!block)
					return (0);
				continue;
			case -1:
				if (errno == EINTR)
					continue;
				return (-2);
			case 1:
				if (FD_ISSET(fd, &selfds))
					switch (read(fd, &cmd, 1)) {
						case 0: /* EOF */
							return (-1);
						case -1:
							if (errno == EINTR)
								continue;
							return (-3);
						case 1:
							return (cmd);
						default: /* ?!? */
							return (-4);
					}
				else /* ?!? */
					return (-5);
			default: /* ?!? */
				return (-6);
		}
	}
}

int xfermem_putcmd (int fd, byte cmd)
{
	for (;;) {
		switch (write(fd, &cmd, 1)) {
			case 1:
				return (1);
			case -1:
				if (errno != EINTR)
					return (-1);
		}
	}
}

int xfermem_block (int readwrite, txfermem *xf)
{
	int myfd = xf->fd[readwrite];
	int result;

	xf->wakeme[readwrite] = TRUE;
	if (xf->wakeme[1 - readwrite])
		xfermem_putcmd (myfd, XF_CMD_WAKEUP);
	result = xfermem_getcmd(myfd, TRUE);
	xf->wakeme[readwrite] = FALSE;
	return ((result <= 0) ? -1 : result);
}

#elif defined(WIN32)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef HAVE_INCLUDED_MPG123_H
#include "mpg123.h"
#else /* HAVE_INCLUDED_MPG123_H */
#include "xfermem.h"
#endif /* HAVE_INCLUDED_MPG123_H */

extern int errno;

void xfermem_init (txfermem **xf, int bufsize, int msize, int skipbuf)
{
  return;
}
void xfermem_done (txfermem *xf)
{
  return;
}
void xfermem_init_writer (txfermem *xf)
{
  return;
}
void xfermem_init_reader (txfermem *xf)
{
  return;
}
int xfermem_get_freespace (txfermem *xf)
{
  return 0;
}
int xfermem_get_usedspace (txfermem *xf)
{
  return 0;
}
int xfermem_getcmd (int fd, int block)
{
  return 0;
}
int xfermem_putcmd (int fd, byte cmd)
{
  return 0;
}
int xfermem_block (int readwrite, txfermem *xf)
{
  return 0;
}
#endif

/* eof */

