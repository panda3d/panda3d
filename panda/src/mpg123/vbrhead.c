/*
 * This checks for the VBR Header defined by Xing(tm)
 */

#include "mpg123.h"

static unsigned long get32bits(unsigned char *buf) {
    unsigned long ret = 0;

    ret = (((unsigned long) buf[0]) << 24) |
	(((unsigned long) buf[1]) << 16) |
	(((unsigned long) buf[2]) << 8) |
	((unsigned long) buf[3]) ;

    return ret;
}

int getVBRHeader(struct vbrHeader *head,unsigned char *buf, struct frame *fr) 
{
    int ssize;

    if(fr->lay != 3)
	return 0;

    if(fr->lsf)
	ssize = (fr->stereo == 1) ? 9 : 17;
    else
	ssize = (fr->stereo == 1) ? 17 : 32;


    buf += ssize;

    if(( buf[0] != 'X' ) || ( buf[1] != 'i' ) ||
       ( buf[2] != 'n' ) || ( buf[3] != 'g' ) ) 
	return 0;
    buf+=4;
    
    head->flags = get32bits(buf);
    buf+=4;
    
    if(head->flags & VBR_FRAMES_FLAG) {
	head->frames = get32bits(buf);
	buf += 4;
    }

    if(head->flags & VBR_BYTES_FLAG) {
	head->bytes  = get32bits(buf); 
	buf += 4;
    }

    if(head->flags & VBR_TOC_FLAG) {
	memcpy(head->toc,buf,100);
	buf += 100;
    }

    if(head->flags & VBR_SCALE_FLAG) {
	head->scale = get32bits(buf);
	buf += 4;
    }

    fprintf(stderr,"Found XING %04lx\n",head->flags);

    return 1;

}














