import string
import pprint
import sys
import os
import ConfigParser
import pprint
import shutil
import tempfile
import ltoc
import tocfilter
import resource
import archive
import archivebuilder
import carchive

logfile = None
autopath = []
built = {}
copyFile = None

class Target:
    def __init__(self, cfg, sectnm, cnvrts):
        self.children = []
        self._dependencies = ltoc.lTOC() # the stuff an outer package will need to use me
        self.cfg = cfg
        self.__name__ = 'joe'
        for optnm in cfg.options(sectnm):
            cnvrt = cnvrts.get(optnm, 'getstringlist')
            if cnvrt:
                f = getattr(self, cnvrt, None)
                if f:
                    self.__dict__[optnm] = f(cfg.get(sectnm, optnm))
        if not hasattr(self, 'name'):
            self.name = self.__name__
        print "Initializing", self.__name__
        self.pathprefix = autopath + self.pathprefix
        self.pathprefix.append(os.path.join(pyinsthome, 'support'))
        for z in self.zlib:
            if z in self.cfg.sections():
                self.children.append(z)
            else:
                raise ValueError, "%s - zlib '%s' does not refer to a sections" \
                      % (self.name, z)
        for i in range(len(self.misc)):
            x = self.misc[i]
            if x in self.cfg.sections():
                if self.cfg.get(x, "type") == 'PYZ':
                    self.zlib.append(x)
                    self.misc[i] = None
                self.children.append(x)
        self.misc = filter(None, self.misc)
        self.edit()
        self.toc = ltoc.lTOC()
        for thingie in self.excludes:
            try:
                fltr = tocfilter.makefilter(thingie, self.pathprefix)
            except ValueError:
                print "Warning: '%s' not found - no filter created" % thingie
            else:
                self.toc.addFilter(fltr)
        if self.exstdlib:
            self.toc.addFilter(tocfilter.StdLibFilter())
        if self.extypes:
            self.toc.addFilter(tocfilter.ExtFilter(self.extypes))
        if self.expatterns:
            self.toc.addFilter(tocfilter.PatternFilter(self.expatterns))

        ##------utilities------##
    def dump(self):
        logfile.write("---- %s: %s -----\n" % (self.__class__.__name__, self.name))
        pprint.pprint(self.__dict__, logfile)
    def getstringlist(self, opt):
        tmp = string.split(opt, ',')
        return filter(None, map(string.strip, tmp))
    def getstring(self, opt):
        return opt
    def getbool(self, opt):
        if opt in ('0','f','F','n','N'):
            return 0
        return 1
        ##-----framework-----##
    def build(self):
        print "Gathering components of %s" % self.name
        self.gather()
        logfile.write("Final Table of Contents for %s:\n" % self.name)
        pprint.pprint(self.toc.toList(), logfile)
        print "Creating %s" % self.name
        self.assemble()
        ##-----overrideables-----##
    def edit(self):
        pass
    def gather(self):
        pass
    def assemble(self):
        pass

class PYZTarget(Target):
    def __init__(self, cfg, sectnm, cnvrts):
        Target.__init__(self, cfg, sectnm, cnvrts)
        # to use a PYZTarget, you'll need imputil and archive
        archivebuilder.GetCompiled([os.path.join(pyinsthome, 'imputil.py')])
        print "pyinsthome:", pyinsthome
        imputil = resource.makeresource('imputil.py', [pyinsthome])
        self._dependencies.append(imputil)
        archivebuilder.GetCompiled([os.path.join(pyinsthome, 'archive_rt.py')])
        archmodule = resource.makeresource('archive_rt.py', [pyinsthome])
        self._dependencies.merge(archmodule.dependencies())
        self._dependencies.append(archmodule)
        self.toc.addFilter(archmodule)
        self.toc.addFilter(imputil)
        for mod in archmodule.modules:
            self.toc.addFilter(mod)
    def edit(self):
        if self.extypes:
            print "PYZ target %s ignoring extypes = %s" % (self.__name__, self.extypes)

    def gather(self):
        for script in self.dependencies:
            rsrc = resource.makeresource(script, self.pathprefix)
            if not isinstance(rsrc, resource.scriptresource):
                print "Bug alert - Made %s from %s!" % (rsrc, script)
            self.toc.merge(rsrc.modules)
        logfile.write("lTOC after expanding 'depends':\n")
        pprint.pprint(self.toc.toList(), logfile)
        for thingie in self.includes + self.directories + self.packages:
            rsrc = resource.makeresource(thingie, self.pathprefix)
