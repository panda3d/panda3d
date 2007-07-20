// Filename: nullAudioDSP.h
// Created by:  Stan Rosenbaum "Staque" - Spring 2006

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

#ifndef __NULLAUDIODSP_H__
#define __NULLAUDIODSP_H__

#include "config_audio.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"

class AudioManager;


class EXPCL_PANDA_AUDIO nullAudioDSP : public TypedReferenceCount {
	PUBLISHED:
	    virtual void reset();
	    virtual void remove();
		virtual void set_bypass(bool bypass);
		virtual void set_parameter(const string &name, float value);
		virtual float get_parameter_value(const string &name);

		virtual bool get_bypass();
		void list_parameters_info();

		virtual int get_num_parameters();
		virtual string get_parameter_name(int index);
		virtual string get_parameter_description(int index);
		virtual float get_parameter_min(int index);
		virtual float get_parameter_max(int index);

		
		virtual ~nullAudioDSP();

	protected:
		nullAudioDSP();

		////////////////////////////////////////////////////////////
		//These are needed for Panda's Pointer System. DO NOT ERASE!
		////////////////////////////////////////////////////////////

	public:
		static TypeHandle get_class_type() {
			return _type_handle;
		}
		static void init_type() {
			TypedReferenceCount::init_type();
			register_type(_type_handle, "nullAudioDSP",
						TypedReferenceCount::get_class_type());
		}
		virtual TypeHandle get_type() const {
			return get_class_type();
		}
		virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

	private:
		static TypeHandle _type_handle;
		
		////////////////////////////////////////////////////////////
		//DONE
		////////////////////////////////////////////////////////////

	};

#endif /* __nullAudioDSP_H__ */
