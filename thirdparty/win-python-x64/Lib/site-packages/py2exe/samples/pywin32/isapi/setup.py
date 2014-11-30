# setup_interp.py
# A distutils setup script for the ISAPI "redirector" sample.
import os
from distutils.core import setup
import py2exe

# Find the ISAPI sample - the redirector.
import isapi
script = os.path.join(isapi.__path__[0], "samples", "redirector.py")

setup(name="ISAPI sample",
      # The ISAPI dll.
      isapi = [script],
      # command-line installation tool.
      console=[script],
)
