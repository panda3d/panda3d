// Filename: audio_win_traits.cxx
// Created by:  cary (27Sep00)
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
  }

// #define MULTI_TO_WIDE(_in, _out) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _in, -1, _out, DMUS_MAX_FILENAME)
#define MULTI_TO_WIDE(x,y) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH);


static void update_win(void) {
}

static void initialize(void) {
  if (have_initialized)
    return;
  if (!audio_is_active)
    return;

  if (audio_cat->is_debug())
    audio_cat->debug() << "in winAudio initialize" << endl;

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
  CHECK_RESULT(result, "could not create a Direct Sound (tm) object (c)");

  // set the cooperative level
  result = soundDirectSound->SetCooperativeLevel(global_hwnd, DSSCL_PRIORITY);
  if (FAILED(result)) {
    audio_cat->warning() << "could not set Direct Sound co-op level to "
			 << "DSSCL_PRIORITY, trying DSSCL_NORMAL" << endl;
    result = soundDirectSound->SetCooperativeLevel(global_hwnd, DSSCL_NORMAL);
    CHECK_RESULT(result, "failed setting to DSSCL_NORMAL");
  }

  // Move any unused portions of onboard sound memory to a contiguous block
  // so that the largest portion of free memory will be available.
  soundDirectSound->Compact();

  // create the primary buffer
  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize  =  sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
  result = soundDirectSound->CreateSoundBuffer(&dsbd, &soundPrimaryBuffer,
					       NULL);
  CHECK_RESULT(result, "could not create primary buffer");

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
  if (result == DSERR_PRIOLEVELNEEDED)
    audio_cat->warning() << "could not set format of Primary Buffer because "
			 << "we didn't get DSSCL_PRIORITY" << endl;

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

  if (audio_cat->is_debug())
    audio_cat->debug() << "out of winAudio initialize" << endl;
}

static void shutdown(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winaudio shutdown" << endl;

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

  if (audio_cat->is_debug())
    audio_cat->debug() << "out of winaudio shutdown" << endl;
}

WinSample::~WinSample(void) {
  // we may or may not be leaking the _data
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsample destructor called" << endl;
}

float WinSample::length(void) const {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsample length called" << endl;
  return _len / (audio_mix_freq * 2. * 2.);
}

AudioTraits::PlayingClass* WinSample::get_state(void) const {
  WinSamplePlaying* ret = new WinSamplePlaying((WinSample*)this);
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsample get_state returning 0x" << (void*)ret
		       << endl;
  return ret;
}

AudioTraits::PlayerClass* WinSample::get_player(void) const {
  AudioTraits::PlayerClass* ret = WinSamplePlayer::get_instance();
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsample get_player returning 0x" << (void*)ret
		       << endl;
  return ret;
}

AudioTraits::DeletePlayingFunc* WinSample::get_delstate(void) const {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsample get_delstate returning 0x"
		       << (void*)(WinSamplePlaying::destroy) << endl;
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
    if (*ppwfxInfo == NULL)
      return E_FAIL;
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
  if (cbDataIn > pckIn->cksize)
    cbDataIn = pckIn->cksize;
  for (DWORD cT=0; cT<cbDataIn; ++cT) {
    // copy bytes from the io to the buffer
    if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead) {
      if (mmioAdvance(hmmio, &mmioinfoIn, MMIO_READ) != 0)
	return E_FAIL;
      if (mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
	return E_FAIL;
    }
    // actual copy
    *((BYTE*)pbDest+cT) = *((BYTE*)mmioinfoIn.pchNext);
    mmioinfoIn.pchNext++;
  }
  if (mmioSetInfo(hmmio, &mmioinfoIn, 0) != 0)
    return E_FAIL;
  *cbActualRead = cbDataIn;
  return S_OK;
}

HRESULT wave_load_internal(const CHAR* filename, WAVEFORMATEX& wavInfo,
			   BYTE*& wavData, UINT& wavSize) {
  wavData = NULL;
  wavSize = 0;
  HRESULT result = wave_open_file(filename, &hmmioIn, &pwfx, &ckInRiff);
  if (FAILED(result))
    return result;
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
  if (pwfx) {
    delete pwfx;
    pwfx = NULL;
  }
  return result;
}

