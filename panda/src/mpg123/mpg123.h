/*
 * mpg123 defines 
 * used source: musicout.h from mpegaudio package
 */

#include        <stdio.h>
#include        <string.h>
#include        <signal.h>

#ifndef WIN32
#include        <sys/signal.h>
#include        <unistd.h>
#endif

#include        <math.h>

typedef unsigned char byte;

#ifdef OS2
#include <float.h>
#endif

#define MPG123_REMOTE
#ifdef HPUX
#define random rand
#define srandom srand
#endif

#define FRONTEND_NONE     0
#define FRONTEND_SAJBER   1
#define FRONTEND_TK3PLAY  2
#define FRONTEND_GENERIC  3

#define SKIP_JUNK 1

#ifdef _WIN32	/* Win32 Additions By Tony Million */
# undef WIN32
# define WIN32

# define M_PI       3.14159265358979323846
# define M_SQRT2	1.41421356237309504880
# define REAL_IS_FLOAT
# define NEW_DCT9

# define random rand
# define srandom srand

# undef MPG123_REMOTE           /* Get rid of this stuff for Win32 */
#endif

#include "xfermem.h"

#ifdef SUNOS
#define memmove(dst,src,size) bcopy(src,dst,size)
#endif

#ifdef REAL_IS_FLOAT
#  define real float
#elif defined(REAL_IS_LONG_DOUBLE)
#  define real long double
#else
#  define real double
#endif

#ifdef __GNUC__
#define INLINE inline
#else
#define INLINE
#endif

#include "audio.h"

/* AUDIOBUFSIZE = n*64 with n=1,2,3 ...  */
#define		AUDIOBUFSIZE		16384

#define         FALSE                   0
#define         TRUE                    1

#define         MAX_NAME_SIZE           81
#define         SBLIMIT                 32
#define         SCALE_BLOCK             12
#define         SSLIMIT                 18

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

#define MAXFRAMESIZE 1792
#define HDRCMPMASK 0xfffffd00

#define MAXOUTBURST 32768

/* Pre Shift fo 16 to 8 bit converter table */
#define AUSHIFT (3)


struct al_table 
{
  short bits;
  short d;
};

struct frame {
    struct al_table *alloc;
    int (*synth)(real *,int,unsigned char *,int *);
    int (*synth_mono)(real *,unsigned char *,int *);
#ifdef USE_3DNOW
    void (*dct36)(real *,real *,real *,real *,real *);
#endif
    int stereo;
    int jsbound;
    int single;
    int II_sblimit;
    int down_sample_sblimit;
    int lsf;
    int mpeg25;
    int down_sample;
    int header_change;
    int lay;
    int error_protection;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
    int framesize; /* computed framesize */
};

#define VBR_TOC_SIZE        100

#define VBR_FRAMES_FLAG     0x0001
#define VBR_BYTES_FLAG      0x0002
#define VBR_TOC_FLAG        0x0004
#define VBR_SCALE_FLAG      0x0008

struct vbrHeader {
	unsigned long flags;
	unsigned long frames;
	unsigned long bytes;
	unsigned long scale;
	unsigned char toc[100];
};

struct parameter {
    int aggressive; /* renice to max. priority */
    int shuffle;	/* shuffle/random play */
    int remote;	/* remote operation */
    int outmode;	/* where to out the decoded sampels */
    int quiet;	/* shut up! */
    int xterm_title;  /* print filename in xterm title */
    long usebuffer;	/* second level buffer size */
    int tryresync;  /* resync stream after error */
    int verbose;    /* verbose level */
#ifdef TERM_CONTROL
    int term_ctrl;
#endif
    int force_mono;
    int force_stereo;
    int force_8bit;
    long force_rate;
    int down_sample;
    int checkrange;
    long doublespeed;
    long halfspeed;
    int force_reopen;
    int stat_3dnow; /* automatic/force/force-off 3DNow! optimized code */
    int test_3dnow;
    long realtime;
    char filename[256];
    char *esdserver;
    char *equalfile;
    int  enable_equalizer;
    long outscale;
    long startFrame;
};

