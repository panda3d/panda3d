// Filename: audio_load_midi.cxx
// Created by:  cary (26Sep00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"

Configure(audio_load_midi);

#ifdef USE_MIKMOD

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

#else

#ifdef PENV_WIN32

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
#else

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

#endif /* win32 */
#endif /* mikmod */

ConfigureFn(audio_load_midi) {
  AudioPool::register_music_loader("midi", AudioLoadMidi);
  AudioPool::register_music_loader("mid", AudioLoadMidi);
}
