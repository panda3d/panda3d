# copyright 1999 McMillan Enterprises, Inc.
# demo code - use as you please.
import os
import stat

def copyFile(srcFiles, destFile, append=0):
    '''
    Copy one or more files to another file.  If srcFiles is a list, then all
    will be concatenated together to destFile.  The append flag is also valid
    for single file copies.

    destFile will have the mode, ownership and timestamp of the last file
    copied/appended.
    '''
    if type(srcFiles) == type([]):
        # in case we need to overwrite on the first file...
        copyFile(srcFiles[0], destFile, append)
        for file in srcFiles[1:]:
            copyFile(file, destFile, 1)
        return

    mode = 'wb'
    if append:
        mode = 'ab'
    print " ", srcFiles, "->",
    input = open(srcFiles, 'rb')
    if input:
        print destFile
        output = open(destFile, mode)
        while 1:
            bytesRead = input.read(8192)
            if bytesRead:
                output.write(bytesRead)
            else:
                break

        input.close()
        output.close()

        stats = os.stat(srcFiles)
        os.chmod(destFile, stats[stat.ST_MODE])
        try:        # FAT16 file systems have only one file time
            os.utime(destFile, (stats[stat.ST_ATIME], stats[stat.ST_MTIME]))
        except:
            pass
        try:        
            os.chown(destFile, stats[stat.ST_UID], stats[stat.ST_GID])
        except:
            pass

def ensure(dirct):
    dirnm = dirct
    plist = []
    try:
        while not os.path.exists(dirnm):
            dirnm, base = os.path.split(dirnm)
            if base == '':
                break
            plist.insert(0, base)
        for d in plist:
            dirnm = os.path.join(dirnm, d)
            os.mkdir(dirnm)
    except:
        return 0
    return 1

def getinstalldir(prompt="Enter an installation directory: "):
    while 1:
        installdir = raw_input("Enter an installation directory: ")
        installdir = os.path.normpath(installdir)
        if ensure(installdir):
            break
        else:
            print installdir, "is not a valid pathname"
            r = raw_input("Try again (y/n)?: ")
            if r in 'nN':
                sys.exit(0)
    return installdir

def installCArchive(nm, basedir, suffixdir):
    import carchive_rt
    fulldir = os.path.join(basedir, suffixdir)
    if ensure(fulldir):
        pkg = carchive_rt.CArchive(nm)
        for fnm in pkg.contents():
            stuff = pkg.extract(fnm)[1]
            outnm = os.path.join(fulldir, fnm)
            if ensure(os.path.dirname(outnm)):
                open(outnm, 'wb').write(stuff)
        pkg = None
        os.remove(nm)
