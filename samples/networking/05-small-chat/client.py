from direct.showbase.ShowBase import ShowBase
from direct.distributed.ClientRepository import ClientRepository
from panda3d.core import URLSpec, ConfigVariableInt, ConfigVariableString
from message import Message
from ClientRepository import GameClientRepository
from direct.gui.DirectGui import DirectButton, DirectEntry, DirectFrame
from direct.gui import DirectGuiGlobals as DGG

# initialize the engine
base = ShowBase()

client = GameClientRepository()

# Setup the GUI

# this frame will contain all our GUI elements
frameMain = DirectFrame(frameColor = (0, 0, 0, 1))

def clearText():
    ''' Write an empty string in the textbox '''
    txt_msg.enterText('')

def setDefaultText():
    ''' Write the default message in the textbox '''
    txt_msg.enterText('Your Message')

def send(args=None):
    ''' Send the text written in the message textbox to the clients '''
    client.sendMessage(txt_msg.get())
    txt_msg.enterText('')

# the Textbox where we write our Messages, which we
# want to send over to the other users
txt_msg = DirectEntry(
    initialText = 'Your Message',
    cursorKeys = True,
    pos = (-0.9,0,-0.9),
    scale = 0.1,
    width = 14,
    focusInCommand = clearText,
    focusOutCommand = setDefaultText,
    command = send,
    parent = frameMain)

# a button to initiate the sending of the message
btn_send = DirectButton(
    text = 'Send',
    pos = (0.75,0,-0.9),
    scale = 0.15,
    command = send,
    parent = frameMain)

# This will show all sent messages
txt_messages = DirectEntry(
    cursorKeys = False,
    pos = (-0.8,0,0.5),
    scale = 0.1,
    width = 14,
    numLines = 10,
    state = DGG.DISABLED,
    parent = frameMain)

# Function which will write the given text in the
# textbox, where we store all send and recieved messages.
# This Function will only write the last 10 messages and
# cut of the erlier messages
def setText(messageText):
    # get all messages from the textbox
    parts = txt_messages.get().split('\n')
    if len(parts) >= 10:
        cutParts = ''
        # as the textbox can only hold 10 lines cut out the first entry
        for i in range(1,len(parts)):
            cutParts += parts[i] + '\n'
        txt_messages.enterText(cutParts + messageText)
    else:
        txt_messages.enterText(txt_messages.get() + '\n' + messageText)

# create a DirectObject instance, which will then catch the events and
# handle them with the given functions
base.accept('setText', setText)
base.accept('escape', exit)

# start the application
base.run()
