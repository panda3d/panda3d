// Filename: audio_win_traits.cxx
// Created by:  cary (27Sep00)
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

#include "audio_win_traits.h"

#ifdef AUDIO_USE_WIN32

#include "audio_manager.h"
#include "config_audio.h"
#include <config_util.h>

#include <direct.h>

static bool have_initialized = false;
static HWND global_hwnd;

// these are used by the direct sound playing stuff
static LPDIRECTSOUNDBUFFER soundPrimaryBuffer = NULL;
static LPDIRECTSOUND soundDirectSound = NULL;

// these are used by the direct music playing stuff
static IDirectMusic* musicDirectMusic = NULL;
static IDirectSound* musicDirectSound = NULL;

#define CHECK_RESULT(_result, _msg) \
  if (FAILED(_result)) { \
    audio_cat->error() << _msg << " at " << __FILE__ << ":" << __LINE__ \
                       << endl; \
    audio_is_active = false; \
    return; \
  } else {}

#define CHECK_RESULT_SFX(_result, _msg) \
  if (FAILED(_result)) { \
    audio_cat->error() << _msg << " at " << __FILE__ << ":" << __LINE__ \
                       << endl; \
    AudioManager::set_hard_sfx_active(false); \
    return; \
  } else {}

#define CHECK_RESULT_MUSIC(_result, _msg) \
  if (FAILED(_result)) { \
    audio_cat->error() << _msg << " at " << __FILE__ << ":" << __LINE__ \
                       << endl; \
    AudioManager::set_hard_music_active(false); \
    return; \
  } else {}

// #define MULTI_TO_WIDE(_in, _out) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _in, -1, _out, DMUS_MAX_FILENAME)
#define MULTI_TO_WIDE(x,y) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH);


static void update_win() {
}

static void initialize() {
  if (have_initialized) {
    return;
  }
  if (!audio_is_active) {
    return;
  }

  audio_debug("in winAudio initialize");

  // rumor has it this will work, if it doesn't we need to create an invisible
  // application window for this kind of thing
  global_hwnd = GetDesktopWindow();

  // initialize COM
  HRESULT result = CoInitialize(NULL);
  CHECK_RESULT(result, "CoInitialize failed");

  //
  // initialize the globals for the direct sound drivers
  //

  // create a direct sound object
  result = DirectSoundCreate(NULL, &soundDirectSound, NULL);
  CHECK_RESULT_SFX(result, "could not create a Direct Sound (tm) object (c)");

  // set the cooperative level
  result = soundDirectSound->SetCooperativeLevel(global_hwnd, DSSCL_PRIORITY);
  if (FAILED(result)) {
    audio_warning("could not set Direct Sound co-op level to "
        << "DSSCL_PRIORITY, trying DSSCL_NORMAL");
    result = soundDirectSound->SetCooperativeLevel(global_hwnd, DSSCL_NORMAL);
    CHECK_RESULT_SFX(result, "failed setting to DSSCL_NORMAL");
  }

  // Move any unused portions of onboard sound memory to a contiguous block
  // so that the largest portion of free memory will be available.
  soundDirectSound->Compact();

  // create the primary buffer
  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize  =  sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
  result = soundDirectSound->CreateSoundBuffer(&dsbd, 
      &soundPrimaryBuffer, NULL);
  CHECK_RESULT_SFX(result, "could not create primary buffer");

  // set primary buffer format to 22kHz and 16-bit output
  // COME BACK LATER TO MAKE THIS CONFIG
  WAVEFORMATEX wfx;
  ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
  wfx.wFormatTag      = WAVE_FORMAT_PCM;
  wfx.nChannels       = 2;
  wfx.nSamplesPerSec  = audio_mix_freq;
  wfx.wBitsPerSample  = 16;
  wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
  result = soundPrimaryBuffer->SetFormat(&wfx);
  // SetFormat requires at least DSSCL_PRIORITY, which we may not have
  if (result == DSERR_PRIOLEVELNEEDED) {
    audio_warning("could not set format of Primary Buffer because "
        << "we didn't get DSSCL_PRIORITY");
  }

/*
  //
  // initialize the globals for the direct music drivers
  //

  // create the direct sound object
  result = DirectSoundCreate(NULL, &musicDirectSound, NULL);
  CHECK_RESULT(result,
           "could not create a second Direct Sound (tm) object (c)");

  // set the cooperative level
  result = musicDirectSound->SetCooperativeLevel(global_hwnd, DSSCL_PRIORITY);
  if (FAILED(result)) {
    audio_cat->warning() << "could not set Direct Sound (2) co-op level to "
             << "DSSCL_PRIORITY, trying DSSCL_NORMAL" << endl;
    result = musicDirectSound->SetCooperativeLevel(global_hwnd, DSSCL_NORMAL);
    CHECK_RESULT(result, "failed setting to DSSCL_NORMAL");
  }

  // create the direct music object
  result = CoCreateInstance(CLSID_DirectMusic, NULL, CLSCTX_INPROC,
                IID_IDirectMusic, (void**)&musicDirectMusic);
  CHECK_RESULT(result, "could not create Direct Music (tm) object (c)");

  // set direct sound for direct music
  result = musicDirectMusic->SetDirectSound(musicDirectSound, NULL);
  CHECK_RESULT(result, "could not add Direct Sound (tm) to Direct Music (tm)");
*/

  //
  // finish out with our stuff
  //

  AudioManager::set_update_func(update_win);
  have_initialized = true;
  audio_debug("out of winAudio initialize");
}

