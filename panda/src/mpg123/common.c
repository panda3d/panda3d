/* GPL clean */

#include <ctype.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <sys/time.h>
#endif /* WIN32 */

#ifdef WIN32
#include <io.h>
#endif

#include <fcntl.h>

#ifdef READ_MMAP
#include <sys/mman.h>
#ifndef MAP_FAILED
#define MAP_FAILED ( (void *) -1 )
#endif
#endif

#include "mpg123.h"
#include "genre.h"
#include "common.h"

int tabsel_123[2][3][16] = {
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} },

   { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
     {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} }
};

long freqs[9] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 , 12000 , 8000 };

struct bitstream_info bsi;

static int fsizeold=0,ssize;
static unsigned char bsspace[2][MAXFRAMESIZE+512]; /* MAXFRAMESIZE */
static unsigned char *bsbuf=bsspace[1],*bsbufold;
static int bsnum=0;

static unsigned long oldhead = 0;
unsigned long firsthead=0;

unsigned char *pcm_sample;
int pcm_point = 0;
int audiobufsize = AUDIOBUFSIZE;

static int decode_header(struct frame *fr,unsigned long newhead);

void audio_flush(int outmode, struct audio_info_struct *ai)
{
    if (pcm_point) {
        switch (outmode) {
        case DECODE_FILE:
            write (OutputDescriptor, pcm_sample, pcm_point);
            break;
        case DECODE_AUDIO:
            audio_play_samples (ai, pcm_sample, pcm_point);
            break;
        case DECODE_BUFFER:
            write (buffer_fd[1], pcm_sample, pcm_point);
            break;
        case DECODE_WAV:
        case DECODE_CDR:
        case DECODE_AU:
            wav_write(pcm_sample, pcm_point);
            break;
        }
        pcm_point = 0;
    }
}

#if !defined(WIN32) && !defined(GENERIC)
void (*catchsignal(int signum, void(*handler)()))()
{
    struct sigaction new_sa;
    struct sigaction old_sa;

#ifdef DONT_CATCH_SIGNALS
    printf ("Not catching any signals.\n");
    return ((void (*)()) -1);
#endif

    new_sa.sa_handler = handler;
    sigemptyset(&new_sa.sa_mask);
    new_sa.sa_flags = 0;
    if (sigaction(signum, &new_sa, &old_sa) == -1)
        return ((void (*)()) -1);
    return (old_sa.sa_handler);
}
#endif

void read_frame_init (void)
{
    oldhead = 0;
    firsthead = 0;
}

int head_check(unsigned long head)
{
/* fprintf(stderr,"HC"); */
    if( (head & 0xffe00000) != 0xffe00000)
        return FALSE;
    if(!((head>>17)&3))
        return FALSE;
    if( ((head>>12)&0xf) == 0xf)
        return FALSE;
    if( ((head>>10)&0x3) == 0x3 )
        return FALSE;
#if 0
    /* this would match on MPEG1/Layer1 streams with CRC = off */
    if ((head & 0xffff0000) == 0xffff0000)
        return FALSE;
#endif

    return TRUE;
}



/*****************************************************************
 * read next frame
 */
