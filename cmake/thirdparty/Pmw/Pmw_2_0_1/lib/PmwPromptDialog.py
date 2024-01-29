# Based on iwidgets2.2.0/promptdialog.itk code.

import Pmw

# A Dialog with an entryfield

class PromptDialog(Pmw.Dialog):
    def __init__(self, parent = None, **kw):
        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('borderx',     20,    INITOPT),
            ('bordery',     20,    INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.Dialog.__init__(self, parent)

        # Create the components.
        interior = self.interior()
        aliases = (
            ('entry', 'entryfield_entry'),
            ('label', 'entryfield_label'),
        )
        self._promptDialogEntry = self.createcomponent('entryfield',
                aliases, None,
                Pmw.EntryField, (interior,))
        self._promptDialogEntry.pack(fill='x', expand=1,
                padx = self['borderx'], pady = self['bordery'])

        if 'activatecommand' not in kw:
            # Whenever this dialog is activated, set the focus to the
            # EntryField's entry widget.
            tkentry = self.component('entry')
            self.configure(activatecommand = tkentry.focus_set)

        # Check keywords and initialise options.
        self.initialiseoptions()

    # Supply aliases to some of the entry component methods.
    def insertentry(self, index, text):
        self._promptDialogEntry.insert(index, text)

    def deleteentry(self, first, last=None):
        self._promptDialogEntry.delete(first, last)

    def indexentry(self, index):
        return self._promptDialogEntry.index(index)

Pmw.forwardmethods(PromptDialog, Pmw.EntryField, '_promptDialogEntry')
