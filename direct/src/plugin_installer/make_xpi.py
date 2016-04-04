#! /usr/bin/env python

import os
import sys
import shutil
import platform
import tempfile
import zipfile
from optparse import OptionParser
import subprocess

try:
    from hashlib import sha1 as sha
except ImportError:
    from sha import sha

usage = """
This command creates a Firefox XPI installer for the Panda3D Firefox
plugin.  Also see make_installer.py.

  %prog [opts]"""

parser = OptionParser(usage = usage)
parser.add_option('-v', '--version', dest = 'version',
                  help = 'The product version',
                  default = None)
parser.add_option('-i', '--plugin_root', dest = 'plugin_root',
                  help = 'The root of a directory hierarchy in which the Firefox plugins for various platforms can be found, to build a Firefox xpi file.  This is normally the same as the staging directory populated by the -i parameter to ppackage.  This directory should contain a directory called "plugin", which contains in turn a number of directories named for the platform, by the Panda plugin convention, e.g. linux_i386, osx_ppc, and so on.  Each platform directory should contain a Firefox plugin, e.g. nppanda3d.so.')
parser.add_option('', '--host_url', dest = 'host_url',
                  help = "The URL at which plugin_root will be hosted.  This is used to construct the update URL for the xpi file.  This is required if you specify --plugin_root.")

(options, args) = parser.parse_args()

this_dir = os.path.split(sys.argv[0])[0]

assert options.version, "A version number must be supplied!"
assert options.plugin_root, "The plugin_root must be supplied!"
assert options.host_url, "The host_url must be supplied!"

# A mapping of Panda's platform strings to Firefox's equivalent
# strings.

# I'm leaving out the Linux platforms for now.  I think there is too
# much variance between distro's for this to be reliable; we'll make
# each Linux user install their distro-specific plugin instead of
# going through this mechanism.
FirefoxPlatformMap = {
    'win32' : 'WINNT_x86-msvc',
    'win_i386' : 'WINNT_x86-msvc',
    'win_amd64' : 'WINNT_x86_64-msvc',
#    'linux_i386' : 'Linux_x86-gcc3',
#    'linux_amd64' : 'Linux_x86_64-gcc3',
#    'linux_ppc' : 'Linux_ppc-gcc3',
    'osx_i386' : 'Darwin_x86-gcc3',
    'osx_amd64' : 'Darwin_x86_64-gcc3',
    'osx_ppc' : 'Darwin_ppc-gcc3',
    'freebsd_i386' : 'FreeBSD_x86-gcc3',
    'freebsd_amd64' : 'FreeBSD_x86_64-gcc3',
    }

##############################################################################
#
# This install.rdf file is used when building a Firefox XPI file.
#
##############################################################################

install_rdf = """<?xml version="1.0"?>
<RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:em="http://www.mozilla.org/2004/em-rdf#">
  <Description about="urn:mozilla:install-manifest">
    <em:id>%(package_id)s</em:id>
    <em:unpack>true</em:unpack>
    <em:name>Panda3D Game Engine Plug-In</em:name>
    <em:description>Runs 3-D games and interactive applets</em:description>
    <em:version>%(version)s</em:version>
    <em:targetApplication>
      <Description>
        <em:id>{ec8030f7-c20a-464f-9b0e-13a3a9e97384}</em:id>
        <em:minVersion>3.0</em:minVersion>
        <em:maxVersion>*</em:maxVersion>
      </Description>
    </em:targetApplication>
    <em:homepageURL>http://www.panda3d.org/</em:homepageURL>
    <em:updateURL>%(host_url)s/plugin/firefox/update.rdf</em:updateURL>
  </Description>
</RDF>
"""

##############################################################################
#
# This update.rdf file is used when building a Firefox XPI file.
#
##############################################################################

update_rdf = """<?xml version="1.0"?>
<RDF:RDF xmlns:RDF="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
         xmlns:em="http://www.mozilla.org/2004/em-rdf#">

  <RDF:Description about="urn:mozilla:extension:%(package_id)s">
    <em:updates>
      <RDF:Seq>
        <RDF:li>
          <RDF:Description>
            <em:version>%(version)s</em:version>
            <em:targetApplication>
              <RDF:Description>
                <em:id>{ec8030f7-c20a-464f-9b0e-13a3a9e97384}</em:id>
                <em:minVersion>3.0</em:minVersion>
                <em:maxVersion>*</em:maxVersion>
                <em:updateLink>%(host_url)s/plugin/firefox/nppanda3d.xpi</em:updateLink>
                <em:updateHash>sha1:%(xpi_hash)s</em:updateHash>
              </RDF:Description>
            </em:targetApplication>
          </RDF:Description>
        </RDF:li>
      </RDF:Seq>
    </em:updates>
  </RDF:Description>
</RDF:RDF>
"""

def makeXpiFile():
    """ Creates a Firefox XPI file, based on the various platform
    version files. """

    if not options.host_url:
        print("Cannot generate xpi file without --host-url.")
        sys.exit(1)

    print("Generating xpi file")
    root = options.plugin_root
    if os.path.isdir(os.path.join(root, 'plugin')):
        root = os.path.join(root, 'plugin')

    xpi = zipfile.ZipFile('nppanda3d.xpi', 'w')

    package_id = 'runtime@panda3d.org' #TODO: maybe more customizable?

    tempFile = tempfile.mktemp('.txt', 'p3d_')
    rdf = open(tempFile, 'w')
    rdf.write(install_rdf % {
        'package_id' : package_id,
        'version' : options.version,
        'host_url' : options.host_url,
        })
    rdf.close()
    xpi.write(tempFile, 'install.rdf')
    os.unlink(tempFile)

    subdirs = os.listdir(root)
    for subdir in subdirs:
        platform = FirefoxPlatformMap.get(subdir, None)
        path = os.path.join(root, subdir)
        if platform and os.path.isdir(path):
            if subdir in ['win32', 'osx_i386']:
                pluginsXpiDir = 'plugins'
            else:
                # Create the XPI directory platform/<platform name>/plugins
                pluginsXpiDir = 'platform/%s/plugins' % (platform)

            # Copy the Firefox plugin into this directory.
            if subdir.startswith('win32'):
                pluginFilename = 'nppanda3d.dll'
            elif subdir.startswith('osx'):
                pluginFilename = 'nppanda3d.plugin'
            else:
                pluginFilename = 'nppanda3d.so'

            addZipTree(xpi, os.path.join(path, pluginFilename),
                       pluginsXpiDir + '/' + pluginFilename)
    xpi.close()

    # Now that we've generated the xpi file, get its hash.
    data = open('nppanda3d.xpi', 'rb').read()
    xpi_hash = sha(data).hexdigest()

    # And now we can generate the update.rdf file.
    update = open('update.rdf', 'w')
    update.write(update_rdf % {
        'package_id' : package_id,
        'version' : options.version,
        'host_url' : options.host_url,
        'xpi_hash' : xpi_hash,
        })
    update.close()

def addZipTree(zip, sourceFile, zipName):
    """ Adds the sourceFile to the zip archive at the indicated name.
    If it is a directory, recursively adds all nested files as
    well. """

    if os.path.isdir(sourceFile):
        subdirs = os.listdir(sourceFile)
        for subdir in subdirs:
            addZipTree(zip, os.path.join(sourceFile, subdir),
                       zipName + '/' + subdir)

    else:
        # Not a directory, just add the file.
        zip.write(sourceFile, zipName)

makeXpiFile()

