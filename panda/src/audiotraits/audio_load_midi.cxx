// Filename: audio_load_midi.cxx
// Created by:  cary (26Sep00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"
#include "audio_trait.h"

Configure(audio_load_midi);

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

void AudioDestroyMidi(AudioTraits::MusicClass* music) {
  MikModMidi::destroy(music);
}

void AudioLoadMidi(AudioTraits::MusicClass** music,
		   AudioTraits::PlayerClass** player,
		   AudioTraits::DeleteMusicFunc** destroy, Filename filename) {
  *music = MikModMidi::load_midi(filename);
  if (*music == (AudioTraits::MusicClass*)0L)
    return;
  *player = MikModMidiPlayer::get_instance();
  *destroy = AudioDestroyMidi;
}

#else /* AUDIO_USE_MIKMOD */

#ifdef AUDIO_USE_WIN32

#include "audio_win_traits.h"

void AudioDestroyMidi(AudioTraits::MusicClass* music) {
  WinMusic::destroy(music);
}

void AudioLoadMidi(AudioTraits::MusicClass** music,
		   AudioTraits::PlayerClass** player,
		   AudioTraits::DeleteMusicFunc** destroy, Filename filename) {
  *music = WinMusic::load_midi(filename);
  if (*music == (AudioTraits::MusicClass*)0L)
    return;
  *player = WinPlayer::get_instance();
  *destroy = AudioDestroyMidi;
  audio_cat->debug() << "sanity check: music = " << (void*)*music
		     << "  player = " << (void*)*player << "  destroy = "
		     << (void*)*destroy << endl;
}
#else /* AUDIO_USE_WIN32 */

#ifdef AUDIO_USE_LINUX

void AudioDestroyMidi(AudioTraits::MusicClass* music) {
  delete music;
}

void AudioLoadMidi(AudioTraits::MusicClass** music,
		   AudioTraits::PlayerClass** player,
		   AudioTraits::DeleteMusicFunc** destroy, Filename) {
  audio_cat->warning() << "linux doesn't support reading midi data yet"
		       << endl;
  *music = (AudioTraits::MusicClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroyMidi;
}

#else /* AUDIO_USE_LINUX */

#ifdef AUDIO_USE_NULL

// Null driver
#include "audio_null_traits.h"

void AudioDestroyMidi(AudioTraits::MusicClass* music) {
  delete music;
}

void AudioLoadMidi(AudioTraits::MusicClass** music,
		   AudioTraits::PlayerClass** player,
		   AudioTraits::DeleteMusicFunc** destroy, Filename) {
  *music = new NullMusic();
  *player = new NullPlayer();
  *destroy = AudioDestroyMidi;
}

#else /* AUDIO_USE_NULL */

#error "unknown driver type"

#endif /* AUDIO_USE_NULL */
#endif /* AUDIO_USE_LINUX */
#endif /* AUDIO_USE_WIN32 */
#endif /* AUDIO_USE_MIKMOD */

ConfigureFn(audio_load_midi) {
  AudioPool::register_music_loader("midi", AudioLoadMidi);
  AudioPool::register_music_loader("mid", AudioLoadMidi);
}