static void shutdown() {
  audio_debug("in winaudio shutdown");

  // release the primary sound buffer
  if (soundPrimaryBuffer) {
    soundPrimaryBuffer->Release();
    soundPrimaryBuffer = NULL;
  }

  // release direct sound object
  if (soundDirectSound) {
    soundDirectSound->Release();
    soundDirectSound = NULL;
  }

  // release direct music object
  if (musicDirectMusic) {
    musicDirectMusic->Release();
    musicDirectMusic = NULL;
  }

  if (musicDirectSound) {
    musicDirectSound->Release();
    musicDirectSound = NULL;
  }

  // shutdown COM
  CoUninitialize();
  audio_debug("out of winaudio shutdown");
}

WinSample::~WinSample() {
  // we may or may not be leaking the _data
  audio_debug("winsample destructor called");
}

float WinSample::length() const {
  audio_debug("winsample length called");
  return _len / (audio_mix_freq * 2. * 2.);
}

AudioTraits::PlayingClass* WinSample::get_state() const {
  WinSamplePlaying* ret = new WinSamplePlaying((WinSample*)this);
  audio_debug("winsample get_state returning 0x" << (void*)ret);
  return ret;
}

AudioTraits::PlayerClass* WinSample::get_player() const {
  AudioTraits::PlayerClass* ret = WinSamplePlayer::get_instance();
  audio_debug("winsample get_player returning 0x" << (void*)ret);
  return ret;
}

AudioTraits::DeletePlayingFunc* WinSample::get_delstate() const {
  audio_debug("winsample get_delstate returning 0x"
               << (void*)(WinSamplePlaying::destroy));
  return WinSamplePlaying::destroy;
}

// these are used by the wav loader
WAVEFORMATEX* pwfx;
HMMIO hmmioIn;
MMCKINFO ckIn;
MMCKINFO ckInRiff;

HRESULT readMMIO(HMMIO hmmio, MMCKINFO* pckInRIFF, WAVEFORMATEX** ppwfxInfo) {
  MMCKINFO ckin;
  PCMWAVEFORMAT pcmWaveFormat;

  *ppwfxInfo = NULL;
  if (mmioDescend(hmmio, pckInRIFF, NULL, 0) != 0)
    return E_FAIL;
  if ((pckInRIFF->ckid != FOURCC_RIFF) ||
      (mmioFOURCC('W', 'A', 'V', 'E') != pckInRIFF->fccType))
    return E_FAIL;
  ckin.ckid = mmioFOURCC('f', 'm', 't', ' ');
  if (mmioDescend(hmmio, &ckin, pckInRIFF, MMIO_FINDCHUNK) != 0)
    return E_FAIL;
  if (ckin.cksize < (LONG)sizeof(PCMWAVEFORMAT))
    return E_FAIL;
  if (mmioRead(hmmio, (HPSTR)&pcmWaveFormat, sizeof(pcmWaveFormat)) !=
      sizeof(pcmWaveFormat))
    return E_FAIL;
  if (pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM) {
    if ((*ppwfxInfo = new WAVEFORMATEX) == NULL)
      return E_FAIL;
    memcpy(*ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat));
    (*ppwfxInfo)->cbSize = 0;
  } else {
    WORD cbExtraBytes = 0L;
    if (mmioRead(hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD))
      return E_FAIL;
    *ppwfxInfo = (WAVEFORMATEX*)new CHAR[sizeof(WAVEFORMATEX)+cbExtraBytes];
    if (*ppwfxInfo == NULL) {
      return E_FAIL;
    }
    memcpy(*ppwfxInfo, &pcmWaveFormat, sizeof(pcmWaveFormat));
    (*ppwfxInfo)->cbSize = cbExtraBytes;
    if (mmioRead(hmmio, (CHAR*)(((BYTE*)&((*ppwfxInfo)->cbSize))+sizeof(WORD)),
         cbExtraBytes) != cbExtraBytes) {
      delete *ppwfxInfo;
      *ppwfxInfo = NULL;
      return E_FAIL;
    }
  }
  if (mmioAscend(hmmio, &ckin, 0) != 0) {
    delete *ppwfxInfo;
    *ppwfxInfo = NULL;
    return E_FAIL;
  }
  return S_OK;
}

