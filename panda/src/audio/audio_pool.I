// Filename: audio_pool.I
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::has_sample
//       Access: Public, Static
//  Description: Returns true if the sample has ever been loaded,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AudioPool::has_sample(const string& filename) {
  return get_ptr()->ns_has_sample(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::verify_sample
//       Access: Public, Static
//  Description: Loads the given filename up into a sample, if it has
//               not already been loaded, and returns true to indicate
//               success, or false to indicate failure.  If this
//               returns true, it is guaranteed that a subsequent call
//               to load_sample() with the same sample name will
//               return a valid AudioSample pointer.
////////////////////////////////////////////////////////////////////
INLINE bool AudioPool::verify_sample(const string& filename) {
  return load_sample(filename) != (AudioSample*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::load_sample
//       Access: Public, Static
//  Description: Loads the given filename up into a sample, if it has
//               not already been loaded, and returns the new sample.
//               If a sample with the same filename was previously
//               loaded, returns that one instead.  If the sample
//               file cannot be found, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE AudioSample* AudioPool::load_sample(const string& filename) {
  return get_ptr()->ns_load_sample(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::release_sample
//       Access: Public, Static
//  Description: Removes the indicated sample from the pool,
//               indicating it will never be loaded again; the sample
//               may then be freed.  If this function is never called,
//               a reference count will be maintained on every sample
//               ever loaded, and samples will never be freed.
//
//               The sample's name should not have been changed
//               during its lifetime, or this function may fail to
//               locate it in the pool.
////////////////////////////////////////////////////////////////////
INLINE void AudioPool::release_sample(AudioSample* sample) {
  get_ptr()->ns_release_sample(sample);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::release_all_samples
//       Access: Public, Static
//  Description: Releases all samples in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void AudioPool::release_all_samples(void) {
  get_ptr()->ns_release_all_samples();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::has_music
//       Access: Public, Static
//  Description: Returns true if the music has ever been loaded,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool AudioPool::has_music(const string& filename) {
  return get_ptr()->ns_has_music(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::verify_music
//       Access: Public, Static
//  Description: Loads the given filename up into a music, if it has
//               not already been loaded, and returns true to indicate
//               success, or false to indicate failure.  If this
//               returns true, it is guaranteed that a subsequent call
//               to load_music() with the same music name will
//               return a valid AudioMusic pointer.
////////////////////////////////////////////////////////////////////
INLINE bool AudioPool::verify_music(const string& filename) {
  return load_music(filename) != (AudioMusic*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::load_music
//       Access: Public, Static
//  Description: Loads the given filename up into a music, if it has
//               not already been loaded, and returns the new music.
//               If a music with the same filename was previously
//               loaded, returns that one instead.  If the music
//               file cannot be found, returns NULL.
////////////////////////////////////////////////////////////////////
INLINE AudioMusic* AudioPool::load_music(const string& filename) {
  return get_ptr()->ns_load_music(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::release_music
//       Access: Public, Static
//  Description: Removes the indicated music from the pool,
//               indicating it will never be loaded again; the music
//               may then be freed.  If this function is never called,
//               a reference count will be maintained on every music
//               ever loaded, and music will never be freed.
//
//               The music's name should not have been changed
//               during its lifetime, or this function may fail to
//               locate it in the pool.
////////////////////////////////////////////////////////////////////
INLINE void AudioPool::release_music(AudioMusic* music) {
  get_ptr()->ns_release_music(music);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::release_all_music
//       Access: Public, Static
//  Description: Releases all music in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void AudioPool::release_all_music(void) {
  get_ptr()->ns_release_all_music();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioPool::Constructor
//       Access: Private
//  Description: The constructor is not intended to be called
//               directly; there's only supposed to be one AudioPool
//               in the universe and it constructs itself.
////////////////////////////////////////////////////////////////////
INLINE AudioPool::AudioPool(void) {}
