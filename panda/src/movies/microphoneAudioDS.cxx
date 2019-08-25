/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file microphoneAudioDS.cxx
 * @author jyelon
 * @date 2007-11-01
 *
 * It goes against Panda3D coding style conventions to hide an
 * entire class in a C++ file and not expose it through header
 * files at all.  However, in this case, these classes are so full
 * of OS-specific junk that I feel it is better to hide them
 * entirely.  - Josh
 */

#ifdef HAVE_DIRECTCAM

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#undef Configure

#include <windows.h>
#include <mmsystem.h>

/**
 * The directshow implementation of microphones.
 */
class MicrophoneAudioDS : public MicrophoneAudio {
public:
  static void find_all_microphones_ds();
  friend void find_all_microphones_ds();

private:
  virtual PT(MovieAudioCursor) open();

  int _device_id;
  int _manufacturer_id;
  int _product_id;

  struct AudioBuf {
    HGLOBAL   _storage_gh;
    HGLOBAL   _header_gh;
    LPSTR     _storage;
    LPWAVEHDR _header;
  };
  typedef pvector <AudioBuf> AudioBuffers;

  static void delete_buffers(AudioBuffers &buffers);

  friend class MicrophoneAudioCursorDS;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MicrophoneAudio::init_type();
    register_type(_type_handle, "MicrophoneAudioDS",
                  MicrophoneAudio::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

TypeHandle MicrophoneAudioDS::_type_handle;

/**
 * The directshow implementation of microphones.
 */

class MicrophoneAudioCursorDS : public MovieAudioCursor
{
public:
  typedef MicrophoneAudioDS::AudioBuffers AudioBuffers;
  MicrophoneAudioCursorDS(MicrophoneAudioDS *src, AudioBuffers &bufs, HWAVEIN hwav);
  virtual ~MicrophoneAudioCursorDS();

  AudioBuffers _buffers;
  HWAVEIN _hwavein;
  int _samples_per_buffer;

public:
  virtual int read_samples(int n, int16_t *data);
  virtual int ready() const;

public:
  void cleanup();

  HWAVEIN _handle;
  int     _next;    // Which buffer is the next one to read from.
  int     _offset;  // How many samples to skip in the buffer.

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieAudioCursor::init_type();
    register_type(_type_handle, "MicrophoneAudioCursorDS",
                  MovieAudioCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

TypeHandle MicrophoneAudioCursorDS::_type_handle;

/**
 * Finds all DirectShow microphones and adds them to the global list
 * _all_microphones.
 */
void MicrophoneAudioDS::
find_all_microphones_ds() {
  MMRESULT stat;
  static int freqs[] = { 11025, 22050, 44100, 48000, 0 };

  int ndevs = waveInGetNumDevs();
  for (int i=0; i<ndevs; i++) {
    WAVEINCAPS caps;
    stat = waveInGetDevCaps(i, &caps, sizeof(caps));
    if (stat != MMSYSERR_NOERROR) continue;
    for (int chan=1; chan<=2; chan++) {
      for (int fselect=0; freqs[fselect]; fselect++) {
        WAVEFORMATEX format;
        int freq = freqs[fselect];
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = chan;
        format.nSamplesPerSec = freq;
        format.nAvgBytesPerSec = freq * chan * 2;
        format.nBlockAlign = 2 * chan;
        format.wBitsPerSample = 16;
        format.cbSize = 0;
        stat = waveInOpen(nullptr, i, &format, 0, 0, WAVE_FORMAT_QUERY);
        if (stat == MMSYSERR_NOERROR) {
          PT(MicrophoneAudioDS) p = new MicrophoneAudioDS();
          std::ostringstream name;
          name << "WaveIn: " << caps.szPname << " Chan:" << chan << " HZ:" << freq;
          p->set_name(name.str());
          p->_device_id = i;
          p->_manufacturer_id = caps.wMid;
          p->_product_id = caps.wPid;
          p->_rate = freq;
          p->_channels = chan;
          _all_microphones.push_back((MicrophoneAudioDS*)p);
        }
      }
    }
  }
}

void find_all_microphones_ds() {
  MicrophoneAudioDS::init_type();
  // MicrophoneAudioCursorDS::init_type();
  MicrophoneAudioDS::find_all_microphones_ds();
}

/**
 * Delete a set of audio buffers.
 */
void MicrophoneAudioDS::
delete_buffers(AudioBuffers &buffers) {
  for (int i=0; i<(int)buffers.size(); i++) {
    AudioBuf &buf = buffers[i];
    if (buf._header_gh) {
      GlobalUnlock(buf._header_gh);
      GlobalFree(buf._header_gh);
    }
    if (buf._storage_gh) {
      GlobalUnlock(buf._storage_gh);
      GlobalFree(buf._storage_gh);
    }
  }
  buffers.clear();
}

/**
 * Open this video, returning a MovieVideoCursor.
 */
PT(MovieAudioCursor) MicrophoneAudioDS::
open() {

  // Allocate the buffers.  64 buffers, not quite 120 sec each.
  int samples;
  switch (_rate) {
  case 11025: samples=512; break;
  case 22050: samples=1024; break;
  case 44100: samples=2048; break;
  }
  int bytes = _channels * samples * 2;

  bool failed = false;
  AudioBuffers buffers;
  for (int i=0; i<64; i++) {
    AudioBuf buf;
    buf._storage_gh = 0;
    buf._header_gh = 0;
    buf._storage = 0;
    buf._header = 0;

    buf._storage_gh = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, bytes);
    buf._header_gh = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
    if (buf._storage_gh != 0) {
      buf._storage = (LPSTR)GlobalLock(buf._storage_gh);
    }
    if (buf._header_gh != 0) {
      buf._header = (LPWAVEHDR)GlobalLock(buf._header_gh);
    }
    if (buf._storage && buf._header) {
      ZeroMemory(buf._header, sizeof(WAVEHDR));
      buf._header->lpData = buf._storage;
      buf._header->dwBufferLength = bytes;
    } else {
      failed = true;
    }
    buffers.push_back(buf);
    if (failed) break;
  }

  if (failed) {
    delete_buffers(buffers);
    nassert_raise("Could not allocate audio input buffers.");
    return nullptr;
  }

  WAVEFORMATEX format;
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = _channels;
  format.nSamplesPerSec = _rate;
  format.nAvgBytesPerSec = _rate * _channels * 2;
  format.nBlockAlign = 2 * _channels;
  format.wBitsPerSample = 16;
  format.cbSize = 0;

  HWAVEIN hwav;
  MMRESULT stat = waveInOpen(&hwav, _device_id, &format, 0, 0, CALLBACK_NULL);

  if (stat != MMSYSERR_NOERROR) {
    delete_buffers(buffers);
    nassert_raise("Could not open audio input device.");
    return nullptr;
  }

  for (int i=0; i<(int)buffers.size(); i++) {
    stat = waveInPrepareHeader(hwav, buffers[i]._header, sizeof(WAVEHDR));
    if (stat == MMSYSERR_NOERROR) {
      stat = waveInAddBuffer(hwav, buffers[i]._header, sizeof(WAVEHDR));
    }
    if (stat != MMSYSERR_NOERROR) {
      waveInClose(hwav);
      delete_buffers(buffers);
      nassert_raise("Could not queue buffers for audio input device.");
      return nullptr;
    }
  }
  stat = waveInStart(hwav);
  if (stat != MMSYSERR_NOERROR) {
    waveInClose(hwav);
    delete_buffers(buffers);
    nassert_raise("Could not start recording on input device.");
    return nullptr;
  }
  return new MicrophoneAudioCursorDS(this, buffers, hwav);
}

/**
 *
 */
MicrophoneAudioCursorDS::
MicrophoneAudioCursorDS(MicrophoneAudioDS *src, AudioBuffers &bufs, HWAVEIN hwav) :
  MovieAudioCursor(src),
  _buffers(bufs),
  _handle(hwav),
  _next(0),
  _offset(0)
{
  _audio_rate = src->get_rate();
  _audio_channels = src->get_channels();
  _length = 1.0E10;
  _can_seek = false;
  _can_seek_fast = false;
  _aborted = false;
  _samples_per_buffer = bufs[0]._header->dwBufferLength / (2 * _audio_channels);
}

/**
 *
 */
void MicrophoneAudioCursorDS::
cleanup() {
  if (_handle) {
    waveInClose(_handle);
    _handle = 0;
  }
  MicrophoneAudioDS::delete_buffers(_buffers);
  _next = 0;
  _offset = 0;
}

/**
 *
 */
MicrophoneAudioCursorDS::
~MicrophoneAudioCursorDS() {
  cleanup();
}

/**
 *
 */
int MicrophoneAudioCursorDS::
read_samples(int n, int16_t *data) {
  int orign = n;
  if (_handle) {
    while (1) {
      int index = _next % _buffers.size();
      if ((_buffers[index]._header->dwFlags & WHDR_DONE)==0) {
        break;
      }

      // Find start of data in buffer.
      int16_t *src = (int16_t*)(_buffers[index]._storage);
      src += (_offset * _audio_channels);

      // Decide how many samples to extract from this buffer.
      int samples = _samples_per_buffer;
      samples -= _offset;
      if (samples > n) samples = n;

      // Copy data to output buffer.
      memcpy(data, src, samples * 2 * _audio_channels);

      // Advance pointers.
      data += samples * _audio_channels;
      n -= samples;
      _offset += samples;
      _samples_read += samples;
      if (_offset != _samples_per_buffer) {
        break;
      }
      _buffers[index]._header->dwFlags &= ~(WHDR_DONE);
      MMRESULT stat = waveInUnprepareHeader(_handle, _buffers[index]._header, sizeof(WAVEHDR));
      if (stat == MMSYSERR_NOERROR) {
        stat = waveInPrepareHeader(_handle, _buffers[index]._header, sizeof(WAVEHDR));
      }
      if (stat == MMSYSERR_NOERROR) {
        stat = waveInAddBuffer(_handle, _buffers[index]._header, sizeof(WAVEHDR));
      }
      if (stat != MMSYSERR_NOERROR) {
        movies_cat.error() << "Could not requeue audio buffers, closing microphone.\n";
        cleanup();
        break;
      }
      _next += 1;
      _offset = 0;
    }
  }
  if (n > 0) {
    memcpy(data, 0, n*2*_audio_channels);
  }
  return orign - n;
}

/**
 *
 */
int MicrophoneAudioCursorDS::
ready() const {
  if (_handle == 0) return 0;
  int total = 0;
  for (int i=0; i<(int)_buffers.size(); i++) {
    int index = (_next + i) % (_buffers.size());
    if ((_buffers[index]._header->dwFlags & WHDR_DONE)==0) {
      break;
    }
    total += _samples_per_buffer;
  }
  total -= _offset;
  return total;
}


#endif // HAVE_DIRECTSHOW
