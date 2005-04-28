# copyright 1999 McMillan Enterprises, Inc.
# license: use as you please. No warranty.
#
# A subclass of Archive that can be understood
# by a C program. See uplaunch.cpp for unpacking
# from C.

#carchive_rt is a stripped down version of MEInc.Dist.carchive.
#It has had all building logic removed.
#It's purpose is to bootstrap the Python installation.

import archive_rt
import struct
import zlib
import strop

class CTOC:
  ENTRYSTRUCT = 'iiiibc' #(structlen, dpos, dlen, ulen, flag, typcd) followed by name
  def __init__(self):
    self.data = []
  
  def frombinary(self, s):
    entrylen = struct.calcsize(self.ENTRYSTRUCT)
    p = 0
    while p<len(s):
      (slen, dpos, dlen, ulen, flag, typcd) = struct.unpack(self.ENTRYSTRUCT, 
                                                  s[p:p+entrylen]) 
      nmlen = slen - entrylen 
      p = p + entrylen
      (nm,) = struct.unpack(`nmlen`+'s', s[p:p+nmlen])
      p = p + nmlen 
      self.data.append((dpos, dlen, ulen, flag, typcd, nm[:-1]))

##  def tobinary(self):
##    import string
##    entrylen = struct.calcsize(self.ENTRYSTRUCT)
##    rslt = []
##    for (dpos, dlen, ulen, flag, typcd, nm) in self.data:
##      nmlen = len(nm) + 1     # add 1 for a '\0'
##      rslt.append(struct.pack(self.ENTRYSTRUCT+`nmlen`+'s',
##        nmlen+entrylen, dpos, dlen, ulen, flag, typcd, nm+'\0'))
##    return string.join(rslt, '')
##
##  def add(self, dpos, dlen, ulen, flag, typcd, nm):
##    self.data.append(dpos, dlen, ulen, flag, typcd, nm)

  def get(self, ndx):
    return self.data[ndx]

  def __getitem__(self, ndx):
    return self.data[ndx]

  def find(self, name):
    for i in range(len(self.data)):
      if self.data[i][-1] == name:
        return i
    return -1

class CArchive(archive_rt.Archive):
  MAGIC = 'MEI\014\013\012\013\015'
  HDRLEN = 0
  TOCTMPLT = CTOC
  TRLSTRUCT = '8siii'
  TRLLEN = 20
  LEVEL = 9
  def __init__(self, path=None, start=0, len=0):
    self.len = len
    archive_rt.Archive.__init__(self, path, start)

  def checkmagic(self):
    #magic is at EOF; if we're embedded, we need to figure where that is
    if self.len:
      self.lib.seek(self.start+self.len, 0)
    else:
      self.lib.seek(0, 2)
    filelen = self.lib.tell()
    if self.len:
      self.lib.seek(self.start+self.len-self.TRLLEN, 0)
    else:
      self.lib.seek(-self.TRLLEN, 2)
    (magic, totallen, tocpos, toclen) = struct.unpack(self.TRLSTRUCT, 
                                                self.lib.read(self.TRLLEN))
    if magic != self.MAGIC:
      raise RuntimeError, "%s is not a valid %s archive file" \
                % (self.path, self.__class__.__name__)
    self.pkgstart = filelen - totallen
    if self.len:
      if totallen != self.len or self.pkgstart != self.start:
        raise RuntimeError, "Problem with embedded archive in %s" % self.path
    self.tocpos, self.toclen = tocpos, toclen

  def loadtoc(self):
    self.toc = self.TOCTMPLT()
    self.lib.seek(self.pkgstart+self.tocpos)
    tocstr = self.lib.read(self.toclen)
    self.toc.frombinary(tocstr)

  def extract(self, name):
    if type(name) == type(''):
      ndx = self.toc.find(name)
      if ndx == -1:
        return None
    else:
      ndx = name
    (dpos, dlen, ulen, flag, typcd, nm) = self.toc.get(ndx)
    self.lib.seek(self.pkgstart+dpos)
    rslt = self.lib.read(dlen)
    if flag == 1:
      rslt = zlib.decompress(rslt)
    if typcd == 'M':
      return (1, rslt)
    return (0, rslt)

  def contents(self):
    rslt = []
    for (dpos, dlen, ulen, flag, typcd, nm) in self.toc:
      rslt.append(nm)
    return rslt

##  def add(self, entry):
##    (nm, pathnm, flag, typcd) = entry[:4]
##    if flag == 2:
##        s = open(pathnm, 'r').read()
##        s = s + '\0'
##    else:
##        s = open(pathnm, 'rb').read()
##    ulen = len(s)
##    if flag == 1:
##      s = zlib.compress(s, self.LEVEL)
##    dlen = len(s)
##    where = self.lib.tell()
##    if typcd == 'm':
##      if strop.find(pathnm, '.__init__.py') > -1:
##        typcd = 'M'
##    self.toc.add(where, dlen, ulen, flag, typcd, nm)
##    self.lib.write(s)
##
##  def save_toc(self, tocpos):
##    self.tocpos = tocpos
##    tocstr = self.toc.tobinary()
##    self.toclen = len(tocstr)
##    self.lib.write(tocstr)
##
##  def save_trailer(self, tocpos):
##    totallen = tocpos + self.toclen + self.TRLLEN
##    trl = struct.pack(self.TRLSTRUCT, self.MAGIC, totallen, 
##                      tocpos, self.toclen)
##    self.lib.write(trl)

  def openEmbedded(self, name):
    ndx = self.toc.find(name)
    if ndx == -1:
      raise KeyError, "Member '%s' not found in %s" % (name, self.path)
    (dpos, dlen, ulen, flag, typcd, nm) = self.toc.get(ndx)
    if flag:
      raise ValueError, "Cannot open compressed archive %s in place"
    return CArchive(self.path, self.pkgstart+dpos, dlen)
