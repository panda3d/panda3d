// Filename: audio_win_traits.cxx
// Created by:  cary (27Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_win_traits.h"
#include "audio_manager.h"
#include "config_audio.h"

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
    return; \
  }

// #define MULTI_TO_WIDE(_in, _out) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _in, -1, _out, DMUS_MAX_FILENAME)
#define MULTI_TO_WIDE(x,y) MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH);

static void update_win(void) {
}

static void initialize(void) {
  if (have_initialized)
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
  wfx.nSamplesPerSec  = 22050;
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
}

static void shutdown(void) {
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
}

WinSample::~WinSample(void) {
  // unload any data we have
  if (_channel) {
    _channel->Release();
    _channel = NULL;
  }
  // we may or may not be leaking the _data
}

float WinSample::length(void) {
  // DO THIS
  return 0.;
}

AudioTraits::SampleClass::SampleStatus WinSample::status(void) {
  if (_channel) {
    DWORD dwStatus;
    _channel->GetStatus(&dwStatus);
    if (dwStatus & DSBSTATUS_PLAYING)
      return AudioTraits::SampleClass::PLAYING;
  }
  return AudioTraits::SampleClass::READY;
}

BYTE* WinSample::lock(void) {
  HRESULT result = _channel->Lock(0, 0, (void**)&_data, &_len, NULL, 0,
				  DSBLOCK_ENTIREBUFFER);
  if (FAILED(result)) {
    audio_cat->error() << "failed to lock buffer" << endl;
    return NULL;
  }
  return _data;
}

void WinSample::unlock(void) {
  HRESULT result = _channel->Unlock(_data, _len, NULL, 0);
  CHECK_RESULT(result, "failed to unlock buffer");
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
  WinSample* ret = (WinSample*)0L;

  initialize();
  WAVEFORMATEX wavInfo;
  UINT wavSize = 0;
  BYTE* wavData = NULL;

  HRESULT result = wave_load(filename.c_str(), wavInfo, wavData, wavSize);
  if (FAILED(result)) {
    if (wavData)
      delete [] wavData;
    return ret;
  }

  // create direct sound channel
  ret = new WinSample();
  DSBUFFERDESC dsbdDesc;
  ZeroMemory(&dsbdDesc, sizeof(DSBUFFERDESC));
  dsbdDesc.dwSize = sizeof(DSBUFFERDESC);
/*
  dsbdDesc.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC |
                     DSBCAPS_GLOBALFOCUS;
*/
  dsbdDesc.dwFlags = DSBCAPS_STATIC |
                     DSBCAPS_GLOBALFOCUS;
  dsbdDesc.dwBufferBytes = wavSize;
  dsbdDesc.lpwfxFormat = &wavInfo;
  dsbdDesc.lpwfxFormat->cbSize = sizeof(wavInfo);
  result = soundDirectSound->CreateSoundBuffer(&dsbdDesc, &(ret->_channel), NULL);
  if (FAILED(result)) {
    delete ret;
    ret = (WinSample*)0L;
  }

  if (ret) {
    BYTE* dst = NULL;
    dst = ret->lock();
    try {
      memcpy(dst, wavData, wavSize);
    }
    catch(...) {
      delete ret;
      ret = (WinSample*)0L;
    }
    if (ret)
      ret->unlock();
  }

  if (wavData)
    delete [] wavData;

  return ret;
}

void WinSample::destroy(AudioTraits::SampleClass* sample) {
  delete sample;
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

AudioTraits::MusicClass::MusicStatus WinMusic::status(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusic::status()" << endl;

  if (_performance && _music) {
    if (_performance->IsPlaying(_music, NULL) == S_OK) {
      if (audio_cat->is_debug())
	audio_cat->debug() << "returning PLAYING" << endl;
      return PLAYING;
    }
  } else
    if (audio_cat->is_debug())
      audio_cat->debug() << "MusicStatus no performance or music!" << endl;
  if (audio_cat->is_debug())
    audio_cat->debug() << "returning READY" << endl;
  return READY;
}

WinMusic* WinMusic::load_midi(Filename filename) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusic::load_midi()" << endl;
  initialize();
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

  DMUS_OBJECTDESC fdesc;
  fdesc.guidClass = CLSID_DirectMusicSegment;
  fdesc.dwSize = sizeof(DMUS_OBJECTDESC);
  // MULTI_TO_WIDE(filename.c_str(), fdesc.wszFileName);
  MULTI_TO_WIDE(fdesc.wszFileName, filename.c_str());
  // fdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME | DMUS_OBJ_FULLPATH;
  fdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
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

void WinMusic::destroy(AudioTraits::MusicClass* music) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinMusic::destroy()" << endl;
  delete music;
  if (audio_cat->is_debug())
    audio_cat->debug() << "out of WinMusic::destroy()" << endl;
}

WinPlayer* WinPlayer::_global_instance = (WinPlayer*)0L;

WinPlayer::~WinPlayer(void) {
}

void WinPlayer::play_sample(AudioTraits::SampleClass* sample) {
  initialize();
  WinSample* wsample = (WinSample*)sample;
  LPDIRECTSOUNDBUFFER chan = wsample->get_channel();
  if (chan) {
    chan->Stop();
    HRESULT result = chan->Play(0, 0, 0);
    if (FAILED(result))
      audio_cat->error() << "sample play failed" << endl;
  }
}

void WinPlayer::play_music(AudioTraits::MusicClass* music) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in WinPlayer::play_music()" << endl;
  initialize();
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
    audio_cat->debug() << "out of WinPlayer::play_music()" << endl;
}

void WinPlayer::set_volume(AudioTraits::SampleClass*, int) {
}

void WinPlayer::set_volume(AudioTraits::MusicClass*, int) {
}

WinPlayer* WinPlayer::get_instance(void) {
  if (_global_instance == (WinPlayer*)0L)
    _global_instance = new WinPlayer();
  return _global_instance;
}