WinSample* WinSample::load_wav(Filename filename) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsample load_wav" << endl;
  WinSample* ret = (WinSample*)0L;

  initialize();

  if (!audio_is_active)
    return ret;

  WAVEFORMATEX wavInfo;
  UINT wavSize = 0;
  BYTE* wavData = NULL;

  string stmp = filename.to_os_specific();
  HRESULT result = wave_load(stmp.c_str(), wavInfo, wavData, wavSize);
  if (FAILED(result)) {
    if (wavData)
      delete [] wavData;
    if (audio_cat->is_debug())
      audio_cat->debug() << "wave_load failed, returning NULL" << endl;
    return ret;
  }

  ret = new WinSample();
  memcpy(&(ret->_info), &wavInfo, sizeof(WAVEFORMATEX));
  ret->_data = wavData;
  ret->_len = wavSize;

  if (audio_cat->is_debug())
    audio_cat->debug() << "returning 0x" << (void*)ret << endl;
  return ret;
}

WinSample* WinSample::load_raw(unsigned char* data, unsigned long size) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsample load_raw" << endl;
  WinSample* ret = (WinSample*)0L;

  initialize();

  if (!audio_is_active)
    return ret;

  // synth a wav header for this data
  WAVEFORMATEX wavInfo;
  ZeroMemory(&wavInfo, sizeof(WAVEFORMATEX));
  wavInfo.wFormatTag = WAVE_FORMAT_PCM;
  wavInfo.nChannels = 2;
  wavInfo.nSamplesPerSec = audio_mix_freq;
  wavInfo.wBitsPerSample = 16;
  wavInfo.nBlockAlign = wavInfo.wBitsPerSample / 8 * wavInfo.nChannels;
  wavInfo.nAvgBytesPerSec = wavInfo.nSamplesPerSec * wavInfo.nBlockAlign;

  if (data == (unsigned char*)0L) {
    if (audio_cat->is_debug())
      audio_cat->debug() << "data is null, returning same" << endl;
    return ret;
  }

  // create a direct sound channel for this data
  ret = new WinSample();
  memcpy(&(ret->_info), &wavInfo, sizeof(WAVEFORMATEX));
  ret->_data = data;
  ret->_len = size;
  if (audio_cat->is_debug())
    audio_cat->debug() << "WinSample::load_raw returning 0x" << (void*)ret
		       << endl;
  return ret;
}

WinMusic::~WinMusic(void) {
  // AudioManager::stop(this);
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusic::~WinMusic()" << endl;

  if (_music) {
    _music->Release();
    _music = NULL;
  }

  if (_synth) {
    _synth->Release();
    _synth = NULL;
  }

  if (_performance) {
    _performance->Release();
    _performance = NULL;
  }

  if (_buffer) {
    _buffer->Release();
    _buffer = NULL;
  }

  if (audio_cat->is_debug())
    audio_cat->debug() << "out of WinMusic::~WinMusic()" << endl;
}

void WinMusic::init(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusic::init()" << endl;

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
  CHECK_RESULT(result, "could not initialize performance object");

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
  if (result == DMUS_E_NOT_INIT)
    audio_cat->error() << "got DMUS_N_NOT_INIT" << endl;
  else if (result == DMUS_E_CANNOT_OPEN_PORT)
    audio_cat->error() << "got DMUS_E_CANNOT_OPEN_PORT" << endl;
  else if (result == E_OUTOFMEMORY)
    audio_cat->error() << "got E_OUTOFMEMORY" << endl;
  else if (result == E_POINTER)
    audio_cat->error() << "got E_POINTER" << endl;
  CHECK_RESULT(result, "failed to add synth to performance");

/*
  // allocate performance channels
  result = _performance->AssignPChannelBlock(0, _synth, 1);
  CHECK_RESULT(result, "failed to assign performance channels");
*/

  if (audio_cat->is_debug())
    audio_cat->debug() << "out of WinMusic::init()  _performance = "
                       << (void*)_performance << "  _synth = "
                       << (void*)_synth << "  _buffer = " << (void*)_buffer
                       << endl;
}

float WinMusic::length(void) const {
  // DO THIS
  if (audio_cat->is_debug())
    audio_cat->debug() << "winmusic length" << endl;
  return -1.;
}

AudioTraits::PlayingClass* WinMusic::get_state(void) const {
  WinMusicPlaying* ret = new WinMusicPlaying((WinMusic*)this);
  if (audio_cat->is_debug())
    audio_cat->debug() << "winmusic get_state returning 0x"
		       << (void*)ret << endl;
  return ret;
}

