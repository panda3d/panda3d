"""Undocumented Module"""

__all__ = ['WidgetPropertiesDialog']

from direct.showbase.TkGlobal import *
from Tkinter import *
import types, string, Pmw

"""
TODO:
  Checkboxes for None?
  Floaters to adjust float values
  OK and Cancel to allow changes to be delayed
  Something other than Return to accept a new value
"""

class WidgetPropertiesDialog(Toplevel):
    """Class to open dialogs to adjust widget properties."""
    def __init__(self, propertyDict, propertyList = None, parent = None,
                 title = 'Widget Properties'):
        """Initialize a dialog.
        Arguments:
            propertyDict -- a dictionary of properties to be edited
            parent -- a parent window (the application window)
            title -- the dialog title
        """
        # Record property list
        self.propertyDict = propertyDict
        self.propertyList = propertyList
        if self.propertyList is None:
            self.propertyList = self.propertyDict.keys()
            self.propertyList.sort()
        # Use default parent if none specified
        if not parent:
            import Tkinter
            parent = Tkinter._default_root
        # Create toplevel window
        Toplevel.__init__(self, parent)
        self.transient(parent)
        # Set title
        if title:
            self.title(title)
        # Record parent
        self.parent = parent
        # Initialize modifications
        self.modifiedDict = {}
        # Create body
        body = Frame(self)
        self.initial_focus = self.body(body)
        body.pack(padx=5, pady=5)
        # Create OK Cancel button
        self.buttonbox()
        # Initialize window state
        self.grab_set()
        self.protocol("WM_DELETE_WINDOW", self.cancel)
        self.geometry("+%d+%d" % (parent.winfo_rootx()+50,
                                  parent.winfo_rooty()+50))
        self.initial_focus.focus_set()
        self.wait_window(self)

    def destroy(self):
        """Destroy the window"""
        self.propertyDict = {}
        self.initial_focus = None
        # Clean up balloons!
        for balloon in self.balloonList:
            balloon.withdraw()
        Toplevel.destroy(self)

    #
    # construction hooks
    def body(self, master):
        """create dialog body.
        return entry that should have initial focus.
        This method should be overridden, and is called
        by the __init__ method.
        """
        count = 0
        entryList = []
        self.balloonList = []
        for property in self.propertyList:
            propertySet = self.propertyDict[property]
            # Widget
            widget = propertySet.get('widget', None)
            # Get initial value
            initialvalue = widget[property]
            # Type of entry
            entryType = propertySet.get('type', 'real')
            # Is None an allowable value?
            fAllowNone = propertySet.get('fNone', 0)
            # Help string specified?
            helpString = propertySet.get('help', None)
            # Create label
            label = Label(master, text=property, justify=LEFT)
            label.grid(row=count, column = 0, padx=5, sticky=W)

            # Create entry
            entry = Pmw.EntryField(master, entry_justify = 'right')
            entry.grid(row=count, column = 1, padx=5, sticky=W+E)
            if initialvalue is None:
                entry.insert(0, 'None')
            else:
                entry.insert(0, initialvalue)

            # Create balloon for help
            balloon = Pmw.Balloon(state = 'balloon')
            self.balloonList.append(balloon)
            # extra info if None is allowed value
            if helpString is None:
                if fAllowNone:
                    extra = ' or None'
                else:
                    extra = ''
            # Set up help string and validator based upon type
            if entryType == 'real':
                # Only allow real numbers
                if fAllowNone:
                    entry['validate'] = { 'validator': self.realOrNone }
                else:
                    entry['validate'] = { 'validator': 'real' }
                if helpString is None:
                    helpString = 'Enter a floating point number' + extra + '.'
            elif entryType == 'integer':
                # Only allow integer values
                if fAllowNone:
                    entry['validate'] = { 'validator': self.intOrNone }
                else:
                    entry['validate'] = { 'validator': 'integer' }
                if helpString is None:
                    helpString = 'Enter an integer' + extra + '.'
            else:
                # Anything goes with a string widget
                if helpString is None:
                    helpString = 'Enter a string' + extra + '.'
            # Bind balloon with help string to entry
            balloon.bind(entry, helpString)
            # Create callback to execute whenever a value is changed
            modifiedCallback = (lambda f=self.modified, w=widget, e=entry,
                                p=property, t=entryType, fn=fAllowNone:
                                f(w, e, p, t, fn))
            entry['modifiedcommand'] = modifiedCallback
            # Keep track of the entrys
            entryList.append(entry)
            count += 1
        # Set initial focus
        if len(entryList) > 0:
            entry = entryList[0]
            entry.select_range(0, END)
            # Set initial focus to first entry in the list
            return entryList[0]
        else:
            # Just set initial focus to self
            return self

    def modified(self, widget, entry, property, type, fNone):
        self.modifiedDict[property] = (widget, entry, type, fNone)

    def buttonbox(self):
        """add standard button box buttons.
        """
        box = Frame(self)
        # Create buttons
        w = Button(box, text="OK", width=10, command=self.ok)
        w.pack(side=LEFT, padx=5, pady=5)
        # Create buttons
        w = Button(box, text="Cancel", width=10, command=self.cancel)
        w.pack(side=LEFT, padx=5, pady=5)
        # Bind commands
        self.bind("<Return>", self.ok)
        self.bind("<Escape>", self.cancel)
        # Pack
        box.pack()

    def realOrNone(self, val):
        val = string.lower(val)
        if string.find('none', val) != -1:
            if val == 'none':
                return Pmw.OK
            else:
                return Pmw.PARTIAL
        return Pmw.realvalidator(val)

    def intOrNone(self, val):
        val = string.lower(val)
        if string.find('none', val) != -1:
            if val == 'none':
                return Pmw.OK
            else:
                return Pmw.PARTIAL
        return Pmw.integervalidator(val)

    #
    # standard button semantics
    def ok(self, event=None):
        self.withdraw()
        self.update_idletasks()
        self.validateChanges()
        self.apply()
        self.cancel()

    def cancel(self, event=None):
        # put focus back to the parent window
        self.parent.focus_set()
        self.destroy()

    def validateChanges(self):
        for property in self.modifiedDict.keys():
            tuple = self.modifiedDict[property]
            widget = tuple[0]
            entry = tuple[1]
            type = tuple[2]
            fNone = tuple[3]
            value = entry.get()
            lValue = string.lower(value)
            if (string.find('none', lValue) != -1):
                if fNone and (lValue == 'none'):
                    widget[property] = None
            else:
                if type == 'real':
                    value = string.atof(value)
                elif type == 'integer':
                    value = string.atoi(value)
                widget[property] = value

    def apply(self):
        """process the data

        This method is called automatically to process the data, *after*
        the dialog is destroyed. By default, it does nothing.
        """
        pass # override

