#ifndef VRPN_KEYBOARD_H
#define VRPN_KEYBOARD_H

///////////////////////////////////////////////////////////////////////////
// vrpn_Keyboard is a VRPN server class to publish events from the PC's keyboard.
// It provides a 256-channel vrpn_Button for keyboard buttons, reporting the
// scan codes for the key.
//
// This implementation is Windows-specific, as it leverages the windows mouse calls.
//

#include "vrpn_Button.h"

class VRPN_API vrpn_Keyboard: public vrpn_Button_Filter
{
  public:
    vrpn_Keyboard (const char * name, vrpn_Connection * c);
    ~vrpn_Keyboard () ;

    /// Called once through each main loop iteration to handle updates.
    virtual void mainloop ();

  protected:
    /// Read the current status.  Return 1 if a report was found,
    // 0 otherwise (this only makes sense for buffered implementations;
    // return 0 if it is not a buffered implementation.
    virtual	int get_report(void);
};

#endif