HRESULT wave_open_file(const CHAR* strFileName, HMMIO* phmmioIn,
               WAVEFORMATEX** ppwfxInfo, MMCKINFO* pckInRIFF) {
  HMMIO hmmio = NULL;
  if ((hmmio = mmioOpen(const_cast<CHAR*>(strFileName), NULL,
            MMIO_ALLOCBUF | MMIO_READ)) == NULL)
    return E_FAIL;
  HRESULT hr;
  if (FAILED(hr = readMMIO(hmmio, pckInRIFF, ppwfxInfo))) {
    mmioClose(hmmio, 0);
    return hr;
  }
  *phmmioIn = hmmio;
  return S_OK;
}

HRESULT wave_start_data_read(HMMIO* phmmioIn, MMCKINFO* pckIn,
                 MMCKINFO* pckInRIFF) {
  // seek to the data
  if (mmioSeek(*phmmioIn, pckInRIFF->dwDataOffset + sizeof(FOURCC),
           SEEK_SET) == -1)
    return E_FAIL;
  //search the input file for the 'data' chunk
  pckIn->ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (mmioDescend(*phmmioIn, pckIn, pckInRIFF, MMIO_FINDCHUNK) != 0)
    return E_FAIL;
  return S_OK;
}

HRESULT wave_read_file(HMMIO hmmio, UINT cbRead, BYTE* pbDest, MMCKINFO* pckIn,
               UINT* cbActualRead) {
  MMIOINFO mmioinfoIn;
  *cbActualRead = 0;
  if (mmioGetInfo(hmmio, &mmioinfoIn, 0) != 0)
    return E_FAIL;
  UINT cbDataIn = cbRead;
  if (cbDataIn > pckIn->cksize) {
    cbDataIn = pckIn->cksize;
  }
  for (DWORD cT=0; cT<cbDataIn; ++cT) {
    // copy bytes from the io to the buffer
    if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead) {
      if (mmioAdvance(hmmio, &mmioinfoIn, MMIO_READ) != 0) {
        return E_FAIL;
      }
      if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead) {
        return E_FAIL;
      }
    }
    // actual copy
    *((BYTE*)pbDest+cT) = *((BYTE*)mmioinfoIn.pchNext);
    mmioinfoIn.pchNext++;
  }
  if (mmioSetInfo(hmmio, &mmioinfoIn, 0) != 0) {
    return E_FAIL;
  }
  *cbActualRead = cbDataIn;
  return S_OK;
}

HRESULT wave_load_internal(const CHAR* filename, WAVEFORMATEX& wavInfo,
               BYTE*& wavData, UINT& wavSize) {
  wavData = NULL;
  wavSize = 0;
  HRESULT result = wave_open_file(filename, &hmmioIn, &pwfx, &ckInRiff);
  if (FAILED(result)) {
    return result;
  }
  result = wave_start_data_read(&hmmioIn, &ckIn, &ckInRiff);
  if (SUCCEEDED(result)) {
    memcpy(&wavInfo, pwfx, sizeof(WAVEFORMATEX));
    DWORD size = ckIn.cksize + ckIn.dwDataOffset;
    wavData = new BYTE[size];
    result = wave_read_file(hmmioIn, size, wavData, &ckIn, &wavSize);
  }
  mmioClose(hmmioIn, 0);
  return result;
}

HRESULT wave_load(const CHAR* filename, WAVEFORMATEX& wavInfo, BYTE*& wavData,
          UINT& wavSize) {
  pwfx = NULL;
  HRESULT result = wave_load_internal(filename, wavInfo, wavData, wavSize);
  delete pwfx;
  pwfx = NULL;
  return result;
}

