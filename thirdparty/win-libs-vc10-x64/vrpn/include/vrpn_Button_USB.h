#ifndef	VRPN_BUTTON_USB_H
#ifdef	_WIN32
#include "vrpn_Button.h"

// USB button code. 
// This class is derived from the vrpn_Button_Filter class, so that it
// can be made to toggle its buttons using messages from the client.
class VRPN_API vrpn_Button_USB : public vrpn_Button {
  public:
	vrpn_Button_USB(const char *name, const char *deviceName, vrpn_Connection *c);
	~vrpn_Button_USB();

	virtual void mainloop();

  protected:
	void read(void);
	//! writes data to the device
	bool USBWrite(const unsigned long &data);
	//! reads data from the device
	bool USBRead(unsigned long &data, int port);
	//! basic io handeling
	bool USB_IO(unsigned long lIn, int lInSize, unsigned long &lOut, int lOutSize);
	HANDLE m_hDevice;	//!< the usb device
};
#endif
#endif
