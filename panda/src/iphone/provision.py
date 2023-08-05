#! /usr/bin/env python

import shutil
import os

identity="iPhone Developer: David Rose"
library='/Users/drose/Library'
profile="31FDB595-E85E-41D8-BE12-7C67B777B6C3"
appPrefix="VGEKRBUPUE"
appId="com.ddrose.iphone.pview"
entitlements="/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS2.2.1.sdk/Entitlements.plist"
app="/tmp/iphone/pview.app"
xcent="/tmp/file.xcent"


sourceFilename = '%s/MobileDevice/Provisioning Profiles/%s.mobileprovision' % (library, profile)
targetFilename = '%s/embedded.mobileprovision' % (app)
shutil.copyfile(sourceFilename, targetFilename)

origtext = open(entitlements, 'r').read()
dict = origtext.find('<dict>')
newtext = origtext[:dict + 6] + '\n<key>application-identifier</key>\n<string>%s.%s</string>' % (appPrefix, appId) + origtext[dict + 6:]

# Some kind of crazy binary header on the xcent file.  The second
# group of four bytes is the file length.
header = '\xfa\xde\x71\x71'
length = len(header) + 4 + len(newtext)
lengthstr = chr(length >> 24) + chr((length >> 16) & 0xff) + chr((length >> 8) & 0xff) + chr(length & 0xff)
open(xcent, 'w').write(header + lengthstr + newtext)

command = 'env CODESIGN_ALLOCATE="/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/codesign_allocate" PATH="/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin" /usr/bin/codesign -f -s "%(identity)s" --resource-rules="%(app)s/ResourceRules.plist" --entitlements "%(xcent)s" "%(app)s"' % {
    'identity' : identity,
    'app' : app,
    'xcent' : xcent,
    }

print(command)
result = os.system(command)
if result != 0:
    raise StandardError

os.unlink(xcent)


