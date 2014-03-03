# N.B. PandaModules is generated at build time by CMake
from PandaModules import *
import __builtin__

# Now import all extensions:
from direct.extensions_native.extension_native_helpers import *
extensions = [
    'CInterval', 'EggGroupNode', 'EggPrimitive', 'HTTPChannel', 'Mat3',
    'NodePath', 'NodePathCollection', 'OdeBody', 'OdeGeom', 'OdeJoint',
    'OdeSpace', 'Ramfile', 'StreamReader', 'VBase3', 'VBase4'
]

# Prior to importing, we need to make the Dtool_funcToMethod function and
# the extended class available globally. This is hacky, but tacking it on
# __builtin__ works just fine:
import __builtin__
__builtin__.Dtool_funcToMethod = Dtool_funcToMethod
__builtin__.Dtool_ObjectToDict = Dtool_ObjectToDict

for extension in extensions:
    if extension not in locals():
        # Not a class we have compiled in, skip it!
        continue

    module = 'direct.extensions_native.%s_extensions' % extension

    setattr(__builtin__, extension, locals()[extension])
    __import__(module)
    del __builtin__.__dict__[extension]

del __builtin__.Dtool_funcToMethod
del __builtin__.Dtool_ObjectToDict
