/** @file	vrpn_Analog_5dtUSB.h
	@brief	header for 5DT USB (HID) dataglove driver

	@date	2011

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

#ifndef VRPN_ANALOG_5DTUSB_H
#define VRPN_ANALOG_5DTUSB_H

#include "vrpn_HumanInterface.h"
#include "vrpn_Analog.h"
#include <string>

#if defined(VRPN_USE_HID)

/** @brief 5th Dimension Technologies (5dt) "Ultra" USB data glove driver

	This supports connecting to 5dt gloves over USB that present a HID device interface.
	This includes the 5DT Data Glove 5 Ultra and the 5DT Data Glove 14 Ultra, as well
	as either of those using the wireless kit. (I, Ryan Pavlik, have only tested this
	with the 5DT Data Glove 5 Ultras since that's all I have access to, but support based
	on what I would expect from a 14-sensor glove is included.)
	
	Each sensor's raw values are exposed as an analog channel (5 or 14) between 0.0 and 1.0.
	Note that there is pretty significant need for calibration since the used range within the
	entire representable range is pretty small and seemingly device-dependent.
	Your code will probably have to at least perform some scaling based on establishing a
	"min" and "max" for each sensor.

	For serial 5dt glove access, see the vrpn_5DT16 (for the 16-sensor model) and 
	vrpn_5dt (for the 5-sensor wired or wireless model, in vrpn_Analog_5dt.h) classes.

	The inherited method vrpn_Analog::getNumChannels() 

	This base class does all the work: the inherited classes just create the right filters
	and input for the base class.
*/
class VRPN_API vrpn_Analog_5dtUSB : public vrpn_Analog, protected vrpn_HidInterface {
	public:
		/// Destructor
		virtual ~vrpn_Analog_5dtUSB();

		/// Standard VRPN mainloop method.
		virtual void mainloop();

		/// Returns a string description of the device we've connected to. Used internally,
		/// but also possibly useful externally.
		std::string get_description() const;

		/// Accessor to know if this is a left hand glove.
		bool isLeftHand() const { return _isLeftHand; }

		/// Accessor to know if this is a right hand glove.
		bool isRightHand() const { return !_isLeftHand; }

	protected:
		/// Protected constructor: use a subclass to specify the glove variant to use.
		vrpn_Analog_5dtUSB(vrpn_HidAcceptor *filter, int num_sensors,
		                   bool isLeftHand, const char *name, vrpn_Connection *c = 0);
		/// Extracts the sensor values from each report.
		void on_data_received(size_t bytes, vrpn_uint8 *buffer);

		/// Timestamp updated during mainloop()
		struct timeval _timestamp;

		/// The raw values extracted from the report: which ones we use to set
		/// analog channels varies based on the kind of device this is.
		double _rawVals[16];

		/// Flag for left handedness.
		bool _isLeftHand;

		/// Flag indicating whether we were connected last time through the mainloop.
		/// Used to send a "normal"-severity message when we connect with info on the
		/// device.
		bool _wasConnected;

		/// Send report iff changed
		void report_changes(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
		/// Send report whether or not changed
		void report(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
};

/// Specialization of vrpn_Analog_5dtUSB for a 5-sensor, left-hand glove.
class VRPN_API vrpn_Analog_5dtUSB_Glove5Left: public vrpn_Analog_5dtUSB {
	public:
		vrpn_Analog_5dtUSB_Glove5Left(const char *name, vrpn_Connection *c = 0);
		virtual ~vrpn_Analog_5dtUSB_Glove5Left() {};
};

/// Specialization of vrpn_Analog_5dtUSB for a 5-sensor, right-hand glove.
class VRPN_API vrpn_Analog_5dtUSB_Glove5Right: public vrpn_Analog_5dtUSB {
	public:
		vrpn_Analog_5dtUSB_Glove5Right(const char *name, vrpn_Connection *c = 0);
		virtual ~vrpn_Analog_5dtUSB_Glove5Right() {};
};

/// Specialization of vrpn_Analog_5dtUSB for a 14-sensor, left-hand glove.
/// Not tested as of 8-Mar-2011 because I don't have access to one.
class VRPN_API vrpn_Analog_5dtUSB_Glove14Left: public vrpn_Analog_5dtUSB {
	public:
		vrpn_Analog_5dtUSB_Glove14Left(const char *name, vrpn_Connection *c = 0);
		virtual ~vrpn_Analog_5dtUSB_Glove14Left() {};
};

/// Specialization of vrpn_Analog_5dtUSB for a 14-sensor, right-hand glove.
/// Not tested as of 8-Mar-2011 because I don't have access to one.
class VRPN_API vrpn_Analog_5dtUSB_Glove14Right: public vrpn_Analog_5dtUSB {
	public:
		vrpn_Analog_5dtUSB_Glove14Right(const char *name, vrpn_Connection *c = 0);
		virtual ~vrpn_Analog_5dtUSB_Glove14Right() {};
};

/// HID acceptor subclass used by vrpn_Analog_5dtUSB since the bits of
/// the product ID for these devices describe the device in a useful way.
class VRPN_API vrpn_HidProductMaskAcceptor: public vrpn_HidAcceptor {
	public:
		vrpn_HidProductMaskAcceptor(vrpn_uint16 vendorId, vrpn_uint16 productMask = 0x0000, vrpn_uint16 desiredProduct = 0x0000) :
			vendor(vendorId),
			product(desiredProduct),
			mask(productMask) {}

		~vrpn_HidProductMaskAcceptor() {}

		bool accept(const vrpn_HIDDEVINFO &device) {
			return (device.vendor == vendor) && ((device.product & mask) == (product & mask));
		}
	private:
		vrpn_uint16 vendor;
		vrpn_uint16 product;
		vrpn_uint16 mask;
};

#endif // end of ifdef VRPN_USE_HID

// end of VRPN_ANALOG_5DTUSB_H
#endif

