/* Filename: readers.c
 * Created by:  
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
 *
 * All use of this software is subject to the terms of the Panda 3d
 * Software license.  You should have received a copy of this license
 * along with this source code; you will also find a current copy of
 * the license at http://www.panda3d.org/license.txt .
 *
 * To contact the maintainers of this program write to
 * panda3d@yahoogroups.com .
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#endif
#include "mpg123.h"
#include "mpgbuffer.h"
#include "common.h"

#ifdef READ_MMAP
#include <sys/mman.h>
#ifndef MAP_FAILED
#define MAP_FAILED ( (void *) -1 )
#endif
#endif

static int get_fileinfo(struct reader *,char *buf);


/*******************************************************************
 * stream based operation
 */
static int fullread(int fd,unsigned char *buf,int count)
{
    int ret,cnt=0;
    while(cnt < count) {
        ret = read(fd,buf+cnt,count-cnt);
        if(ret < 0)
            return ret;
        if(ret == 0)
            break;
        cnt += ret;
    }

    return cnt;
}

static int default_init(struct reader *rds)
{
    char buf[128];

    rds->filepos = 0;
    rds->filelen = get_fileinfo(rds,buf);

    if(rds->filelen > 0) {
        if(!strncmp(buf,"TAG",3)) {
            rds->flags |= READER_ID3TAG;
            memcpy(rds->id3buf,buf,128);
        }
    }
    return 0;
}

void stream_close(struct reader *rds)
{
    if (rds->flags & READER_FD_OPENED)
        close(rds->filept);
}

/****************************************
 * HACK,HACK,HACK: step back <num> frames
 * can only work if the 'stream' isn't a real stream but a file
 */
static int stream_back_bytes(struct reader *rds,int bytes)
{
    if(lseek(rds->filept,-bytes,SEEK_CUR) < 0)
        return -1;
    if(param.usebuffer)
        buffer_resync();
    return 0;
}

static int stream_back_frame(struct reader *rds,struct frame *fr,int num)
{
    long bytes;
    unsigned char buf[4];
    unsigned long newhead;

    if(!firsthead)
        return 0;

    bytes = (fr->framesize+8)*(num+2);

    /* Skipping back/forth requires a bit more work in buffered mode.
     * See mapped_back_frame().
     */
    if(param.usebuffer)
        bytes += (long)(xfermem_get_usedspace(buffermem) /
                        (buffermem->buf[0] * buffermem->buf[1]
                         * (buffermem->buf[2] & AUDIO_FORMAT_MASK ?
                            16.0 : 8.0 ))
                        * (tabsel_123[fr->lsf][fr->lay-1][fr->bitrate_index] << 10));
    /*
      bytes += (long)(compute_buffer_offset(fr)*compute_bpf(fr));
    */
    if(lseek(rds->filept,-bytes,SEEK_CUR) < 0)
        return -1;

    if(fullread(rds->filept,buf,4) != 4)
        return -1;

    newhead = (buf[0]<<24) + (buf[1]<<16) + (buf[2]<<8) + buf[3];

    while( (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK) ) {
        if(fullread(rds->filept,buf,1) != 1)
            return -1;
        newhead <<= 8;
        newhead |= buf[0];
        newhead &= 0xffffffff;
    }

    if( lseek(rds->filept,-4,SEEK_CUR) < 0)
        return -1;

    read_frame(fr);
    read_frame(fr);

    if(fr->lay == 3) {
        set_pointer(512);
    }

    if(param.usebuffer)
        buffer_resync();

    return 0;
}

static int stream_head_read(struct reader *rds,unsigned long *newhead)
{
    unsigned char hbuf[4];

    if(fullread(rds->filept,hbuf,4) != 4)
        return FALSE;

    *newhead = ((unsigned long) hbuf[0] << 24) |
        ((unsigned long) hbuf[1] << 16) |
        ((unsigned long) hbuf[2] << 8)  |
        (unsigned long) hbuf[3];

    return TRUE;
}

static int stream_head_shift(struct reader *rds,unsigned long *head)
{
    unsigned char hbuf;

    if(fullread(rds->filept,&hbuf,1) != 1)
        return 0;
    *head <<= 8;
    *head |= hbuf;
    *head &= 0xffffffff;
    return 1;
}