WinSample* WinSample::load_wav(Filename filename) {
  audio_debug("in winsample load_wav");
  initialize();
  if (!audio_is_active) {
    return 0;
  }
  // Load the wave:
  WAVEFORMATEX wavInfo;
  UINT wavSize = 0;
  BYTE* wavData = NULL;
  string stmp = filename.to_os_specific();
  HRESULT result = wave_load(stmp.c_str(), wavInfo, wavData, wavSize);
  if (FAILED(result)) {
    delete [] wavData;
    audio_debug("wave_load failed, returning NULL");
    return 0;
  }
  // Create the sample:
  WinSample* ret = new WinSample();
  nassertr(ret, 0); // new should have thrown.
  memcpy(&(ret->_info), &wavInfo, sizeof(WAVEFORMATEX));
  ret->_data = wavData;
  ret->_len = wavSize;
  audio_debug("returning 0x" << (void*)ret);
  return ret;
}

WinSample* WinSample::load_raw(unsigned char* data, unsigned long size) {
  audio_debug("in winsample load_raw");
  initialize();
  if (!audio_is_active) {
    return 0;
  }
  if (!data) {
    audio_debug("data is null, returning same");
    return 0;
  }

  // synth a wav header for this data
  WAVEFORMATEX wavInfo;
  ZeroMemory(&wavInfo, sizeof(WAVEFORMATEX));
  wavInfo.wFormatTag = WAVE_FORMAT_PCM;
  wavInfo.nChannels = 2;
  wavInfo.nSamplesPerSec = audio_mix_freq;
  wavInfo.wBitsPerSample = 16;
  wavInfo.nBlockAlign = wavInfo.wBitsPerSample / 8 * wavInfo.nChannels;
  wavInfo.nAvgBytesPerSec = wavInfo.nSamplesPerSec * wavInfo.nBlockAlign;

  // create a direct sound channel for this data
  WinSample* ret = new WinSample();
  nassertr(ret, 0);
  memcpy(&(ret->_info), &wavInfo, sizeof(WAVEFORMATEX));
  ret->_data = data;
  ret->_len = size;
  audio_debug("WinSample::load_raw returning 0x" << (void*)ret);
  return ret;
}

WinMusic::~WinMusic() {
  // AudioManager::stop(this);
  audio_debug("in WinMusic::~WinMusic()");

  if (_music) {
    _music->Release();
    _music = NULL;
  }

  if (_synth) {
    _synth->Release();
    _synth = NULL;
  }

  if (_performance) {
    _performance->CloseDown();
    _performance->Release();
    _performance = NULL;
  }

  if (_buffer) {
    _buffer->Release();
    _buffer = NULL;
  }

  if (audio_cat.is_debug()) {
    audio_cat->debug() << "out of WinMusic::~WinMusic()" << endl;
  }
}

