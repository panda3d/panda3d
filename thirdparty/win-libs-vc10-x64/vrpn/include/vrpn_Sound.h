// vrpn_Sound.h
// 
// April 12 2000 - ZK

#ifndef	VRPN_SOUND_H

#include "vrpn_Tracker.h"
#include "vrpn_Text.h"


#define MAX_MATERIAL_NAME_LENGTH 128
#define MAX_NUMBER_SOUNDS 1024
#define MAX_NUMBER_MATERIALS 64
#define MAX_NUMBER_POLYGONS 2048
#define MAX_FILENAME_LENGTH 2048

// everything is on order found in these structs!

    typedef vrpn_int32 vrpn_SoundID;

    typedef struct _vrpn_PoseDef
	{
	  vrpn_float64 position[3];
	  vrpn_float64 orientation[4];
	} vrpn_PoseDef;

	typedef struct _vrpn_SoundDef
	{
	  vrpn_PoseDef pose;
	  vrpn_float64 velocity[4];
	  vrpn_float64 max_front_dist;
	  vrpn_float64 min_front_dist;
	  vrpn_float64 max_back_dist;
	  vrpn_float64 min_back_dist;
	  vrpn_float64 cone_inner_angle;
	  vrpn_float64 cone_outer_angle;
	  vrpn_float64 cone_gain;
	  vrpn_float64 dopler_scale;
	  vrpn_float64 equalization_val;
	  vrpn_float64 pitch;
	  vrpn_float32 volume;		// Jason Clark calls this volume, but really its gain!
	} vrpn_SoundDef;

	typedef struct _vrpn_ListenerDef
	{
	  vrpn_PoseDef pose;
	  vrpn_float64 velocity[4];
	} vrpn_ListenerDef;

	typedef struct _vrpn_MaterialDef 
	{
		char         material_name[MAX_MATERIAL_NAME_LENGTH];
		vrpn_float64 transmittance_gain;
		vrpn_float64 transmittance_highfreq;
		vrpn_float64 reflectance_gain;
		vrpn_float64 reflectance_highfreq;
	} vrpn_MaterialDef;

	typedef struct _vrpn_QuadDef
	{
		vrpn_int32   subQuad;  // really a bool
		vrpn_float64 openingFactor;
		vrpn_int32	 tag;
		vrpn_float64 vertices[4][3];
		char         material_name[MAX_MATERIAL_NAME_LENGTH];
	} vrpn_QuadDef;

	typedef struct _vrpn_TriDef
	{
		vrpn_int32   subTri;
		vrpn_float64 openingFactor;
		vrpn_int32	 tag;
		vrpn_float64 vertices[3][3];
		char         material_name[MAX_MATERIAL_NAME_LENGTH];
	} vrpn_TriDef;
	
class VRPN_API vrpn_Sound : public vrpn_BaseClass {

public:
	vrpn_Sound(const char * name, vrpn_Connection * c);
	~vrpn_Sound();

protected:

	vrpn_int32 load_sound_local;			 // ID of message to load a sound from server side
	vrpn_int32 load_sound_remote;			 // ID of message to load a sound from client side
	vrpn_int32 unload_sound;				 // ID of message to unload a sound
	vrpn_int32 play_sound;					 // ID of message to play a sound
	vrpn_int32 stop_sound;					 // ID of message to stop a sound
	vrpn_int32 change_sound_status;			 // ID of message to change the sound's status
	vrpn_int32 set_listener_pose;			 // ID of message to set the listener's pos/orient
	vrpn_int32 set_listener_velocity;		 // ID of message to set the listener's velocity
	vrpn_int32 set_sound_pose;				 // 
	vrpn_int32 set_sound_velocity;			 //
	vrpn_int32 set_sound_distanceinfo;		 //
	vrpn_int32 set_sound_coneinfo;			 //
	vrpn_int32 set_sound_doplerfactor;		 //
	vrpn_int32 set_sound_eqvalue;			 //
	vrpn_int32 set_sound_pitch;
	vrpn_int32 set_sound_volume;		     //

	vrpn_int32 load_model_local;			 // load model file from server side
	vrpn_int32 load_model_remote;            // load model file from client side
	vrpn_int32 load_polyquad;				 // ID of message to load a quad polygon
	vrpn_int32 load_polytri;				 // ID of message to load a tri polygon
	vrpn_int32 load_material;				 // ID of message to load a material definition
	vrpn_int32 set_polyquad_vertices;		
	vrpn_int32 set_polytri_vertices;
	vrpn_int32 set_poly_openingfactor;
	vrpn_int32 set_poly_material;

	vrpn_int32 receive_text_message;
		
	struct timeval timestamp;				 // Current timestamp

	int register_types(void);

