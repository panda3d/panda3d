import sys
import os
import ctypes
import _winreg

"""Class to extract system information from a MS-Windows Box:

Instructions for Using:

Instance the class WindowsSystemInformation
Then in the instance, access the following variables:

os
cpu
totalRAM
totalVM
availableVM
availableRAM

Example:

s = SystemInformation()
print s.os

s.refresh() # if you need to refresh the dynamic data (i.e. Memory stats, etc)
"""

def get_registry_value(key, subkey, value):
    if sys.platform != 'win32':
        raise OSError("get_registry_value is only supported on Windows")
        
    key = getattr(_winreg, key)
    handle = _winreg.OpenKey(key, subkey)
    (value, type) = _winreg.QueryValueEx(handle, value)
    return value

c_ulong = ctypes.c_ulong

class MEMORYSTATUS(ctypes.Structure):
            _fields_ = [
                ('dwLength', c_ulong),
                ('dwMemoryLoad', c_ulong),
                ('dwTotalPhys', c_ulong),
                ('dwAvailPhys', c_ulong),
                ('dwTotalPageFile', c_ulong),
                ('dwAvailPageFile', c_ulong),
                ('dwTotalVirtual', c_ulong),
                ('dwAvailVirtual', c_ulong)
            ]

class SystemInformation:
    def __init__(self):

        # Just in in case somebody called this class by accident, we should
        # check to make sure the OS is MS-Windows before continuing.

        assert sys.platform == 'win32', "Not an MS-Windows Computer. This class should not be called"
        
        # os contains the Operating System Name with Service Pack and Build
        # Example: Microsoft Windows XP Service Pack 2 (build 2600)
        
        self.os = self._os_version().strip()
        
        # cpu contains the CPU model and speed
        # Example: Intel Core(TM)2 CPU 6700 @ 2.66GHz

        self.cpu = self._cpu().strip()

        self.totalRAM, self.availableRAM, self.totalPF, self.availablePF, self.memoryLoad, self.totalVM, self.availableVM = self._ram()

        # totalRam contains the total amount of RAM in the system

        self.totalRAM = self.totalRAM / 1024

        # totalVM contains the total amount of VM available to the system
        
        self.totalVM = self.totalVM / 1024

        # availableVM contains the amount of VM that is free
        
        self.availableVM = self.availableVM / 1024

        # availableRam: Ammount of available RAM in the system
        
        self.availableRAM = self.availableRAM / 1024

    def refresh(self):
         self.totalRAM, self.availableRAM, self.totalPF, self.availablePF, self.memoryLoad, self.totalVM, self.availableVM = self._ram()
         self.totalRAM = self.totalRAM / 1024
         self.totalVM = self.totalVM / 1024
         self.availableVM = self.availableVM / 1024
         self.availableRAM = self.availableRAM / 1024

    def _os_version(self):
        def get(key):
            return get_registry_value(
                "HKEY_LOCAL_MACHINE", 
                "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                key)
        os = get("ProductName")
        sp = get("CSDVersion")
        build = get("CurrentBuildNumber")
        return "%s %s (build %s)" % (os, sp, build)
            
    def _cpu(self):
        return get_registry_value(
            "HKEY_LOCAL_MACHINE", 
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            "ProcessorNameString")
            
    def _ram(self):
        kernel32 = ctypes.windll.kernel32
        

        memoryStatus = MEMORYSTATUS()
        memoryStatus.dwLength = ctypes.sizeof(MEMORYSTATUS)
        kernel32.GlobalMemoryStatus(ctypes.byref(memoryStatus))
        return (memoryStatus.dwTotalPhys, memoryStatus.dwAvailPhys, memoryStatus.dwTotalPageFile, memoryStatus.dwAvailPageFile, memoryStatus.dwMemoryLoad, memoryStatus.dwTotalVirtual, memoryStatus.dwAvailVirtual)

# To test, execute the script standalone.

if __name__ == "__main__":
    s = SystemInformation()
    print s.os
    print s.cpu
    print "RAM : %dKb total" % s.totalRAM
    print "RAM : %dKb free" % s.availableRAM
    print "Total VM: %dKb" % s.totalVM
    print "Available VM: %dKb" % s.availableVM
