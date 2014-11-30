// Device drivers for the LUDL family of translation stage controllers.
// The only device currently implemented is the USBMAC6000 controller in its
// two-axis configuration controlling the stage for the Panoptes system
// at UNC Chapel Hill.

#ifndef VRPN_LUDL_H
#define VRPN_LUDL_H

#include "vrpn_Analog.h"
#include "vrpn_Analog_Output.h"

#if defined(VRPN_USE_LIBUSB_1_0)
#include <libusb.h>

// This driver uses the VRPN-preferred LibUSB-1.0 to control the device.
// It exposes the vrpn_Analog and the
// vrpn_Analog_Output interfaces, to report and set the stage position.
//
// Analog/Analog_Output channel 0 is X (0 to maximum #ticks, in ticks).
// Analog/Analog_Output channel 1 is Y (0 to maximum #ticks, in ticks).

class vrpn_LUDL_USBMAC6000 :
  public vrpn_Analog, 
  public vrpn_Analog_Output
{
public:
  vrpn_LUDL_USBMAC6000(const char *name, vrpn_Connection *c = 0, bool do_recenter = false);
  virtual ~vrpn_LUDL_USBMAC6000();

  virtual void mainloop();

protected:
  struct libusb_context       *_context;          // LibUSB context used for this device
  struct libusb_device_handle *_device_handle;    // Handle for the USB device
  struct timeval _timestamp;
  unsigned  _endpoint;        // Which endpoint to use to communicate with the device

  // Buffer to store incoming data from the device and count of how many characters
  // we got.  Function to check and read any incoming data (and set _incount).
  // Function to parse accumulated data.
  static const unsigned _INBUFFER_SIZE = 1024;
  vrpn_uint8 _inbuffer[_INBUFFER_SIZE]; // MUST CHANGE the sizeof() code if this becomes not an array.
  unsigned _incount;
  bool check_for_data();  // False if error.  True even if no data.
  bool interpret_usbmac_ascii_response(const vrpn_uint8 *buffer, int *value_return);

  // XXX I think we're using ASCII send and response because Ryan had some kind of trouble
  // with the binary send/response.  Not sure about this, though.  It sure would be
  // faster to read and easier to parse the binary ones, which all have the same length.

  // vrpn_Analog overridden methods.
  // Send report iff changed
  void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE);
  // Send report whether or not changed
  void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE);

  // No actual types to register, derived classes will be analogs and analog_outputs
  int register_types(void) { return 0; }

  // Handlers for the Analog_Output messages.
  /// Responds to a request to change one of the values by
  /// setting the channel to that value.
  static int VRPN_CALLBACK handle_request_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Responds to a request to change multiple channels at once.
  static int VRPN_CALLBACK handle_request_channels_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Responds to a connection request with a report of the values
  static int VRPN_CALLBACK handle_connect_message(void *userdata, vrpn_HANDLERPARAM p);

private:
  // Helper functions for communication with the stage
  void  flush_input_from_ludl(void);
  bool  send_usbmac_command(unsigned device, unsigned command, unsigned index, int value);
  bool  recenter(void);
  bool  ludl_axis_moving(unsigned axis);  // Returns true if the axis is still moving
  bool  move_axis_to_position(int axis, int position);

  // Stores whether we think each axis is moving and where we think each axis is
  // going if it is moving.  These are set in the move_axis_to_position() routine
  // and used in the mainloop() routine to decide if it is time to report that we
  // have gotten where we want to be.
  bool    *_axis_moving;
  vrpn_float64  *_axis_destination;
};

// end of OS selection
#endif

// end of VRPN_LUDL_H
#endif