##            if not isinstance(rsrc, resource.pythonresource):
##                print "PYZ target %s ignoring include %s" % (self.name, thingie)
##            else:
            self.toc.merge(rsrc.contents())
        logfile.write("lTOC after includes, dir, pkgs:\n")
        pprint.pprint(self.toc.toList(), logfile)
        self.toc.addFilter(tocfilter.ExtFilter(['.py', '.pyc', '.pyo'], 1))
        logfile.write("Applying the following filters:\n")
        pprint.pprint(self.toc.filters, logfile)
        self.toc.filter()

    def assemble(self):
        contents = self.toc.toList()
        if contents:
            lib = archive.ZlibArchive()
            lib.build(self.name, archivebuilder.GetCompiled(self.toc.toList()))

class CollectTarget(Target):
    def __init__(self, cfg, sectnm, cnvrts):
        Target.__init__(self, cfg, sectnm, cnvrts)

    _rsrcdict = {'COLLECT': resource.dirresource, 'PYZ': resource.zlibresource, 'CARCHIVE': resource.archiveresource}

    def gather(self):
        if self.support:
            # the bare minimum
            self.toc.merge([resource.makeresource('python20.dll')])
            self.toc.merge([resource.makeresource('exceptions.pyc').asBinary()])
        # zlib, bindepends, misc, trees, destdir
        for i in range(len(self.zlib)):
            # z refers to the section name
            z = self.zlib[i]
            nm = self.cfg.get(z, 'name')
            try:
                self.toc.merge([resource.makeresource(nm, ['.'])])
            except ValueError:
                # zlibs aren't written if they turn out to be empty
                self.zlib[i] = None
        self.zlib = filter(None, self.zlib)
        if self.zlib:
            target = built.get(self.zlib[0], None)
            if target:
                self.toc.merge(target._dependencies)
        for script in self.bindepends:
            rsrc = resource.makeresource(script, self.pathprefix)
            self.toc.merge(rsrc.binaries)
        logfile.write('ltoc after bindepends:\n')
        pprint.pprint(self.toc.toList(), logfile)
        for thingie in self.misc:
            if thingie in self.cfg.sections():
                name = self.cfg.get(thingie, "name")
                typ = self.cfg.get(thingie, "type")
                klass = self._rsrcdict.get(typ, resource.dataresource)
                rsrc = apply(klass, (name, name))
                #now make sure we have the stuff the resource requires
                target = built.get(thingie, None)
                if target:
                    self.toc.merge(target._dependencies)
            else:
                rsrc = resource.makeresource(thingie, self.pathprefix)
            self.toc.merge(rsrc.contents())
        logfile.write('ltoc after misc:\n')
        pprint.pprint(self.toc.toList(), logfile)
        for script in self.script:
            if string.find(script, '.') == -1:
                script = script + '.py'
            rsrc = resource.makeresource(script, self.pathprefix)
            if rsrc.typ == 'm':
                rsrc.typ = 's'
            self.toc.merge([rsrc])
        logfile.write('ltoc after scripts:\n')
        pprint.pprint(self.toc.toList(), logfile)
        for tree in self.trees:
            try:
                rsrc = resource.treeresource('.', tree)
            except ValueError:
                print "tree %s not found" % tree
            else:
                self.toc.merge(rsrc.contents())
        logfile.write('ltoc after trees:\n')
        pprint.pprint(self.toc.toList(), logfile)
        self.toc.addFilter(tocfilter.TypeFilter(['d']))
        logfile.write("Applying the following filters:\n")
        pprint.pprint(self.toc.filters, logfile)
        self.toc.filter()
        #don't dupe stuff in a zlib that's part of this target
        if self.zlib:
           ztoc = ltoc.lTOC()
           for zlibnm in self.zlib:
               target = built.get(zlibnm, None)
               if target:
                   ztoc.merge(target.toc)
           for i in range(len(self.toc)-1, -1, -1):
               rsrc = self.toc[i]
               if isinstance(rsrc, resource.moduleresource) and rsrc in ztoc:
                   del self.toc[i]

    def assemble(self):
        if os.path.exists(self.name):
            if os.path.isdir(self.name):
                for fnm in os.listdir(self.name):
                    try:
                        os.remove(os.path.join(self.name, fnm))
                    except:
                        print "Could not delete file %s" % os.path.join(self.name, fnm)
        else:
            os.makedirs(self.name)
        mysite = []
        for nm, path, typ in self.toc.toList():
            shutil.copy2(path, self.name)
            if typ == 'z':
                mysite.append('imputil.FuncImporter(archive.ZlibArchive("%s", 0).get_code).install()' % nm)
        if mysite:
            mysite.insert(0, 'import archive, imputil')
            open(os.path.join(self.name, 'site.py'),'w').write(string.join(mysite, '\n'))