static int stream_skip_bytes(struct reader *rds,int len)
{
    if (!param.usebuffer)
        return lseek(rds->filept,len,SEEK_CUR);

    else {

        int ret = lseek(rds->filept,len,SEEK_CUR);
        buffer_resync();
        return ret;

    }
}

static int stream_read_frame_body(struct reader *rds,unsigned char *buf,
                                  int size)
{
    long l;

    if( (l=fullread(rds->filept,buf,size)) != size)
        {
            if(l <= 0)
                return 0;
            memset(buf+l,0,size-l);
        }

    return 1;
}

static long stream_tell(struct reader *rds)
{
    return lseek(rds->filept,0,SEEK_CUR);
}

static void stream_rewind(struct reader *rds)
{
    lseek(rds->filept,0,SEEK_SET);
    if(param.usebuffer)
        buffer_resync();
}

/*
 * returns length of a file (if filept points to a file)
 * reads the last 128 bytes information into buffer
 */
static int get_fileinfo(struct reader *rds,char *buf)
{
    int len;

    if((len=lseek(rds->filept,0,SEEK_END)) < 0) {
        return -1;
    }
    if(lseek(rds->filept,-128,SEEK_END) < 0)
        return -1;
    if(fullread(rds->filept,(unsigned char *)buf,128) != 128) {
        return -1;
    }
    if(!strncmp(buf,"TAG",3)) {
        len -= 128;
    }
    if(lseek(rds->filept,0,SEEK_SET) < 0)
        return -1;
    if(len <= 0)
        return -1;
    return len;
}


#ifdef READ_MMAP
/*********************************************************+
 * memory mapped operation
 *
 */
static unsigned char *mapbuf;
static unsigned char *mappnt;
static unsigned char *mapend;

static int mapped_init(struct reader *rds)
{
    long len;
    char buf[128];

    len = get_fileinfo(rds,buf);
    if(len < 0)
        return -1;

    if(!strncmp(buf,"TAG",3)) {
        rds->flags |= READER_ID3TAG;
        memcpy(rds->id3buf,buf,128);
    }

    mappnt = mapbuf = (unsigned char *)
        mmap(NULL, len, PROT_READ, MAP_SHARED , rds->filept, 0);
    if(!mapbuf || mapbuf == MAP_FAILED)
        return -1;

    mapend = mapbuf + len;

    if(param.verbose > 1)
        fprintf(stderr,"Using memory mapped IO for this stream.\n");

    rds->filelen = len;
    return 0;
}

static void mapped_rewind(struct reader *rds)
{
    mappnt = mapbuf;
    if (param.usebuffer)
        buffer_resync();
}

static void mapped_close(struct reader *rds)
{
    munmap((void *)mapbuf,mapend-mapbuf);
    if (rds->flags & READER_FD_OPENED)
        close(rds->filept);
}

static int mapped_head_read(struct reader *rds,unsigned long *newhead)
{
    unsigned long nh;

    if(mappnt + 4 > mapend)
        return FALSE;

    nh = (*mappnt++)  << 24;
    nh |= (*mappnt++) << 16;
    nh |= (*mappnt++) << 8;
    nh |= (*mappnt++) ;

    *newhead = nh;
    return TRUE;
}

static int mapped_head_shift(struct reader *rds,unsigned long *head)
{
    if(mappnt + 1 > mapend)
        return FALSE;
    *head <<= 8;
    *head |= *mappnt++;
    *head &= 0xffffffff;
    return TRUE;
}

static int mapped_skip_bytes(struct reader *rds,int len)
{
    if(mappnt + len > mapend)
        return FALSE;
    mappnt += len;
    if (param.usebuffer)
        buffer_resync();
    return TRUE;
}

static int mapped_read_frame_body(struct reader *rds,unsigned char *buf,
                                  int size)
{
    if(size <= 0) {
        fprintf(stderr,"Ouch. Read_frame called with size <= 0\n");
        return FALSE;
    }
    if(mappnt + size > mapend)
        return FALSE;
    memcpy(buf,mappnt,size);
    mappnt += size;

    return TRUE;
}

