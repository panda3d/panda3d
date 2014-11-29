import cphidgets
from Phidget import Phidget

class Accelerometer(Phidget):
    """
    the PhidgetAccelerometer device. This device is a two axis accelerometer.
    """
    def __init__(self,serialNumber=-1,exclusive=0,threaded=1):
        """
        @param serialNumber: the unique serial number of the board to connect to.
            -1 will choose the first available board. default is first available.
        @param exclusive: if true, request exclusive access to this device so that
            no other program can connect to it. default is non-exclusive.
        @param threaded: if true, this class will start a thread to call the read()
            function so that you don't need to call it yourself. default is to
            use the read thread. You probably don't want to change this.
            
        @raise phidgets.error: if there are no available devices of this type, this
            serial number doesn't exist, if exclusive access can't be granted, or
            if a device is already in use by another program with exclusive access.
        """
        Phidget.__init__(self)
        self.ptr = cphidgets.Accelerometer_open(serialNumber,exclusive)
        
        if (threaded):
            cphidgets.Accelerometer_startReadThread(self.ptr)
    # end __init__
    
    def close(self):
        """
        terminates the connection with the device
        """
        if (self.ptr):
            cphidgets.Accelerometer_close(self.ptr)
    # end close
    
    def __del__(self):
        self.close()
        Phidget.__del__(self)
    # end __del__
    
    def read(self):
        """
        polls the device for new data. Phidgets devices must be polled or the 
        data returned will not change. If you created this device with the the
        parameter to start the read thread, the default, then you shouldn't need
        to call this function.
        
        @returns: 0 on sucess, else there was a problem reading
        """
        if (self.ptr):
            return cphidgets.Accelerometer_read(self.ptr)
        else:
            return -1
    # end read
    
    def getNumAxes(self):
        """
        get the number of axes this accelerometer has
        
        @returns: the number of axes of this device
        """
        if (self.ptr):
            return cphidgets.Accelerometer_getNumAxis(self.ptr)
        else:
            return 0
    # end getNumAxes
    
    def getAcceleration(self,index):
        """
        get the amount of acceleration on an axis of this accelerometer
        @param index: the index of the axis you want to read. In the range
            of 0 to L{getNumAxes()<getNumAxes>}
        @returns: the acceleration on this axis
        """
        if (self.ptr):
            return cphidgets.Accelerometer_getAcceleration(self.ptr,index)
        else:
            return -1
    # end getAcceleration
    
    def getAccelerationChangeTrigger(self,index):
        """
        get the amount of change necessary to trigger an acceleration change event
        @param index: the index of the axis you want check the trigger for. In the range
            of 0 to L{getNumAxes()<getNumAxes>}
        @returns: the amount of change necessary to trigger an acceleration change event on analog input C{index}
        """
        if (self.ptr):
            return (cphidgets.Accelerometer_getAccelerationChangeTrigger(self.ptr,index))
        else:
            return -1
    # end getAccelerationChangeTrigger

    def setAccelerationChangeTrigger(self,index,val):
        """
        set the amount of change necessary to trigger an acceleration change event
        @param index: the index of the axis you want to change the trigger amount on. In the range
            of 0 to L{getNumAxes()<getNumAxes>}
        @param val: the amount of change necessary to trigger an acceleration change event on analog input C{index}
        """
        if (self.ptr):
            cphidgets.Accelerometer_setAccelerationChangeTrigger(self.ptr,index,val)
    # end setAccelerationChangeTrigger

    def setAccelerationChangeHandler(self,callback):
        """
        register a callback when the acceleration changes on an axis of this accelerometr
        
        @param callback: a function to call when the acceleration changes on an axis of this accelerometer. This
            function should accept two parameters, the C{index} of the changed axis
            and the new acceleration C{value} of the axis.
        """
        if (self.ptr):
            cphidgets.Accelerometer_setAccelerationChangeHandler(self.ptr,callback)
    # end setAccelerationChangeHandler
    
# end class Accelerometer