class ArchiveTarget(CollectTarget):
    usefullname = 1
    def __init__(self, cfg, sectnm, cnvrts):
        CollectTarget.__init__(self, cfg, sectnm, cnvrts)
        archivebuilder.GetCompiled([os.path.join(pyinsthome, 'carchive_rt.py')])
        carchmodule = resource.makeresource('carchive_rt.py', [pyinsthome])
        self._dependencies.merge(carchmodule.dependencies())
        self._dependencies.append(carchmodule)

    def edit(self):
        if self.destdir:
            print "Warning 'destdir = %s' ignored for %s" % (self.destdir, self.name)

    def gather(self):
        CollectTarget.gather(self)

    _cdict = {'s':2,'m':1,'b':1,'x':1,'a':0,'z':0, 'p':1}

    def assemble(self, pkgnm=None):
        if pkgnm is None:
            pkgnm = self.name
        arch = carchive.CArchive()
        toc = []
        pytoc = []
        for nm, path, typ in self.toc.toList():
            compress = self._cdict[typ]
            if typ == 'b' or (self.usefullname and typ in 'ms'):
                nm = os.path.basename(path)
            if typ == 'm':
                pytoc.append((nm, path, compress, typ))
            else:
                toc.append((nm, path, compress, typ))
        toc = toc + archivebuilder.GetCompiled(pytoc)
        arch.build(pkgnm, toc)
        return arch

class FullExeTarget(ArchiveTarget):
    usefullname = 0
    def __init__(self, cfg, sectnm, cnvrts):
        ArchiveTarget.__init__(self, cfg, sectnm, cnvrts)

    def gather(self):
        for script in self.script:
            #print "FullExeTarget.gather: script is", `script`
            rsrc = resource.makeresource(script, self.pathprefix)
            rsrc = resource.scriptresource(rsrc.name, rsrc.path)
            #print " resource is", `rsrc`
            self.toc.merge(rsrc.binaries)
        ArchiveTarget.gather(self)
        if not self.zlib:
            self.toc.merge(rsrc.modules)
        self._dependencies = ltoc.lTOC()

    _cdict = {'s':2,'m':0,'b':1,'x':0,'a':0,'z':0}
    _edict = { (1, 1):'Runw_d.exe', (1, 0):'Runw.exe', (0, 1):'Run_d.exe', (0, 0):'Run.exe'}

    def assemble(self):
        pkgname = tempfile.mktemp()
        arch = ArchiveTarget.assemble(self, pkgname)
        exe = self._edict[(self.userunw, self.debug)]
        exe = os.path.normpath(os.path.join(pyinsthome, 'support', exe))
##        copyFile([exe, pkgname], self.name)
##        os.remove(pkgname)
        # Thomas Heller's icon code
        # my version
        if self.icon:
            myexe = tempfile.mktemp()
            copyFile (exe, myexe)
            try:
                from icon import CopyIcons
                CopyIcons(myexe, self.icon)
            except ImportError:
                print "win32api is required for updating icons"
                print "You should have win32api.pyd and PyWinTypes20.dll"
                print "in the installation directory."
                print "Please copy them to Python's DLLS subdirectory"
                print "(or install Mark Hammond's Win32 extensions)."
##        iconfile = None
##        for name in self.cfg.sections():
##            if self.cfg.get (name, "type") == "STANDALONE":
##                try:
##                    iconfile = self.cfg.get (name, "iconfile")
##                except:
##                    pass
##        if iconfile:
##            from icon import CopyIcons
##            CopyIcons (myexe, iconfile)
            copyFile ([myexe, pkgname], self.name)
            os.remove(myexe)
        else:
            copyFile([exe, pkgname], self.name)
        #os.remove(pkgname)

class ExeTarget(FullExeTarget):
    def __init__(self, cfg, sectnm, cnvrts):
        FullExeTarget.__init__(self, cfg, sectnm, cnvrts)

    def edit(self):
        if not self.script:
            raise ValueError, "EXE target %s requires 'script= <script>'" % self.__name__

    def gather(self):
        FullExeTarget.gather(self)
        for i in range(len(self.toc)-1, -1, -1):
            rsrc = self.toc[i]
            if rsrc.typ == 'b':
                self._dependencies.append(rsrc)
                del self.toc[i]

installpreamble = """\
import sys, os
import installutils
import carchive_rt
idir = installutils.getinstalldir()
me = sys.argv[0]
if me[:-4] != '.exe':
    me = me + '.exe'
this = carchive_rt.CArchive(sys.argv[0])
here = sys.path[0]
"""
mvfile = "installutils.copyFile(os.path.join(here, '%s'), os.path.join(idir, '%s'))\n"
extractfile = "open(os.path.join(idir, '%s'), 'wb').write(this.extract('%s')[1])\n"
sitepreamble = """\
import archive_rt
import imputil
import sys
"""
importzlib = "imputil.FuncImporter(archive_rt.ZlibArchive(sys.path[0]+'/%s').get_code).install()\n"