void WinMusic::init() {
  if (audio_cat.is_debug()) {
    audio_cat->debug() << "in WinMusic::init()" << endl;
  }

  initialize();
  // create the direct sound performance object
  HRESULT result = CoCreateInstance(CLSID_DirectMusicPerformance, NULL,
                    CLSCTX_INPROC,
                    IID_IDirectMusicPerformance2,
                    (void**)&_performance);
  if (FAILED(result)) {
    audio_cat->error() << "could not create performance object" << endl;
    _performance = NULL;
    return;
  }

  // initialize performance object
  // result = _performance->Init(&musicDirectMusic, NULL, NULL);
  result = _performance->Init(NULL, NULL, NULL);
  CHECK_RESULT_MUSIC(result, "could not initialize performance object");

/*
  // create the output synth object
  DMUS_PORTPARAMS params;
  ZeroMemory(&params, sizeof(DMUS_PORTPARAMS));
  params.dwSize = sizeof(DMUS_PORTPARAMS);
  result = musicDirectMusic->CreatePort(GUID_NULL, &params, &_synth, NULL);
  CHECK_RESULT(result, "could not create synth");

  // create sound buffer
  WAVEFORMATEX format;
  DWORD formatExSize;
  DWORD bufferSize;
  ZeroMemory(&format, sizeof(WAVEFORMATEX));
  formatExSize = format.cbSize = sizeof(WAVEFORMATEX);
  result = _synth->GetFormat(&format, &formatExSize, &bufferSize);
  CHECK_RESULT(result, "failed to get format from synth");
  DSBUFFERDESC bufferDesc;
  ZeroMemory(&bufferDesc, sizeof(DSBUFFERDESC));
  bufferDesc.dwSize = sizeof(DSBUFFERDESC);
*
  bufferDesc.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_STICKYFOCUS;
*
  bufferDesc.dwFlags = DSBCAPS_STICKYFOCUS;
  bufferDesc.dwBufferBytes = bufferSize;
  bufferDesc.lpwfxFormat = &format;
  bufferDesc.lpwfxFormat->cbSize = sizeof(WAVEFORMATEX);
  result = musicDirectSound->CreateSoundBuffer(&bufferDesc, &_buffer, NULL);
  CHECK_RESULT(result, "could not create buffer for music");

  // initialize synth
  result = _synth->SetDirectSound(musicDirectSound, _buffer);
  CHECK_RESULT(result, "failed to initialize synth");

  // activate synth
  result = _synth->Activate(TRUE);
  CHECK_RESULT(result, "failed to activate synth");
*/

  // add the synth to the performance
  // result = _performance->AddPort(_synth);
  result = _performance->AddPort(NULL);
  if (result == DMUS_E_NOT_INIT) {
    audio_cat->error() << "got DMUS_N_NOT_INIT" << endl;
  }
  else if (result == DMUS_E_CANNOT_OPEN_PORT)
    audio_cat->error() << "got DMUS_E_CANNOT_OPEN_PORT" << endl;
  else if (result == E_OUTOFMEMORY)
    audio_cat->error() << "got E_OUTOFMEMORY" << endl;
  else if (result == E_POINTER)
    audio_cat->error() << "got E_POINTER" << endl;
  CHECK_RESULT_MUSIC(result, "failed to add synth to performance");

/*
  // allocate performance channels
  result = _performance->AssignPChannelBlock(0, _synth, 1);
  CHECK_RESULT(result, "failed to assign performance channels");
*/

  audio_debug("out of WinMusic::init()  _performance = "
                       << (void*)_performance << "  _synth = "
                       << (void*)_synth << "  _buffer = " << (void*)_buffer);
}

float WinMusic::length() const {
  // DO THIS
  audio_debug("winmusic length");
  return -1.;
}

AudioTraits::PlayingClass* WinMusic::get_state() const {
  WinMusicPlaying* ret = new WinMusicPlaying((WinMusic*)this);
  audio_debug("winmusic get_state returning 0x" << (void*)ret);
  return ret;
}

AudioTraits::PlayerClass* WinMusic::get_player() const {
  AudioTraits::PlayerClass* ret = WinMusicPlayer::get_instance();
  audio_debug("winmusic get_player returning 0x" << (void*)ret);
  return ret;
}

AudioTraits::DeletePlayingFunc* WinMusic::get_delstate() const {
  audio_debug("winmusic get_delstate returning 0x"
               << (void*)(WinMusicPlaying::destroy));
  return WinMusicPlaying::destroy;
}

