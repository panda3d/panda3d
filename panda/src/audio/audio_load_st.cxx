// Filename: audio_load_st.C
// Created by:  cary (03Oct00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"
#include "audio_trait.h"

#ifdef HAVE_SOXST

extern "C" {
#include <sox/st.h>
#include <sox/patchlvl.h>
}

#if (PATCHLEVEL == 16)
#define FORMATS formats
#define ENCODEFIELD style
#define EFFECTS_TYPE struct effect
#define STREAM_TYPE struct soundstream
#define GETTYPE gettype
#define COMPAT_EOF (-1)
#define CHECKFORMAT checkformat
#define SIZES sizes
#define ENCODING styles
#define COPYFORMAT copyformat
#define UPDATEEFFECT(a, b, c, d) \
  (a)->ininfo.channels = (b)->info.channels; \
  (a)->outinfo.channels = (c)->info.channels; \
  (a)->ininfo.rate = (b)->info.rate; \
  (a)->outinfo.rate = (c)->info.rate;
#else /* PATCHLEVEL != 16 */
#define FORMATS st_formats
#define ENCODEFIELD encoding
#define EFFECTS_TYPE struct st_effect
#define STREAM_TYPE struct st_soundstream
#define GETTYPE st_gettype
#define COMPAT_EOF ST_EOF
#define CHECKFORMAT st_checkformat
#define SIZES st_sizes_str
#define ENCODING st_encodings_str
#define COPYFORMAT st_copyformat
#define UPDATEEFFECT st_updateeffect
#endif /* PATCHLEVEL */

#endif /* HAVE_SOXST */

Configure(audio_load_st);

#ifdef HAVE_SOXST

// the effects we will be using are to change the rate and number of channels
static EFFECTS_TYPE efftab[5];  // left/mono channel effects
static EFFECTS_TYPE efftabR[5]; // right channel effects
static int neffects;            // how many effects are in action

static STREAM_TYPE iformat;    // holder for the input
static STREAM_TYPE oformat;   // holder for fake output;

INLINE static void init_stream(void) {
  iformat.info.rate = 0;
  iformat.info.size = -1;
  iformat.info.ENCODEFIELD = -1;
  iformat.info.channels = -1;
  iformat.comment = (char*)0L;
  iformat.swap = 0;
  iformat.filetype = (char*)0L;
  iformat.fp = stdin;
  iformat.filename = "input";

  oformat.info.rate = audio_mix_freq;
  oformat.info.size = -1;
  oformat.info.ENCODEFIELD = -1;
  oformat.info.channels = 2;
  oformat.comment = (char*)0L;
  oformat.swap = 0;
  oformat.filetype = (char*)0L;
  oformat.fp = stdout;
  oformat.filename = "output";
}

INLINE static void compat_geteffect(EFFECTS_TYPE* eff, const char* name) {
#if (PATCHLEVEL == 16)
  eff->name = (char*)name;
  geteffect(eff);
#else /* PATCHLEVEL == 16 */
  st_geteffect(eff, name);
#endif /* PATCHLEVEL == 16 */
}

typedef void effOptFunc(EFFECTS_TYPE*, int, char**);
#ifndef HAVE_DEFINED_BYTE
typedef unsigned char byte;
#define HAVE_DEFINED_BYTE
#endif /* HAVE_DEFINED_BYTE */

INLINE static void check_effects(void) {
  bool needrate = (iformat.info.rate != audio_mix_freq);
  bool needchan = (iformat.info.channels != 2);
  effOptFunc* func;

  // efftab[0] is always the input stream and always exists
  neffects = 1;

  // if reducing the number of samples, it is faster to run all effects
  // after the resample effect
  if (needrate) {
    compat_geteffect(&efftab[neffects], "resample");
    // setup and give default opts
    func = (effOptFunc*)(efftab[neffects].h->getopts);
    (*func)(&efftab[neffects],(int)0,(char**)0L);
    // copy format info to effect table
    UPDATEEFFECT(&efftab[neffects], &iformat, &oformat, 0);
    // rate can't handle multiple channels so be sure and account for that
    if (efftab[neffects].ininfo.channels > 1)
      memcpy(&efftabR[neffects], &efftab[neffects], sizeof(EFFECTS_TYPE));
    ++neffects;
  }
  // if we ever have more then 2 channels in an input file, we will need to
  // deal with that somewhere here
  if (needchan) {
    compat_geteffect(&efftab[neffects], "avg");
    //setup and give default opts
    func = (effOptFunc*)(efftab[neffects].h->getopts);
    (*func)(&efftab[neffects],(int)0,(char**)0L);
    // copy format info to effect table
    UPDATEEFFECT(&efftab[neffects], &iformat, &oformat, 0);
    ++neffects;
  }
}

typedef void effFlowFunc(EFFECTS_TYPE*, LONG*, LONG*, LONG*, LONG*);

