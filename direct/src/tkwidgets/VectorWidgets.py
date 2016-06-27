"""Undocumented Module"""

__all__ = ['VectorEntry', 'Vector2Entry', 'Vector3Entry', 'Vector4Entry', 'ColorEntry']

from direct.showbase.TkGlobal import *
from . import Valuator
import Pmw
import sys

if sys.version_info >= (3, 0):
    from tkinter.colorchooser import askcolor
else:
    from tkColorChooser import askcolor


class VectorEntry(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):

        # Default vector size
        DEFAULT_DIM = 3
        # Default value depends on *actual* vector size, test for user input
        DEFAULT_VALUE = [0.0] * kw.get('dim', DEFAULT_DIM)
        DEFAULT_LABELS = ['v[%d]' % x for x in range(kw.get('dim', DEFAULT_DIM))]

        # Process options
        INITOPT = Pmw.INITOPT
        optiondefs = (
            ('dim',                 DEFAULT_DIM,    INITOPT),
            ('value',        DEFAULT_VALUE,  INITOPT),
            ('resetValue',          DEFAULT_VALUE,  None),
            ('label_width',         12,             None),
            ('labelIpadx',          2,              None),
            ('command',             None,           None),
            ('entryWidth',          8,              self._updateEntryWidth),
            ('relief',              GROOVE,         self._updateRelief),
            ('bd',                  2,              self._updateBorderWidth),
            ('text',                'Vector:',      self._updateText),
            ('min',                 None,           self._updateValidate),
            ('max',                 None,           self._updateValidate),
            ('numDigits',           2,              self._setSigDigits),
            ('type',                'floater',      None),
            ('state',               'normal',       self._setState),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize superclass
        Pmw.MegaWidget.__init__(self, parent)

        # Initialize value
        # Make sure its a list (and as a byproduct, make a distinct copy)
        self._value = list(self['value'])
        self['resetValue'] = self['value']
        self._floaters = None
        self.entryFormat = '%.2f'

        # Get a handle on the parent container
        interior = self.interior()

        # This does double duty as a menu button
        self._label = self.createcomponent('label', (), None,
                                           Menubutton, (interior,),
                                           text = self['text'],
                                           activebackground = '#909090')
        self.menu = self._label['menu'] = Menu(self._label)
        self.menu.add_command(label = 'Reset', command = self.reset)
        self.menu.add_command(label = 'Popup sliders', command = self.popupSliders)
        self._label.pack(side = LEFT, fill = X, ipadx = self['labelIpadx'])

        self.variableList = []
        self.entryList = []
        for index in range(self['dim']):
            var = StringVar()
            self.variableList.append(var)
            # To set the configuration of all entrys in a vector use:
            # ve.configure(Entry_XXX = YYY)
            # To configure an individual entryfield's entry use:
            # ve.configure(entry0_XXX = YYY)
            entry = self.createcomponent(
                'entryField%d' % index,
                (('entry%d' % index,
                  'entryField%d_entry' % index),),
                'Entry',
                Pmw.EntryField, (interior,),
                entry_justify = RIGHT,
                entry_textvariable = var,
                command = lambda s = self, i = index: s._entryUpdateAt(i))
            entry.pack(side = LEFT, expand = 1, fill = X)
            self.entryList.append(entry)

        # To configure the floaterGroup use:
        # ve.configure(floaterGroup_XXX = YYY)
        # ve.configure(fGroup_XXX = YYY) or
        # To set the configuration all floaters in a group use:
        # ve.configure(Valuator_XXX = YYY)
        # To configure an individual floater in a group use:
        # ve.configure(floaterGroup_floater0_XXX = YYY) or
        # ve.configure(fGroup_floater0_XXX = YYY)
        self._floaters = self.createcomponent(
            'floaterGroup',
            (('fGroup', 'floaterGroup'),
             ('valuator', 'floaterGroup_valuator'),), None,
            Valuator.ValuatorGroupPanel, (self.interior(),),
            dim = self['dim'],
            #title = self['text'],
            type = self['type'],
            command = self.set)
        # Note: This means the 'X' on the menu bar doesn't really destroy
        # the panel, just withdraws it.  This is to avoid problems which occur
        # if the user kills the floaterGroup and then tries to pop it open again
        self._floaters.userdeletefunc(self._floaters.withdraw)
        self._floaters.withdraw()


        # Make sure entries are updated
        self.set(self['value'])

        # Record entry color
        self.entryBackground = self.cget('Entry_entry_background')

        # Make sure input variables processed
        self.initialiseoptions(VectorEntry)

    def menu(self):
        return self.menu

    def label(self):
        return self._label

    def entry(self, index):
        return self.entryList[index]

    def entryList(self):
        return self.entryList

    def floaters(self):
        return self._floaters

    def _clearFloaters(self):
        self._floaters.withdraw()

    def _updateText(self):
        self._label['text'] = self['text']

    def _updateRelief(self):
        self.interior()['relief'] = self['relief']

    def _updateBorderWidth(self):
        self.interior()['bd'] = self['bd']

    def _updateEntryWidth(self):
        self['Entry_entry_width'] = self['entryWidth']

    def _setSigDigits(self):
        sd = self['numDigits']
        self.entryFormat = '%.' + '%d' % sd + 'f'
        self.configure(valuator_numDigits = sd)
        # And refresh value to reflect change
        for index in range(self['dim']):
            self._refreshEntry(index)

    def _updateValidate(self):
        # Update entry field to respect new limits
        self.configure(Entry_validate = {
            'validator': 'real',
            'min': self['min'],
            'max': self['max'],
            'minstrict': 0,
            'maxstrict': 0})
        # Reflect changes in floaters
        self.configure(valuator_min = self['min'],
                       valuator_max = self['max'])

    def get(self):
        return self._value

    def getAt(self, index):
        return self._value[index]

    def set(self, value, fCommand = 1):
        if type(value) in (float, int):
            value = [value] * self['dim']
        for i in range(self['dim']):
            self._value[i] = value[i]
            self.variableList[i].set(self.entryFormat % value[i])
        self.action(fCommand)

    def setAt(self, index, value, fCommand = 1):
        self.variableList[index].set(self.entryFormat % value)
        self._value[index] = value
        self.action(fCommand)

    def _entryUpdateAt(self, index):
        entryVar = self.variableList[index]
        # Did we get a valid float?
        try:
            newVal = float(entryVar.get())
        except ValueError:
            return

        # Clamp value
        if self['min'] is not None:
            if newVal < self['min']:
                newVal = self['min']
        if self['max'] is not None:
            if newVal > self['max']:
                newVal = self['max']

        # Update vector's value
        self._value[index] = newVal

        # refresh entry to reflect formatted value
        self._refreshEntry(index)

        # Update the floaters and call the command
        self.action()

    def _refreshEntry(self, index):
        self.variableList[index].set(self.entryFormat % self._value[index])
        self.entryList[index].checkentry()

    def _refreshFloaters(self):
        if self._floaters:
            self._floaters.set(self._value, 0)

    def action(self, fCommand = 1):
        self._refreshFloaters()
        if fCommand and (self['command'] != None):
            self['command'](self._value)

    def reset(self):
        self.set(self['resetValue'])

    def addMenuItem(self, label = '', command = None):
        self.menu.add_command(label = label, command = command)

    def popupSliders(self):
        self._floaters.set(self.get()[:])
        self._floaters.show()

    def _setState(self):
        if self['state'] == 'disabled':
            # Disable entry
            self.configure(Entry_entry_state = 'disabled')
            self.configure(Entry_entry_background = '#C0C0C0')
            # Disable floater Group scale
            self.component('fGroup').configure(
                valuator_state = 'disabled')
            # Disable floater group entry
            self.component('fGroup').configure(
                valuator_entry_state = 'disabled')
            self.component('fGroup').configure(
                valuator_entry_background = '#C0C0C0')
        else:
            # Disable entry
            self.configure(Entry_entry_state = 'normal')
            self.configure(Entry_entry_background = self.entryBackground)
            # Disable floater Group scale
            self.component('fGroup').configure(
                valuator_state = 'normal')
            # Disable floater group entry
            self.component('fGroup').configure(
                valuator_entry_state = 'normal')
            self.component('fGroup').configure(
                valuator_entry_background = self.entryBackground)

class Vector2Entry(VectorEntry):
    def __init__(self, parent = None, **kw):
        # Initialize options for the class
        optiondefs = (
            ('dim',    2,       Pmw.INITOPT),
            ('fGroup_labels',   ('X','Y','Z'),  None),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass, make sure dim makes it to superclass
        VectorEntry.__init__(self, parent, dim = self['dim'])
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(Vector2Entry)

class Vector3Entry(VectorEntry):
    def __init__(self, parent = None, **kw):
        # Initialize options for the class
        optiondefs = (
            ('dim',    3,       Pmw.INITOPT),
            ('fGroup_labels',   ('X','Y','Z'),  None),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass, make sure dim makes it to superclass
        VectorEntry.__init__(self, parent, dim = self['dim'])
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(Vector3Entry)

class Vector4Entry(VectorEntry):
    def __init__(self, parent = None, **kw):
        # Initialize options for the class
        optiondefs = (
            ('dim',     4,      Pmw.INITOPT),
            ('fGroup_labels',   ('X','Y','Z','W'),  None),
            )
        self.defineoptions(kw, optiondefs)
        # Initialize the superclass, make sure dim makes it to superclass
        VectorEntry.__init__(self, parent, dim = self['dim'])
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(Vector4Entry)

class ColorEntry(VectorEntry):
    def __init__(self, parent = None, **kw):
        # Initialize options for the class (overriding some superclass options)
        optiondefs = (
            ('dim',                     4,                  Pmw.INITOPT),
            ('type',                    'slider',           Pmw.INITOPT),
            ('fGroup_labels',           ('R','G','B','A'),  None),
            ('min',                     0.0,                None),
            ('max',                     255.0,              None),
            ('nuDigits',                0,                  None),
            ('valuator_resolution',     1.0,                None),
            )
        self.defineoptions(kw, optiondefs)

        # Initialize the superclass, make sure dim makes it to superclass
        VectorEntry.__init__(self, parent, dim = self['dim'])
        # Add menu item to popup color picker
        self.addMenuItem(
            'Popup color picker',
            command = lambda s = self: s.popupColorPicker())
        # Needed because this method checks if self.__class__ is myClass
        # where myClass is the argument passed into inialiseoptions
        self.initialiseoptions(ColorEntry)

    def popupColorPicker(self):
        # Can pass in current color with: color = (255, 0, 0)
        color = askcolor(
            parent = self.interior(),
            # Initialize it to current color
            initialcolor = tuple(self.get()[:3]))[0]
        if color:
            self.set((color[0], color[1], color[2], self.getAt(3)))

if __name__ == '__main__':
    root = Toplevel()
    root.title('Vector Widget demo')

    ve = VectorEntry(root); ve.pack()
    v3e = Vector3Entry(root); v3e.pack()
    v4e = Vector4Entry(root); v4e.pack()
    ce = ColorEntry(root); ce.pack()
