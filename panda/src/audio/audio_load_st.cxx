// Filename: audio_load_st.C
// Created by:  cary (03Oct00)
// 
////////////////////////////////////////////////////////////////////

#include <config.h>
#include "audio_pool.h"
#include "config_audio.h"
#include "audio_trait.h"

#include <sox/st.h>
#include <sox/patchlvl.h>

#if (PATCHLEVEL == 16)
#define FORMATS formats
#define EFFECTS_TYPE struct effect
#define STREAM_TYPE struct soundstream
#define GETTYPE gettype
#define COMPAT_EOF (-1)
#define CHECKFORMAT checkformat
#define SIZES sizes
#define ENCODING styles
#define COPYFORMAT copyformat
#define GETEFFECT geteffect
#define UPDATEEFFECT(a, b, c, d)
#else /* PATCHLEVEL != 16 */
#define FORMATS st_formats
#define EFFECTS_TYPE struct st_effect
#define STREAM_TYPE struct st_soundstream
#define GETTYPE st_gettype
#define COMPAT_EOF ST_EOF
#define CHECKFORMAT st_checkformat
#define SIZES st_sizes_str
#define ENCODING st_encodings_str
#define COPYFORMAT st_copyformat
#define GETEFFECT st_geteffect
#define UPDATEEFFECT st_updateeffect
#endif /* PATCHLEVEL */

Configure(audio_load_st);

// the effects we will be using are to change the rate and number of channels
static EFFECTS_TYPE efftab[5];  // left/mono channel effects
static EFFECTS_TYPE efftabR[5]; // right channel effects
static int neffects;            // how many effects are in action

static STREAM_TYPE informat;    // holder for the input
static STREAM_TYPE outformat;   // holder for fake output;

INLINE static void init_stream(void) {
  informat.info.rate = 0;
  informat.info.size = -1;
  informat.info.encoding = -1;
  informat.info.channels = -1;
  informat.comment = (char*)0L;
  informat.swap = 0;
  informat.filetype = (char*)0L;
  informat.fp = stdin;
  informat.filename = "input";

  outformat.info.rate = audio_mix_freq;
  outformat.info.size = -1;
  outformat.info.encoding = -1;
  outformat.info.channels = 2;
  outformat.comment = (char*)0L;
  outformat.swap = 0;
  outformat.filetype = (char*)0L;
  outformat.fp = stdout;
  outformat.filename = "output";
}

INLINE static void check_effects(void) {
  bool needchan = (informat.info.rate != audio_mix_freq);
  bool needrate = (informat.info.channels != 2);

  // efftab[0] is always the input stream and always exists
  neffects = 1;

  // if reducing the number of samples, it is faster to run all effects
  // after the resample effect
  if (needrate) {
    GETEFFECT(&efftab[neffects], "resample");
    // setup and give default opts
    (*efftab[neffects].h->getopts)(&efftab[neffects],(int)0,(char**)0L);
    // copy format info to effect table
    UPDATEEFFECT(&efftab[neffects], &informat, &outformat, 0);
    // rate can't handle multiple channels so be sure and account for that
    if (efftab[neffects].ininfo.channels > 1)
      memcpy(&efftabR[neffects], &efftab[neffects], sizeof(EFFECTS_TYPE));
    ++neffects;
  }
  // if we ever have more then 2 channels in an input file, we will need to
  // deal with that somewhere here
  if (needchan) {
    GETEFFECT(&efftab[neffects], "avg");
    //setup and give default opts
    (*efftab[neffects].h->getopts)(&efftab[neffects],(int)0,(char**)0L);
    // copy format info to effect table
    UPDATEEFFECT(&efftab[neffects], &informat, &outformat, 0);
    ++neffects;
  }
}