static LONG ibufl[BUFSIZ/2];
static LONG ibufr[BUFSIZ/2];
static LONG obufl[BUFSIZ/2];
static LONG obufr[BUFSIZ/2];

static int flow_effect(int e) {
  LONG i, done, idone, odone, idonel, odonel, idoner, odoner;
  LONG *ibuf, *obuf;
  effFlowFunc* eflow;

  // is there any input data?
  if (efftab[e-1].odone == efftab[e-1].olen)
    return 0;
  if (!efftabR[e].name) {
    // no stereo data, or effect can handle stereo data.  so run effect
    // over the entire buffer
    idone = efftab[e-1].olen - efftab[e-1].odone;
    odone = BUFSIZ;
    eflow = (effFlowFunc*)(efftab[e].h->flow);
    (*eflow)(&efftab[e], &efftab[e-1].obuf[efftab[e-1].odone], efftab[e].obuf,
	     &idone, &odone);
    efftab[e-1].odone += idone;
    efftab[e].odone = 0;
    efftab[e].olen = odone;
    done = idone + odone;
  } else {
    // put stereo data in two seperate buffers and run effect on each of them
    idone = efftab[e-1].olen - efftab[e-1].odone;
    odone = BUFSIZ;
    ibuf = &efftab[e-1].obuf[efftab[e-1].odone];
    for (i=0; i<idone; i+=2) {
      ibufl[i/2] = *ibuf++;
      ibufr[i/2] = *ibuf++;
    }
    // left
    idonel = (idone + 1)/2; // odd-length logic
    odonel = odone / 2;
    eflow = (effFlowFunc*)(efftab[e].h->flow);
    (*eflow)(&efftab[e], ibufl, obufl, &idonel, &odonel);
    // right
    idoner = idone/2;  // odd-length logic
    odoner = odone/2;
    eflow = (effFlowFunc*)(efftabR[e].h->flow);
    (*eflow)(&efftabR[e], ibufr, obufr, &idoner, &odoner);
    obuf = efftab[e].obuf;
    // this loop implies that left and right effects will always output
    // the same amount of data
    for (i=0; i<odoner; i++) {
      *obuf++ = obufl[i];
      *obuf++ = obufr[i];
    }
    efftab[e-1].odone += idonel + idoner;
    efftab[e].odone = 0;
    efftab[e].olen = odonel + odoner;
    done = idonel + idoner + odonel + odoner;
  }
  if (done == 0)
    audio_cat->error() << "Effect took & gave no samples!" << endl;
  return 1;
}

typedef void effDrainFunc(EFFECTS_TYPE*, LONG*, LONG*);

static int drain_effect(int e) {
  LONG i, olen, olenl, olenr;
  LONG *obuf;
  effDrainFunc* edrain;

  if (!efftabR[e].name) {
    efftab[e].olen = BUFSIZ;
    edrain = (effDrainFunc*)(efftab[e].h->drain);
    (*edrain)(&efftab[e], efftab[e].obuf, &efftab[e].olen);
  } else {
    olen = BUFSIZ;
    // left
    olenl = olen / 2;
    edrain = (effDrainFunc*)(efftab[e].h->drain);
    (*edrain)(&efftab[e], obufl, &olenl);
    // right
    olenr = olen / 2;
    edrain = (effDrainFunc*)(efftab[e].h->drain);
    (*edrain)(&efftabR[e], obufr, &olenr);
    obuf = efftab[e].obuf;
    // this loop implies left and right effect will always output the same
    // amount of data
    for (i=0; i<olenr; ++i) {
      *obuf++ = obufl[i];
      *obuf++ = obufr[i];
    }
    efftab[e].olen = olenl + olenr;
  }
  return (efftab[e].olen);
}

typedef int formatSReadFunc(STREAM_TYPE*);
typedef LONG formatReadFunc(STREAM_TYPE*, LONG*, LONG);
typedef int formatStopReadFunc(STREAM_TYPE*);
typedef void effStartFunc(EFFECTS_TYPE*);
typedef void effStopFunc(EFFECTS_TYPE*);

