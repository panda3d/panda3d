#!/usr/bin/env python

# Helper script when freezing Pmw applications.  It concatenates all
# Pmw megawidget files into a single file, 'Pmw.py', in the current
# directory.  The script must be called with one argument, being the
# path to the 'lib' directory of the required version of Pmw.
# To freeze a Pmw application, you will also need to copy the
# following files to the application directory before freezing:
#
#    PmwBlt.py PmwColor.py

import os
import re
import string
import sys

# The order of these files is significant.  Files which reference
# other files must appear later.  Files may be deleted if they are not
# used.
files = [
    'Dialog', 'TimeFuncs', 'Balloon', 'ButtonBox', 'EntryField',
    'Group', 'LabeledWidget', 'MainMenuBar', 'MenuBar', 'MessageBar',
    'MessageDialog', 'NoteBook', 'OptionMenu', 'PanedWidget', 'PromptDialog',
    'RadioSelect', 'ScrolledCanvas', 'ScrolledField', 'ScrolledFrame',
    'ScrolledListBox', 'ScrolledText', 'HistoryText', 'SelectionDialog',
    'TextDialog', 'TimeCounter', 'AboutDialog', 'ComboBox', 'ComboBoxDialog',
    'Counter', 'CounterDialog',
]

# Set this to 0 if you do not use any of the Pmw.Color functions:
needColor = 1

# Set this to 0 if you do not use any of the Pmw.Blt functions:
needBlt = 1

def expandLinks(path):
    if not os.path.isabs(path):
        path = os.path.join(os.getcwd(), path)
    while 1:
        if not os.path.islink(path):
            break
        dir = os.path.dirname(path)
        path = os.path.join(dir, os.readlink(path))

    return path

def mungeFile(file):
    # Read the file and modify it so that it can be bundled with the
    # other Pmw files.
    file = 'Pmw' + file + '.py'
    text = open(os.path.join(srcdir, file)).read()
    text = re.sub(r'import Pmw\b', '', text)
    text = re.sub('INITOPT = Pmw.INITOPT', '', text)
    text = re.sub(r'\bPmw\.', '', text)
    text = '\n' + ('#' * 70) + '\n' + '### File: ' + file + '\n' + text
    return text

# Work out which version is being bundled.
file = sys.argv[0]
file = os.path.normpath(file)
file = expandLinks(file)

dir = os.path.dirname(file)
dir = expandLinks(dir)
dir = os.path.dirname(dir)
dir = expandLinks(dir)
dir = os.path.basename(dir)

version = string.replace(dir[4:], '_', '.')

# Code to import the Color module.
colorCode = """
import PmwColor
Color = PmwColor
del PmwColor
"""

# Code to import the Blt module.
bltCode = """
import PmwBlt
Blt = PmwBlt
del PmwBlt
"""

# Code used when not linking with PmwBlt.py.
ignoreBltCode = """
_bltImported = 1
_bltbusyOK = 0
"""

# Code to define the functions normally supplied by the dynamic loader.
extraCode = """

### Loader functions:

_VERSION = '%s'

def setversion(version):
    if version != _VERSION:
        raise ValueError, 'Dynamic versioning not available'

def setalphaversions(*alpha_versions):
    if alpha_versions != ():
        raise ValueError, 'Dynamic versioning not available'

def version(alpha = 0):
    if alpha:
        return ()
    else:
        return _VERSION

def installedversions(alpha = 0):
    if alpha:
        return ()
    else:
        return (_VERSION,)

"""

if '-noblt' in sys.argv:
    sys.argv.remove('-noblt')
    needBlt = 0

if '-nocolor' in sys.argv:
    sys.argv.remove('-nocolor')
    needColor = 0

if len(sys.argv) != 2:
    print('usage: bundlepmw.py [-noblt] [-nocolor] /path/to/Pmw/Pmw_X_X_X/lib')
    sys.exit()

srcdir = sys.argv[1]

if os.path.exists('Pmw.py'):
    print('Pmw.py already exists. Remove it and try again.')
    sys.exit()

outfile = open('Pmw.py', 'w')

if needColor:
    outfile.write(colorCode)

if needBlt:
    outfile.write(bltCode)

outfile.write(extraCode % version)

# Specially handle PmwBase.py file:
text = mungeFile('Base')
text = re.sub('import PmwLogicalFont', '', text)
text = re.sub('PmwLogicalFont._font_initialise', '_font_initialise', text)
outfile.write(text)
if not needBlt:
    outfile.write(ignoreBltCode)

files.append('LogicalFont')
for file in files:
    text = mungeFile(file)
    outfile.write(text)

print('')
print('   Pmw.py has been created.')

if needColor or needBlt:
    print('   Before running freeze, also copy the following file(s):')
    if needBlt:
        print('   ' + os.path.join(srcdir, 'PmwBlt.py'))
    if needColor:
        print('   ' + os.path.join(srcdir, 'PmwColor.py'))
