// Filename: audio_mikmod_traits.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_mikmod_traits.h"

#ifdef AUDIO_USE_MIKMOD

#include "audio_manager.h"
#include "config_audio.h"
#include <list>
#include <serialization.h>

static bool have_initialized = false;
static bool initialization_error = false;

static void update_mikmod(void) {
  MikMod_Update();
}

static void initialize(void) {
  if (have_initialized)
    return;
  if (initialization_error)
    return;
  /* register the drivers */
  MikMod_RegisterAllDrivers();
  /* initialize the mikmod library */
  md_mixfreq = audio_mix_freq;
  {
    // I think this is defined elsewhere
    typedef list<string> StrList;
    typedef Serialize::Deserializer<StrList> OptBuster;
    StrList opts;
    OptBuster buster(*audio_mode_flags, " ");
    opts = buster();
    for (StrList::iterator i=opts.begin(); i!=opts.end(); ++i) {
      if ((*i) == "DMODE_INTERP") {
	md_mode |= DMODE_INTERP;
      } else if ((*i) == "DMODE_REVERSE") {
	md_mode |= DMODE_REVERSE;
      } else if ((*i) == "DMODE_SURROUND") {
	md_mode |= DMODE_SURROUND;
      } else if ((*i) == "DMODE_16BITS") {
	md_mode |= DMODE_16BITS;
      } else if ((*i) == "DMODE_HQMIXER") {
	md_mode |= DMODE_HQMIXER;
      } else if ((*i) == "DMODE_SOFT_MUSIC") {
	md_mode |= DMODE_SOFT_MUSIC;
      } else if ((*i) == "DMODE_SOFT_SNDFX") {
	md_mode |= DMODE_SOFT_SNDFX;
      } else if ((*i) == "DMODE_STEREO") {
	md_mode |= DMODE_STEREO;
      } else {
	audio_cat->error() << "unknown audio driver flag '" << *i << "'"
			   << endl;
      }
    }
    if (audio_cat->is_debug()) {
      audio_cat->debug() << "final driver mode is (";
      bool any_out = false;
      if (md_mode & DMODE_INTERP) {
	audio_cat->debug(false) << "DMODE_INTERP";
	any_out = true;
      }
      if (md_mode & DMODE_REVERSE) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_REVERSE";
	any_out = true;
      }
      if (md_mode & DMODE_SURROUND) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_SURROUND";
	any_out = true;
      }
      if (md_mode & DMODE_16BITS) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_16BITS";
	any_out = true;
      }
      if (md_mode & DMODE_HQMIXER) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_HQMIXER";
	any_out = true;
      }
      if (md_mode & DMODE_SOFT_MUSIC) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_SOFT_MUSIC";
	any_out = true;
      }
      if (md_mode & DMODE_SOFT_SNDFX) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_SOFT_SNDFX";
	any_out = true;
      }
      if (md_mode & DMODE_STEREO) {
	if (any_out)
	  audio_cat->debug(false) << ", ";
	audio_cat->debug(false) << "DMODE_STEREO";
	any_out = true;
      }
      audio_cat->debug(false) << ")" << endl;
    }
  }
  md_device = audio_driver_select;
  if (MikMod_Init((char*)(audio_driver_params->c_str()))) {
    audio_cat->error() << "Could not initialize the audio drivers.  '"
		       << MikMod_strerror(MikMod_errno) << "'" << endl;
    initialization_error = true;
    return;
  }
  if (audio_cat->is_debug()) {
    audio_cat->debug() << "driver info" << endl << MikMod_InfoDriver() << endl;
  }
  MikMod_SetNumVoices(-1, audio_sample_voices);
  AudioManager::set_update_func(update_mikmod);
  have_initialized = true;
}

MikModSample::MikModSample(SAMPLE* sample) : _sample(sample) {
}

MikModSample::~MikModSample(void) {
  Sample_Free(_sample);
}

float MikModSample::length(void) const {
  float len = _sample->length;
  float speed = _sample->speed;
  return len / speed;
}