WinMusic* WinMusic::load_midi(Filename filename) {
  audio_debug("in WinMusic::load_midi()");
  initialize();
  if (!audio_is_active) {
    return (WinMusic*)0L;
  }

  // WinMusic* ret = (WinMusic*)0L;
  WinMusic* ret = new WinMusic();
  if (ret->_performance && ret->_music) {
    audio_debug("for some reason, have to stop");
    ret->_performance->Stop(NULL, NULL, 0, 0);
  }
  ret->_music = NULL;
  IDirectMusicLoader* loader;
  HRESULT result = CoCreateInstance(
      CLSID_DirectMusicLoader, NULL,
      CLSCTX_INPROC, IID_IDirectMusicLoader,
      (void**)&loader);
  if (FAILED(result)) {
    audio_error("could not create music loader");
    delete ret;
    return 0;
  }

  /*
  char szDir[_MAX_PATH];
  WCHAR wszDir[_MAX_PATH];
  if (_getcwd(szDir, _MAX_PATH)==NULL) {
    audio_cat->error() << "could not getcwd" << endl;
    delete ret;
    ret = (WinMusic*)0L;
    return ret;
  }
  MULTI_TO_WIDE(wszDir, szDir);
  result = loader->SetSearchDirectory(GUID_DirectMusicAllTypes, wszDir, FALSE);
  if (FAILED(result)) {
    audio_cat->error() << "could not set search directory" << endl;
    delete ret;
    ret = (WinMusic*)0L;
    return ret;
  }
  */

  DMUS_OBJECTDESC fdesc;
  fdesc.guidClass = CLSID_DirectMusicSegment;
  fdesc.dwSize = sizeof(DMUS_OBJECTDESC);
  // MULTI_TO_WIDE(filename.to_os_specific().c_str(), fdesc.wszFileName);
/*
  if (!(filename.resolve_filename(get_sound_path()))) {
    audio_cat->error() << "could not find '" << filename << "' on sound path"
               << endl;
    loader->Release();
    delete ret;
    ret = (WinMusic*)0L;
    return ret;
  }
*/
  string stmp = filename.to_os_specific();
  MULTI_TO_WIDE(fdesc.wszFileName, stmp.c_str());
  audio_debug("os_specific name '" << stmp << "'");
  if (filename.is_local()) {
    fdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
    audio_debug("is local");
    char szDir[2] = ".";
    WCHAR wszDir[2];
    MULTI_TO_WIDE(wszDir, szDir);
    result = loader->SetSearchDirectory(
        GUID_DirectMusicAllTypes, wszDir, FALSE);
    if (FAILED(result)) {
      audio_error("could not set search directory to '.'");
      loader->Release();
      delete ret;
      return 0;
    }
  } else {
    fdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH;
    audio_debug("is not local");
  }
  result = loader->GetObject(&fdesc, IID_IDirectMusicSegment2,
      (void**)&(ret->_music));
  if (FAILED(result)) {
    audio_error("failed to load file");
    loader->Release();
    delete ret;
    return 0;
  }
  ret->_music->SetParam(GUID_StandardMIDIFile, 
      -1, 0, 0, (void*)(ret->_performance));
  ret->_music->SetParam(GUID_Download, -1, 0, 0, (void*)(ret->_performance));
  audio_debug("out of WinMusic::load_midi()  _music = "
      << (void*)ret->_music);
  return ret;
}

WinSamplePlaying::WinSamplePlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
  audio_debug("in WinSamplePlaying constructor");
  initialize();
  if (!audio_is_active) {
    return;
  }

  WinSample* ws = (WinSample*)s;
  if (!ws) {
    audio_debug("the sample we were handed is NULL, returning");
    return;
  }
  if (!ws->_data) {
    audio_debug("the sample has null data, returning");
    return;
  }

  DSBUFFERDESC dsbdDesc;
  ZeroMemory(&dsbdDesc, sizeof(DSBUFFERDESC));
  dsbdDesc.dwSize = sizeof(DSBUFFERDESC);
  dsbdDesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS;
  dsbdDesc.dwBufferBytes = ws->_len;
  dsbdDesc.lpwfxFormat = &(ws->_info);
  dsbdDesc.lpwfxFormat->cbSize = sizeof(ws->_info);
  HRESULT result = soundDirectSound->CreateSoundBuffer(&dsbdDesc, 
      &_channel, NULL);

  if (FAILED(result)) {
    _channel = NULL;
    audio_debug("failed to create a channel");
    return;
  }
  BYTE* dst = NULL;
  dst = this->lock();
  audio_debug("WinSamplePlaying::WinSamplePlaying _data = 0x"
      << (void*)(ws->_data) << "  dst = 0x" << (void*)dst);

  // The Intel compiler dumps core if we attempt to protect this in a
  // try .. catch block.  We probably shouldn't be using exception
  // handling anyway.

  //  try {
  memcpy(dst, ws->_data, ws->_len);
  //  }
  //  catch (...) {
  //    _channel = NULL;
  //    if (audio_cat.is_debug())
  //      audio_cat->debug() << "memcpy failed.  dst = 0x" << (void*)dst
  //             << "  data = 0x" << (void*)(ws->_data)
  //             << "   len = " << ws->_len << endl;
  //    return;
  //  }
  this->unlock();
}

WinSamplePlaying::~WinSamplePlaying() {
  audio_debug("winsampleplaying destructor");
}

void WinSamplePlaying::destroy(AudioTraits::PlayingClass* play) {
  audio_debug("winsampleplaying destroy got 0x" << (void*)play);
  delete play;
}

AudioTraits::PlayingClass::PlayingStatus WinSamplePlaying::status() {
  if (_channel) {
    DWORD dwStatus;
    _channel->GetStatus(&dwStatus);
    if (dwStatus & DSBSTATUS_PLAYING) {
      return AudioTraits::PlayingClass::PLAYING;
    }
  }
  return AudioTraits::PlayingClass::READY;
}