AudioTraits::PlayerClass* WinMusic::get_player(void) const {
  AudioTraits::PlayerClass* ret = WinMusicPlayer::get_instance();
  if (audio_cat->is_debug())
    audio_cat->debug() << "winmusic get_player returning 0x" << (void*)ret
		       << endl;
  return ret;
}

AudioTraits::DeletePlayingFunc* WinMusic::get_delstate(void) const {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winmusic get_delstate returning 0x"
		       << (void*)(WinMusicPlaying::destroy) << endl;
  return WinMusicPlaying::destroy;
}

WinMusic* WinMusic::load_midi(Filename filename) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusic::load_midi()" << endl;
  initialize();

  if (!audio_is_active)
    return (WinMusic*)0L;

  // WinMusic* ret = (WinMusic*)0L;
  WinMusic* ret = new WinMusic();
  if (ret->_performance && ret->_music) {
    if (audio_cat->is_debug())
      audio_cat->debug() << "for some reason, have to stop" << endl;
    ret->_performance->Stop(NULL, NULL, 0, 0);
  }
  ret->_music = NULL;
  IDirectMusicLoader* loader;
  HRESULT result = CoCreateInstance(CLSID_DirectMusicLoader, NULL,
				    CLSCTX_INPROC, IID_IDirectMusicLoader,
				    (void**)&loader);
  if (FAILED(result)) {
    audio_cat->error() << "could not create music loader" << endl;
    delete ret;
    ret = (WinMusic*)0L;
    return ret;
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
  audio_cat->debug() << "os_specific name '" << stmp << "'" << endl;
  if (filename.is_local()) {
    fdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
    audio_cat->debug() << "is local" << endl;
  } else {
    fdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH;
    audio_cat->debug() << "is not local" << endl;
  }
  result = loader->GetObject(&fdesc, IID_IDirectMusicSegment2,
			     (void**)&(ret->_music));
  if (FAILED(result)) {
    audio_cat->error() << "failed to load file" << endl;
    loader->Release();
    delete ret;
    ret = (WinMusic*)0L;
    return ret;
  }
  ret->_music->SetParam(GUID_StandardMIDIFile, -1, 0, 0,
			(void*)(ret->_performance));
  ret->_music->SetParam(GUID_Download, -1, 0, 0, (void*)(ret->_performance));
  // ret->_buffer->SetVolume(0);
  // ret->_buffer->SetPan(0);
  if (audio_cat->is_debug())
    audio_cat->debug() << "out of WinMusic::load_midi()  _music = "
                       << (void*)ret->_music << endl;
  return ret;
}

WinSamplePlaying::WinSamplePlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinSamplePlaying constructor" << endl;

  initialize();

  if (!audio_is_active)
    return;

  WinSample* ws = (WinSample*)s;

  if (ws == (WinSample*)0L) {
    if (audio_cat->is_debug())
      audio_cat->debug() << "the sample we were handed is NULL, returning"
			 << endl;
    return;
  }
  if (ws->_data == (unsigned char*)0L) {
    if (audio_cat->is_debug())
      audio_cat->debug() << "the sample has null data, returning" << endl;
    return;
  }

  DSBUFFERDESC dsbdDesc;
  ZeroMemory(&dsbdDesc, sizeof(DSBUFFERDESC));
  dsbdDesc.dwSize = sizeof(DSBUFFERDESC);
  dsbdDesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GLOBALFOCUS;
  dsbdDesc.dwBufferBytes = ws->_len;
  dsbdDesc.lpwfxFormat = &(ws->_info);
  dsbdDesc.lpwfxFormat->cbSize = sizeof(ws->_info);
  HRESULT result = soundDirectSound->CreateSoundBuffer(&dsbdDesc, &_channel,
                                                        NULL);

  if (FAILED(result)) {
    _channel = NULL;
    if (audio_cat->is_debug())
      audio_cat->debug() << "failed to create a channel" << endl;
    return;
  }
  BYTE* dst = NULL;
  dst = this->lock();
  if (audio_cat->is_debug())
    audio_cat->debug() << "WinSamplePlaying::WinSamplePlaying _data = 0x"
		       << (void*)(ws->_data) << "  dst = 0x"
		       << (void*)dst << endl;

  // The Intel compiler dumps core if we attempt to protect this in a
  // try .. catch block.  We probably shouldn't be using exception
  // handling anyway.

  //  try {
  memcpy(dst, ws->_data, ws->_len);
  //  }
  //  catch (...) {
  //    _channel = NULL;
  //    if (audio_cat->is_debug())
  //      audio_cat->debug() << "memcpy failed.  dst = 0x" << (void*)dst
  //			 << "  data = 0x" << (void*)(ws->_data)
  //			 << "   len = " << ws->_len << endl;
  //    return;
  //  }
  this->unlock();
}

