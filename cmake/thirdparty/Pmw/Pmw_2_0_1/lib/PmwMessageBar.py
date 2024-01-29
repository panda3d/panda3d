# Class to display messages in an information line.

import string
import tkinter
import Pmw

class MessageBar(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        defaultMessageTypes = {
                           # (priority, showtime, bells, logmessage)
            'systemerror'  : (5, 10, 2, 1),
            'usererror'    : (4, 5, 1, 0),
            'busy'         : (3, 0, 0, 0),
            'systemevent'  : (2, 5, 0, 0),
            'userevent'    : (2, 5, 0, 0),
            'help'         : (1, 5, 0, 0),
            'state'        : (0, 0, 0, 0),
        }
        optiondefs = (
            ('labelmargin',    0,                     INITOPT),
            ('labelpos',       None,                  INITOPT),
            ('messagetypes',   defaultMessageTypes,   INITOPT),
            ('silent',         0,                     None),
            ('sticky',         'ew',                  INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.MegaWidget.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        self._messageBarEntry = self.createcomponent('entry',
                (), None,
                tkinter.Entry, (interior,))

        # Can't always use 'disabled', since this greys out text in Tk 8.4.2
        try:
            self._messageBarEntry.configure(state = 'readonly')
        except tkinter.TclError:
            self._messageBarEntry.configure(state = 'disabled')

        self._messageBarEntry.grid(column=2, row=2, sticky=self['sticky'])
        interior.grid_columnconfigure(2, weight=1)
        interior.grid_rowconfigure(2, weight=1)

        self.createlabel(interior)

        # Initialise instance variables.
        self._numPriorities = 0
        for info in list(self['messagetypes'].values()):
            if self._numPriorities < info[0]:
                self._numPriorities = info[0]

        self._numPriorities = self._numPriorities + 1
        self._timer = [None] * self._numPriorities
        self._messagetext = [''] * self._numPriorities
        self._activemessage = [0] * self._numPriorities

        # Check keywords and initialise options.
        self.initialiseoptions()

    def destroy(self):
        for timerId in self._timer:
            if timerId is not None:
                self.after_cancel(timerId)
        self._timer = [None] * self._numPriorities
        Pmw.MegaWidget.destroy(self)

    def message(self, type, text):
        # Display a message in the message bar.

        (priority, showtime, bells, logmessage) = self['messagetypes'][type]

        if not self['silent']:
            for i in range(bells):
                if i != 0:
                    self.after(100)
                self.bell()

        self._activemessage[priority] = 1
        if text is None:
            text = ''
        self._messagetext[priority] = text.replace('\n', ' ')
        self._redisplayInfoMessage()

        if logmessage:
            # Should log this text to a text widget.
            pass

        if showtime > 0:
            if self._timer[priority] is not None:
                self.after_cancel(self._timer[priority])

            # Define a callback to clear this message after a time.
            def _clearmessage(self=self, priority=priority):
                self._clearActivemessage(priority)

            mseconds = int(showtime * 1000)
            self._timer[priority] = self.after(mseconds, _clearmessage)

    def helpmessage(self, text):
        if text is None:
            self.resetmessages('help')
        else:
            self.message('help', text)

    def resetmessages(self, type):
        priority = self['messagetypes'][type][0]
        self._clearActivemessage(priority)
        for messagetype, info in list(self['messagetypes'].items()):
            thisPriority = info[0]
            showtime = info[1]
            if thisPriority < priority and showtime != 0:
                self._clearActivemessage(thisPriority)

    def _clearActivemessage(self, priority):
        self._activemessage[priority] = 0
        if self._timer[priority] is not None:
            self.after_cancel(self._timer[priority])
            self._timer[priority] = None
        self._redisplayInfoMessage()

    def _redisplayInfoMessage(self):
        text = ''
        for priority in range(self._numPriorities - 1, -1, -1):
            if self._activemessage[priority]:
                text = self._messagetext[priority]
                break
        self._messageBarEntry.configure(state = 'normal')
        self._messageBarEntry.delete(0, 'end')
        self._messageBarEntry.insert('end', text)

        # Can't always use 'disabled', since this greys out text in Tk 8.4.2
        try:
            self._messageBarEntry.configure(state = 'readonly')
        except tkinter.TclError:
            self._messageBarEntry.configure(state = 'disabled')

Pmw.forwardmethods(MessageBar, tkinter.Entry, '_messageBarEntry')
