#!/usr/bin/env python

# This script parses the version number in dtool/PandaVersion.pp
# and returns it on the command-line.  This is useful for the
# automated scripts that build the Panda3D releases.

from makepandacore import ParsePandaVersion, ParsePluginVersion
import sys

if '--runtime' in sys.argv:
    version = ParsePluginVersion("dtool/PandaVersion.pp")
else:
    version = ParsePandaVersion("dtool/PandaVersion.pp")

version = version.strip()
sys.stdout.write(version)
sys.stdout.flush()