WinSamplePlaying::~WinSamplePlaying(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsampleplaying destructor" << endl;
}

void WinSamplePlaying::destroy(AudioTraits::PlayingClass* play) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsampleplaying destroy got 0x" << (void*)play
		       << endl;
  delete play;
}

AudioTraits::PlayingClass::PlayingStatus WinSamplePlaying::status(void) {
  if (_channel) {
    DWORD dwStatus;
    _channel->GetStatus(&dwStatus);
    if (dwStatus & DSBSTATUS_PLAYING) {
      return AudioTraits::PlayingClass::PLAYING;
    }
  }
  return AudioTraits::PlayingClass::READY;
}

BYTE* WinSamplePlaying::lock(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsampleplaying lock" << endl;
  WinSample* s = (WinSample*)(_sound);
  HRESULT result = _channel->Lock(0, 0, (void**)&_data, &(s->_len), NULL,
				  0, DSBLOCK_ENTIREBUFFER);
  if (FAILED(result)) {
    audio_cat->error() << "failed to lock buffer" << endl;
    return NULL;
  }
  if (audio_cat->is_debug())
    audio_cat->debug() << "returning 0x" << (void*)_data << endl;
  return _data;
}

void WinSamplePlaying::unlock(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsampleplaying unlock" << endl;
  WinSample* s = (WinSample*)(_sound);
  HRESULT result = _channel->Unlock(_data, s->_len, NULL, 0);
  CHECK_RESULT(result, "failed to unlock buffer");
  if (audio_cat->is_debug())
    audio_cat->debug() << "out of winsampleplaying unlock" << endl;
}

WinMusicPlaying::WinMusicPlaying(AudioTraits::SoundClass* s)
  : AudioTraits::PlayingClass(s) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winmusicplaying constructor" << endl;
  initialize();
  if (audio_cat->is_debug())
    audio_cat->debug() << "out of winmusicplaying constructor" << endl;
}

WinMusicPlaying::~WinMusicPlaying(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winmusicplaying destructor" << endl;
}

void WinMusicPlaying::destroy(AudioTraits::PlayingClass* play) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winmusicplaying destroy got 0x" << (void*)play
		       << endl;
  delete play;
}

AudioTraits::PlayingClass::PlayingStatus WinMusicPlaying::status(void) {
  WinMusic* wm = (WinMusic*)_sound;

  if (wm->get_performance() && wm->get_music()) {
    if (wm->get_performance()->IsPlaying(wm->get_music(), NULL) == S_OK) {
      return PLAYING;
    }
  }
  return READY;
}

WinSamplePlayer* WinSamplePlayer::_global_instance = (WinSamplePlayer*)0L;

WinSamplePlayer::~WinSamplePlayer(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsampleplayer destructor" << endl;
}

void WinSamplePlayer::play_sound(AudioTraits::SoundClass* sample,
			   AudioTraits::PlayingClass* play, float start_time) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsampleplayer play_sound" << endl;
  initialize();
  if (!audio_is_active)
    return;
  WinSample* wsample = (WinSample*)sample;
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  if (chan) {
    chan->Stop();
    DWORD l = wsample->get_length();
    WAVEFORMATEX f = wsample->get_format();
    float factor = ((float)l) / wsample->get_format().nAvgBytesPerSec;
    factor = start_time / factor;
    if (factor > 1.)
      factor = 1.;
    DWORD p = (DWORD)(l * factor);
    p = (p >> 2) << 2;  // zero the last 2 bits
    chan->SetCurrentPosition(p);
    HRESULT result = chan->Play(0, 0, 0);
    if (FAILED(result))
      audio_cat->error() << "sample play failed" << endl;
  }
  if (audio_cat->is_debug())
    audio_cat->debug() << "out of winsampleplayer play_sound" << endl;
}

void WinSamplePlayer::stop_sound(AudioTraits::SoundClass*,
			   AudioTraits::PlayingClass* play) {
  initialize();
  if (!audio_is_active)
    return;
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  if (chan)
    chan->Stop();
}