int read_frame(struct frame *fr)
{
    unsigned long newhead;
    static unsigned char ssave[34];

    fsizeold=fr->framesize;       /* for Layer3 */

    if (param.halfspeed) {
        static int halfphase = 0;
        if (halfphase--) {
            bsi.bitindex = 0;
            bsi.wordpointer = (unsigned char *) bsbuf;
            if (fr->lay == 3)
                memcpy (bsbuf, ssave, ssize);
            return 1;
        }
        else
            halfphase = param.halfspeed - 1;
    }

 read_again:
    if(!rd->head_read(rd,&newhead))
        return FALSE;

    if(1 || oldhead != newhead || !oldhead) {

    init_resync:

        fr->header_change = 2;
        if(oldhead) {
            if((oldhead & 0xc00) == (newhead & 0xc00)) {
                if( (oldhead & 0xc0) == 0 && (newhead & 0xc0) == 0)
                    fr->header_change = 1; 
                else if( (oldhead & 0xc0) > 0 && (newhead & 0xc0) > 0)
                    fr->header_change = 1;
            }
        }


#ifdef SKIP_JUNK
        if(!firsthead && !head_check(newhead) ) {
            int i;

            fprintf(stderr,"Junk at the beginning %08lx\n",newhead);

            /* I even saw RIFF headers at the beginning of MPEG streams ;( */
            if(newhead == ('R'<<24)+('I'<<16)+('F'<<8)+'F') {
                if(!rd->head_read(rd,&newhead))
                    return 0;
                while(newhead != ('d'<<24)+('a'<<16)+('t'<<8)+'a') {
                    if(!rd->head_shift(rd,&newhead))
                        return 0;
                }
                if(!rd->head_read(rd,&newhead))
                    return 0;
                /* fprintf(stderr,"Skipped RIFF header!\n"); */
                goto read_again;
            }
            {
                /* step in byte steps through next 64K */
                for(i=0;i<65536;i++) {
                    if(!rd->head_shift(rd,&newhead))
                        return 0;
                    if(head_check(newhead))
                        break;
                }
                if(i == 65536) {
                    fprintf(stderr,"Giving up searching valid MPEG header\n");
                    return 0;
                }
            }
            /* 
             * should we additionaly check, whether a new frame starts at
             * the next expected position? (some kind of read ahead)
             * We could implement this easily, at least for files.
             */
        }
#endif

        if( (newhead & 0xffe00000) != 0xffe00000) {
            if (!param.quiet)
                fprintf(stderr,"Illegal Audio-MPEG-Header 0x%08lx at offset 0x%lx.\n",
                        newhead,rd->tell(rd)-4);
            /* and those ugly ID3 tags */
            if((newhead & 0xffffff00) == ('T'<<24)+('A'<<16)+('G'<<8)) {
                rd->skip_bytes(rd,124);
                fprintf(stderr,"Skipped ID3 Tag!\n");
                goto read_again;
            }
            if (param.tryresync) {
                int try = 0;
                /* Read more bytes until we find something that looks
                   reasonably like a valid header.  This is not a
                   perfect strategy, but it should get us back on the
                   track within a short time (and hopefully without
                   too much distortion in the audio output).  */
                do {
                    try++;
                    if(!rd->head_shift(rd,&newhead))
                        return 0;
                    if (!oldhead)
                        goto init_resync;       /* "considered harmful", eh? */

                } while ((newhead & HDRCMPMASK) != (oldhead & HDRCMPMASK)
                         && (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK));
                if (!param.quiet)
                    fprintf (stderr, "Skipped %d bytes in input.\n", try);
            }
            else
                return (0);
        }

/* fprintf(stderr,"+"); */

        if (!firsthead) {
            if(!decode_header(fr,newhead)) {
/* fprintf(stderr,"A"); */
                goto read_again;
            }
            firsthead = newhead;

        }
        else if(!decode_header(fr,newhead)) {
/* fprintf(stderr,"B: %08lx\n",newhead); */
            return 0;
        }

/* fprintf(stderr,"-"); */
    }
    else
        fr->header_change = 0;

/* fprintf(stderr,"FS: %d\n",fr->framesize); */

    /* flip/init buffer for Layer 3 */
    bsbufold = bsbuf;
    bsbuf = bsspace[bsnum]+512;
    bsnum = (bsnum + 1) & 1;

    /* read main data into memory */
    if(!rd->read_frame_body(rd,bsbuf,fr->framesize))
        return 0;

    { 
      /* Test */
      static struct vbrHeader head;
      static int vbr = 0;
      if(!vbr) {
        getVBRHeader(&head,bsbuf,fr);
        vbr = 1;
      }
    }

/* fprintf(stderr,"Got it\n"); */

    bsi.bitindex = 0;
    bsi.wordpointer = (unsigned char *) bsbuf;

    if (param.halfspeed && fr->lay == 3)
        memcpy (ssave, bsbuf, ssize);

    return 1;
}

/****************************************
 * HACK,HACK,HACK: step back <num> frames
 * can only work if the 'stream' isn't a real stream but a file
 */
int back_frame(struct reader *rds,struct frame *fr,int num)
{
    long bytes;
    unsigned long newhead;
  
    if(!firsthead)
        return 0;
  
    bytes = (fr->framesize+8)*(num+2);
  
    if(rds->back_bytes(rds,bytes) < 0)
        return -1;
    if(!rds->head_read(rds,&newhead))
        return -1;
  
    while( (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK) ) {
        if(!rds->head_shift(rds,&newhead))
            return -1;
    }
  
    if(rds->back_bytes(rds,4) <0)
        return -1;

    read_frame(fr);
    read_frame(fr);
  
    if(fr->lay == 3) {
        set_pointer(512);
    }
  
    return 0;
}


