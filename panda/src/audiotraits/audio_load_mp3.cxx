// Filename: audio_load_mp3.cxx
// Created by:  cary (11Oct00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"
#include "audio_trait.h"

Configure(audio_load_mp3);

extern "C" {
#include <mpg123.h>
}

static bool initialized = false;
static struct audio_info_struct ai;
static struct frame fr;
struct parameter param = {
  FALSE, /* aggressive */
  FALSE, /* shuffle */
  FALSE, /* remote */
  DECODE_AUDIO, /* write samples to audio device */
  FALSE, /* silent operation */
  FALSE, /* xterm title on/off */
  0, /* second level buffer size */
  TRUE, /* resync after stream error */
  0, /* verbose level */
#ifdef TERM_CONTROL
  FALSE, /* term control */
#endif /* TERM_CONTROL */
  -1, /* force mono */
  0, /* force stereo */
  0, /* force 8-bit */
  0, /* force rate */
  0, /* down sample */
  FALSE, /* check range */
  0, /* double speed */
  0, /* half speed */
  0, /* force re-open. always (re)opens audio device for next song */
  0, /* 3Dnow: autodetect from CPUFLAGS */
  FALSE, /* 3Dnow: normal operation */
  FALSE, /* try to run process in 'realtime mode' */
  { 0, }, /* wav, cdr, au filename */
  NULL, /* esdserver */
  NULL, /* equalfile */
  0, /* enable_equalizer */
  32768, /* outscale */
  0, /* startFrame */
};
static long numframes = -1;
static int intflag = FALSE;
static struct mpstr mp;
// stuff I have to have to make the linkage happy
int OutputDescriptor;
int buffer_fd[2];
struct reader *rd;
txfermem* buffermem;

static void set_synth_functions(struct frame* fr) {
  typedef int (*func)(real*, int, unsigned char*, int*);
  typedef int (*func_mono)(real*, unsigned char*, int*);
  typedef void (*func_dct36)(real*, real*, real*, real*, real*);

  int ds = fr->down_sample;
  int p8=0;

  static func funcs[][4] = {
    { synth_1to1,
      synth_2to1,
      synth_4to1,
      synth_ntom } ,
    { synth_1to1_8bit,
      synth_2to1_8bit,
      synth_4to1_8bit,
      synth_ntom_8bit }
  };
  static func_mono funcs_mono[2][2][4] = {
    { { synth_1to1_mono2stereo,
	synth_2to1_mono2stereo,
	synth_4to1_mono2stereo,
	synth_ntom_mono2stereo } ,
      { synth_1to1_8bit_mono2stereo,
	synth_2to1_8bit_mono2stereo,
	synth_4to1_8bit_mono2stereo,
	synth_ntom_8bit_mono2stereo } } ,
    { { synth_1to1_mono,
	synth_2to1_mono,
	synth_4to1_mono,
	synth_ntom_mono } ,
      { synth_1to1_8bit_mono,
	synth_2to1_8bit_mono,
	synth_4to1_8bit_mono,
	synth_ntom_8bit_mono } } ,
  };

  if ((ai.format & AUDIO_FORMAT_MASK) == AUDIO_FORMAT_8)
    p8 = 1;
  fr->synth = funcs[p8][ds];
  fr->synth_mono = funcs_mono[param.force_stereo?0:1][p8][ds];
  if (p8)
    make_conv16to8_table(ai.format);
}

static void initialize(void) {
  if (initialized)
    return;
  // make sure params say what we want
  param.quiet = TRUE;
  param.force_stereo = 1;
  param.force_rate = audio_mix_freq;

  memset(&mp, 0, sizeof(struct mpstr));
  audio_info_struct_init(&ai);
  audio_capabilities(&ai);
  set_synth_functions(&fr);
  make_decode_tables(param.outscale);
  init_layer2(); /* inits also shared tables with layer1 */
  init_layer3(fr.down_sample);
  equalizer_cnt = 0;
  for (int i=0; i<32; ++i) {
    equalizer[0][i] = equalizer[1][i] = 1.0;
    equalizer_sum[0][i] = equalizer_sum[1][i] = 0.0;
  }
  initialized = true;
}

ostream* my_outstream;