BYTE* WinSamplePlaying::lock() {
  audio_debug("in winsampleplaying lock");
  WinSample* s = (WinSample*)(_sound);
  HRESULT result = _channel->Lock(0, 
      0, (void**)&_data, &(s->_len), NULL, 0, DSBLOCK_ENTIREBUFFER);
  if (FAILED(result)) {
    audio_error("failed to lock buffer");
    return NULL;
  }
  audio_debug("returning 0x" << (void*)_data);
  return _data;
}

void WinSamplePlaying::unlock() {
  audio_debug("in winsampleplaying unlock");
  WinSample* s = (WinSample*)(_sound);
  HRESULT result = _channel->Unlock(_data, s->_len, NULL, 0);
  CHECK_RESULT(result, "failed to unlock buffer");
  audio_debug("out of winsampleplaying unlock");
}

WinMusicPlaying::WinMusicPlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
  audio_debug("in winmusicplaying constructor");
  initialize();
  audio_debug("out of winmusicplaying constructor");
}

WinMusicPlaying::~WinMusicPlaying() {
  audio_debug("winmusicplaying destructor");
}

void WinMusicPlaying::destroy(AudioTraits::PlayingClass* play) {
  audio_debug("winmusicplaying destroy got 0x" << (void*)play);
  delete play;
}

AudioTraits::PlayingClass::PlayingStatus WinMusicPlaying::status() {
  WinMusic* wm = (WinMusic*)_sound;
  if (wm->get_performance() && wm->get_music()) {
    if (wm->get_performance()->IsPlaying(wm->get_music(), NULL) == S_OK) {
      return PLAYING;
    }
  }
  return READY;
}

WinSamplePlayer* WinSamplePlayer::_global_instance = (WinSamplePlayer*)0L;

WinSamplePlayer::~WinSamplePlayer() {
  audio_debug("in winsampleplayer destructor");
}

void WinSamplePlayer::play_sound(AudioTraits::SoundClass* sample,
               AudioTraits::PlayingClass* play, float start_time) {
  nassertv(sample);
  nassertv(play);
  audio_debug("in winsampleplayer play_sound ");
  initialize();
  if (!audio_is_active) {
    return;
  }
  if (!AudioManager::get_sfx_active()) {
    return;
  }
  WinSample* wsample = (WinSample*)sample;
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  if (chan) {
    chan->Stop();
    DWORD l = wsample->get_length();
    WAVEFORMATEX f = wsample->get_format();
    float factor = ((float)l) / f.nAvgBytesPerSec;
    factor = start_time / factor;
    if (factor > 1.) {
      factor = 1.;
    }
    DWORD p = (DWORD)(l * factor);
    p = (p >> 2) << 2;  // zero the last 2 bits
    chan->SetCurrentPosition(p);
    HRESULT result = chan->Play(0, 0, 0);
    if (FAILED(result)) {
      audio_error("sample play failed");
    }
  }
  audio_debug("out of winsampleplayer play_sound");
}

void WinSamplePlayer::stop_sound(AudioTraits::SoundClass*,
               AudioTraits::PlayingClass* play) {
  initialize();
  if (!audio_is_active) {
    return;
  }
  if (!AudioManager::get_sfx_active()) {
    return;
  }
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  if (chan) {
    chan->Stop();
  }
}