/*
 * decode a header and write the information
 * into the frame structure
 */
static int decode_header(struct frame *fr,unsigned long newhead)
{
    if(!head_check(newhead)) {
        fprintf(stderr,"Oopps header is wrong\n");
        return 0;
    }

    if( newhead & (1<<20) ) {
        fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
        fr->mpeg25 = 0;
    }
    else {
        fr->lsf = 1;
        fr->mpeg25 = 1;
    }
    
    if (!param.tryresync || !oldhead) {
        /* If "tryresync" is true, assume that certain
           parameters do not change within the stream! */
        fr->lay = 4-((newhead>>17)&3);
        if( ((newhead>>10)&0x3) == 0x3) {
            fprintf(stderr,"Stream error\n");
            exit(1);
        }
        if(fr->mpeg25) {
            fr->sampling_frequency = 6 + ((newhead>>10)&0x3);
        }
        else
            fr->sampling_frequency = ((newhead>>10)&0x3) + (fr->lsf*3);
        fr->error_protection = ((newhead>>16)&0x1)^0x1;
    }

    fr->bitrate_index = ((newhead>>12)&0xf);
    fr->padding   = ((newhead>>9)&0x1);
    fr->extension = ((newhead>>8)&0x1);
    fr->mode      = ((newhead>>6)&0x3);
    fr->mode_ext  = ((newhead>>4)&0x3);
    fr->copyright = ((newhead>>3)&0x1);
    fr->original  = ((newhead>>2)&0x1);
    fr->emphasis  = newhead & 0x3;

    fr->stereo    = (fr->mode == MPG_MD_MONO) ? 1 : 2;

    oldhead = newhead;

    if(!fr->bitrate_index) {
        fprintf(stderr,"Free format not supported: (head %08lx)\n",newhead);
        return (0);
    }

    switch(fr->lay) {
    case 1:
        fr->framesize  = (long) tabsel_123[fr->lsf][0][fr->bitrate_index] * 12000;
        fr->framesize /= freqs[fr->sampling_frequency];
        fr->framesize  = ((fr->framesize+fr->padding)<<2)-4;
        break;
    case 2:
        fr->framesize = (long) tabsel_123[fr->lsf][1][fr->bitrate_index] * 144000;
        fr->framesize /= freqs[fr->sampling_frequency];
        fr->framesize += fr->padding - 4;
        break;
    case 3:
        if(fr->lsf)
            ssize = (fr->stereo == 1) ? 9 : 17;
        else
            ssize = (fr->stereo == 1) ? 17 : 32;
        if(fr->error_protection)
            ssize += 2;
        fr->framesize  = (long) tabsel_123[fr->lsf][2][fr->bitrate_index] * 144000;
        fr->framesize /= freqs[fr->sampling_frequency]<<(fr->lsf);
        fr->framesize = fr->framesize + fr->padding - 4;
        break; 
    default:
        fprintf(stderr,"Sorry, unknown layer type.\n"); 
        return (0);
    }
    return 1;
}

#ifdef MPG123_REMOTE
void print_rheader(struct frame *fr)
{
    static char *modes[4] = { "Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel" };
    static char *layers[4] = { "Unknown" , "I", "II", "III" };
    static char *mpeg_type[2] = { "1.0" , "2.0" };

    /* version, layer, freq, mode, channels, bitrate, BPF */
    fprintf(stderr,"@I %s %s %ld %s %d %d %d\n",
            mpeg_type[fr->lsf],layers[fr->lay],freqs[fr->sampling_frequency],
            modes[fr->mode],fr->stereo,
            tabsel_123[fr->lsf][fr->lay-1][fr->bitrate_index],
            fr->framesize+4);
}
#endif

