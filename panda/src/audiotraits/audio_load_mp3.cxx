// Filename: audio_load_mp3.cxx
// Created by:  cary (11Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"
#include "audio_trait.h"
#include "config_util.h"

Configure(audio_load_mp3);

#if !(defined(WIN32) && defined(AUDIO_USE_RAD_MSS))

#include <math.h>

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
  // make sure params say what we want
  param.quiet = TRUE;
  param.force_stereo = 1;
  param.force_rate = audio_mix_freq;

  memset(&mp, 0, sizeof(struct mpstr));
  audio_info_struct_init(&ai);
  audio_capabilities(&ai);

  if (initialized)
    return;

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

class BufferStuff {
private:
  typedef pvector<unsigned char> Buffer;
  typedef pvector<Buffer> Buffers;
  Buffers _bufs;
public:
  BufferStuff(void) {}
  ~BufferStuff(void) {
  }
  void add(unsigned char* b, unsigned long l) {
    _bufs.push_back(Buffer(b, b+l));
  }
  unsigned long length(void) const {
    unsigned long ret = 0;
    for (Buffers::const_iterator i=_bufs.begin(); i!=_bufs.end(); ++i)
      ret += (*i).size();
    return ret;
  }
  void output(unsigned char* b) {
    for (Buffers::const_iterator i=_bufs.begin(); i!=_bufs.end(); ++i)
      for (Buffer::const_iterator j=(*i).begin(); j!=(*i).end(); ++j)
    *(b++) = (*j);
  }
};

static BufferStuff* my_buf;

/*
class BufferPart {
private:
  unsigned char* _ptr;
  unsigned long _len;
  BufferPart* _next;

  BufferPart(void) : _ptr((unsigned char*)0L), _len(0), _next((BufferPart*)0L)
     {}
public:
  BufferPart(unsigned char* b, unsigned long l) : _next((BufferPart*)0L),
                                                  _len(l) {
    _ptr = new unsigned char[l];
    memcpy(_ptr, b, l);
  }
  ~BufferPart(void) {
    delete _next;
    delete [] _ptr;
  }
  BufferPart* add(unsigned char* b, unsigned long l) {
    _next = new BufferPart(b, l);
    return _next;
  }
  unsigned long length(void) const {
    unsigned long ret = _len;
    if (_next != (BufferPart*)0L)
      ret += _next->length();
    return ret;
  }
  void output(unsigned char* b) {
     memcpy(b, _ptr, _len);
     if (_next != (BufferPart*)0L)
    _next->output(b+_len);
  }
};

static BufferPart* my_buf_head;
static BufferPart* my_buf_curr;
*/

/*
string my_buf;
*/

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
  /*
  if (my_buf_head == (BufferPart*)0L) {
    my_buf_head = my_buf_curr = new BufferPart(buf, len);
  } else {
    my_buf_curr = my_buf_curr->add(buf, len);
  }
  */

  if (my_buf == (BufferStuff*)0L)
    my_buf = new BufferStuff;
  my_buf->add(buf, len);

  /*
  string tmp;
  for (int i=0; i<len; ++i)
    tmp += buf[i];
  my_buf += tmp;
  */

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

// static unsigned char* real_sample_buf;

static void init_output(void) {
  //  static int init_done = FALSE;
  //  if (init_done)
  //    return;
  //  init_done = TRUE;
  // + 1024 for NtoM rate converter
  //  if (!(real_sample_buf=(unsigned char*)malloc(2*(audiobufsize*2 + 2*1024)))) {
  if (!(pcm_sample=(unsigned char*)malloc(audiobufsize*2 + 2*1024))) {
    audio_cat->fatal() << "cannot allocate sample buffer" << endl;
    exit(1);
  }
  //  pcm_sample = &(real_sample_buf[1024]);
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

  initialize();
  //  my_buf_head = my_buf_curr = (BufferPart*)0L;
  my_buf = (BufferStuff*)0L;
  //  my_buf = "";
  if (open_stream((char*)(filename.to_os_specific().c_str()), -1)) {
    long leftFrames, newFrame;

    read_frame_init();
    init = 1;
    newFrame = param.startFrame;
    leftFrames = numframes;
    for (frameNum=0; read_frame(&fr) && leftFrames && !intflag; ++frameNum) {
      if ((frameNum % 100) == 0)
        if (audio_cat.is_debug())
          audio_cat->debug(false) << ".";
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
  if (audio_cat.is_debug())
    audio_cat->debug(false) << endl;
  audio_flush(param.outmode, &ai);
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
  /*
  if (real_sample_buf != (unsigned char*)0L) {
    free(real_sample_buf);
    pcm_sample = (unsigned char*)0L;
  }
  */
  if (pcm_sample != (unsigned char*)0L) {
    free(pcm_sample);
    pcm_sample = (unsigned char*)0L;
  }
  // generate output
  /*
  slen = my_buf_head->length();
  *buf = new byte[slen];
  my_buf_head->output(*buf);
  delete my_buf_head;
  */

  slen = my_buf->length();
  *buf = new byte[slen];
  my_buf->output(*buf);
  delete my_buf;
  my_buf = (BufferStuff*)0L;

  /*
  slen = my_buf.size();
  *buf = new byte[slen];
  memcpy(*buf, my_buf.data(), slen);
  */
}
#endif

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

AudioTraits::SoundClass* AudioLoadMp3(Filename) {
  audio_cat->warning() << "Mikmod doesn't support reading mp3 data yet"
               << endl;
  return (AudioTraits::SoundClass*)0L;
}

#elif defined(AUDIO_USE_RAD_MSS)

#include "audio_rad_mss_traits.h"

EXPCL_MISC AudioTraits::SoundClass* AudioLoadMp3(Filename filename) {
  return MilesSound::load(filename);
}

#elif defined(AUDIO_USE_WIN32)

#include "audio_win_traits.h"

EXPCL_MISC AudioTraits::SoundClass* AudioLoadMp3(Filename filename) {
  unsigned char* buf;
  unsigned long len;
  read_file(filename, &buf, len);
  if (buf != (unsigned char*)0L) {
    return WinSample::load_raw(buf, len);
  }
  return (AudioTraits::SoundClass*)0L;
}

#elif defined(AUDIO_USE_LINUX)

#include "audio_linux_traits.h"

AudioTraits::SoundClass* AudioLoadMp3(Filename filename) {
  unsigned char* buf;
  unsigned long len;
  read_file(filename, &buf, len);
  if (buf != (unsigned char*)0L) {
    return LinuxSample::load_raw(buf, len);
  }
  return (AudioTraits::SoundClass*)0L;
}

#elif defined(AUDIO_USE_NULL)

#include "audio_null_traits.h"

AudioTraits::SoundClass* AudioLoadMp3(Filename) {
  return new NullSound();
}

#else

#error "unknown audio driver type"

#endif

ConfigureFn(audio_load_mp3) {
  AudioPool::register_sound_loader("mp3", AudioLoadMp3);
}
