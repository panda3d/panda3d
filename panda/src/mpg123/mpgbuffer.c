/*
 *   buffer.c
 *
 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
 *   Mon Apr 14 03:53:18 MET DST 1997
 */

#include <stdlib.h>
#include <errno.h>

#include "mpg123.h"

int outburst = MAXOUTBURST;
int preload;

static int intflag = FALSE;
static int usr1flag = FALSE;

static void catch_interrupt (void)
{
	intflag = TRUE;
}

static void catch_usr1 (void)
{
	usr1flag = TRUE;
}

/* Interfaces to writer process */

extern void buffer_sig(int signal, int block);

void buffer_ignore_lowmem(void)
{
#ifndef NOXFERMEM
	if(buffermem->wakeme[XF_READER])
		xfermem_putcmd(buffermem->fd[XF_WRITER], XF_CMD_WAKEUP);
#endif
}

void buffer_end(void)
{
#ifndef NOXFERMEM
	xfermem_putcmd(buffermem->fd[XF_WRITER], XF_CMD_TERMINATE);
#endif
}

void buffer_resync(void)
{
	buffer_sig(SIGINT, TRUE);
}

void buffer_reset(void)
{
	buffer_sig(SIGUSR1, TRUE);
}

void buffer_start(void)
{
	buffer_sig(SIGCONT, FALSE);
}

void buffer_stop(void)
{
	buffer_sig(SIGSTOP, FALSE);
}

extern int buffer_pid;

void buffer_sig(int signal, int block)
{
	
#ifndef NOXFERMEM
	
	kill(buffer_pid, signal);
	
	if (!buffermem || !block)
		return;

	if(xfermem_block(XF_WRITER, buffermem) != XF_CMD_WAKEUP) 
		perror("Could not resync/reset buffers");
#endif
	
	return;
}

#ifndef NOXFERMEM

void buffer_loop(struct audio_info_struct *ai, sigset_t *oldsigset)
{
	int bytes;
	int my_fd = buffermem->fd[XF_READER];
	txfermem *xf = buffermem;
	int done = FALSE;

	catchsignal (SIGINT, catch_interrupt);
	catchsignal (SIGUSR1, catch_usr1);
	sigprocmask (SIG_SETMASK, oldsigset, NULL);
	if (param.outmode == DECODE_AUDIO) {
		if (audio_open(ai) < 0) {
			perror("audio");
			exit(1);
		}
	}

	for (;;) {
		if (intflag) {
			intflag = FALSE;
			if (param.outmode == DECODE_AUDIO)
				audio_queueflush (ai);
			xf->readindex = xf->freeindex;
			if (xf->wakeme[XF_WRITER])
				xfermem_putcmd(my_fd, XF_CMD_WAKEUP);
		}
		if (usr1flag) {
			usr1flag = FALSE;
			/*   close and re-open in order to flush
			 *   the device's internal buffer before
			 *   changing the sample rate.   [OF]
			 */
			/* writer must block when sending SIGUSR1
			 * or we will lose all data processed 
			 * in the meantime! [dk]
			 */
			xf->readindex = xf->freeindex;
			/* We've nailed down the new starting location -
			 * writer is now safe to go on. [dk]
			 */
			if (xf->wakeme[XF_WRITER])
				xfermem_putcmd(my_fd, XF_CMD_WAKEUP);
			if (param.outmode == DECODE_AUDIO) {
				audio_close (ai);
				ai->rate = xf->buf[0]; 
				ai->channels = xf->buf[1]; 
				ai->format = xf->buf[2];
				if (audio_open(ai) < 0) {
					perror("audio");
					exit(1);
				}
			}
		}
		if ( (bytes = xfermem_get_usedspace(xf)) < outburst ) {
			/* if we got a buffer underrun we first
			 * fill 1/8 of the buffer before continue/start
			 * playing */
			preload = xf->size>>3;
			if(preload < outburst)
				preload = outburst;
		}
		if(bytes < preload) {
			int cmd;
			if (done && !bytes) { 
				break;
			}

			if(!done) {

				cmd = xfermem_block(XF_READER, xf);

				switch(cmd) {

					/* More input pending. */
					case XF_CMD_WAKEUP_INFO:
						continue;
					/* Yes, we know buffer is low but
					 * know we don't care.
					 */
					case XF_CMD_WAKEUP:
						break;	/* Proceed playing. */
					case XF_CMD_TERMINATE:
						/* Proceed playing without 
						 * blocking any further.
						 */
						done=TRUE;
						break;
					case -1:
						if(errno==EINTR)
							continue;
						perror("Yuck! Error in buffer handling...");
						done = TRUE;
						xf->readindex = xf->freeindex;
						xfermem_putcmd(xf->fd[XF_READER], XF_CMD_TERMINATE);
						break;
					default:
						fprintf(stderr, "\nEh!? Received unknown command 0x%x in buffer process. Tell Daniel!\n", cmd);
				}
			}
		}
		preload = outburst; /* set preload to lower mark */
		if (bytes > xf->size - xf->readindex)
			bytes = xf->size - xf->readindex;
		if (bytes > outburst)
			bytes = outburst;

		if (param.outmode == DECODE_FILE)
			bytes = write(OutputDescriptor, xf->data + xf->readindex, bytes);
		else if (param.outmode == DECODE_AUDIO)
			bytes = audio_play_samples(ai,
				(unsigned char *) (xf->data + xf->readindex), bytes);

		if(bytes < 0) {
			bytes = 0;
			if(errno != EINTR) {
				perror("Ouch ... error while writing audio data: ");
				/*
				 * done==TRUE tells writer process to stop
				 * sending data. There might be some latency
				 * involved when resetting readindex to 
				 * freeindex so we might need more than one
				 * cycle to terminate. (The number of cycles
				 * should be finite unless I managed to mess
				 * up something. ;-) [dk]
				 */
				done = TRUE;	
				xf->readindex = xf->freeindex;
				xfermem_putcmd(xf->fd[XF_READER], XF_CMD_TERMINATE);
			}
		}

		xf->readindex = (xf->readindex + bytes) % xf->size;
		if (xf->wakeme[XF_WRITER])
			xfermem_putcmd(my_fd, XF_CMD_WAKEUP);
	}

	if (param.outmode == DECODE_AUDIO)
		audio_close (ai);
}

#endif

/* EOF */