void print_header(struct frame *fr)
{
    static char *modes[4] = { "Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel" };
    static char *layers[4] = { "Unknown" , "I", "II", "III" };

    fprintf(stderr,"MPEG %s, Layer: %s, Freq: %ld, mode: %s, modext: %d, BPF : %d\n", 
            fr->mpeg25 ? "2.5" : (fr->lsf ? "2.0" : "1.0"),
            layers[fr->lay],freqs[fr->sampling_frequency],
            modes[fr->mode],fr->mode_ext,fr->framesize+4);
    fprintf(stderr,"Channels: %d, copyright: %s, original: %s, CRC: %s, emphasis: %d.\n",
            fr->stereo,fr->copyright?"Yes":"No",
            fr->original?"Yes":"No",fr->error_protection?"Yes":"No",
            fr->emphasis);
    fprintf(stderr,"Bitrate: %d Kbits/s, Extension value: %d\n",
            tabsel_123[fr->lsf][fr->lay-1][fr->bitrate_index],fr->extension);
}

void print_header_compact(struct frame *fr)
{
    static char *modes[4] = { "stereo", "joint-stereo", "dual-channel", "mono" };
    static char *layers[4] = { "Unknown" , "I", "II", "III" };
 
    fprintf(stderr,"MPEG %s layer %s, %d kbit/s, %ld Hz %s\n",
            fr->mpeg25 ? "2.5" : (fr->lsf ? "2.0" : "1.0"),
            layers[fr->lay],
            tabsel_123[fr->lsf][fr->lay-1][fr->bitrate_index],
            freqs[fr->sampling_frequency], modes[fr->mode]);
}

void print_id3_tag(unsigned char *buf)
{
    struct id3tag {
        char tag[3];
        char title[30];
        char artist[30];
        char album[30];
        char year[4];
        char comment[30];
        unsigned char genre;
    };
    struct id3tag *tag = (struct id3tag *) buf;
    char title[31]={0,};
    char artist[31]={0,};
    char album[31]={0,};
    char year[5]={0,};
    char comment[31]={0,};
    char genre[31]={0,};

    if(param.quiet)
        return;

    strncpy(title,tag->title,30);
    strncpy(artist,tag->artist,30);
    strncpy(album,tag->album,30);
    strncpy(year,tag->year,4);
    strncpy(comment,tag->comment,30);

    if ( tag->genre < sizeof(genre_table)/sizeof(*genre_table) ) {
        strncpy(genre, genre_table[tag->genre], 30);
    } else {
        strncpy(genre,"Unknown",30);
    }

    fprintf(stderr,"Title  : %-30s  Artist: %s\n",title,artist);
    fprintf(stderr,"Album  : %-30s  Year  : %4s\n",album,year);
    fprintf(stderr,"Comment: %-30s  Genre : %s\n",comment,genre);
}

#if 0
/* removed the strndup for better portability */
/*
 *   Allocate space for a new string containing the first
 *   "num" characters of "src".  The resulting string is
 *   always zero-terminated.  Returns NULL if malloc fails.
 */
char *strndup (const char *src, int num)
{
    char *dst;

    if (!(dst = (char *) malloc(num+1)))
        return (NULL);
    dst[num] = '\0';
    return (strncpy(dst, src, num));
}
#endif

/*
 *   Split "path" into directory and filename components.
 *
 *   Return value is 0 if no directory was specified (i.e.
 *   "path" does not contain a '/'), OR if the directory
 *   is the same as on the previous call to this function.
 *
 *   Return value is 1 if a directory was specified AND it
 *   is different from the previous one (if any).
 */

int split_dir_file (const char *path, char **dname, char **fname)
{
    static char *lastdir = NULL;
    char *slashpos;

    if ((slashpos = strrchr(path, '/'))) {
        *fname = slashpos + 1;
        *dname = strdup(path); /* , 1 + slashpos - path); */
        if(!(*dname)) {
            perror("memory");
            exit(1);
        }
        (*dname)[1 + slashpos - path] = 0;
        if (lastdir && !strcmp(lastdir, *dname)) {
            /***   same as previous directory   ***/
            free (*dname);
            *dname = lastdir;
            return 0;
        }
        else {
            /***   different directory   ***/
            if (lastdir)
                free (lastdir);
            lastdir = *dname;
            return 1;
        }
    }
    else {
        /***   no directory specified   ***/
        if (lastdir) {
            free (lastdir);
            lastdir = NULL;
        };
        *dname = NULL;
        *fname = (char *)path;
        return 0;
    }
}

void set_pointer(long backstep)
{
    bsi.wordpointer = bsbuf + ssize - backstep;
    if (backstep)
        memcpy(bsi.wordpointer,bsbufold+fsizeold-backstep,backstep);
    bsi.bitindex = 0; 
}

/********************************/

