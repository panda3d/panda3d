#!/usr/bin/env python
import marshal
import os
import struct

from direct.showutil import FreezeTool
import panda3d.core as p3d

def make_module_list_entry(code, offset, modulename, module):
    size = len(code)
    if getattr(module, "__path__", None):
        # Indicate package by negative size
        size = -size
    return struct.pack('<256sIi', bytes(modulename, 'ascii'), offset, size)

def make_forbidden_module_list_entry(modulename):
    return struct.pack('<256sIi', bytes(modulename, 'ascii'), 0, 0)

def get_modules(freezer):
    # Now generate the actual export table.
    moduleBlob = bytes()
    codeOffset = 0
    moduleList = []

    for moduleName, mdef in freezer.getModuleDefs():
        origName = mdef.moduleName
        if mdef.forbid:
            # Explicitly disallow importing this module.
            moduleList.append(make_forbidden_module_list_entry(moduleName))
            continue

        assert not mdef.exclude
        # Allow importing this module.
        module = freezer.mf.modules.get(origName, None)
        code = getattr(module, "__code__", None)
        if code:
            code = marshal.dumps(code)
            moduleList.append(make_module_list_entry(code, codeOffset, moduleName, module))
            moduleBlob += code
            codeOffset += len(code)
            continue

        # This is a module with no associated Python code.  It is either
        # an extension module or a builtin module.  Get the filename, if
        # it is the former.
        extensionFilename = getattr(module, '__file__', None)

        if extensionFilename or freezer.linkExtensionModules:
            freezer.extras.append((moduleName, extensionFilename))

        # If it is a submodule of a frozen module, Python will have
        # trouble importing it as a builtin module.  Synthesize a frozen
        # module that loads it as builtin.
        if '.' in moduleName and freezer.linkExtensionModules:
            code = compile('import sys;del sys.modules["%s"];import imp;imp.init_builtin("%s")' % (moduleName, moduleName), moduleName, 'exec')
            code = marshal.dumps(code)
            moduleList.append(make_module_list_entry(code, codeOffset, moduleName, module))
            moduleBlob += code
            codeOffset += len(code)
        elif '.' in moduleName:
            # Nothing we can do about this case except warn the user they
            # are in for some trouble.
            print('WARNING: Python cannot import extension modules under '
                  'frozen Python packages; %s will be inaccessible.  '
                  'passing either -l to link in extension modules or use '
                  '-x %s to exclude the entire package.' % (moduleName, moduleName.split('.')[0]))

    return moduleBlob, moduleList


if __name__ == '__main__':
    # Setup Freezer
    freezer = FreezeTool.Freezer()
    freezer.keepTemporaryFiles = True
    freezer.addModule('__main__', filename='app.py')
    freezer.done(addStartupModules=True)

    # Build from pre-built binary stub
    stub_path = os.path.join(os.path.dirname(p3d.ExecutionEnvironment.get_dtool_name()), '..', 'bin', 'deploy-stub')
    out_path = 'frozen_app'
    with open(stub_path, 'rb') as f:
        stubbin = f.read()

    modblob, modlist = get_modules(freezer)

    with open(out_path, 'wb') as f:
        f.write(stubbin)
        listoffset = f.tell()
        for mod in modlist:
            f.write(mod)
        modsoffset = f.tell()
        f.write(modblob)
        f.write(struct.pack('<I', listoffset))
        f.write(struct.pack('<I', modsoffset))
        f.write(struct.pack('<I', len(modlist)))
    os.chmod(out_path, 0o755)
