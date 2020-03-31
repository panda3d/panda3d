#!/usr/bin/env python

# This script parses the version number in setup.cfg
# and returns it on the command-line.  This is useful for the
# automated scripts that build the Panda3D releases.

from makepandacore import ParsePandaVersion, GetMetadataValue
import sys

version = GetMetadataValue('version')

version = version.strip()
sys.stdout.write(version)
sys.stdout.flush()
