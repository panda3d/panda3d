# Print some simple information using the WScript.Network object.
import sys
from win32com.client.gencache import EnsureDispatch

ob = EnsureDispatch('WScript.Network')

# For the sake of ensuring the correct module is used...
mod = sys.modules[ob.__module__]
print "The module hosting the object is", mod

# Now use the object.
print "About this computer:"
print "Domain =", ob.UserDomain
print "Computer Name =", ob.ComputerName
print "User Name =", ob.UserName