void WinSamplePlayer::set_volume(AudioTraits::PlayingClass* play, float v) {
  audio_debug("winsampleplayer set_volume");
  initialize();
  if (!audio_is_active) {
    return;
  }
  if (!AudioManager::get_sfx_active()) {
    return;
  }
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  float tmpv = v * AudioManager::get_master_sfx_volume();
  if (chan) {
    LONG v2 = (tmpv * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    chan->SetVolume(v2);
  }
  play->set_volume(v);
}

bool WinSamplePlayer::adjust_volume(AudioTraits::PlayingClass* play) {
  audio_debug("winsampleplayer adjust_volume");
  initialize();
  if (!audio_is_active) {
    return true;
  }
  if (!AudioManager::get_sfx_active()) {
    return true;
  }
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  float tmpv = play->get_volume() * AudioManager::get_master_sfx_volume();
  if (chan) {
    LONG v2 = (tmpv * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    chan->SetVolume(v2);
  }
  return false;
}

WinSamplePlayer* WinSamplePlayer::get_instance() {
  audio_debug("in winsampleplayer get_instance");
  if (!_global_instance) {
    _global_instance = new WinSamplePlayer();
  }
  audio_debug("winsampleplayer returning 0x" << (void*)_global_instance);
  return _global_instance;
}

WinMusicPlayer* WinMusicPlayer::_global_instance = (WinMusicPlayer*)0L;

WinMusicPlayer::~WinMusicPlayer() {
  audio_debug("in winmusicplayer destructor");
}

void WinMusicPlayer::play_sound(AudioTraits::SoundClass* music,
                AudioTraits::PlayingClass*, float) {
  audio_debug("in WinMusicPlayer::play_sound()");
  initialize();
  if (!audio_is_active) {
    return;
  }
  if (!AudioManager::get_music_active()) {
    return;
  }
  WinMusic* wmusic = (WinMusic*)music;
  IDirectMusicPerformance* _perf = wmusic->get_performance();
  IDirectMusicSegment* _msc = wmusic->get_music();
  audio_debug("about to jump in: _perf = " 
      << (void*)_perf << "  _msc = " << (void*)_msc);
  if (_perf && _msc) {
    audio_debug("made it inside");
    // _msc->SetRepeats(0);
    IDirectMusicSegmentState* segState;
    // HRESULT result = _perf->PlaySegment(_msc, 0, 0, NULL);
    HRESULT result = _perf->PlaySegment(_msc, 0, 0, &segState);
    if (result != S_OK) {
      audio_error("music play failed");
      switch (result) {
      case E_OUTOFMEMORY:
        audio_error("reports out of memory");
        break;
      case E_POINTER:
        audio_error("reports invalid pointer");
        break;
      case DMUS_E_NO_MASTER_CLOCK:
        audio_error("reports no master clock");
        break;
      case DMUS_E_SEGMENT_INIT_FAILED:
        audio_error("reports segment init failed");
        break;
      case DMUS_E_TIME_PAST:
        audio_error("reports time past");
        break;
      };
    }
  }
  audio_debug("out of WinMusicPlayer::play_sound()");
}

void WinMusicPlayer::stop_sound(AudioTraits::SoundClass* music,
                AudioTraits::PlayingClass*) {
  WinMusic* wmusic = (WinMusic*)music;
  IDirectMusicPerformance* _perf = wmusic->get_performance();
  IDirectMusicSegment* _msc = wmusic->get_music();

  if (!AudioManager::get_music_active()) {
    return;
  }
  if (_perf && _msc) {
    HRESULT result = _perf->Stop(_msc, 0, 0, 0);
    if (result != S_OK) {
      audio_error("music stop failed");
    } else {
      audio_debug("music stop succeeded");
    }
  }
}

void WinMusicPlayer::set_volume(AudioTraits::PlayingClass* play, float v) {
  audio_debug("WinMusicPlayer::set_volume()");
  WinMusicPlaying* wplay = (WinMusicPlaying*)play;
  IDirectMusicPerformance* perf = wplay->get_performance();
  float tmpv = v * AudioManager::get_master_music_volume();
  if (!AudioManager::get_music_active()) {
    return;
  }
  if (perf) {
    LONG v2 = (tmpv * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    perf->SetGlobalParam(GUID_PerfMasterVolume, &v2, sizeof(LONG));
  }
  play->set_volume(v);
}

bool WinMusicPlayer::adjust_volume(AudioTraits::PlayingClass* play) {
  audio_debug("WinMusicPlayer::adjust_volume()");
  WinMusicPlaying* wplay = (WinMusicPlaying*)play;
  IDirectMusicPerformance* perf = wplay->get_performance();
  float tmpv = play->get_volume() * AudioManager::get_master_music_volume();
  if (!AudioManager::get_music_active()) {
    return true;
  }
  if (perf) {
    LONG v2 = (tmpv * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    perf->SetGlobalParam(GUID_PerfMasterVolume, &v2, sizeof(LONG));
  }
  return false;
}

WinMusicPlayer* WinMusicPlayer::get_instance() {
  audio_debug("in WinMusicPlayer::get_instance");
  if (!_global_instance) {
    _global_instance = new WinMusicPlayer();
  }
  audio_debug("WinMusicPlayer::get_instance returning 0x"
      << (void*)_global_instance);
  return _global_instance;
}

#endif /* AUDIO_USE_WIN32 */
