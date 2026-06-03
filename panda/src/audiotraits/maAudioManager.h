/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_maAudio.h
 * @author Katie & J0y
 */


#ifndef MAAUDIOMANAGER_H
#define MAAUDIOMANAGER_H

#include "pandabase.h"

#include "audioManager.h"
#include "plist.h"
#include "pmap.h"
#include "pset.h"
#include "movieAudioCursor.h"
#include "reMutex.h"
#include "vector_string.h"


#include "miniaudio.h"

class MaAudioSound;


class EXPCL_MA_AUDIO MaAudioManager : public AudioManager {
    friend class MaAudioSound;

    public:
        // Constructor and Destructor
        MaAudioManager();
        virtual ~MaAudioManager();

        virtual int get_speaker_setup();
        void set_speaker_setup(SpeakerModeCategory cat);
        bool configure_filters(FilterProperties *config);

        virtual void shutdown();

        virtual bool is_valid();

        virtual PT(AudioSound) get_sound(const Filename &file_name, bool positional = false, int mode=SM_heuristic);
        virtual PT(AudioSound) get_sound(MovieAudio *source, bool positional = false, int mode=SM_heuristic);

        virtual void uncache_sound(const Filename &file_name);
        virtual void clear_cache();
        virtual void set_cache_limit(unsigned int count);
        virtual unsigned int get_cache_limit() const;

        virtual void set_volume(PN_stdfloat volume);
        virtual PN_stdfloat get_volume() const;
        
        virtual void set_active(bool flag);
        virtual bool get_active() const;

        virtual void set_concurrent_sound_limit(unsigned int limit = 0);
        virtual unsigned int get_concurrent_sound_limit() const;

        virtual void reduce_sounds_playing_to(unsigned int count);
        virtual void stop_all_sounds();
        virtual void update(); // *must* be called every frame

        virtual void audio_3d_set_listener_attributes(
            PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
            PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz,
            PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz,
            PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz
        );

        virtual void audio_3d_get_listener_attributes(
            PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz,
            PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz,
            PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz,
            PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz
        );

        virtual void audio_3d_set_distance_factor(PN_stdfloat factor);
        virtual PN_stdfloat audio_3d_get_distance_factor() const;

        virtual void audio_3d_set_doppler_factor(PN_stdfloat factor);
        virtual PN_stdfloat audio_3d_get_doppler_factor() const;

        virtual void audio_3d_set_drop_off_factor(PN_stdfloat factor);
        virtual PN_stdfloat audio_3d_get_drop_off_factor() const;
        
        virtual void output(std::ostream &out) const;
        virtual void write(std::ostream &out) const;
        virtual void register_AudioManager_creator(Create_AudioManager_proc* proc);
        
        virtual TypeHandle get_type() const;
        virtual TypeHandle force_init_type();
    
}