void WinSamplePlayer::set_volume(AudioTraits::PlayingClass* play, float v) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsampleplayer set_volume" << endl;
  initialize();
  if (!audio_is_active)
    return;
  WinSamplePlaying* wplay = (WinSamplePlaying*)play;
  LPDIRECTSOUNDBUFFER chan = wplay->get_channel();
  if (chan) {
    LONG v2 = (v * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    chan->SetVolume(v2);
  }
}

WinSamplePlayer* WinSamplePlayer::get_instance(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winsampleplayer get_instance" << endl;
  if (_global_instance == (WinSamplePlayer*)0L)
    _global_instance = new WinSamplePlayer();
  if (audio_cat->is_debug())
    audio_cat->debug() << "winsampleplayer returning 0x"
		       << (void*)_global_instance << endl;
  return _global_instance;
}

WinMusicPlayer* WinMusicPlayer::_global_instance = (WinMusicPlayer*)0L;

WinMusicPlayer::~WinMusicPlayer(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in winmusicplayer destructor" << endl;
}

void WinMusicPlayer::play_sound(AudioTraits::SoundClass* music,
				AudioTraits::PlayingClass*, float) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusicPlayer::play_sound()" << endl;
  initialize();
  if (!audio_is_active)
    return;
  WinMusic* wmusic = (WinMusic*)music;
  IDirectMusicPerformance* _perf = wmusic->get_performance();
  IDirectMusicSegment* _msc = wmusic->get_music();
  if (audio_cat->is_debug())
    audio_cat->debug() << "about to jump in: _perf = " << (void*)_perf
                       << "  _msc = " << (void*)_msc << endl;
  if (_perf && _msc) {
    if (audio_cat->is_debug())
      audio_cat->debug() << "made it inside" << endl;
    // _msc->SetRepeats(0);
    IDirectMusicSegmentState* segState;
    // HRESULT result = _perf->PlaySegment(_msc, 0, 0, NULL);
    HRESULT result = _perf->PlaySegment(_msc, 0, 0, &segState);
    if (result != S_OK) {
      audio_cat->error() << "music play failed" << endl;
      switch (result) {
      case E_OUTOFMEMORY: audio_cat->error() << "reports out of memory" << endl;
        break;
      case E_POINTER: audio_cat->error() << "reports invalid pointer" << endl;
        break;
      case DMUS_E_NO_MASTER_CLOCK: audio_cat->error() << "reports no master clock" << endl;
        break;
      case DMUS_E_SEGMENT_INIT_FAILED: audio_cat->error() << "reports segment init failed" << endl;
        break;
      case DMUS_E_TIME_PAST: audio_cat->error() << "reports time past" << endl;
        break;
      };
    }
  }
  if (audio_cat->is_debug())
    audio_cat->debug() << "out of WinMusicPlayer::play_sound()" << endl;
}

void WinMusicPlayer::stop_sound(AudioTraits::SoundClass* music,
				AudioTraits::PlayingClass*) {
  WinMusic* wmusic = (WinMusic*)music;
  IDirectMusicPerformance* _perf = wmusic->get_performance();
  IDirectMusicSegment* _msc = wmusic->get_music();

  if (_perf && _msc) {
    HRESULT result = _perf->Stop(_msc, 0, 0, 0);
    if (result != S_OK)
      audio_cat->error() << "music stop failed" << endl;
  }
}

void WinMusicPlayer::set_volume(AudioTraits::PlayingClass* play, float v) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "WinMusicPlayer::set_volume()" << endl;
  WinMusicPlaying* wplay = (WinMusicPlaying*)play;
  IDirectMusicPerformance* perf = wplay->get_performance();
  if (perf) {
    LONG v2 = (v * (DSBVOLUME_MAX - DSBVOLUME_MIN)) + DSBVOLUME_MIN;
    perf->SetGlobalParam(GUID_PerfMasterVolume, &v2, sizeof(LONG));
  }
}

WinMusicPlayer* WinMusicPlayer::get_instance(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusicPlayer::get_instance" << endl;
  if (_global_instance == (WinMusicPlayer*)0L)
    _global_instance = new WinMusicPlayer();
  if (audio_cat->is_debug())
    audio_cat->debug() << "WinMusicPlayer::get_instance returning 0x"
		       << (void*)_global_instance << endl;
  return _global_instance;
}

#endif /* AUDIO_USE_WIN32 */
