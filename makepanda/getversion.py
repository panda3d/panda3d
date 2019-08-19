#!/usr/bin/env python

# This script parses the version number in dtool/PandaVersion.pp
# and returns it on the command-line.  This is useful for the
# automated scripts that build the Panda3D releases.

from makepandacore import ParsePandaVersion, ParsePluginVersion, GetMetadataValue
import sys

if '--runtime' in sys.argv:
    version = ParsePluginVersion("dtool/PandaVersion.pp")
else:
    version = GetMetadataValue('version')

version = version.strip()
sys.stdout.write(version)
sys.stdout.flush()
