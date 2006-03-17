# copyright 1999 McMillan Enterprises, Inc.
# license: use as you please. No warranty.
#
# use dumpbin.exe (if present) to find the binary
# dependencies of an extension module.
# if dumpbin not available, pick apart the PE hdr of the binary
# while this appears to work well, it is complex and subject to
# problems with changes to PE hdrs (ie, this works only on 32 bit Intel
# Windows format binaries)
#
# Note also that you should check the results to make sure that the
# dlls are redistributable. I've listed most of the common MS dlls
# under "excludes" below; add to this list as necessary (or use the
# "excludes" option in the INSTALL section of the config file).

import os
import time
import string
import sys
import tempfile
import finder

seen = {}
excludes = {'KERNEL32.DLL':1,
      'ADVAPI.DLL':1,
      'MSVCRT.DLL':1,
      'ADVAPI32.DLL':1,
      'COMCTL32.DLL':1,
      'CRTDLL.DLL':1,
      'GDI32.DLL':1,
      'MFC42.DLL':1,
      'NTDLL.DLL':1,
      'OLE32.DLL':1,
      'OLEAUT32.DLL':1,
      'RPCRT4.DLL':1,
      'SHELL32.DLL':1,
      'USER32.DLL':1,
      'WINSPOOL.DRV':1,
      'WS2HELP.DLL':1,
      'WS2_32.DLL':1,
      'WSOCK32.DLL':1,
      'WINMM.DLL':1,
      'COMDLG32.DLL':1,
      'ZLIB.DLL':1,
      'ODBC32.DLL':1,
      'VERSION.DLL':1}

def getfullnameof(mod, xtrapath = None):
  """Return the full path name of MOD.

      MOD is the basename of a dll or pyd.
      XTRAPATH is a path or list of paths to search first.
      Return the full path name of MOD.
      Will search the full Windows search path, as well as sys.path"""
  epath = finder.getpath()
  if mod[-4:] in ('.pyd', '.PYD'):
    epath = epath + sys.path
  if xtrapath is not None:
    if type(xtrapath) == type(''):
      epath.insert(0, xtrapath)
    else:
      epath = xtrapath + epath
  for p in epath:
    npth = os.path.join(p, mod)
    if os.path.exists(npth):
      return npth
  return ''

def getImports1(pth):
    """Find the binary dependencies of PTH.

        This implementation (not used right now) uses the MSVC utility dumpbin"""
    rslt = []
    tmpf = tempfile.mktemp()
    os.system('dumpbin /IMPORTS "%s" >%s' %(pth, tmpf))
    time.sleep(0.1)
    txt = open(tmpf,'r').readlines()
    os.remove(tmpf)
    i = 0
    while i < len(txt):
        tokens = string.split(txt[i])
        if len(tokens) == 1 and string.find(tokens[0], '.') > 0:
            rslt.append(string.strip(tokens[0]))
        i = i + 1
    return rslt

def getImports2(pth):
    """Find the binary dependencies of PTH.

        This implementation walks through the PE header"""
    import struct
    rslt = []
    try:
      f = open(pth, 'rb').read()
      pehdrd = struct.unpack('l', f[60:64])[0]
      magic = struct.unpack('l', f[pehdrd:pehdrd+4])[0]
      numsecs = struct.unpack('h', f[pehdrd+6:pehdrd+8])[0]
      numdirs = struct.unpack('l', f[pehdrd+116:pehdrd+120])[0]
      idata = ''
      if magic == 17744:
          importsec, sz = struct.unpack('2l', f[pehdrd+128:pehdrd+136])
          secttbl = pehdrd + 120 + 8*numdirs
          secttblfmt = '8s7l2h'
          seclist = []
          for i in range(numsecs):
              seclist.append(struct.unpack(secttblfmt, f[secttbl+i*40:secttbl+(i+1)*40]))
              #nm, vsz, va, rsz, praw, preloc, plnnums, qrelocs, qlnnums, flags \
              # = seclist[-1]
          for i in range(len(seclist)-1):
              if seclist[i][2] <= importsec < seclist[i+1][2]:
                  break
          vbase = seclist[i][2]
          raw = seclist[i][4]
          idatastart = raw + importsec - vbase
          idata = f[idatastart:idatastart+seclist[i][1]]
          i = 0
          while 1:
              vsa =  struct.unpack('5l', idata[i*20:i*20+20])[3]
              if vsa == 0:
                  break
              sa = raw + vsa - vbase
              end = string.find(f, '\000', sa)
              rslt.append(f[sa:end])
              i = i + 1
    except IOError:
      print "bindepend cannot analyze %s - file not found!"
    except struct.error:
      print "bindepend cannot analyze %s - error walking thru pehdr"
    return rslt

def Dependencies(lTOC):
  """Expand LTOC to include all the closure of binary dependencies.

     LTOC is a logical table of contents, ie, a seq of tuples (name, path).
     Return LTOC expanded by all the binary dependencies of the entries
     in LTOC, except those listed in the module global EXCLUDES"""
  for (nm, pth) in lTOC:
    fullnm = string.upper(os.path.basename(pth))
    if seen.get(string.upper(nm), 0):
      continue
    print "analyzing", nm
    seen[string.upper(nm)] = 1
    dlls = getImports(pth)
    for lib in dlls:
        print " found", lib
        if excludes.get(string.upper(lib), 0):
          continue
        if seen.get(string.upper(lib), 0):
          continue
        npth = getfullnameof(lib)
        if npth:
          lTOC.append((lib, npth))
        else:
          print " lib not found:", lib, "dependency of",
  return lTOC


##if getfullnameof('dumpbin.exe') == '':
##    def getImports(pth):
##        return getImports2(pth)
##else:
##    def getImports(pth):
##        return getImports1(pth)

def getImports(pth):
    """Forwards to either getImports1 or getImports2
    """
    return getImports2(pth)

