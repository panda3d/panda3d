#ifndef VRPN_DIRECTXFFJOYSTICK_H
#define VRPN_DIRECTXFFJOYSTICK_H

#include "vrpn_Connection.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_ForceDevice.h"
#include "vrpn_ForceDeviceServer.h"

#if defined(_WIN32) && defined(VRPN_USE_DIRECTINPUT)
#ifndef DIRECTINPUT_VERSION
#define	DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <basetsd.h>

class VRPN_API vrpn_DirectXFFJoystick: public vrpn_Analog
			     ,public vrpn_Button
			     ,public vrpn_ForceDeviceServer
{
public:
    /// Set forceRate to 0 to disable force feedback
    vrpn_DirectXFFJoystick (const char * name, vrpn_Connection * c,
      double readRate = 60, double forceRate = 200);

    ~vrpn_DirectXFFJoystick ();

    // Called once through each main loop iteration to handle
    // updates.
    virtual void mainloop ();

protected:
    HWND  _hWnd;	// Handle to the console window
    int	  _status;

    int	  _numbuttons;	  // How many buttons
    int	  _numchannels;	  // How many analog channels
    int	  _numforceaxes;  // How many force-feedback channels we have

    struct timeval _timestamp;	// Time of the last report from the device
    double _read_rate;		// How many times per second to read the device
    double _force_rate;		// How many times per second to update forces

    double _fX, _fY;		// Force to display in X and Y

    virtual int get_report(void);	// Try to read a report from the device
    void	clear_values(void);	// Clear the Analog and Button values

    // send report iff changed
    virtual void report_changes (vrpn_uint32 class_of_service
	      = vrpn_CONNECTION_LOW_LATENCY);
    // send report whether or not changed
    virtual void report (vrpn_uint32 class_of_service
	      = vrpn_CONNECTION_LOW_LATENCY);
    // NOTE:  class_of_service is only applied to vrpn_Analog
    //  values, not vrpn_Button

    // Send forces to joystick, where forces range from -1 to 1 on X and Y axes.
    void  send_normalized_force(double fx, double fy);

    HRESULT InitDirectJoystick( void );
    LPDIRECTINPUT8	  _DirectInput;   // Handle to Direct Input
    LPDIRECTINPUTDEVICE8  _Joystick;	  // Handle to the joystick we are using
    LPDIRECTINPUTEFFECT	  _ForceEffect;	  // Handle to the constant force effect
    static  BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* selfPtr );
    static  BOOL CALLBACK    EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* selfPtr );

    static  int	VRPN_CALLBACK handle_last_connection_dropped(void *selfPtr, vrpn_HANDLERPARAM p);
    static  int	VRPN_CALLBACK handle_plane_change_message(void *selfPtr, vrpn_HANDLERPARAM p);
    static  int VRPN_CALLBACK handle_forcefield_change_message(void *selfPtr, vrpn_HANDLERPARAM p);

    //-------------------------------------------------------------------------------
    // None of the scene-orienting or object-creation methods are supported yet, but
    // we need to create non-empty functions to handle them.

    // Add an object to the haptic scene as root (parent -1 = default) or as child (ParentNum =the number of the parent)
    virtual bool addObject(vrpn_int32 objNum, vrpn_int32 ParentNum=-1) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // Add an object next to the haptic scene as root 
    virtual bool addObjectExScene(vrpn_int32 objNum) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };
 
    // vertNum normNum and triNum start at 0
    virtual bool setVertex(vrpn_int32 objNum, vrpn_int32 vertNum,vrpn_float32 x,vrpn_float32 y,vrpn_float32 z) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // NOTE: ghost dosen't take normals, 
    //       and normals still aren't implemented for Hcollide
    virtual bool setNormal(vrpn_int32 objNum, vrpn_int32 normNum,vrpn_float32 x,vrpn_float32 y,vrpn_float32 z) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    virtual bool setTriangle(vrpn_int32 objNum, vrpn_int32 triNum,vrpn_int32 vert0,vrpn_int32 vert1,vrpn_int32 vert2,
	    vrpn_int32 norm0=-1,vrpn_int32 norm1=-1,vrpn_int32 norm2=-1) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    virtual bool removeTriangle(vrpn_int32 objNum, vrpn_int32 triNum) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };
    
    // should be called to incorporate the above changes into the 
    // displayed trimesh 
    virtual bool updateTrimeshChanges(vrpn_int32 objNum,vrpn_float32 kspring, vrpn_float32 kdamp, vrpn_float32 fdyn, vrpn_float32 fstat) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // set trimesh type
    virtual bool setTrimeshType(vrpn_int32 objNum,vrpn_int32 type) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // set the trimesh's homogen transform matrix (in row major order)
    virtual bool setTrimeshTransform(vrpn_int32 objNum, vrpn_float32 homMatrix[16]) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // set position of an object
    virtual bool setObjectPosition(vrpn_int32 objNum, vrpn_float32 Pos[3]) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // set orientation of an object
    virtual bool setObjectOrientation(vrpn_int32 objNum, vrpn_float32 axis[3], vrpn_float32 angle) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // set Scale of an object
    virtual bool setObjectScale(vrpn_int32 objNum, vrpn_float32 Scale[3]) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // remove an object from the scene
    virtual bool removeObject(vrpn_int32 objNum) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    virtual bool clearTrimesh(vrpn_int32 objNum) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    /** Functions to organize the scene	**********************************************************/
    // Change The parent of an object
    virtual bool moveToParent(vrpn_int32 objNum, vrpn_int32 ParentNum) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // Set the Origin of the haptic scene
    virtual bool setHapticOrigin(vrpn_float32 Pos[3], vrpn_float32 axis[3], vrpn_float32 angle) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };
    
    // Set the Scale factor of the haptic scene
    virtual bool setHapticScale(vrpn_float32 Scale) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

    // Set the Origin of the haptic scene
    virtual bool setSceneOrigin(vrpn_float32 Pos[3], vrpn_float32 axis[3], vrpn_float32 angle) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };
    
    // make an object touchable or not
    virtual bool setObjectIsTouchable(vrpn_int32 objNum, vrpn_bool IsTouchable=true) {
      struct timeval now;
      gettimeofday(&now, NULL);
      send_text_message("vrpn_DirectXFFJoystick: Called a function not supported",now, vrpn_TEXT_ERROR);
      return false;
    };

};

#endif
#endif