struct bitstream_info {
  int bitindex;
  unsigned char *wordpointer;
};

struct mpstr {
  int bsize;
  int framesize;
  int fsizeold;
  struct frame fr;
 /* int (*do_layer)(struct mpstr *,struct frame *fr,int,struct audio_info_struct *); */
  unsigned char bsspace[2][MAXFRAMESIZE+512]; /* MAXFRAMESIZE */
  real hybrid_block[2][2][SBLIMIT*SSLIMIT];
  int  hybrid_blc[2];
  unsigned long header;
  int bsnum;
  real synth_buffs[2][2][0x110];
  int  synth_bo;

  struct bitstream_info bsi;
};

extern struct bitstream_info bsi;

struct reader {
  int  (*init)(struct reader *);
  void (*close)(struct reader *);
  int  (*head_read)(struct reader *,unsigned long *newhead);
  int  (*head_shift)(struct reader *,unsigned long *head);
  int  (*skip_bytes)(struct reader *,int len);
  int  (*read_frame_body)(struct reader *,unsigned char *,int size);
  int  (*back_bytes)(struct reader *,int bytes);
  int  (*back_frame)(struct reader *,struct frame *,int num);
  long (*tell)(struct reader *);
  void (*rewind)(struct reader *);
  long filelen;
  long filepos;
  int  filept;
  int  flags;
  unsigned char id3buf[128];
};
#define READER_FD_OPENED 0x1
#define READER_ID3TAG    0x2

extern struct reader *rd,readers[];

extern int halfspeed;
extern int buffer_fd[2];
extern txfermem *buffermem;
extern char *prgName, *prgVersion;

#ifndef NOXFERMEM
extern void buffer_loop(struct audio_info_struct *ai,sigset_t *oldsigset);
#endif


/* ------ Declarations from "httpget.c" ------ */

extern char *proxyurl;
extern unsigned long proxyip;
extern int http_open (char *url);
extern char *httpauth;

/* ------ Declarations from "common.c" ------ */

extern void audio_flush(int, struct audio_info_struct *);
extern void (*catchsignal(int signum, void(*handler)()))();

extern void print_header(struct frame *);
extern void print_header_compact(struct frame *);
extern void print_id3_tag(unsigned char *buf);

extern int split_dir_file(const char *path, char **dname, char **fname);

extern unsigned int   get1bit(struct bitstream_info *);
extern unsigned int   getbits(struct bitstream_info *,int);
extern unsigned int   getbits_fast(struct bitstream_info *,int);
extern void           backbits(struct bitstream_info *,int);
extern int            getbitoffset(struct bitstream_info *);
extern int            getbyte(struct bitstream_info *);

extern void set_pointer(long);

extern unsigned char *pcm_sample;
extern int pcm_point;
extern int audiobufsize;

extern int OutputDescriptor;

struct gr_info_s {
      int scfsi;
      unsigned part2_3_length;
      unsigned big_values;
      unsigned scalefac_compress;
      unsigned block_type;
      unsigned mixed_block_flag;
      unsigned table_select[3];
      unsigned subblock_gain[3];
      unsigned maxband[3];
      unsigned maxbandl;
      unsigned maxb;
      unsigned region1start;
      unsigned region2start;
      unsigned preflag;
      unsigned scalefac_scale;
      unsigned count1table_select;
      real *full_gain[3];
      real *pow2gain;
};

struct III_sideinfo
{
  unsigned main_data_begin;
  unsigned private_bits;
  struct {
    struct gr_info_s gr[2];
  } ch[2];
};

