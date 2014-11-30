#ifndef INCLUDED_BUTTONFLY
#define INCLUDED_BUTTONFLY

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"
#include "vrpn_Analog.h"
#include <quat.h>

const int vrpn_BUTTONFLY_MAXAXES = 200;

// This parameter is passed to the constructor for the ButtonFly; it
// describes the action of a single button, describing which
// direction to translate or rotate and how fast.  Translation is
// expressed as a vector with the length of the translation, expressed
// in meters/second.  Rotation is specified as a vector with the number
// of rotations to make per second around X, Y, and Z.

class VRPN_API vrpn_TBF_axis {

  public:

    vrpn_TBF_axis(void)
      { strcpy(name,""); channel = 0;
	vec[0] = vec[1] = vec[2] = 0.0;
	rot[0] = rot[1] = rot[2] = 0.0;
	absolute = false;
      };
    vrpn_TBF_axis(const char *n, int ch, const float v[],
	const float rv[], bool absolut)
      { strncpy(name, n, sizeof(name)-1); name[sizeof(name)-1] = '\0';
	channel = ch;
	memcpy(vec, v, sizeof(vec));
	memcpy(rot, rv, sizeof(rot));
	absolute = absolut;
      };

    char  name[200];  //< Name of the Button device driving this axis
    int	  channel;    //< Which channel to use from the Button device
    float vec[3];     //< Vector telling how far and which way to move in 1 second
    float rot[3];     //< Vector telling rotation about X,Y,and Z
    bool  absolute;   //< Whether this is an absolute or differential change
};

class VRPN_API vrpn_Tracker_ButtonFlyParam {

  public:

    vrpn_Tracker_ButtonFlyParam (void) {
      strcpy(vel_scale_name, "");
      strcpy(rot_scale_name, "");
      num_axes = 0;
    }

    /// Add an axis command to the parameter list
    bool  add_axis(const vrpn_TBF_axis &b) {
      if (num_axes >= vrpn_BUTTONFLY_MAXAXES) { return false; }
      axes[num_axes] = b;
      num_axes++;
      return true;
    }

    /// List of buttons that control axes
    vrpn_TBF_axis axes[vrpn_BUTTONFLY_MAXAXES];
    int		  num_axes;	//< How many axes have been filled in

    /// Analog device that scales the translation
    char  vel_scale_name[200];
    int	  vel_scale_channel;
    float vel_scale_offset;
    float vel_scale_scale;
    float vel_scale_power;

    /// Analog device that scales the rotation
    char  rot_scale_name[200];
    int	  rot_scale_channel;
    float rot_scale_offset;
    float rot_scale_scale;
    float rot_scale_power;
};

class VRPN_API vrpn_Tracker_ButtonFly;	// Forward reference

class VRPN_API vrpn_TBF_fullaxis {
  public:
    vrpn_TBF_fullaxis (void) { btn = NULL; active = false; bf = NULL; };

    vrpn_TBF_axis axis;
    vrpn_Button_Remote * btn;
    vrpn_Tracker_ButtonFly * bf;
    bool  active;
};

/// This class will turn a button device into a tracker by interpreting
// the buttons as constant-velocity inputs and "flying" the user around based
// on which buttons are held down for how long.
// The mapping from buttons to directions (or orientation changes) is
// described in the vrpn_Tracker_ButtonFlyParam parameter. Translations are
// specified as vectors giving direction and speed (meters per second).
// Rotations are given as an axis to rotate around and speed to rotate 
// (revolutions per second) around the given axis.
// The time reported by button trackers is the local time that the report was
// generated.  Velocity reports are also generated for the tracker.

// If reportChanges is TRUE, updates are ONLY sent if there has been a
// change since the last update, in which case they are generated no faster
// than update_rate. 

class VRPN_API vrpn_Tracker_ButtonFly : public vrpn_Tracker {
  public:
    vrpn_Tracker_ButtonFly (const char * name, vrpn_Connection * trackercon,
			    vrpn_Tracker_ButtonFlyParam * params,
                            float update_rate,
                            bool reportChanges = VRPN_FALSE);

    virtual ~vrpn_Tracker_ButtonFly (void);

    virtual void mainloop ();
    virtual void reset (void);

    void update (q_matrix_type &);

    static int VRPN_CALLBACK handle_newConnection (void *, vrpn_HANDLERPARAM);

  protected:

    double	    d_update_interval;	//< How long to wait between sends
    struct timeval  d_prevtime;		//< Time of the previous report
    bool       d_reportChanges;	//< Report only when something changes?

    vrpn_TBF_fullaxis	d_axes[vrpn_BUTTONFLY_MAXAXES];
    int			d_num_axes;

    /// Analog device that scales the translation
    vrpn_Analog_Remote	*d_vel_scale;
    char  d_vel_scale_name[200];
    int	  d_vel_scale_channel;
    float d_vel_scale_offset;
    float d_vel_scale_scale;
    float d_vel_scale_power;
    float d_vel_scale_value;	//< Value computed from above, scales vel

    /// Analog device that scales the rotation
    vrpn_Analog_Remote	*d_rot_scale;
    char  d_rot_scale_name[200];
    int	  d_rot_scale_channel;
    float d_rot_scale_offset;
    float d_rot_scale_scale;
    float d_rot_scale_power;
    float d_rot_scale_value;	//< Value computed from above, scales rotation

    /// Button that resets the transformation
    vrpn_Button_Remote	* d_reset_button;
    int			d_which_button;

    /// Initial, current, and velocity matrices for the tracker
    q_matrix_type d_initMatrix, d_currentMatrix, d_velMatrix;

    void    update_matrix_based_on_values (double time_interval);
    void    convert_matrix_to_tracker (void);

    bool shouldReport (double elapsedInterval);

    int setup_channel(vrpn_TBF_fullaxis * full);
    int teardown_channel(vrpn_TBF_fullaxis * full);

    static void	VRPN_CALLBACK handle_velocity_update(void * userdata, const vrpn_ANALOGCB info);
    static void	VRPN_CALLBACK handle_rotation_update(void * userdata, const vrpn_ANALOGCB info);
    static void	VRPN_CALLBACK handle_button_update(void * userdata, const vrpn_BUTTONCB info);
    static void VRPN_CALLBACK handle_reset_press(void * userdata, const vrpn_BUTTONCB info);
};

#endif
