from DirectGui import *
from whrandom import *

# EXAMPLE CODE
# Load a model
smiley = loader.loadModel('models/directmodels/smiley')

# Create a button with a background image, smiley as a geometry element,
# and a text overlay, set a different text for the four button states:
# (normal, press, rollover, and disabled), set scale = .15, and relief raised
db = DirectButton(image = 'phase_4/maps/middayskyB.jpg',
                  geom = smiley,
                  text = ('Hi!', 'Ouch!', 'Bye!', 'ZZZZ!'),
                  scale = .15, relief = 'raised',
                  # Here we set an option for a component of the button
                  geom1_color = Vec4(1,0,0,1),
                  # Here is an example of a component group option
                  text_pos = (.6, -.8))

# You can set component or component group options after a gui item
# has been created
db['text_scale'] = 0.5

# Here we specify the button's command
def dummyCmd():
    print 'POW!!!!'

db['command'] = dummyCmd

# Get a handle on the geometry for the rollover state
rolloverSmiley = db.component('geom2')

# Define some commands to bind to enter, exit and click events
def shrink(event):
    db['text2_text'] = 'Hi!'
    taskMgr.removeTasksNamed('shrink')
    taskMgr.removeTasksNamed('expand')
    rolloverSmiley.setScale(db.component('geom0').getScale()[0])
    rolloverSmiley.lerpScale(.1,.1,.1, 1.0, blendType = 'easeInOut',
                             task = 'shrink')

def expand(event):
    db['text0_text'] = 'Bye!'
    taskMgr.removeTasksNamed('shrink')
    taskMgr.removeTasksNamed('expand')
    db.component('geom0').setScale(db.component('geom2').getScale()[0])
    db.component('geom0').lerpScale(1,1,1, 1, blendType = 'easeInOut',
                             task = 'expand')
    db.component('geom2').clearColor()

def ouch(event):
    taskMgr.removeTasksNamed('shrink')
    taskMgr.removeTasksNamed('expand')
    taskMgr.removeTasksNamed('runAway')
    db.component('geom0').setScale(db.component('geom2').getScale()[0])
    db.component('geom1').setScale(db.component('geom2').getScale()[0])
    db['text2_text'] = 'Ouch!'
    db['geom2_color'] = Vec4(1,0,0,1)
    newX = -1.0 + random() * 2.0
    newZ = -1.0 + random() * 2.0
    db.lerpPos(Point3(newX, 0, newZ), 1.0, task = 'runAway',
               blendType = 'easeOut')

# Bind the commands
db.bind(ENTER, shrink)
db.bind(EXIT, expand)
db.bind(B1PRESS, ouch)
# Pop up placer when button 2 is pressed
db.bind(B2PRESS, lambda x, s = db: s.place())

# To get rid of button and clear out hooks call:
# db.destroy()
