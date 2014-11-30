import sys

print "Hello from py2exe"

print "frozen", repr(getattr(sys, "frozen", None))

print "sys.path", sys.path
print "sys.executable", sys.executable
print "sys.prefix", sys.prefix
print "sys.argv", sys.argv

