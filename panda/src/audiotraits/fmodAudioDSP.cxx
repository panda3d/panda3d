// Filename: fmodAudioDSP.cxx
// Created by:  Stan Rosenbaum "Staque" - Spring 2006
//
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "dcast.h"

#ifdef HAVE_FMODEX //[

//Panda Headers
#include "config_audio.h"
#include "fmodAudioDSP.h"


TypeHandle FmodAudioDSP::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::FmodAudioDSP
//       Access: Protected
//  Description: Constructor
//               This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
FmodAudioDSP::
FmodAudioDSP(AudioManager *manager, AudioManager::DSP_category cat) {
  // Intentionally blank.

  audio_debug("FmodAudioDSP::FmodAudioDSP() Creating new DSP " );
  
  //Local Variables that are needed.
  FMOD_RESULT result;

  //Assign the values we need
  DCAST_INTO_V(_manager, manager);

  FMOD_DSP_TYPE dsptype = (FMOD_DSP_TYPE)cat;

  result = _manager->_system->createDSPByType( dsptype, &_dsp);
  fmod_audio_errcheck("_system->createDSPByType()", result);

  set_in_chain(false);

  audio_debug("DSP Loaded");
}




////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::Destructor
//       Access: Published, Virtual
//  Description: DESTRUCTOR!!!
////////////////////////////////////////////////////////////////////
FmodAudioDSP::
~FmodAudioDSP() {
  audio_debug("FmodAudioSound::FmodAudioDSP() Destruction!!! " );

  //Local Variables that are needed.
  FMOD_RESULT result;

  result = _dsp->remove();
  fmod_audio_errcheck("_dsp->remove()", result);

  result = _dsp->release();
  fmod_audio_errcheck("_dsp->release()", result);

  audio_debug("DSP GONE");
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::reset
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
//               [This resets an FMOD DSP to its default values]
////////////////////////////////////////////////////////////////////
void FmodAudioDSP::
reset() {
  audio_debug("FmodAudioSound::reset() Reset DSP to default settings." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  result = _dsp->reset();
  fmod_audio_errcheck("_dsp->reset()", result);

  audio_debug("DSP Reset.");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::remove
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
//               [This removes the DSP from an Effects Chain]
////////////////////////////////////////////////////////////////////
void FmodAudioDSP::
remove() {
  audio_debug("FmodAudioSound::remove() Removes a DSP from and effect chain." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  result = _dsp->remove();
  fmod_audio_errcheck("_dsp->remove()", result);

  audio_debug("DSP Removed from relative effects chain.");
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::set_bypass
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
//         [This turns the Bypass for an Effect on and off]/
////////////////////////////////////////////////////////////////////
void FmodAudioDSP::
set_bypass(bool bypass) {
  audio_debug("FmodAudioSound::set_bypass() ." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  result = _dsp->setBypass(bypass);
  fmod_audio_errcheck("_dsp->setBypass()", result);

  audio_debug("DSP Bypass set to:" << bypass );
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_bypass
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
bool FmodAudioDSP::
get_bypass() {
  audio_debug("FmodAudioSound::get_bypass() ." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  bool bypass;

  result = _dsp->getBypass(&bypass);
  fmod_audio_errcheck("_dsp->getBypass()", result);

  return bypass;
}



////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::set_parameter
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
void FmodAudioDSP::
set_parameter(const string &name, float value) {
  int parameterIndex = find_parameter(name);
  if (parameterIndex < 0) {
    return;
  }

  //Local Variables that are needed.
  FMOD_RESULT result;

  result = _dsp->setParameter(parameterIndex, value);
  fmod_audio_errcheck("_dsp->setParameter()", result);
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_num_parameters
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
int FmodAudioDSP::
get_num_parameters() {
  audio_debug("FmodAudioSound::get_num_parameters() ." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  int numOfParameters;

  result = _dsp->getNumParameters(&numOfParameters);
  fmod_audio_errcheck("_dsp->getNumParameters()", result);

  return numOfParameters;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_parameter_name
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
string FmodAudioDSP::
get_parameter_name(int parameterIndex) {

  audio_debug("FmodAudioSound::get_parameter_name()" );

  //Local Variables that are needed.
  FMOD_RESULT result;

  //int   parameterIndex;
  char  parameterName[32]; 
  char  parameterLabel[32]; 
  char  parameterDescription[32];
  int   parameterDescriptionLength = 0;
  float parameterMin;
  float parameterMax;

  result = _dsp->getParameterInfo(parameterIndex, parameterName, parameterLabel, parameterDescription, parameterDescriptionLength, &parameterMin, &parameterMax);
  fmod_audio_errcheck("_dsp->getParameterInfo()", result);

  string returnInfo = (parameterName);

  return returnInfo;

  //return "";
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_parameter_description
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
//               This Method actually returns FMOD's Parameter Label
//               Information, and not Description.
//               The reason is, that most of the FMOD's Description
//               Properties seem to be empty.
//               Also the Label sort of serves as as a description by
//               return the type of unit the cooresponding parameter
//               modifies for a DSP.
//               IE.  For the Echo.   The first parameter is 'Delay'
//               and the units for measuring the Delay is in Milliseconds.
//               The Label returns Milliseconds letting you know that.
////////////////////////////////////////////////////////////////////
string FmodAudioDSP::
get_parameter_description(int parameterIndex) {
  // intentionally blank

  audio_debug("FmodAudioSound::get_parameter_description()." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  //int   parameterIndex;
  char  parameterName[32]; 
  char  parameterLabel[32]; 
  char  parameterDescription[32];
  int   parameterDescriptionLength = 0;
  float parameterMin;
  float parameterMax;

  result = _dsp->getParameterInfo(parameterIndex, parameterName, parameterLabel, parameterDescription, parameterDescriptionLength, &parameterMin, &parameterMax);
  fmod_audio_errcheck("_dsp->getParameterInfo()", result);

  return parameterLabel;

}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_parameter_min
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float FmodAudioDSP::
get_parameter_min(int parameterIndex) {
  
  audio_debug("FmodAudioSound::get_parameter_min()." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  //int   parameterIndex;
  char  parameterName[32]; 
  char  parameterLabel[32]; 
  char  parameterDescription[32];
  int   parameterDescriptionLength = 0;
  float parameterMin;
  float parameterMax;

  result = _dsp->getParameterInfo(parameterIndex, parameterName, parameterLabel, parameterDescription, parameterDescriptionLength, &parameterMin, &parameterMax);
  fmod_audio_errcheck("_dsp->getParameterInfo()", result);

  return parameterMin;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_parameter_max
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float FmodAudioDSP::
get_parameter_max(int parameterIndex) {
  
  audio_debug("FmodAudioSound::get_parameter_min()." );

  //Local Variables that are needed.
  FMOD_RESULT result;

  //int   parameterIndex;
  char  parameterName[32]; 
  char  parameterLabel[32]; 
  char  parameterDescription[32];
  int   parameterDescriptionLength = 0;
  float parameterMin;
  float parameterMax;

  result = _dsp->getParameterInfo(parameterIndex, parameterName, parameterLabel, parameterDescription, parameterDescriptionLength, &parameterMin, &parameterMax);
  fmod_audio_errcheck("_dsp->getParameterInfo()", result);

  return parameterMax;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_parameter_value
//       Access: Published, Virtual
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
float FmodAudioDSP::
get_parameter_value(const string &name) {
  
  int parameterIndex = find_parameter(name);
  if (parameterIndex < 0) {
    return 0.0;
  }

  //Local Variables that are needed.
  FMOD_RESULT result;
 
  float parameterValue; 
  char  valuestr[32];
  int   valuestrlen = 32;


  result = _dsp->getParameter(parameterIndex, &parameterValue, valuestr, valuestrlen);
  fmod_audio_errcheck("_dsp->getParameter()", result);

  return parameterValue;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::find_parameter
//       Access: Private
//  Description: Convert a parameter name to an fmod parameter index.
////////////////////////////////////////////////////////////////////
int FmodAudioDSP::
find_parameter(const string &name) {
  int np = get_num_parameters();
  for (int i=0; i<np; i++) {
    if ( name == get_parameter_name(i) ) {

      audio_debug("FmodAudioSound::find_parameter() returning: " << get_parameter_name(i) << " " << i );

      return i;
    }
  }
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_dsp_name()
//       Access: Protected
//  Description: This is a thin wrapper around FMOD-EX.
//               See the FMOD-EX documentation.
////////////////////////////////////////////////////////////////////
string FmodAudioDSP::
get_dsp_name() {
  audio_debug("FmodAudioSound::get_dsp_name()." );

  //Local Variables that are needed.
  FMOD_RESULT result;
  char  name[32];
  unsigned int  version;
  int   channels;
  int   configwidth;
  int   configheight;

  result = _dsp->getInfo(name, &version, &channels, &configwidth, &configheight);
  fmod_audio_errcheck("_dsp->getInfo()", result);

  string returnInfo = (name);
  //returnInfo.append(" Version: ");
  //returnInfo.append(version);
  //returnInfo.append("\n");

  return returnInfo;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::get_in_chain()
//       Access: Published, Virtual
//  Description: This is a functiont to query if a DSP have been assigned
//         to the GLOBAL or a SOUND's effect chain.
//         This is to make sure you 'remove' an effect from a chain
//         before you move it somewhere else or destroy it.
////////////////////////////////////////////////////////////////////
bool FmodAudioDSP::
get_in_chain() {
  audio_debug("FmodAudioSound::get_in_chain()." );

  return _in_chain;
}


////////////////////////////////////////////////////////////////////
//     Function: FmodAudioDSP::set_in_chain()
//       Access: Published, Virtual
//  Description: This is a functiont to set if a DSP have been assigned
//         to the GLOBAL or a SOUND's effect chain.
////////////////////////////////////////////////////////////////////
void FmodAudioDSP::
set_in_chain(bool chain_state) {
  audio_debug("FmodAudioSound::set_in_chain()." );

  _in_chain = chain_state;
}


#endif //]
