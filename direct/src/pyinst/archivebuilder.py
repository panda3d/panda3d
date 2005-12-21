# copyright 1999 McMillan Enterprises, Inc.
# license: use as you please. No warranty.
# Gordon McMillan gmcm@hypernet.com
#
# A collection of routines for building a logical Table Of Contents
# that Archive (subclasses) use to build themselves.
# A logical Table of Contents is a sequence, each element of which is
# a sequence, with at least 2 entries - "name" and "path".

import os

import string

import py_compile

def GetCompiled(seq, lvl='c'):
  """SEQ is a list of .py files, or a logical TOC.
     Return as .pyc or .pyo files (LVL) after ensuring their existence"""
  if len(seq) == 0:
    return seq
  rslt = []
  isTOC = 0
  if type(seq[0]) == type(()):
    isTOC = 1
  for py in seq:
    if isTOC:
      (nm, fnm), rest = py[:2], py[2:]
    else:
      fnm = py
    fnm = os.path.splitext(fnm)[0] + '.py'
    cmpl = 1
    pyc = fnm + lvl
    if os.path.exists(pyc):
      pytm = long(os.stat(fnm)[8])
      ctm = long(os.stat(pyc)[8])
      if pytm < ctm:
        cmpl = 0
    if cmpl:
      py_compile.compile(fnm, pyc)
    if isTOC:
      rslt.append((nm, pyc)+rest)
    else:
      rslt.append(pyc)
  return rslt

import modulefinder
MF = modulefinder
import sys

def Dependencies(script):
  """Get a logical TOC directly from the dependencies of a script.
  
     The returned TOC does NOT contain the script.
     It does contain extension modules. Uses modulefinder."""
  rslt = []
  (dir, name) = os.path.split(script)
  if dir:
    ppath = [os.path.normpath(dir)] + sys.path
  else:
    ppath = sys.path[:]
  mf = MF.ModuleFinder(ppath, 0)
  try:
    mf.run_script(script)
  except IOError:
    print " Script not found:", script
    return []
  del mf.modules['__main__']
  for (k, v) in mf.modules.items():
    if v.__file__ is None:
      del mf.modules[k]  # a builtin
  for (k, v) in mf.modules.items():
    #ispkg = os.path.basename(v.__file__) == '__init__.py'
    d = os.path.dirname(v.__file__)
    if not d:
      v.__file__ = os.path.join(os.getcwd(), v.__file__)
    #if ispkg:
    #    rslt.append(k+'.__init__', v.__file__)
    #else:
    rslt.append((k, v.__file__))
  return rslt

