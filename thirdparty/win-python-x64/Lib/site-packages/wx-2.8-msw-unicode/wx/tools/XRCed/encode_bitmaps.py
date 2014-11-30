"""
A simple script to encode all the images the XRCed needs into a Python module
"""

import sys, os, glob
from wx.tools import img2py

def main(filemask, output):
    # get the list of PNG files
    files = glob.glob(filemask)
    files.sort()

    # Truncate the inages module
    open(output, 'w')

    # call img2py on each file
    for file in files:

        # extract the basename to be used as the image name
        name = os.path.splitext(os.path.basename(file))[0]

        # encode it
        if file == files[0]:
            cmd = "-F -u -n %s %s %s" % (name, file, output)
        else:
            cmd = "-a -F -u -n %s %s %s" % (name, file, output)
        img2py.main(cmd.split())

    # Encode icons
    files = glob.glob('src-images/*.ico')
    files.sort()
    for file in files:
        # extract the basename to be used as the image name
        name = os.path.splitext(os.path.basename(file))[0]
        # encode it
        cmd = "-a -F -i -u -n %s %s %s" % (name, file, output)
        img2py.main(cmd.split())

if __name__ == "__main__":
    main('src-images/*.png', 'images.py')
    main('src-images/32x32/*.png', 'images_32x32.py')