AudioTraits::PlayingClass* MikModSample::get_state(void) const {
  return new MikModSamplePlaying((MikModSample*)this);
}

AudioTraits::PlayerClass* MikModSample::get_player(void) const {
  return MikModSamplePlayer::get_instance();
}

AudioTraits::DeleteSoundFunc* MikModSample::get_destroy(void) const {
  return MikModSample::destroy;
}

AudioTraits::DeletePlayingFunc* MikModSample::get_delstate(void) const {
  return MikModSamplePlaying::destroy;
}

MikModSample* MikModSample::load_wav(Filename filename) {
  initialize();
  SAMPLE* sample = Sample_Load((char*)(filename.c_str()));
  if (sample == (SAMPLE*)0L) {
    audio_cat->error() << "error loading sample '" << filename << "' because '"
		       << MikMod_strerror(MikMod_errno) << "'" << endl;
    return (MikModSample*)0L;
  }
  return new MikModSample(sample);
}

void MikModSample::destroy(AudioTraits::SoundClass* sample) {
  delete sample;
}

SAMPLE* MikModSample::get_sample(void) {
  return _sample;
}

int MikModSample::get_freq(void) {
  return _sample->speed;
}

MikModMusic::MikModMusic(void) {
}

MikModMusic::~MikModMusic(void) {
}

float MikModMusic::length(void) const {
  return -1.;
}

AudioTraits::PlayingClass* MikModMusic::get_state(void) const {
  return new MikModMusicPlaying((MikModMusic*)this);
}

AudioTraits::PlayerClass* MikModMusic::get_player(void) const {
  return MikModFmsynthPlayer::get_instance();
}

AudioTraits::DeleteSoundFunc* MikModMusic::get_destroy(void) const {
  return MikModMusic::destroy;
}

AudioTraits::DeletePlayingFunc* MikModMusic::get_delstate(void) const {
  return MikModMusicPlaying::destroy;
}

void MikModMusic::destroy(AudioTraits::SoundClass* music) {
  delete music;
}

MikModMidi::MikModMidi(void) {
}

MikModMidi::~MikModMidi(void) {
}

MikModMidi* MikModMidi::load_midi(Filename) {
  initialize();
  return new MikModMidi();
}

float MikModMidi::length(void) const {
  return -1.;
}

AudioTraits::PlayingClass* MikModMidi::get_state(void) const {
  return new MikModMidiPlaying((MikModMidi*)this);
}

AudioTraits::PlayerClass* MikModMidi::get_player(void) const {
  return MikModMidiPlayer::get_instance();
}

AudioTraits::DeleteSoundFunc* MikModMidi::get_destroy(void) const {
  return MikModMidi::destroy;
}

AudioTraits::DeletePlayingFunc* MikModMidi::get_delstate(void) const {
  return MikModMidiPlaying::destroy;
}

void MikModMidi::destroy(AudioTraits::SoundClass* music) {
  delete music;
}

MikModSamplePlaying::MikModSamplePlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
}

MikModSamplePlaying::~MikModSamplePlaying(void) {
}

AudioTraits::PlayingClass::PlayingStatus MikModSamplePlaying::status(void) {
  return AudioTraits::PlayingClass::BAD;
}

void MikModSamplePlaying::set_voice(int v) {
  _voice = v;
}

int MikModSamplePlaying::get_voice(void) const {
  return _voice;
}

void MikModSamplePlaying::destroy(AudioTraits::PlayingClass* play) {
  delete play;
}

MikModMusicPlaying::MikModMusicPlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
}

MikModMusicPlaying::~MikModMusicPlaying(void) {
}

AudioTraits::PlayingClass::PlayingStatus MikModMusicPlaying::status(void) {
  return AudioTraits::PlayingClass::BAD;
}

void MikModMusicPlaying::destroy(AudioTraits::PlayingClass* play) {
  delete play;
}

MikModMidiPlaying::MikModMidiPlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
}

MikModMidiPlaying::~MikModMidiPlaying(void) {
}

