include header and static lib / build system integration

MaAudioManager:
- init
- cacheing/uncacheing samples and sounds (hashmaps/sets from `pmap.h`)
- playing sounds, looping or ending
- modifying sounds
- attaching sounds to nodepaths
- removing sounds
- clean close
- offer TypeHandle

### Crucial engine tools
- `SoundData` and `MovieAudio` objects have refcounts.
- `SoundsPlaying::iterator` handles allocation and deallocation safely.
- `ExpirationQueue::iterator`

#### Other tools to note / concepts involved
- `nassertv` - assert a condition
- `ReMutexHolder` - mutex
- `phash_map<std::string, SoundData *>` - fast table (hashmap) of sample data
- `phash_set<PT(MaAudioSound)>` - fast set of sounds
- `pset<MaAudioManager *>` - fast set of audio managers
- `plist<void *>` - fast list of pointers
- `vector_string` -
- 'friend classes'
- iterators
- inheritance
- bit masks
- heap allocation / memory management