extern "C" {
int audio_open(struct audio_info_struct* ai) {
  return 0;
}

int audio_reset_parameters(struct audio_info_struct* ai) {
  audio_set_format(ai);
  audio_set_channels(ai);
  audio_set_rate(ai);
  return 0;
}

int audio_rate_best_match(struct audio_info_struct* ai) {
  if (!ai || ai->rate < 0)
    return -1;
  ai->rate = audio_mix_freq;
  return 0;
}

int audio_set_rate(struct audio_info_struct* ai) {
  if (ai->rate != audio_mix_freq)
    audio_cat->warning()
      << "trying to decode mp3 to rate other then mix rate (" << ai->rate
      << " != " << audio_mix_freq << ")" << endl;
  return 0;
}

int audio_set_channels(struct audio_info_struct* ai) {
  if (ai->channels != 2)
    audio_cat->warning() << "trying to decode mp3 to non-stereo ("
			 << ai->channels << " != 2)" << endl;
  return 0;
}

int audio_set_format(struct audio_info_struct* ai) {
  if (ai->format != AUDIO_FORMAT_SIGNED_16)
    audio_cat->warning()
      << "trying to decode mp3 to format other then signed 16-bit" << endl;
  return 0;
}

int audio_get_formats(struct audio_info_struct* ai) {
  return AUDIO_FORMAT_SIGNED_16;
}

int audio_play_samples(struct audio_info_struct* ai, unsigned char* buf,
		       int len) {
  for (int i=0; i<len; ++i)
    (*my_outstream) << buf[i];
  return len;
}

int audio_close(struct audio_info_struct* ai) {
  return 0;
}

// we won't use these functions, but they have to exist
int cdr_open(struct audio_info_struct *ai, char *ame) { return 0; }
int au_open(struct audio_info_struct *ai, char *name) { return 0; }
int wav_open(struct audio_info_struct *ai, char *wavfilename) { return 0; }
int wav_write(unsigned char *buf,int len) { return 0; }
int cdr_close(void) { return 0; }
int au_close(void) { return 0; }
int wav_close(void) { return 0; }
int xfermem_get_usedspace(txfermem*) { return 0; }
}

static void init_output(void) {
  static int init_done = FALSE;
  if (init_done)
    return;
  init_done = TRUE;
  // + 1024 for NtoM rate converter
  if (!(pcm_sample = (unsigned char*)malloc(audiobufsize*2 + 2*1024))) {
    audio_cat->fatal() << "cannot allocate sample buffer" << endl;
    exit(1);
  }
  switch (param.outmode) {
  case DECODE_AUDIO:
    if (audio_open(&ai) < 0) {
      audio_cat->fatal() << "could not open output stream" << endl;
      exit(1);
    }
    break;
  case DECODE_WAV:
    wav_open(&ai, param.filename);
    break;
  case DECODE_AU:
    au_open(&ai, param.filename);
    break;
  case DECODE_CDR:
    cdr_open(&ai, param.filename);
    break;
  }
}

static void reset_audio(void) {
  if (param.outmode == DECODE_AUDIO) {
    audio_close(&ai);
    if (audio_open(&ai) < 0) {
      audio_cat->fatal() << "couldn't reopen" << endl;
      exit(1);
    }
  }
}

int play_frame(struct mpstr* mp, int init, struct frame* fr) {
  int clip;
  long newrate;
  long old_rate, old_format, old_channels;

  if (fr->header_change || init) {
    if (fr->header_change > 1 || init) {
      old_rate = ai.rate;
      old_format = ai.format;
      old_channels = ai.channels;
      newrate = freqs[fr->sampling_frequency]>>(param.down_sample);
      fr->down_sample = param.down_sample;
      audio_fit_capabilities(&ai, fr->stereo, newrate);
      // check whether the fitter set our proposed rate
      if (ai.rate != newrate) {
	if (ai.rate == (newrate >> 1))
	  fr->down_sample++;
	else if (ai.rate == (newrate >> 2))
	  fr->down_sample += 2;
	else {
	  fr->down_sample = 3;
	  audio_cat->warning() << "flexable rate not heavily tested!" << endl;
	}
	if (fr->down_sample > 3)
	  fr->down_sample = 3;
      }
      switch (fr->down_sample) {
      case 0:
      case 1:
      case 2:
	fr->down_sample_sblimit = SBLIMIT >> (fr->down_sample);
	break;
      case 3:
	{
	  long n = freqs[fr->sampling_frequency];
	  long m = ai.rate;
	  synth_ntom_set_step(n, m);
	  if (n>m) {
	    fr->down_sample_sblimit = SBLIMIT * m;
	    fr->down_sample_sblimit /= n;
	  } else
	    fr->down_sample_sblimit = SBLIMIT;
	}
	break;
      }
      set_synth_functions(fr);
      init_output();
      if (ai.rate != old_rate || ai.channels != old_channels ||
	  ai.format != old_format || param.force_reopen) {
	if (param.force_mono < 0) {
	  if (ai.channels == 1)
	    fr->single = 3;
	  else
	    fr->single = -1;
	} else
	  fr->single = param.force_mono;
	param.force_stereo &= ~0x2;
	if (fr->single >= 0 && ai.channels == 2)
	  param.force_stereo |= 0x2;
	set_synth_functions(fr);
	init_layer3(fr->down_sample_sblimit);
	reset_audio();
      }
      if (intflag)
	return !0;
    }
  }
  if (fr->error_protection)
    bsi.wordpointer += 2;
  // do the decoding
  switch (fr->lay) {
  case 1:
    if ((clip=do_layer1(mp, fr, param.outmode, &ai)) < 0)
      return 0;
    break;
  case 2:
    if ((clip=do_layer2(mp, fr, param.outmode, &ai)) < 0)
      return 0;
    break;
  case 3:
    if ((clip=do_layer3(mp, fr, param.outmode, &ai)) < 0)
      return 0;
    break;
  default:
    clip = 0;
  }
  if (clip > 0 && param.checkrange)
    audio_cat->warning() << clip << " samples clipped" << endl;
  return !0;
}

