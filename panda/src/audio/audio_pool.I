// Filename: audio_pool.I
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::has_sound
//       Access: Public, Static
//  Description: Returns true if the sound has ever been loaded,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AudioPool::has_sound(const string& filename) {
  return get_ptr()->ns_has_sound(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::verify_sound
//       Access: Public, Static
//  Description: Loads the given filename up into a sound, if it has
//               not already been loaded, and returns true to indicate
//               success, or false to indicate failure.  If this
//               returns true, it is guaranteed that a subsequent call
//               to load_sound() with the same sound name will
//               return a valid AudioSound pointer.
////////////////////////////////////////////////////////////////////
INLINE bool AudioPool::verify_sound(const string& filename) {
  PT(AudioSound) foo = load_sound(filename);
  return !(foo.is_null());
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::load_sound
//       Access: Public, Static
//  Description: Loads the given filename up into a sound, if it has
//               not already been loaded, and returns the new sound.
//               If a sound with the same filename was previously
//               loaded, returns that one instead.  If the sound
//               file cannot be found, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE AudioSound* AudioPool::load_sound(const string& filename) {
  if (audio_is_active)
    return get_ptr()->ns_load_sound(filename);
  return (AudioSound*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::release_sound
//       Access: Public, Static
//  Description: Removes the indicated sound from the pool,
//               indicating it will never be loaded again; the sound
//               may then be freed.  If this function is never called,
//               a reference count will be maintained on every sound
//               ever loaded, and sounds will never be freed.
//
//               The sound's name should not have been changed
//               during its lifetime, or this function may fail to
//               locate it in the pool.
////////////////////////////////////////////////////////////////////
INLINE void AudioPool::release_sound(AudioSound* sound) {
  get_ptr()->ns_release_sound(sound);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::release_all_sounds
//       Access: Public, Static
//  Description: Releases all sounds in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void AudioPool::release_all_sounds(void) {
  get_ptr()->ns_release_all_sounds();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one AudioPool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE AudioPool::AudioPool(void) {}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::get_num_loaded_sounds
//       Access: Public, static
//  Description: return the number of sounds the audiopool thinks
//               are loaded at the moment
////////////////////////////////////////////////////////////////////
INLINE int AudioPool::get_num_loaded_sounds(void) {
  return get_ptr()->_sounds.size();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::get_nth_sound_name
//       Access: Public, static
//  Description: return the name of the nth loaded  sound
////////////////////////////////////////////////////////////////////
INLINE string AudioPool::get_nth_sound_name(int n) {
  return get_ptr()->ns_get_nth_sound_name(n);
}
