import cphidgets

class Phidget:
    """
    Superclass for all types of Phidgets devices. You should not ever instantiate
    this directly.
    """
    def __init__(self):
        self.ptr = None
    # end __init__
    
    def __del__(self):
        if (self.ptr):
            cphidgets.phidgets_delete(self.ptr)
            self.ptr = None
    # end __del__
    
    def getDeviceType(self):
        """
        get the string describing the name of the phidget. All PhidgetInterfaceKits will return "PhidgetInterfaceKit", PhidgetRFID returns "PhidgetRFID" and so on.
        
        @returns: the string describing the name of the phidget
        @rtype: string
        """
        if (self.ptr):
            return cphidgets.phidgets_getDeviceType(self.ptr)
        return None
    # end getDeviceType
    
    def getSerialNumber(self):
        """
        returns the unique serial number of this phidget. This number is set during manufacturing, and is unique across all phidgets.
               
        @returns: the unique serial number of this phidget
        @rtype: long
        """
        if (self.ptr):
            return cphidgets.phidgets_getSerialNumber(self.ptr)
        return None
    # end getSerialNumber
    
    def getDeviceVersion(self):
        """
        gets the version of this hardware device
        
        @returns: the hardware version of this device
        @rtype: long
        """
        if (self.ptr):
            return cphidgets.phidgets_getDeviceVersion(self.ptr)
        return None
    # end getDeviceVersion

    def getDeviceStatus(self):
        """
        Returns an integer indicating the status of the device
        
        @returns: an integer indicating the status of the device
        @rtype: int
        """        
        if (self.ptr):
            return cphidgets.phidgets_getDeviceStatus(self.ptr)
        return None
    # end getDeviceStatus
# end class Phidget

