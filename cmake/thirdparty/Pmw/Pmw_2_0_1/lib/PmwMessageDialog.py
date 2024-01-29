# Based on iwidgets2.2.0/messagedialog.itk code.

import tkinter
import Pmw

class MessageDialog(Pmw.Dialog):
    def __init__(self, parent = None, **kw):

        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('borderx',       20,    INITOPT),
            ('bordery',       20,    INITOPT),
            ('iconmargin',    20,    INITOPT),
            ('iconpos',       None,  INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.Dialog.__init__(self, parent)

        # Create the components.
        interior = self.interior()

        self._message = self.createcomponent('message',
                (), None,
                tkinter.Label, (interior,))

        iconpos = self['iconpos']
        iconmargin = self['iconmargin']
        borderx = self['borderx']
        bordery = self['bordery']
        border_right = 2
        border_bottom = 2
        if iconpos is None:
            self._message.grid(column = 1, row = 1)
        else:
            self._icon = self.createcomponent('icon',
                    (), None,
                    tkinter.Label, (interior,))
            if iconpos not in 'nsew':
                raise ValueError('bad iconpos option "%s":  should be n, s, e, or w' \
                        % iconpos)

            if iconpos in 'nw':
                icon = 1
                message = 3
            else:
                icon = 3
                message = 1

            if iconpos in 'ns':
                # vertical layout
                self._icon.grid(column = 1, row = icon)
                self._message.grid(column = 1, row = message)
                interior.grid_rowconfigure(2, minsize = iconmargin)
                border_bottom = 4
            else:
                # horizontal layout
                self._icon.grid(column = icon, row = 1)
                self._message.grid(column = message, row = 1)
                interior.grid_columnconfigure(2, minsize = iconmargin)
                border_right = 4

        interior.grid_columnconfigure(0, minsize = borderx)
        interior.grid_rowconfigure(0, minsize = bordery)
        interior.grid_columnconfigure(border_right, minsize = borderx)
        interior.grid_rowconfigure(border_bottom, minsize = bordery)


        # Check keywords and initialise options.
        self.initialiseoptions()