extern struct reader *open_stream(char *,int fd);
extern void read_frame_init (void);
extern int read_frame(struct frame *fr);
extern int play_frame(struct mpstr *mp,int init,struct frame *fr);
extern int do_layer3(struct mpstr *mp,struct frame *fr,int,struct audio_info_struct *);
extern int do_layer2(struct mpstr *mp,struct frame *fr,int,struct audio_info_struct *);
extern int do_layer1(struct mpstr *mp,struct frame *fr,int,struct audio_info_struct *);
extern void do_equalizer(real *bandPtr,int channel);

#ifdef PENTIUM_OPT
extern int synth_1to1_pent (real *,int,unsigned char *);
#endif
extern int synth_1to1 (real *,int,unsigned char *,int *);
extern int synth_1to1_8bit (real *,int,unsigned char *,int *);
extern int synth_1to1_mono (real *,unsigned char *,int *);
extern int synth_1to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_1to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_1to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_2to1 (real *,int,unsigned char *,int *);
extern int synth_2to1_8bit (real *,int,unsigned char *,int *);
extern int synth_2to1_mono (real *,unsigned char *,int *);
extern int synth_2to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_4to1 (real *,int,unsigned char *,int *);
extern int synth_4to1_8bit (real *,int,unsigned char *,int *);
extern int synth_4to1_mono (real *,unsigned char *,int *);
extern int synth_4to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_ntom (real *,int,unsigned char *,int *);
extern int synth_ntom_8bit (real *,int,unsigned char *,int *);
extern int synth_ntom_mono (real *,unsigned char *,int *);
extern int synth_ntom_mono2stereo (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono2stereo (real *,unsigned char *,int *);

extern void rewindNbits(int bits);
extern int  hsstell(void);
extern void set_pointer(long);
extern void huffman_decoder(int ,int *);
extern void huffman_count1(int,int *);
extern void print_stat(struct frame *fr,int no,long buffsize,struct audio_info_struct *ai);
extern int get_songlen(struct frame *fr,int no);

extern void init_layer3(int);
extern void init_layer2(void);
extern void make_decode_tables(long scale);
extern void make_conv16to8_table(int);
extern void dct64(real *,real *,real *);

#ifdef USE_MMX
extern void dct64_MMX(short *a,short *b,real *c);
extern int synth_1to1_MMX(real *, int, short *, short *, int *);
#endif

extern void synth_ntom_set_step(long,long);

extern void control_generic(struct mpstr *,struct frame *fr);
extern void control_sajber(struct mpstr *,struct frame *fr);
extern void control_tk3play(struct mpstr *,struct frame *fr);

extern int cdr_open(struct audio_info_struct *ai, char *ame);
extern int au_open(struct audio_info_struct *ai, char *name);
extern int wav_open(struct audio_info_struct *ai, char *wavfilename);
extern int wav_write(unsigned char *buf,int len);
extern int cdr_close(void);
extern int au_close(void);
extern int wav_close(void);

extern int au_open(struct audio_info_struct *ai, char *aufilename);
extern int au_close(void);

extern int cdr_open(struct audio_info_struct *ai, char *cdrfilename);
extern int cdr_close(void);

extern int getVBRHeader(struct vbrHeader *head,unsigned char *buf, 
	struct frame *fr);


extern unsigned char *conv16to8;
extern long freqs[9];
extern real muls[27][64];
extern real decwin[512+32];
#ifndef USE_MMX
extern real *pnts[5];
#endif

extern real equalizer[2][32];
extern real equalizer_sum[2][32];
extern int equalizer_cnt;

extern struct audio_name audio_val2name[];

extern struct parameter param;

/* 486 optimizations */
#define FIR_BUFFER_SIZE  128
extern void dct64_486(int *a,int *b,real *c);
extern int synth_1to1_486(real *bandPtr,int channel,unsigned char *out,int nb_blocks);

/* 3DNow! optimizations */
#ifdef USE_3DNOW
extern int getcpuflags(void);
extern void dct36(real *,real *,real *,real *,real *);
extern void dct36_3dnow(real *,real *,real *,real *,real *);
extern int synth_1to1_3dnow(real *,int,unsigned char *,int *);
#endif