	/*All encodes and decodes functions are for the purpose of setting up
	  messages to be sent over the network properly (ie to put them in one
	  char buffer and to put them in proper network order and for getting
	  the messages back into a usable format once they have been received*/

	/*Note encodeSound allocates space dynamically for buf, it is your
	  responsibility to free it up*/
	vrpn_int32 encodeSound_local(const char *filename, const vrpn_SoundID id, const vrpn_SoundDef soundDef, char **buf);
	/*Note decodeSound allocates space dynamically for filename, it is your
	  responsibility to free it up*/
	vrpn_int32 decodeSound_local(const char *buf, char **filename, vrpn_SoundID *id, vrpn_SoundDef * soundDef, const int payload);

	// These two are not supported yet!
	vrpn_int32 encodeSound_remote(const char *filename, const vrpn_SoundID id, char **buf);
	vrpn_int32 decodeSound_remote(const char *buf, char **filename, vrpn_SoundID *id, const int payload);

	vrpn_int32 encodeSoundID(const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundID(const char* buf, vrpn_SoundID *id);
	vrpn_int32 encodeSoundDef(const vrpn_SoundDef sound, const vrpn_SoundID id, const vrpn_int32 repeat, char* buf);
	vrpn_int32 decodeSoundDef(const char* buf, vrpn_SoundDef *sound, vrpn_SoundID *id, vrpn_int32 *repeat);
	vrpn_int32 encodeSoundPlay(const vrpn_SoundID id, const vrpn_int32 repeat, char* buf);
	vrpn_int32 decodeSoundPlay(const char* buf, vrpn_SoundID *id, vrpn_int32 *repeat);
	vrpn_int32 encodeListenerVelocity(const vrpn_float64 *velocity, char* buf);
	vrpn_int32 decodeListenerVelocity(const char* buf, vrpn_float64 *velocity);
	vrpn_int32 encodeListenerPose(const vrpn_PoseDef pose, char* buf);
	vrpn_int32 decodeListenerPose(const char* buf, vrpn_PoseDef * pose);


	vrpn_int32 encodeSoundPose(const vrpn_PoseDef pose, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundPose(const char* buf, vrpn_PoseDef *pose, vrpn_SoundID *id);
	vrpn_int32 encodeSoundVelocity(const vrpn_float64 *velocity, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundVelocity(const char* buf, vrpn_float64 *velocity, vrpn_SoundID *id);
	vrpn_int32 encodeSoundDistInfo(const vrpn_float64 min_back, 
		                           const vrpn_float64 max_back, 
								   const vrpn_float64 min_front, 
								   const vrpn_float64 max_front, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundDistInfo(const char* buf, vrpn_float64 *min_back,
		                                            vrpn_float64 * max_back,
													vrpn_float64 * min_front,
													vrpn_float64 * max_front, vrpn_SoundID *id);
	vrpn_int32 encodeSoundConeInfo(const vrpn_float64 cone_inner_angle,
		                           const vrpn_float64 cone_outer_angle,
								   const vrpn_float64 cone_gain, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundConeInfo(const char* buf, vrpn_float64 *cone_inner_angle,
		                                            vrpn_float64 *cone_outer_angle,
								                    vrpn_float64 *cone_gain, vrpn_SoundID *id);
  vrpn_int32 encodeSoundDoplerScale(const vrpn_float64 doplerfactor, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundDoplerScale(const char* buf, vrpn_float64 *doplerfactor, vrpn_SoundID *id);
	vrpn_int32 encodeSoundEqFactor(const vrpn_float64 eqfactor, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundEqFactor(const char* buf, vrpn_float64 *eqfactor, vrpn_SoundID *id);
	vrpn_int32 encodeSoundPitch(const vrpn_float64 pitch, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundPitch(const char* buf, vrpn_float64 *pitch, vrpn_SoundID *id);
	vrpn_int32 encodeSoundVolume(const vrpn_float64 volume, const vrpn_SoundID id, char* buf);
	vrpn_int32 decodeSoundVolume(const char* buf, vrpn_float64 *volume, vrpn_SoundID *id);

	vrpn_int32 encodeLoadModel_local(const char *filename, char **buf);
	vrpn_int32 decodeLoadModel_local(const char *buf, char **filename, const int payload);

	// Remote stuff not supported yet!
	vrpn_int32 encodeLoadModel_remote(const char *filename, char **buf);
	vrpn_int32 decodeLoadModel_remote(const char *buf, char **filename, const int payload);
   
  vrpn_int32 encodeLoadPolyQuad(const vrpn_QuadDef quad, char* buf);
	vrpn_int32 decodeLoadPolyQuad(const char* buf, vrpn_QuadDef * quad);
	vrpn_int32 encodeLoadPolyTri(const vrpn_TriDef tri, char* buf);
	vrpn_int32 decodeLoadPolyTri(const char* buf, vrpn_TriDef * tri);
	vrpn_int32 encodeLoadMaterial(const vrpn_int32 id, const vrpn_MaterialDef material, char* buf);
	vrpn_int32 decodeLoadMaterial(const char* buf, vrpn_MaterialDef * material, vrpn_int32 * id);
	vrpn_int32 encodeSetQuadVert(const vrpn_float64 vertices[4][3], const vrpn_int32 tag, char* buf);
	vrpn_int32 decodeSetQuadVert(const char* buf, vrpn_float64 (* vertices)[4][3], vrpn_int32 *tag);
	vrpn_int32 encodeSetTriVert(const vrpn_float64 vertices[3][3], const vrpn_int32 tag, char* buf);
	vrpn_int32 decodeSetTriVert(const char* buf, vrpn_float64 (*vertices)[3][3], vrpn_int32 *tag);
	vrpn_int32 encodeSetPolyOF(const vrpn_float64 openingfactor, const vrpn_int32 tag, char* buf);
	vrpn_int32 decodeSetPolyOF(const char* buf, vrpn_float64 * openingfactor, vrpn_int32 *tag);
	vrpn_int32 encodeSetPolyMaterial(const char * material, const vrpn_int32 tag, char* buf);
	vrpn_int32 decodeSetPolyMaterial(const char* buf, char ** material, vrpn_int32 *tag, const int payload);
};

class VRPN_API vrpn_Sound_Client : public vrpn_Sound, public vrpn_Text_Receiver {
public:
	vrpn_Sound_Client(const char * name, vrpn_Connection * c);
	~vrpn_Sound_Client();

	//This command starts a sound playing, the repeat value indicates how
	//many times to play it.  Continuously if repeat is set to 0
	vrpn_int32 playSound(const vrpn_SoundID id, vrpn_int32 repeat);
	vrpn_int32 stopSound(const vrpn_SoundID id);
	//Loads a sound into memory on the server side, returns the ID value to be
	//used to refer to the sound from now on.  Pass in the path and filename
	vrpn_SoundID loadSound(const char* sound, const vrpn_SoundID id, const vrpn_SoundDef soundDef);
	vrpn_int32   unloadSound(const vrpn_SoundID id);

	//All the functions with change and sound in them, can change either an
	//already playing sound or one yet to be played
	vrpn_int32 setSoundVolume(const vrpn_SoundID id, const vrpn_float64 volume);
	vrpn_int32 setSoundPose(const vrpn_SoundID id, vrpn_float64 position[3], vrpn_float64 orientation[4]);
	vrpn_int32 setSoundVelocity(const vrpn_SoundID id, const vrpn_float64 velocity[4]);
	vrpn_int32 setSoundDistances(const vrpn_SoundID id, 
		                           const vrpn_float64 max_front_dist, 
								               const vrpn_float64 min_front_dist, 
								               const vrpn_float64 max_back_dist, 
								               const vrpn_float64 min_back_dist);
	vrpn_int32 setSoundConeInfo(const vrpn_SoundID id,
		                        const vrpn_float64 inner_angle,
            								const vrpn_float64 outer_angle,
						            		const vrpn_float64 gain);

	vrpn_int32 setSoundDopScale(const vrpn_SoundID id, vrpn_float64 dopfactor);
	vrpn_int32 setSoundEqValue(const vrpn_SoundID id, vrpn_float64 eq_value);
	vrpn_int32 setSoundPitch(const vrpn_SoundID id, vrpn_float64 pitch);

	vrpn_int32 setListenerPose(const vrpn_float64 position[3], const vrpn_float64 orientation[4]);
	vrpn_int32 setListenerVelocity(const vrpn_float64 velocity[4]);

    vrpn_int32 LoadModel_local(const char *filename);
	
	// Remote stuff not supported yet!
	vrpn_int32 LoadModel_remote(const char *data);
	   
    vrpn_int32 LoadPolyQuad(const vrpn_QuadDef quad);
	vrpn_int32 LoadPolyTri(const vrpn_TriDef tri);
	vrpn_int32 LoadMaterial(const vrpn_int32 id, const vrpn_MaterialDef material);
	
	vrpn_int32 setMaterialName(const int id, const char * materialname);
	vrpn_int32 setMaterialTransGain(const int id, const vrpn_float64 transmittance_gain);
	vrpn_int32 setMaterialTransHF(const int id, const vrpn_float64 transmittance_hf);
	vrpn_int32 setMaterialReflGain(const int id, const vrpn_float64 reflectance_gain);
	vrpn_int32 setMaterialReflHF(const int id, const vrpn_float64 reflectance_hf);

	vrpn_int32 setPolyOF(const int id, const vrpn_float64 OF);
	vrpn_int32 setQuadVertices(const int id, const vrpn_float64 vertices[4][3]);
	vrpn_int32 setPolyMaterialName(const int id, const char * materialname);
	
	vrpn_int32 setTriVertices(const int id, const vrpn_float64 vertices[3][3]);
	
	virtual void mainloop();

    virtual void receiveTextMessage(const char * message, vrpn_uint32 type,vrpn_uint32 level, struct timeval	msg_time);

protected:

private:
  static void VRPN_CALLBACK handle_receiveTextMessage(void *userdata, const vrpn_TEXTCB t);
 
};


/*Note on the server design
  The server is designed in such a way that it expects a sub-class that is implemented
  that actually implements sound functionality to have certain functions that it can
  call to tell the child to play, load, whatever.   This parent server class, handles
  all of the callback functionality and decoding, allowing child classes to only have 
  to worry about sound functionality*/
#ifndef VRPN_CLIENT_ONLY
class VRPN_API vrpn_Sound_Server : public vrpn_Sound, public vrpn_Text_Sender
{
public:
	vrpn_Sound_Server(const char * name, vrpn_Connection * c);
	~vrpn_Sound_Server();

	virtual void playSound(vrpn_SoundID id, vrpn_int32 repeat, vrpn_SoundDef soundDef) = 0;
	virtual void loadSoundLocal(char* filename, vrpn_SoundID id, vrpn_SoundDef soundDef) = 0;
  virtual void loadSoundRemote(char* file, vrpn_SoundID id, vrpn_SoundDef soundDef) = 0;
	virtual void stopSound(vrpn_SoundID id) = 0;
	virtual void unloadSound(vrpn_SoundID id) = 0;
	virtual void changeSoundStatus(vrpn_SoundID id, vrpn_SoundDef soundDef) = 0;
	virtual void setListenerPose(vrpn_PoseDef pose) = 0;
	virtual void setListenerVelocity(vrpn_float64 *velocity) = 0;

	virtual void setSoundPose(vrpn_SoundID id, vrpn_PoseDef pose) = 0;
	virtual void setSoundVelocity(vrpn_SoundID id, vrpn_float64 *velocity) = 0;
	virtual void setSoundDistInfo(vrpn_SoundID id, vrpn_float64 *distinfo) = 0;
	virtual void setSoundConeInfo(vrpn_SoundID id, vrpn_float64 *coneinfo) = 0;
  
	virtual void setSoundDoplerFactor(vrpn_SoundID id, vrpn_float64 doplerfactor) = 0;
	virtual void setSoundEqValue(vrpn_SoundID id, vrpn_float64 eqvalue) = 0;
	virtual void setSoundPitch(vrpn_SoundID id, vrpn_float64 pitch) = 0;
	virtual void setSoundVolume(vrpn_SoundID id, vrpn_float64 volume) = 0;
	virtual void loadModelLocal(const char * filename) = 0;
	virtual void loadModelRemote() = 0;	// not supported
	virtual void loadPolyQuad(vrpn_QuadDef * quad) = 0;
	virtual void loadPolyTri(vrpn_TriDef * tri) = 0;
	virtual void loadMaterial(vrpn_MaterialDef * material, vrpn_int32 id) = 0;
	virtual void setPolyQuadVertices(vrpn_float64 vertices[4][3], const vrpn_int32 id) = 0;
	virtual void setPolyTriVertices(vrpn_float64 vertices[3][3], const vrpn_int32 id) = 0;
	virtual void setPolyOF(vrpn_float64 OF, vrpn_int32 tag) = 0;
	virtual void setPolyMaterial(const char * material, vrpn_int32 tag) = 0;
	
protected:

private:
	
	static int VRPN_CALLBACK handle_loadSoundLocal(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_loadSoundRemote(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_unloadSound(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_playSound(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_stopSound(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_changeSoundStatus(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setListenerPose(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setListenerVelocity(void *userdata, vrpn_HANDLERPARAM p);

	static int VRPN_CALLBACK handle_setSoundPose(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setSoundVelocity(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setSoundDistanceinfo(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setSoundConeinfo(void *userdata, vrpn_HANDLERPARAM p);

    static int VRPN_CALLBACK handle_setSoundDoplerfactor(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setSoundEqvalue(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setSoundPitch(void *userdata, vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_setSoundVolume(void *userdata, vrpn_HANDLERPARAM p);

	static int VRPN_CALLBACK handle_loadModelLocal(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_loadModelRemote(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_loadPolyquad(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_loadPolytri(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_loadMaterial(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setPolyquadVertices(void *userdata, vrpn_HANDLERPARAM p);		
	static int VRPN_CALLBACK handle_setPolytriVertices(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setPolyOpeningfactor(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_setPolyMaterial(void *userdata, vrpn_HANDLERPARAM p);
	
};
#endif //#ifndef VRPN_CLIENT_ONLY

#define VRPN_SOUND_H
#endif
