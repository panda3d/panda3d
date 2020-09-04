#################################################################
# seCameraControl.py
# Originally from DirectCameraControl.py
# Altered by Yi-Hong Lin, yihhongl@andrew.cmu.edu, 2004
#
# We didn't change anything essential.
# Just because we customized the seSession from DirectSession,
# So we need related files can follow the change.
# However, we don't want to change anything inside the original directool
# to let them can work with our scene editor.
# (If we do change original directools, it will force user has to install the latest version of OUR Panda)
#
#################################################################

from direct.directtools.DirectCameraControl import DirectCameraControl

class SeDirectCameraControl(DirectCameraControl):
    def __init__(self):
        DirectCameraControl.__init__(self)
        # remove mouse1 and mouse3 actions and only add mouse2 so we can keep
        # the selection and deselection from the editor commands
        self.actionEvents = [
            ['DIRECT-mouse2', self.mouseFlyStart],
            ['DIRECT-mouse2Up', self.mouseFlyStop],
            ]
