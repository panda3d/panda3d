# Not Based on iwidgets version.

import Pmw

class ComboBoxDialog(Pmw.Dialog):
    # Dialog window with simple combobox.

    # Dialog window displaying a list and entry field and requesting
    # the user to make a selection or enter a value

    def __init__(self, parent = None, **kw):
        # Define the megawidget options.
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('borderx',    10,              INITOPT),
            ('bordery',    10,              INITOPT),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining the options).
        Pmw.Dialog.__init__(self, parent)

        # Create the components.
        interior = self.interior()

        aliases = (
            ('listbox', 'combobox_listbox'),
            ('scrolledlist', 'combobox_scrolledlist'),
            ('entry', 'combobox_entry'),
            ('label', 'combobox_label'),
        )
        self._combobox = self.createcomponent('combobox',
                aliases, None,
                Pmw.ComboBox, (interior,),
                scrolledlist_dblclickcommand = self.invoke,
                dropdown = 0,
        )
        self._combobox.pack(side='top', expand='true', fill='both',
                padx = self['borderx'], pady = self['bordery'])

        if 'activatecommand' not in kw:
            # Whenever this dialog is activated, set the focus to the
            # ComboBox's listbox widget.
            listbox = self.component('listbox')
            self.configure(activatecommand = listbox.focus_set)

        # Check keywords and initialise options.
        self.initialiseoptions()

    # Need to explicitly forward this to override the stupid
    # (grid_)size method inherited from Tkinter.Toplevel.Grid.
    def size(self):
        return self._combobox.size()

    # Need to explicitly forward this to override the stupid
    # (grid_)bbox method inherited from Tkinter.Toplevel.Grid.
    def bbox(self, index):
        return self._combobox.bbox(index)

Pmw.forwardmethods(ComboBoxDialog, Pmw.ComboBox, '_combobox')
