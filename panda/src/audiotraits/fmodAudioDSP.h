// Filename: fmodAudioDSP.h
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
//[FIRST READ FmodAudioManager and then maybe FmodAudioSound for an Introduction
//if you haven't already].
//
//Hello, all future Panda audio code people! This is my errata documentation to
//Help any future programmer maintain FMOD and PANDA.
//
//Finally let’s talk about the DSP, or Digital Signal Processing. To the layman,
//these are filters and shaders for sound.
//
//Currently FmodAudioDSP give you access to all of FMOD’s built in DSP functions.
//
//The enumerated list for the types are found in the ‘AudioManager.h’, but I will
//repeat it here for good fortune.
//
//	enum DSP_category {
//		// These enumerants line up one-to-one
//		// with the FMOD DSP enumerants.
//		DSP_unknown, 
//		DSP_mixer, 
//		DSP_oscillator, 
//		DSP_lowpass, 
//		DSP_itlowpass, 
//		DSP_highpass, 
//		DSP_echo, 
//		DSP_flange, 
//		DSP_distortion, 
//		DSP_normalize, 
//		DSP_parameq, 
//		DSP_pitchshift, 
//		DSP_chorus, 
//		DSP_reverb, 
//		DSP_vstplugin, 
//		DSP_winampplugin, 
//		DSP_itecho,
//		DSP_COUNT
//	};
//
//Now, I want to point a couple things out. First, we have to place the above list
//in AudioManager.h because that was the only way to them to be ‘PUBLISHED’ in
//Panda’s Python bindings when you build Panda.
//
//Second, you only need to use the ‘it####’ named DSP effects if you are using
//mod/tracker files. [If you don’t know what a mod/tracker file is you probably
//aren’t using them so don’t worry about it.]
//
//I think that is everything, the DSP info was pretty short.
////////////////////////////////////////////////////////////////////
//



#ifndef __FMOD_AUDIO_DSP_H__
#define __FMOD_AUDIO_DSP_H__

#include <pandabase.h>

#ifdef HAVE_FMODEX //[

#include "audioManager.h"
#include "audioDSP.h"

#include <fmod.hpp>
#include <fmod_errors.h>

class EXPCL_FMOD_AUDIO FmodAudioDSP : public AudioDSP {

	public:

		FmodAudioDSP(AudioManager *mgr, AudioManager::DSP_category);

	    virtual void reset();
	    virtual void remove();
		virtual void set_bypass(bool bypass);
		virtual void set_parameter(const string &name, float value);
		virtual float get_parameter_value(const string &name);

		virtual bool get_bypass();

		virtual int get_num_parameters();
		virtual string get_parameter_name(int index);
		virtual string get_parameter_description(int index);
		virtual float get_parameter_min(int index);
		virtual float get_parameter_max(int index);

		bool get_in_chain();
		void set_in_chain(bool chain_state);

		virtual ~FmodAudioDSP();

	protected:
		virtual string get_dsp_name();
		

	private:
		int find_parameter(const string &pn);

		bool              _in_chain;
		FmodAudioManager *_manager;
		FMOD::DSP        *_dsp;

		friend class FmodAudioManager;
		friend class FmodAudioSound;


	////////////////////////////////////////////////////////////
	//These are needed for Panda's Pointer System. DO NOT ERASE!
	////////////////////////////////////////////////////////////

	public:
		static TypeHandle get_class_type() {
			return _type_handle;
		}
		static void init_type() {
			AudioDSP::init_type();
			register_type(_type_handle, "FmodAudioDSP", AudioDSP::get_class_type());
		}
		virtual TypeHandle get_type() const {
			return get_class_type();
		}
		virtual TypeHandle force_init_type() {
			init_type(); 
			return get_class_type();
		}

	private:
		static TypeHandle _type_handle;

	////////////////////////////////////////////////////////////
	//DONE
	////////////////////////////////////////////////////////////

};

#include "fmodAudioDSP.I"

#endif //]

#endif /* __FMOD_AUDIO_DSP_H__ */