static byte* read_file(Filename filename) {
  int e, havedata;
  ostringstream out;

  init_stream();
  if ((informat.fp = fopen(filename.c_str(), READBINARY)) == NULL) {
    audio_cat->error() << "could not open '" << filename << "'" << endl;
    return (byte*)0L;
  }
  informat.filname = filename.c_str();
  informat.filetype = filename.get_extension();
  informat.comment = filename.c_str();  // for lack of anything better
  // now we start some more real work
  GETTYPE(&informat);
  // read and write starters can change their formats
  if ((*informat.h->startread)(&informat) == COMPAT_EOF) {
    audio_cat->error() << "failed to start read" << endl;
    return (byte*)0L;
  }
  CHECKFORMAT(&informat);
  if (audio_cat->is_debug())
    audio_cat->debug() << "Input file '" << informat.filename
		       << "': sample rate = " << informat.info.rate
		       << "  size = " << SIZES[informat.info.size]
		       << "  encoding = " << ENCODING[informat.info.encoding]
		       << "  " << informat.info.channels
		       << (informat.info.channels > 1)?"channels":"channel"
		       << endl;
  if (audio_cat->is_debug())
    audio_cat->debug() << "Input file comment: '" << informat.comment << "'"
		       << endl;
  COPYFORMAT(&informat, &outformat);
  check_effects();
  // start all effects
  for (e=1; e<neffects; ++e) {
    (*efftab[e].h->start)(&efftab[e]);
    if (efftabR[e].name)
      (*efftabR[e].h->start)(&efftabR[e]);
  }
  // reserve output buffers for all effects
  for (e=0; e<neffects; e++) {
    efftab[e].obuf = new LONG[BUFSIZ*sizeof(LONG)];
    if (efftabR[e].name)
      efftabR[e].obuf = new LONG[BUFSIZ*sizeof(LONG)];
  }
  // get the main while loop ready, have some data for it to start on
  efftab[0].olen = (*informat.h->read)(&informat, efftab[0].obuf,
				       (LONG)BUFSIZ);
  efftab[0].odone = 0;
  // run input data thru effects and get more until olen == 0
  while (efftab[0].olen > 0) {
    // mark chain as empty
    for (e=1; e<neffects; ++e)
      efftab[e].odone = efftab[e].olen = 0;
    do {
      ULONG w;
      // run entire chain backwards: pull, don't push
      // this is because buffering system isn't a nice queueing system
      for (e=neffects-1; e>0; --e)
	if (flow_effect(e))
	  break;
      // add to output data
      if (efftab[neffects-1].olen>efftab[neffects-1].odone) {
	for (LONG i=0; i<efftab[neffects-1].olen; ++i) {
	  LONG foo = efftab[neffects-1].obuf[i];
	  signed short bar = (foo >> 16);
	  unsigned char *b = &bar;
	  out << *b << *(++b);
	}
	efftab[neffects-1].odone = efftab[neffects-1].olen;
      }
      // if there is still stuff in the pipeline, setup to flow effects again
      havedata = 0;
      for (e=0; e<neffects-1; ++e)
	if (efftab[e].odone < efftab[e].olen) {
	  havedata = 1;
	  break;
	}
    } while (havedata);
    // read another chunk
    efftab[0].olen = (*informat.h->read)(&informat, efftab[0].obuf,
					 (LONG)BUFSIZ);
    efftab[0].odone = 0;
  }
  // drain the effects out first to last.  push the residue thru subsequent
  // effects.  suck.
  for (e=1; e<neffects; ++e)
    while (1) {
      if (drain_effect(e) == 0)
	break;  // get out of while loop
      if (efftab[neffects-1].olen > 0) {
	for (LONG i=0; i<efftab[neffects-1].olen; ++i) {
	  LONG foo = efftab[neffects-1].obuf[i];
	  signed short bar = (foo >> 16);
	  unsigned char *b = &bar;
	  out << *b << *(++b);
	}
      }
      if (efftab[e].olen != BUFSIZ)
	break;
    }
  // stop all effects.  In so doing, some may generate more data
  for (e=1; e<neffects; ++e) {
    (*efftab[e].h->stop)(&efftab[e]);
    if (efftabR[e].name)
      (*efftabR[e].h->stop)(&efftabR[e]);
  }
  // stop reading the file
  if ((*informat.h->stopread)(&informat) == COMPAT_EOF) {
    audio_cat->error() << "error stoping input file" << endl;
  }
  fclose(informat.fp);
  // generate output
  string s = out.str();
  int slen = s.length();
  byte* ret = new byte[slen];
  memcpy(ret, s.data(), slen);
  return ret;
}

#ifdef AUDIO_USE_MIKMOD

void AudioDestroySt(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadSt(AudioTraits::SampleClass** sample,
		 AudioTraits::PlayerClass** player,
		 AudioTraits::DeleteSampleFunc** destroy, Filename) {
  audio_cat->warning() << "MikMod doesn't support reading raw data yet"
		       << endl;
  *sample = (AudioTraits::SampleClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroySt;
}

#else /* AUDIO_USE_MIKMOD */

#ifdef AUDIO_USE_WIN32

void AudioDestroySt(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadSt(AudioTraits::SampleClass** sample,
		 AudioTraits::PlayerClass** player,
		 AudioTraits::DeleteSampleFunc** destroy, Filename) {
  audio_cat->warning() << "win32 doesn't support reading raw data yet"
		       << endl;
  *sample = (AudioTraits::SampleClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroySt;
}

#else /* AUDIO_USE_WIN32 */

#ifdef AUDIO_USE_LINUX

void AudioDestroySt(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadSt(AudioTraits::SampleClass** sample,
		 AudioTraits::PlayerClass** player,
		 AudioTraits::DeleteSampleFunc** destroy, Filename) {
  audio_cat->warning() << "linux doesn't support reading raw data yet"
		       << endl;
  *sample = (AudioTraits::SampleClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroySt;
}

#else /* AUDIO_USE_LINUX */

#ifdef AUDIO_USE_NULL

// Null driver
#include "audio_null_traits.h"

void AudioDestroySt(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadSt(AudioTraits::SampleClass** sample,
		 AudioTraits::PlayerClass** player,
		 AudioTraits::DeleteSampleFunc** destroy, Filename) {
  *sample = new NullSample();
  *player = new NullPlayer();
  *destroy = AudioDestroySt;
}

#else /* AUDIO_USE_NULL */

#error "unknown audio driver type"

#endif /* AUDIO_USE_NULL */
#endif /* AUDIO_USE_LINUX */
#endif /* AUDIO_USE_WIN32 */
#endif /* AUDIO_USE_MIKMOD */

ConfigureFn(audio_load_st) {
  for (int i=0; FORMATS[i].names != (char**)0L; ++i)
    for (int j=0; FORMATS[i].names[j] != (char*)0L; ++j) {
      if (audio_cat->is_debug())
	audio_cat->debug() << "adding reader for '." << FORMATS[i].names[j]
			   << "'" << endl;
      AudioPool::register_sample_loader(FORMATS[i].names[j], AudioLoadSt);
    }
}
