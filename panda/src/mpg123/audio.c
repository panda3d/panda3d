
#include "mpg123.h"

void audio_info_struct_init(struct audio_info_struct *ai)
{
#ifdef AUDIO_USES_FD
  ai->fn = -1;
#endif
#ifdef SGI
#if 0
  ALconfig config;
  ALport port;
#endif
#endif
  ai->rate = -1;
  ai->gain = -1;
  ai->output = -1;
#ifdef ALSA
  ai->handle = NULL;
  ai->alsa_format.format = -1;
  ai->alsa_format.rate = -1;
  ai->alsa_format.channels = -1;
#endif
  ai->device = NULL;
  ai->channels = -1;
  ai->format = -1;
}

#define NUM_CHANNELS 2
#define NUM_ENCODINGS 6
#define NUM_RATES 10

struct audio_name audio_val2name[NUM_ENCODINGS+1] = {
 { AUDIO_FORMAT_SIGNED_16  , "signed 16 bit" , "s16 " } ,
 { AUDIO_FORMAT_UNSIGNED_16, "unsigned 16 bit" , "u16 " } ,  
 { AUDIO_FORMAT_UNSIGNED_8 , "unsigned 8 bit" , "u8  " } ,
 { AUDIO_FORMAT_SIGNED_8   , "signed 8 bit" , "s8  " } ,
 { AUDIO_FORMAT_ULAW_8     , "mu-law (8 bit)" , "ulaw " } ,
 { AUDIO_FORMAT_ALAW_8     , "a-law (8 bit)" , "alaw " } ,
 { -1 , NULL }
};

#if 0
static char *channel_name[NUM_CHANNELS] = 
 { "mono" , "stereo" };
#endif

static int channels[NUM_CHANNELS] = { 1 , 2 };
static int rates[NUM_RATES] = { 
	 8000, 11025, 12000, 
	16000, 22050, 24000,
	32000, 44100, 48000,
	8000	/* 8000 = dummy for user forced */

};
static int encodings[NUM_ENCODINGS] = {
 AUDIO_FORMAT_SIGNED_16, 
 AUDIO_FORMAT_UNSIGNED_16,
 AUDIO_FORMAT_UNSIGNED_8,
 AUDIO_FORMAT_SIGNED_8,
 AUDIO_FORMAT_ULAW_8,
 AUDIO_FORMAT_ALAW_8
};

static char capabilities[NUM_CHANNELS][NUM_ENCODINGS][NUM_RATES];

void audio_capabilities(struct audio_info_struct *ai)
{
	int fmts;
	int i,j,k,k1=NUM_RATES-1;
	struct audio_info_struct ai1 = *ai;

        if (param.outmode != DECODE_AUDIO) {
		memset(capabilities,1,sizeof(capabilities));
		return;
	}

	memset(capabilities,0,sizeof(capabilities));
	if(param.force_rate) {
		rates[NUM_RATES-1] = param.force_rate;
		k1 = NUM_RATES;
	}

	if(audio_open(&ai1) < 0) {
		perror("audio");
		exit(1);
	}

	for(i=0;i<NUM_CHANNELS;i++) {
		for(j=0;j<NUM_RATES;j++) {
			ai1.channels = channels[i];
			ai1.rate = rates[j];
			fmts = audio_get_formats(&ai1);
			if(fmts < 0)
				continue;
			for(k=0;k<NUM_ENCODINGS;k++) {
				if((fmts & encodings[k]) == encodings[k])
					capabilities[i][k][j] = 1;
			}
		}
	}

	audio_close(&ai1);

	if(param.verbose > 1) {
		fprintf(stderr,"\nAudio capabilities:\n        |");
		for(j=0;j<NUM_ENCODINGS;j++) {
			fprintf(stderr," %5s |",audio_val2name[j].sname);
		}
		fprintf(stderr,"\n --------------------------------------------------------\n");
		for(k=0;k<k1;k++) {
			fprintf(stderr," %5d  |",rates[k]);
			for(j=0;j<NUM_ENCODINGS;j++) {
				if(capabilities[0][j][k]) {
					if(capabilities[1][j][k])
						fprintf(stderr,"  M/S  |");
					else
						fprintf(stderr,"   M   |");
				}
				else if(capabilities[1][j][k])
					fprintf(stderr,"   S   |");
				else
					fprintf(stderr,"       |");
			}
			fprintf(stderr,"\n");
		}
		fprintf(stderr,"\n");
	}
}

static int rate2num(int r)
{
	int i;
	for(i=0;i<NUM_RATES;i++) 
		if(rates[i] == r)
			return i;
	return -1;
}


static int audio_fit_cap_helper(struct audio_info_struct *ai,int rn,int f0,int f2,int c)
{
	int i;

        if(rn >= 0) {
                for(i=f0;i<f2;i++) {
                        if(capabilities[c][i][rn]) {
                                ai->rate = rates[rn];
                                ai->format = encodings[i];
                                ai->channels = channels[c];
				return 1;
                        }
                }
        }
	return 0;

}

/*
 * c=num of channels of stream
 * r=rate of stream
 */
void audio_fit_capabilities(struct audio_info_struct *ai,int c,int r)
{
	int rn;
	int f0=0;
	
	if(param.force_8bit) {
		f0 = 2;
	}

	c--; /* stereo=1 ,mono=0 */

	if(param.force_mono >= 0)
		c = 0;
	if(param.force_stereo)
		c = 1;

	if(param.force_rate) {
		rn = rate2num(param.force_rate);
		if(audio_fit_cap_helper(ai,rn,f0,2,c))
			return;
		if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
			return;

		if(c == 1 && !param.force_stereo)
			c = 0;
		else if(c == 0 && !param.force_mono)
			c = 1;

		if(audio_fit_cap_helper(ai,rn,f0,2,c))
			return;
		if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
			return;

		fprintf(stderr,"No supported rate found!\n");
		exit(1);
	}

	rn = rate2num(r>>0);
	if(audio_fit_cap_helper(ai,rn,f0,2,c))
		return;
	rn = rate2num(r>>1);
	if(audio_fit_cap_helper(ai,rn,f0,2,c))
		return;
	rn = rate2num(r>>2);
	if(audio_fit_cap_helper(ai,rn,f0,2,c))
		return;

	rn = rate2num(r>>0);
	if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
		return;
	rn = rate2num(r>>1);
	if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
		return;
	rn = rate2num(r>>2);
	if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
		return;


        if(c == 1 && !param.force_stereo)
		c = 0;
        else if(c == 0 && !param.force_mono)
                c = 1;

        rn = rate2num(r>>0);
        if(audio_fit_cap_helper(ai,rn,f0,2,c))
                return;
        rn = rate2num(r>>1);
        if(audio_fit_cap_helper(ai,rn,f0,2,c))
                return;
        rn = rate2num(r>>2);
        if(audio_fit_cap_helper(ai,rn,f0,2,c))
                return;

        rn = rate2num(r>>0);
        if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
                return;
        rn = rate2num(r>>1);
        if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
                return;
        rn = rate2num(r>>2);
        if(audio_fit_cap_helper(ai,rn,2,NUM_ENCODINGS,c))
                return;

	fprintf(stderr,"No supported rate found!\n");
	exit(1);
}

char *audio_encoding_name(int format)
{
	int i;

	for(i=0;i<NUM_ENCODINGS;i++) {
		if(audio_val2name[i].val == format)
			return audio_val2name[i].name;
	}
	return "Unknown";
}

#if !defined(SOLARIS) && !defined(__NetBSD__) || defined(NAS)
void audio_queueflush(struct audio_info_struct *ai)
{
}
#endif