static void read_file(Filename filename, byte** buf, unsigned long& slen) {
  int e, havedata;
  ostringstream out;
  formatSReadFunc* srfunc;
  formatReadFunc* rfunc;
  formatStopReadFunc* strfunc;
  effStartFunc* esfunc;
  effStopFunc* estfunc;

  init_stream();
  if ((iformat.fp = fopen(filename.c_str(), READBINARY)) == NULL) {
    audio_cat->error() << "could not open '" << filename << "'" << endl;
    *buf = (byte*)0L;
    slen = 0;
    return;
  }
  iformat.filename = (char*)filename.c_str();
  iformat.filetype = (char*)filename.get_extension().c_str();
  iformat.comment = (char*)filename.c_str();  // for lack of anything better
  // now we start some more real work
  GETTYPE(&iformat);
  // read and write starters can change their formats
  srfunc = (formatSReadFunc*)(iformat.h->startread);
  if ((*srfunc)(&iformat) == COMPAT_EOF) {
    audio_cat->error() << "failed to start read" << endl;
    *buf = (byte*)0L;
    slen = 0;
    return;
  }
  CHECKFORMAT(&iformat);
  if (audio_cat->is_debug())
    audio_cat->debug() << "Input file '" << iformat.filename
		       << "': sample rate = " << iformat.info.rate
		       << "  size = " << SIZES[iformat.info.size]
		       << "  encoding = " << ENCODING[iformat.info.ENCODEFIELD]
		       << "  " << iformat.info.channels
		       << ((iformat.info.channels > 1)?"channels":"channel")
		       << endl;
  if (audio_cat->is_debug())
    audio_cat->debug() << "Input file comment: '" << iformat.comment << "'"
		       << endl;
  COPYFORMAT(&iformat, &oformat);
  check_effects();
  // start all effects
  for (e=1; e<neffects; ++e) {
    esfunc = (effStartFunc*)(efftab[e].h->start);
    (*esfunc)(&efftab[e]);
    if (efftabR[e].name) {
      esfunc = (effStartFunc*)(efftabR[e].h->start);
      (*esfunc)(&efftabR[e]);
    }
  }
  // reserve output buffers for all effects
  for (e=0; e<neffects; e++) {
    efftab[e].obuf = new LONG[BUFSIZ*sizeof(LONG)];
    if (efftabR[e].name)
      efftabR[e].obuf = new LONG[BUFSIZ*sizeof(LONG)];
  }
  // get the main while loop ready, have some data for it to start on
  rfunc = (formatReadFunc*)(iformat.h->read);
  efftab[0].olen = (*rfunc)(&iformat, efftab[0].obuf, (LONG)BUFSIZ);
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
	  unsigned char *b = (unsigned char*)&bar;
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
    rfunc = (formatReadFunc*)(iformat.h->read);
    efftab[0].olen = (*rfunc)(&iformat, efftab[0].obuf, (LONG)BUFSIZ);
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
	  unsigned char *b = (unsigned char*)&bar;
	  out << *b << *(++b);
	}
      }
      if (efftab[e].olen != BUFSIZ)
	break;
    }
  // stop all effects.  In so doing, some may generate more data
  for (e=1; e<neffects; ++e) {
    estfunc = (effStopFunc*)(efftab[e].h->stop);
    (*estfunc)(&efftab[e]);
    if (efftabR[e].name) {
      estfunc = (effStopFunc*)(efftabR[e].h->stop);
      (*estfunc)(&efftabR[e]);
    }
  }
  // stop reading the file
  strfunc = (formatStopReadFunc*)(iformat.h->stopread);
  if ((*strfunc)(&iformat) == COMPAT_EOF) {
    audio_cat->error() << "error stoping input file" << endl;
  }
  fclose(iformat.fp);
  // generate output
  string s = out.str();
  slen = s.length();
  *buf = new byte[slen];
  memcpy(*buf, s.data(), slen);
}

extern "C" {
void cleanup(void) {
  // make sure everything is shut down
  if (iformat.fp)
    fclose(iformat.fp);
}
}

#endif /* HAVE_SOXST */

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

#include "audio_win_traits.h"

void AudioDestroySt(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadSt(AudioTraits::SampleClass** sample,
		 AudioTraits::PlayerClass** player,
		 AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
#ifdef HAVE_SOXST
  unsigned char* buf;
  unsigned long len;
  read_file(filename, &buf, len);
  if (buf != (unsigned char*)0L) {
    *sample = WinSample::load_raw(buf, len);
    *player = WinPlayer::get_instance();
    *destroy = AudioDestroySt;
  }
#else /* HAVE_SOXST */
  *sample = (AudioTraits::SampleClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroySt;
#endif /* HAVE_SOXST */
}

#else /* AUDIO_USE_WIN32 */

#ifdef AUDIO_USE_LINUX

#include "audio_linux_traits.h"

void AudioDestroySt(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadSt(AudioTraits::SampleClass** sample,
		 AudioTraits::PlayerClass** player,
		 AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
#ifdef HAVE_SOXST
  byte* buf;
  unsigned long len;
  read_file(filename, &buf, len);
  if (buf != (byte*)0L) {
    *sample = LinuxSample::load_raw(buf, len);
    *player = LinuxPlayer::get_instance();
    *destroy = AudioDestroySt;
  }
#else /* HAVE_SOXST */
  *sample = (AudioTraits::SampleClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroySt;
#endif /* HAVE_SOXST */
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
#ifdef HAVE_SOXST
  for (int i=0; FORMATS[i].names != (char**)0L; ++i)
    for (int j=0; FORMATS[i].names[j] != (char*)0L; ++j) {
      if (audio_cat->is_debug())
	audio_cat->debug() << "adding reader for '." << FORMATS[i].names[j]
			   << "'" << endl;
      AudioPool::register_sample_loader(FORMATS[i].names[j], AudioLoadSt);
    }
#endif /* HAVE_SOXST */
}