static int mapped_back_bytes(struct reader *rds,int bytes)
{
    if( (mappnt - bytes) < mapbuf || (mappnt - bytes + 4) > mapend)
        return -1;
    mappnt -= bytes;
    if(param.usebuffer)
        buffer_resync();
    return 0;
}

static int mapped_back_frame(struct reader *rds,struct frame *fr,int num)
{
    long bytes;
    unsigned long newhead;


    if(!firsthead)
        return 0;

    bytes = (fr->framesize+8)*(num+2);

    /* Buffered mode is a bit trickier. From the size of the buffered
     * output audio stream we have to make a guess at the number of frames
     * this corresponds to.
     */
    if(param.usebuffer)
        bytes += (long)(xfermem_get_usedspace(buffermem) /
                        (buffermem->buf[0] * buffermem->buf[1]
                         * (buffermem->buf[2] & AUDIO_FORMAT_MASK ?
                            16.0 : 8.0 ))
                        * (tabsel_123[fr->lsf][fr->lay-1][fr->bitrate_index] << 10));
    /*
      bytes += (long)(compute_buffer_offset(fr)*compute_bpf(fr));
    */

    if( (mappnt - bytes) < mapbuf || (mappnt - bytes + 4) > mapend)
        return -1;
    mappnt -= bytes;

    newhead = (mappnt[0]<<24) + (mappnt[1]<<16) + (mappnt[2]<<8) + mappnt[3];
    mappnt += 4;

    while( (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK) ) {
        if(mappnt + 1 > mapend)
            return -1;
        newhead <<= 8;
        newhead |= *mappnt++;
        newhead &= 0xffffffff;
    }
    mappnt -= 4;

    read_frame(fr);
    read_frame(fr);

    if(fr->lay == 3)
        set_pointer(512);

    if(param.usebuffer)
        buffer_resync();

    return 0;
}

static long mapped_tell(struct reader *rds)
{
    return mappnt - mapbuf;
}

#endif

/*****************************************************************
 * read frame helper
 */

struct reader *rd;
struct reader readers[] = {
#ifdef READ_SYSTEM
    { system_init,
      NULL,     /* filled in by system_init() */
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL } ,
#endif
#ifdef READ_MMAP
    { mapped_init,
      mapped_close,
      mapped_head_read,
      mapped_head_shift,
      mapped_skip_bytes,
      mapped_read_frame_body,
      mapped_back_bytes,
      mapped_back_frame,
      mapped_tell,
      mapped_rewind } ,
#endif
    { default_init,
      stream_close,
      stream_head_read,
      stream_head_shift,
      stream_skip_bytes,
      stream_read_frame_body,
      stream_back_bytes,
      stream_back_frame,
      stream_tell,
      stream_rewind } ,
    { NULL, }
};


/* open the device to read the bit stream from it */

struct reader *open_stream(char *bs_filenam,int fd)
{
    int i;
    int filept_opened = 1;
    int filept;

    if (!bs_filenam) {
        if(fd < 0) {
            filept = 0;
            filept_opened = 0;
        }
        else
            filept = fd;
    }
    else if (!strncmp(bs_filenam, "http://", 7))
        filept = http_open(bs_filenam);
#ifndef O_BINARY
#define O_BINARY (0)
#endif
    else if ( (filept = open(bs_filenam, O_RDONLY|O_BINARY)) < 0) {
        perror (bs_filenam);
        return NULL;
    }

    rd = NULL;
    for(i=0;;i++) {
        readers[i].filelen = -1;
        readers[i].filept  = filept;
        readers[i].flags = 0;
        if(filept_opened)
            readers[i].flags |= READER_FD_OPENED;
        if(!readers[i].init) {
            fprintf(stderr,"Fatal error!\n");
            exit(1);
        }
        if(readers[i].init(readers+i) >= 0) {
            rd = &readers[i];
            break;
        }
    }

    if(rd && rd->flags & READER_ID3TAG) {
        print_id3_tag(rd->id3buf);
    }

    return rd;
}












