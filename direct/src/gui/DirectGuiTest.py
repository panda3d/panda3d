from DirectGui import *
from whrandom import *

# EXAMPLE CODE
# Load a model
smiley = loader.loadModel('models/directmodels/smiley')

# Here we specify the button's command
def dummyCmd(index):
    print 'Button %d POW!!!!' % index

# Define some commands to bind to enter, exit and click events
def shrink(db):
    db['text2_text'] = 'Hi!'
    taskMgr.removeTasksNamed('shrink')
    taskMgr.removeTasksNamed('expand')
    # Get a handle on the geometry for the rollover state
    rolloverSmiley = db.component('geom2')
    rolloverSmiley.setScale(db.component('geom0').getScale()[0])
    rolloverSmiley.lerpScale(.1,.1,.1, 1.0, blendType = 'easeInOut',
                             task = 'shrink')

def expand(db):
    db['text0_text'] = 'Bye!'
    taskMgr.removeTasksNamed('shrink')
    taskMgr.removeTasksNamed('expand')
    db.component('geom0').setScale(db.component('geom2').getScale()[0])
    db.component('geom0').lerpScale(1,1,1, 1, blendType = 'easeInOut',
                             task = 'expand')
    db.component('geom2').clearColor()

def ouch(db):
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

dl = DirectFrame(image = 'phase_4/maps/middayskyB.jpg')
dl.setScale(.5)

# Create a button with a background image, smiley as a geometry element,
# and a text overlay, set a different text for the four button states:
# (normal, press, rollover, and disabled), set scale = .15, and relief raised
for i in range(10):
    db = DirectButton(parent = dl,
                      image = 'phase_4/maps/middayskyB.jpg',
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
    db['command'] = lambda i = i: dummyCmd(i)

    # Bind the commands
    db.bind(ENTER, lambda x, db = db: shrink(db))
    db.bind(EXIT, lambda x, db = db: expand(db))
    db.bind(B1PRESS, lambda x, db = db: ouch(db))
    # Pop up placer when button 2 is pressed
    db.bind(B3PRESS, lambda x, db = db: db.place())
    
    # To get rid of button and clear out hooks call:
    # db.destroy()