static void read_file(Filename filename, unsigned char** buf,
		      unsigned long& slen) {
  int init;
  unsigned long frameNum = 0;
  ostringstream out;

  initialize();
  my_outstream = &out;
  if (open_stream((char*)(filename.c_str()), -1)) {
    long leftFrames, newFrame;

    read_frame_init();
    init = 1;
    newFrame = param.startFrame;
    leftFrames = numframes;
    for (frameNum=0; read_frame(&fr) && leftFrames && !intflag; ++frameNum) {
      if (frameNum < param.startFrame || (param.doublespeed &&
					  (frameNum % param.doublespeed))) {
	if (fr.lay == 3)
	  set_pointer(512);
	continue;
      }
      if (leftFrames > 0)
	--leftFrames;
      if (!play_frame(&mp, init, &fr)) {
	audio_cat->error() << "Error in frame #" << frameNum << endl;
	break;
      }
      init = 0;
    }
    rd->close(rd);
    if (intflag) {
      intflag = FALSE;
    }
  }
  audio_flush(param.outmode, &ai);
  free(pcm_sample);
  switch (param.outmode) {
  case DECODE_AUDIO:
    audio_close(&ai);
    break;
  case DECODE_WAV:
    wav_close();
    break;
  case DECODE_AU:
    au_close();
    break;
  case DECODE_CDR:
    cdr_close();
    break;
  }
  // generate output
  string s = out.str();
  slen = s.length();
  *buf = new byte[slen];
  memcpy(*buf, s.data(), slen);
  my_outstream = (ostream*)0L;
}

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

void AudioDestroyMp3(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadMp3(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename) {
  audio_cat->warning() << "Mikmod doesn't support reading mp3 data yet"
		       << endl;
  *sample = (AudioTraits::SampleClass*)0L;
  *state = (AudioTraits::PlayingClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroyMp3;
}

#elif defined(AUDIO_USE_WIN32)

#include "audio_win_traits.h"

void AudioDestroyMp3(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadMp3(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename) {
  audio_cat->warning() << "win32 doesn't support reading mp3 data yet"
		       << endl;
  *sample = (AudioTraits::SampleClass*)0L;
  *state = (AudioTraits::PlayingClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroyMp3;
}

#elif defined(AUDIO_USE_LINUX)

#include "audio_linux_traits.h"

void AudioDestroyMp3(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadMp3(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
  unsigned char* buf;
  unsigned long len;
  read_file(filename, &buf, len);
  if (buf != (unsigned char*)0L) {
    *sample = LinuxSample::load_raw(buf, len);
    *state = ((LinuxSample*)(*sample))->get_state();
    *player = LinuxPlayer::get_instance();
    *destroy = AudioDestroyMp3;
  } else {
    *sample = (AudioTraits::SampleClass*)0L;
    *state = (AudioTraits::PlayingClass*)0L;
    *player = (AudioTraits::PlayerClass*)0L;
    *destroy = AudioDestroyMp3;
  }
}

#elif defined(AUDIO_USE_NULL)

#include "audio_null_traits.h"

void AudioDestroyMp3(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadMp3(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename) {
  *sample = (AudioTraits::SampleClass*)0L;
  *state = (AudioTraits::PlayingClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroyMp3;
}

#else

#error "unknown audio driver type"

#endif

ConfigureFn(audio_load_mp3) {
  AudioPool::register_sample_loader("mp3", AudioLoadMp3);
}
