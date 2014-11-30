#ifndef INCLUDED_ANALOGFLY
#define INCLUDED_ANALOGFLY

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"

#include <quat.h>

// This parameter is passed to the constructor for the AnalogFly; it describes
// the channel mapping and parameters of that mapping, as well as the button
// that will be used to reset the tracker when it is pushed. Any entry which
// has a NULL pointer for the name is disabled.

class VRPN_API vrpn_TAF_axis {

  public:

    vrpn_TAF_axis (void)
      { name = NULL; channel = 0; offset = 0.0f; thresh = 0.0f;
        scale = 1.0f; power = 1.0f; };

	char * name;	//< Name of the Analog device driving this axis
        int channel;	//< Which channel to use from the Analog device
	float offset;	//< Offset to apply to values from this channel to reach 0
        float thresh;	//< Threshold to apply after offset within which values count as zero
        float scale;	//< Scale applied to values after offset and threshold
        float power;	//< Power to which values are taken after scaling
};

class VRPN_API vrpn_Tracker_AnalogFlyParam {

  public:

    vrpn_Tracker_AnalogFlyParam (void) {
        x.name = y.name = z.name =
        sx.name = sy.name = sz.name = reset_name = clutch_name = NULL;
    }

    /// Translation along each of these three axes
    vrpn_TAF_axis x, y, z;

    /// Rotation in the positive direction about the three axes
    vrpn_TAF_axis sx, sy, sz;

    /// Button device that is used to reset the matrix to the origin

    char * reset_name;
    int reset_which;

    /// Clutch device that is used to enable relative motion over
    // large distances

    char * clutch_name;
    int clutch_which;
};

class VRPN_API vrpn_Tracker_AnalogFly;	// Forward reference

class VRPN_API vrpn_TAF_fullaxis {
public:
        vrpn_TAF_fullaxis (void) { ana = NULL; value = 0.0; af = NULL; };

	vrpn_TAF_axis axis;
        vrpn_Analog_Remote * ana;
	vrpn_Tracker_AnalogFly * af;
        double value;
};

/// This class will turn an analog device such as a joystick or a camera
// tracker into a tracker by interpreting the joystick
// positions as either position or velocity inputs and "flying" the user
// around based on analog values.
// The "absolute" parameter tells whether the tracker integrates differential
// changes (the default, with FALSE) or takes the analog values as absolute
// positions or orientations.
// The mapping from analog channels to directions (or orientation changes) is
// described in the vrpn_Tracker_AnalogFlyParam parameter. For translations,
// values above threshold are multiplied by the scale and then taken to the
// power; the result is the number of meters (or meters per second) to move
// in that direction. For rotations, the result is taken as the number of
// revolutions (or revolutions per second) around the given axis.
// Note that the reset button has no effect on an absolute tracker.
// The time reported by absolute trackers is as of the last report they have
// had from their analog devices.  The time reported by differential trackers
// is the local time that the report was generated.  This is to allow a
// gen-locked camera tracker to have its time values passed forward through
// the AnalogFly class.

// If reportChanges is TRUE, updates are ONLY sent if there has been a
// change since the last update, in which case they are generated no faster
// than update_rate. 

// If worldFrame is TRUE, then translations and rotations take place in the
// world frame, rather than the local frame. Useful for a simulated wand
// when doing desktop testing of immersive apps - easier to keep under control.

class VRPN_API vrpn_Tracker_AnalogFly : public vrpn_Tracker {
  public:
    vrpn_Tracker_AnalogFly (const char * name, vrpn_Connection * trackercon,
			    vrpn_Tracker_AnalogFlyParam * params,
                            float update_rate, bool absolute = vrpn_FALSE,
                            bool reportChanges = VRPN_FALSE, bool worldFrame = VRPN_FALSE);

    virtual ~vrpn_Tracker_AnalogFly (void);

    virtual void mainloop ();
    virtual void reset (void);

    void update (q_matrix_type &);

    static void VRPN_CALLBACK handle_joystick (void *, const vrpn_ANALOGCB);
    static int VRPN_CALLBACK handle_newConnection (void *, vrpn_HANDLERPARAM);

  protected:

    double	    d_update_interval;	//< How long to wait between sends
    struct timeval  d_prevtime;		//< Time of the previous report
    bool	    d_absolute;		//< Report absolute (vs. differential)?
    bool       d_reportChanges;
    bool       d_worldFrame;

    vrpn_TAF_fullaxis	d_x, d_y, d_z, d_sx, d_sy, d_sz;
    vrpn_Button_Remote	* d_reset_button;
    int			d_which_button;

    vrpn_Button_Remote	* d_clutch_button;
    int			d_clutch_which;
    bool                d_clutch_engaged;

    q_matrix_type d_initMatrix, d_currentMatrix, d_clutchMatrix;

    void    update_matrix_based_on_values (double time_interval);
    void    convert_matrix_to_tracker (void);

    bool shouldReport (double elapsedInterval) const;

    int setup_channel (vrpn_TAF_fullaxis * full);
    int teardown_channel (vrpn_TAF_fullaxis * full);

    static void	VRPN_CALLBACK handle_analog_update (void * userdata,
                                      const vrpn_ANALOGCB info);
    static void VRPN_CALLBACK handle_reset_press (void * userdata, const vrpn_BUTTONCB info);
    static void VRPN_CALLBACK handle_clutch_press (void * userdata, const vrpn_BUTTONCB info);
};

#endif