class InstallTarget(FullExeTarget):
    def __init__(self, cfg, sectnm, cnvrts):
        FullExeTarget.__init__(self, cfg, sectnm, cnvrts)

    def edit(self):
        if not self.script:
            open('gen_install.py', 'w').write(installpreamble)
            self.script = ['gen_install.py']

    def gather(self):
        FullExeTarget.gather(self)
        if self.script[0] == 'gen_install.py':
            f = open(self.script[0], 'a')
            for rsrc in self.toc:
                if isinstance(rsrc, resource.binaryresource):
                    nm = os.path.basename(rsrc.path)
                    f.write(mvfile % (nm, nm))
                elif isinstance(rsrc, resource.pythonresource):
                    pass
                elif isinstance(rsrc, resource.zlibresource):
                    pass
                else:
                    f.write(extractfile % (rsrc.name, rsrc.name))
                    if isinstance(rsrc, resource.archiveresource):
                        #did it come with an install script?
                        target = built.get(rsrc.name, None)
                        if target:
                           if hasattr(target, "installscript"):
                               for script in target.installscript:
                                   s = resource.makeresource(script, self.pathprefix)
                                   txt = open(s.path, 'r').read()
                                   f.write(txt)
            f.close()

dispatch = {
                'PYZ': PYZTarget,
                'CARCHIVE': ArchiveTarget,
                'COLLECT': CollectTarget,
                'STANDALONE': ExeTarget,
                'INSTALL': InstallTarget,
                'FULLEXE': FullExeTarget,
}


def makeTarget(cfg, section):
    return dispatch[cfg.get(section, 'type')](cfg, section, optcnvrts)

optdefaults = { 'type':'PYZ',
                'script':'',            # INSTALL (opt) & STANDALONE (required)
                'zlib':'',              # INSTALL, STANDALONE, COLLECT
                'bindepends':'',        # INSTALL, COLLECT
                'misc':'',              # INSTALL. COLLECT
                'includetk': '0',       # INSTALL, COLLECT
        'userunw': '0',         # STANDALONE
                'dependencies':'',      # PYZ
                'directories':'',       # PYZ
                'excludes':'',          # PYZ, INSTALL, COLLECT
                'expatterns': '',
                'exstdlib': '0',
                'extypes': '',
                'includes':'',          # PYZ
                'packages':'',          # PYZ
                'destdir':'',           # COLLECT
                'pathprefix': '',
                'trees': '',
                'debug': '0',
                'support': '1', # include python20.dll & exceptons.pyc at a minimum
                'icon': '',
}

optcnvrts = {   'type':'',
                'name': 'getstring',
                'exstdlib': 'getbool',
                'console': 'getbool',
                'analyze': 'getbool',
                'debug': 'getbool',
                'includetk': 'getbool',
                'userunw': 'getbool',
                'destdir': 'getstring',
                'support': 'getbool',
                '__name__': 'getstring',
                'icon': 'getstring',
}
def main(opts, args):
    global pyinsthome
    global copyFile
    pyinsthome = os.path.abspath(os.path.dirname(sys.argv[0]))
    # sys.path.insert(0, os.path.join(pyinsthome, 'support'))
    import installutils
    copyFile = installutils.copyFile
    global logfile
    logfile = open('Builder.log','w')
    targets = []
    xref = {}
    cfg = ConfigParser.ConfigParser(optdefaults)
    for arg in args:
        dirnm = os.path.dirname(arg)
        if dirnm == '':
            dirnm = '.'
        autopath.append(os.path.abspath(dirnm))
    cfg.read(args)
    for section in cfg.sections():
        target = makeTarget(cfg, section)
        targets.append(target)
        xref[section] = target
    while targets:
        for i in range(len(targets)):
            target = targets[i]
            for child in target.children:
                if xref[child] in targets:
                    break
            else:       #no break - ready to build
                target.dump()
                target.build()
                built[target.__name__] = target
                built[target.name] = target
                targets[i] = None
                break
        else:       #no break - couldn't find anything to build
            names = map(lambda x: getattr(x, 'name'), targets)
            raise RuntimeError, "circular dependencies in %s" % `names`
        targets = filter(None, targets)

def run(file):
    main ([], file)

if __name__ == '__main__':
    import getopt
    (opts, args) = getopt.getopt(sys.argv[1:], 'dv')
    print "opts:", opts
    print "args:", args
    main(opts, args)