double compute_bpf(struct frame *fr)
{
    double bpf;

    switch(fr->lay) {
    case 1:
        bpf = tabsel_123[fr->lsf][0][fr->bitrate_index];
        bpf *= 12000.0 * 4.0;
        bpf /= freqs[fr->sampling_frequency] <<(fr->lsf);
        break;
    case 2:
    case 3:
        bpf = tabsel_123[fr->lsf][fr->lay-1][fr->bitrate_index];
        bpf *= 144000;
        bpf /= freqs[fr->sampling_frequency] << (fr->lsf);
        break;
    default:
        bpf = 1.0;
    }

    return bpf;
}

double compute_tpf(struct frame *fr)
{
    static int bs[4] = { 0,384,1152,1152 };
    double tpf;

    tpf = (double) bs[fr->lay];
    tpf /= freqs[fr->sampling_frequency] << (fr->lsf);
    return tpf;
}

/*
 * Returns number of frames queued up in output buffer, i.e. 
 * offset between currently played and currently decoded frame.
 */

long compute_buffer_offset(struct frame *fr)
{
    long bufsize;

    /*
     * buffermem->buf[0] holds output sampling rate,
     * buffermem->buf[1] holds number of channels,
     * buffermem->buf[2] holds audio format of output.
     */

    if(!param.usebuffer || !(bufsize=xfermem_get_usedspace(buffermem))
       || !buffermem->buf[0] || !buffermem->buf[1])
        return 0;

    bufsize = (long)((double) bufsize / buffermem->buf[0] / 
                     buffermem->buf[1] / compute_tpf(fr));

    if((buffermem->buf[2] & AUDIO_FORMAT_MASK) == AUDIO_FORMAT_16)
        return bufsize/2;
    else
        return bufsize;
}

void print_stat(struct frame *fr,int no,long buffsize,struct audio_info_struct *ai)
{
    double bpf,tpf,tim1,tim2;
    double dt = 0.0;
    int sno,rno;
    char outbuf[256];

    if(!rd || !fr) 
        return;

    outbuf[0] = 0;

#ifndef GENERIC
    {
        struct timeval t;
        fd_set serr;
        int n,errfd = fileno(stderr);

        t.tv_sec=t.tv_usec=0;

        FD_ZERO(&serr);
        FD_SET(errfd,&serr);
        n = select(errfd+1,NULL,&serr,NULL,&t);
        if(n <= 0)
            return;
    }
#endif

    bpf = compute_bpf(fr);
    tpf = compute_tpf(fr);

    if(buffsize > 0 && ai && ai->rate > 0 && ai->channels > 0) {
        dt = (double) buffsize / ai->rate / ai->channels;
        if( (ai->format & AUDIO_FORMAT_MASK) == AUDIO_FORMAT_16)
            dt *= 0.5;
    }

    rno = 0;
    sno = no;
    if(rd->filelen >= 0) {
        long t = rd->tell(rd);
        rno = (int)((double)(rd->filelen-t)/bpf);
        sno = (int)((double)t/bpf);
    }

    sprintf(outbuf+strlen(outbuf),"\rFrame# %5d [%5d], ",sno,rno);

    tim1 = sno*tpf-dt;
    tim2 = rno*tpf+dt;
#if 0
    tim1 = tim1 < 0 ? 0.0 : tim1;
#endif
    tim2 = tim2 < 0 ? 0.0 : tim2;

    sprintf(outbuf+strlen(outbuf),"Time: %02u:%02u.%02u [%02u:%02u.%02u], ",
            (unsigned int)tim1/60,
            (unsigned int)tim1%60,
            (unsigned int)(tim1*100)%100,
            (unsigned int)tim2/60,
            (unsigned int)tim2%60,
            (unsigned int)(tim2*100)%100);

    if(param.usebuffer)
        sprintf(outbuf+strlen(outbuf),"[%8ld] ",(long)buffsize);
    write(fileno(stderr),outbuf,strlen(outbuf));
#if 0
    fflush(out); /* hmm not really nec. */
#endif
}

int get_songlen(struct frame *fr,int no)
{
    double tpf;

    if(!fr)
        return 0;

    if(no < 0) {
        if(!rd || rd->filelen < 0)
            return 0;
        no = (double) rd->filelen / compute_bpf(fr);
    }

    tpf = compute_tpf(fr);
    return no*tpf;
}


