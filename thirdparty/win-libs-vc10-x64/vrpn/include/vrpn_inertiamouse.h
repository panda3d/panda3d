//-----------------------------------------------------------------------------------------
// Driver for the Bauhaus University Weimar "inertiamouse" device.  The class for this
// device is found at the end of the file, after two helper classes.

#ifndef VRPN_INERTIAMOUSE_H
#define VRPN_INERTIAMOUSE_H

#include "vrpn_Analog.h"
#include "vrpn_Button.h"

// Helper classes
class VRPN_API dcblocker {
public: // ctors, dtor
    dcblocker () 
        : in_ (0.0),
          out_ (0.0)
    {}
    dcblocker (dcblocker const& o) 
        : in_ (o.in_), 
          out_ (o.out_)
    {}
    ~dcblocker () throw () {}
public: // methods
    void swap (dcblocker& o) throw ()
    {
        double t;
        t = in_;  in_ = o.in_;    o.in_ = t;
        t = out_; out_ = o.out_;  o.out_ = t;
    }
    dcblocker& operator= (dcblocker const& o)
    {
        dcblocker tmp (o);
        swap (tmp);
        return *this;
    }
    double filter (double s)
    {
        out_ = s - in_ + (0.95 * out_);
        in_ = s;
        return out_; 
    }
    void reset ()
    {
        in_ = out_ = 0.0;
    }
private: // variables
    double in_;
    double out_;
};

// Helper classes
/*
 *  butterworth lowpass
 */
class VRPN_API lowpass {
public: // ctors, dtor
    lowpass ()
    {
        in_[0] = 0.0;
        in_[1] = 0.0;
        out_[0] = 0.0;
        out_[1] = 0.0;
    }
    lowpass (lowpass const& o)
    {
        in_[0] = o.in_[0];
        in_[1] = o.in_[1];
        out_[0] = o.out_[0];
        out_[1] = o.out_[1];
    }
    ~lowpass () throw () {}
public: // methods
    double filter (double s)
    {
        in_[0] = in_[1];
        in_[1] = s / 6.242183581;
        out_[0] = out_[1];
        out_[1] = in_[0] + in_[1] + (0.6795992982 * out_[0]);
        return out_[1];
    }
    void reset ()
    {
        in_[0] = in_[1] = out_[0] = out_[1] = 0.0;
    }
private: // variables
    double in_[2];
    double out_[2];
};

class VRPN_API vrpn_inertiamouse : public vrpn_Serial_Analog , public vrpn_Button {
public: // constants

    enum {
        Channels = 6,
        Buttons = 2,
        Update_Interval_Hz = 7372800 / 64 / 13 / Channels,
    };
    static const double Vel_Decay;

public: // construction/destruction
    // ctor
    vrpn_inertiamouse (const char* name, 
                       vrpn_Connection* c,
                       const char* port, 
                       int baud_rate);

    // factory method
    static vrpn_inertiamouse* create (const char* name, 
            vrpn_Connection* c,
            const char* port, 
            int baud_rate);
    // dtor
    ~vrpn_inertiamouse () { if (vel_) { delete [] vel_;} };

public: // virtual methods

    /// Called once through each main loop iteration to handle updates.
    virtual void mainloop ();

    virtual int reset(void);	//< Set device back to starting config

protected:
    int status_;		//< Used by mainloop() and get_report()
    int numbuttons_;		//< How many buttons to open
    int numchannels_;		//< How many analog channels to open
    
    int expected_chars_;	//< How many characters to expect in the report
    unsigned char buffer_[512];	//< Buffer of characters in report
    int bufcount_;		//< How many characters we have so far
    
    int null_radius_;		//< The range over which no motion should be 
                                //  reported
    
    struct timeval timestamp;	//< Time of the last report from the device

    double *vel_;               // velocity update

    dcblocker dcb_[Channels]; // dc blockers for all Channels
    lowpass lp_[Channels];    // lowpass filters for all Channels

    // Set all buttons, analogs and encoders back to 0
    virtual void clear_values(void);
    
    /// Try to read a report from the device.  Returns 1 if complete report received,
    /// 0 otherwise.  Sets _status to match current status.
    virtual int get_report(void);
    
    /// send report iff changed
    virtual void report_changes (vrpn_uint32 class_of_service
                                = vrpn_CONNECTION_LOW_LATENCY);
    /// send report whether or not changed
    virtual void report (vrpn_uint32 class_of_service
                         = vrpn_CONNECTION_LOW_LATENCY);
    
    // NOTE:  class_of_service is only applied to vrpn_Analog
    //  values, not vrpn_Button, which are always vrpn_RELIABLE
};

#endif
