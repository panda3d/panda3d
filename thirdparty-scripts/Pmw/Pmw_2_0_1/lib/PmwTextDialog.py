# A Dialog with a ScrolledText widget.

import Pmw

class TextDialog(Pmw.Dialog):
    def __init__(self, parent = None, **kw):
        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('borderx',     10,    INITOPT),
            ('bordery',     10,    INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.Dialog.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        aliases = (
            ('text', 'scrolledtext_text'),
            ('label', 'scrolledtext_label'),
        )
        self._text = self.createcomponent('scrolledtext',
                aliases, None,
                Pmw.ScrolledText, (interior,))
        self._text.pack(side='top', expand=1, fill='both',
                padx = self['borderx'], pady = self['bordery'])

        # Check keywords and initialise options.
        self.initialiseoptions()

    # Need to explicitly forward this to override the stupid
    # (grid_)bbox method inherited from Tkinter.Toplevel.Grid.
    def bbox(self, index):
        return self._text.bbox(index)

Pmw.forwardmethods(TextDialog, Pmw.ScrolledText, '_text')
