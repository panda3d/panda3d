import cphidgets
from Phidget import Phidget

class InterfaceKit(Phidget):
    """
    the PhidgetInterfaceKit device. This device handles digital input, digital
    output, and analog input.
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
        self.ptr = cphidgets.InterfaceKit_open(serialNumber,exclusive)
        
        if (threaded):
            cphidgets.InterfaceKit_startReadThread(self.ptr)
    # end __init__
    
    def close(self):
        """
        terminates the connection with the device
        """
        if (self.ptr):
            cphidgets.InterfaceKit_close(self.ptr)
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
            return cphidgets.InterfaceKit_read(self.ptr)
        else:
            return -1
    # end read
    
    def getNumInputs(self):
        """
        get the number of digital inputs this device can read
        
        @returns: the number of digital inputs on this device
        """
        if (self.ptr):
            return cphidgets.InterfaceKit_getNumInputs(self.ptr)
        else:
            return 0
    # end getNumInputs
    
    def getNumAnalogs(self):
        """
        get the number of analog sensors this device can read

        @returns: the number of analog sensors on this device
        """
        if (self.ptr):
            return cphidgets.InterfaceKit_getNumSensors(self.ptr)
        else:
            return 0
    # end getNumAnalogs
    
    def getNumOutputs(self):
        """
        gets the number of digital outputs this device has
        
        @returns: the number of digital outputs on this device
        """
        if (self.ptr):
            return cphidgets.InterfaceKit_getNumOutputs(self.ptr)
        else:
            return 0
    # end getNumOutputs
    
    def getInput(self,index):
        """
        get the state of a digital input
        @param index: the index of the digital input you want to read. In the range
            of 0 to L{getNumInputs()<getNumInputs>}
        @returns: the state of input C{index}, 0 or 1
        """
        if (self.ptr):
            return cphidgets.InterfaceKit_getInputState(self.ptr,index)
        else:
            return -1
    # end getInput
    
    def getAnalog(self,index):
        """
        get the value of an analog sensor input
        @param index: the index of the analog sensor input you want to read. In the range
            of 0 to L{getNumAnalogs()<getNumAnalogs>}
        @returns: the value of analog sensor C{index} in the range 0.0-1.0
        """
        if (self.ptr):
            return cphidgets.InterfaceKit_getSensorValue(self.ptr,index)/1000.0
        else:
            return -1
    # end getAnalog

    def getOutput(self,index):
        """
        get the current output state of a digital output
        @param index: the index of the digital output you want to check. In the range
            of 0 to L{getNumOutputs()<getNumOutputs>}
        @returns: the current state of output C{index}
        """
        if (self.ptr):
            return cphidgets.InterfaceKit_getOutputState(self.ptr,index)
        else:
            return -1
    # end getOutput

    def setOutput(self,index,state):
        """
        set the state of a digital output
        @param index: the index of the digital output you want to set. In the range
            of 0 to L{getNumOutputs()<getNumOutputs>}
        @param state: the state to set output C{index} to. 0 or 1
        """
        if (self.ptr):
            cphidgets.InterfaceKit_setOutputState(self.ptr,index,state)
    # end setOutput
    
    def getAnalogChangeTrigger(self,index):
        """
        get the amount of change necessary to trigger an analog change event
        @param index: the index of the analog input you want check the trigger for. In the range
            of 0 to L{getNumAnalogs()<getNumAnalogs>}
        @returns: the amount of change necessary to trigger an analog change event on analog input C{index}
        """
        if (self.ptr):
            return (cphidgets.InterfaceKit_getSensorChangeTrigger(self.ptr,index)/1000.0)
        else:
            return -1
    # end getAnalogChangeTrigger

    def setAnalogChangeTrigger(self,index,val):
        """
        set the amount of change necessary to trigger an analog change event
        @param index: the index of the analog input you want to change the trigger amount on. In the range
            of 0 to L{getNumAnalogs()<getNumAnalogs>}
        @param val: the amount of change necessary to trigger an analog change event on analog input C{index}
        """
        if (self.ptr):
            cphidgets.InterfaceKit_setSensorChangeTrigger(self.ptr,index,int(min(max(val*1000,0),1000)))
    # end setAnalogChangeTrigger

    def setInputChangeHandler(self,callback):
        """
        register a callback when a digital input changes
        
        @param callback: a function to call when a digital input changes. This
            function should accept two parameters, the C{index} of the changed input
            and the new C{value} of the input.
        """
        if (self.ptr):
            cphidgets.InterfaceKit_setInputChangeHandler(self.ptr,callback)
    # end setInputChangeHandler
    
    def setAnalogChangeHandler(self,callback):
        """
        register a callback when an analog input changes
        
        @param callback: a function to call when an analog input changes. This
            function should accept two parameters, the C{index} of the changed input
            and the new C{value} of the input.
        """
        if (self.ptr):
            cphidgets.InterfaceKit_setSensorChangeHandler(self.ptr,callback)
    # end setAnalogChangeHandler
# end class InterfaceKit