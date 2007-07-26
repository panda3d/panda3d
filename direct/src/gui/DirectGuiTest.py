"""Undocumented Module"""

__all__ = []


if __name__ == "__main__":
    from direct.directbase import DirectStart
    from DirectGui import *
    #from whrandom import *
    from random import *

    # EXAMPLE CODE
    # Load a model
    smiley = loader.loadModel('models/misc/smiley')

    # Here we specify the button's command
    def dummyCmd(index):
        print 'Button %d POW!!!!' % index

    # Define some commands to bind to enter, exit and click events
    def shrink(db):
        db['text2_text'] = 'Hi!'
        taskMgr.remove('shrink')
        taskMgr.remove('expand')
        # Get a handle on the geometry for the rollover state
        rolloverSmiley = db.component('geom2')
        rolloverSmiley.setScale(db.component('geom0').getScale()[0])
        rolloverSmiley.lerpScale(.1, .1, .1, 1.0, blendType = 'easeInOut',
                                 task = 'shrink')

    def expand(db):
        db['text0_text'] = 'Bye!'
        taskMgr.remove('shrink')
        taskMgr.remove('expand')
        db.component('geom0').setScale(db.component('geom2').getScale()[0])
        db.component('geom0').lerpScale(1, 1, 1, 1, blendType = 'easeInOut',
                                 task = 'expand')
        db.component('geom2').clearColor()

    def ouch(db):
        taskMgr.remove('shrink')
        taskMgr.remove('expand')
        taskMgr.remove('runAway')
        db.component('geom0').setScale(db.component('geom2').getScale()[0])
        db.component('geom1').setScale(db.component('geom2').getScale()[0])
        db['text2_text'] = 'Ouch!'
        db['geom2_color'] = Vec4(1, 0, 0, 1)
        newX = -1.0 + random() * 2.0
        newZ = -1.0 + random() * 2.0
        db.lerpPos(Point3(newX, 0, newZ), 1.0, task = 'runAway',
                   blendType = 'easeOut')

    dl = DirectFrame(image = 'models/maps/noise.rgb')
    dl.setScale(.5)

    # Create a button with a background image, smiley as a geometry element,
    # and a text overlay, set a different text for the four button states:
    # (normal, press, rollover, and disabled), set scale = .15, and relief raised
    dbArray = []
    for i in range(10):
        db = DirectButton(parent = dl,
                          image = 'models/maps/noise.rgb',
                          geom = smiley,
                          text = ('Hi!', 'Ouch!', 'Bye!', 'ZZZZ!'),
                          scale = .15, relief = 'raised',
                          # Here we set an option for a component of the button
                          geom1_color = Vec4(1, 0, 0, 1),
                          # Here is an example of a component group option
                          text_pos = (.6, -.8),
                          # Set audio characteristics
                          clickSound = DirectGuiGlobals.getDefaultClickSound(),
                          rolloverSound = DirectGuiGlobals.getDefaultRolloverSound()
                          )

        # You can set component or component group options after a gui item
        # has been created
        db['text_scale'] = 0.5
        db['command'] = lambda i = i: dummyCmd(i)

        # Bind the commands
        db.bind(DirectGuiGlobals.ENTER, lambda x, db = db: shrink(db))
        db.bind(DirectGuiGlobals.EXIT, lambda x, db = db: expand(db))
        db.bind(DirectGuiGlobals.B1PRESS, lambda x, db = db: ouch(db))
        # Pop up placer when button 2 is pressed
        db.bind(DirectGuiGlobals.B3PRESS, lambda x, db = db: db.place())

        dbArray.append(db)

        # To get rid of button and clear out hooks call:
        # db.destroy()

    # DIRECT ENTRY EXAMPLE
    def printEntryText(text):
        print 'Text:', text

    # Here we create an entry, and specify everything up front
    # CALL de1.get() and de1.set('new text') to get and set entry contents
    de1 = DirectEntry(initialText = 'Hello, how are you?',
                      image = 'models/maps/noise.rgb',
                      image_pos = (4.55, 0, -2.55),
                      image_scale = (5.5, 1, 4),
                      command = printEntryText,
                      pos = (-1.1875, 0, 0.879167),
                      scale = 0.0707855,
                      cursorKeys = 1,
                      )

    # DIRECT DIALOG EXAMPLE
    def printDialogValue(value):
        print 'Value:', value

    simpleDialog = YesNoDialog(text = 'Simple',
                               command = printDialogValue)

    customValues = YesNoDialog(text = 'Not Quite So Simple',
                               buttonValueList = ['Yes', 'No'],
                               command = printDialogValue)


    fancyDialog = YesNoDialog(text = 'Testing Direct Dialog',
                              geom = smiley,
                              geom_scale = .1,
                              geom_pos = (-0.3, 0, 0),
                              command = printDialogValue)

    customDialog = DirectDialog(text = 'Pick a number',
                                buttonTextList = map(str, range(10)),
                                buttonValueList = range(10),
                                command = printDialogValue)



    # NOTE: There are some utility functions which help you get size
    # of a direct gui widget.  These can be used to position and scale an
    # image after you've created the entry.  scale = (width/2, 1, height/2)
    print 'BOUNDS:', de1.getBounds()
    print 'WIDTH:', de1.getWidth()
    print 'HEIGHT:', de1.getHeight()
    print 'CENTER:', de1.getCenter()

    run()
