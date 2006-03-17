#import MkWrap
import imputil
import strop
import zlib
import os
import marshal

class MkImporter:
    def __init__(self, db, viewnm='pylib'):
        self.db = db
        self.view = db.getas(viewnm+'[name:S, ispkg:I, code:M]') # an MkWrap view object
    def setImportHooks(self):
        imputil.FuncImporter(self.get_code).install()
    def get_code(self, parent, modname, fqname):
        if self.view is None:
            return None
        ndx = self.view.search(name=fqname)
        if ndx < len(self.view):
            row = self.view[ndx]
            if row.name == fqname:
                return (row.ispkg, marshal.loads(zlib.decompress(row.code)))
        return None
    def build(self, lTOC):
        for entry in lTOC:
            nm, fnm = entry[0], entry[1]
            ispkg = os.path.splitext(os.path.basename(fnm))[0] == '__init__'
            ndx = self.view.search(name=nm)
            if ndx < len(self.view):
                row = self.view[ndx]
                if row.name != nm:
                    self.view.insert(ndx, {})
                    row = self.view[ndx]
            else:
                ndx = self.view.append({})
                row = self.view[ndx]
            row.name = nm
            row.ispkg = ispkg
            f = open(fnm, 'rb')
            f.seek(8)
            obj = zlib.compress(f.read(), 9)
            row.code = obj
        self.db.commit()