AudioTraits::PlayingClass::PlayingStatus MikModMidiPlaying::status(void) {
  return AudioTraits::PlayingClass::BAD;
}

void MikModMidiPlaying::destroy(AudioTraits::PlayingClass* play) {
  delete play;
}

MikModSamplePlayer* MikModSamplePlayer::_global_instance =
    (MikModSamplePlayer*)0L;

MikModSamplePlayer::MikModSamplePlayer(void) : AudioTraits::PlayerClass() {
}

MikModSamplePlayer::~MikModSamplePlayer(void) {
}

void MikModSamplePlayer::play_sound(AudioTraits::SoundClass* sample,
				    AudioTraits::PlayingClass* playing) {
  if (!have_initialized)
    initialize();
  if (!MikMod_Active()) {
    if (MikMod_EnableOutput()) {
      audio_cat->error() << "could not enable sample output '"
			 << MikMod_strerror(MikMod_errno) << "'" << endl;
    }
  }
  // cast to the correct type
  MikModSample* msample = (MikModSample*)sample;
  MikModSamplePlaying* mplay = (MikModSamplePlaying*)playing;
  // fire it off
  mplay->set_voice(Sample_Play(msample->get_sample(), 0, 0));
  Voice_SetFrequency(mplay->get_voice(), msample->get_freq());
  if (Voice_GetFrequency(mplay->get_voice()) != msample->get_freq())
    audio_cat->error() << "setting freq did not stick!" << endl;
  Voice_SetPanning(mplay->get_voice(), 127);
}

void MikModSamplePlayer::set_volume(AudioTraits::PlayingClass* state, int v) {
  initialize();
  MikModSamplePlaying* mplay = (MikModSamplePlaying*)state;
  Voice_SetVolume(mplay->get_voice(), v);
}

MikModSamplePlayer* MikModSamplePlayer::get_instance(void) {
  if (_global_instance == (MikModSamplePlayer*)0L)
    _global_instance = new MikModSamplePlayer();
  return _global_instance;
}

MikModFmsynthPlayer* MikModFmsynthPlayer::_global_instance =
  (MikModFmsynthPlayer*)0L;

MikModFmsynthPlayer::MikModFmsynthPlayer(void) {
}

MikModFmsynthPlayer::~MikModFmsynthPlayer(void) {
}

void MikModFmsynthPlayer::play_sound(AudioTraits::SoundClass*,
				     AudioTraits::PlayingClass*) {
  audio_cat->error() << "trying to play a sample with a MikModFmsynthPlayer"
		     << endl;
}

void MikModFmsynthPlayer::set_volume(AudioTraits::PlayingClass*, int) {
  audio_cat->error()
    << "trying to set volume on a sample with a MikModFmsynthPlayer" << endl;
}

MikModFmsynthPlayer* MikModFmsynthPlayer::get_instance(void) {
  if (_global_instance == (MikModFmsynthPlayer*)0L)
    _global_instance = new MikModFmsynthPlayer();
  return _global_instance;
}

MikModMidiPlayer* MikModMidiPlayer::_global_instance = (MikModMidiPlayer*)0L;

MikModMidiPlayer::MikModMidiPlayer(void) {
}

MikModMidiPlayer::~MikModMidiPlayer(void) {
}

void MikModMidiPlayer::play_sound(AudioTraits::SoundClass*,
				  AudioTraits::PlayingClass*) {
  audio_cat->error() << "trying to play a sample with a MikModMidiPlayer"
		     << endl;
}

void MikModMidiPlayer::set_volume(AudioTraits::PlayingClass*, int) {
  audio_cat->error()
    << "trying to set volume on a sample with a MikModMidiPlayer" << endl;
}

MikModMidiPlayer* MikModMidiPlayer::get_instance(void) {
  if (_global_instance == (MikModMidiPlayer*)0L)
    _global_instance = new MikModMidiPlayer();
  return _global_instance;
}

#endif /* AUDIO_USE_MIKMOD */
