# This tests Pmw option and component handling.

import tkinter
import Test
import Pmw

Test.initialise()

"""
    Definitions:
        initialisation option:  an option that can be set in the call
            to the constructor but not in configure()
        configuration option:  an option that can be set in the call
            to the constructor and to configure()
        option: either an initialisation option or a configuration option

    Tests
    -----
    in constructor:
    + define an option, its default value and whether it is an
        initialisation or a configuration option
    + set a callback function for a configuration option
    + set a different default for an option of a base class
    + set a different default for an option of a component of a base class
    + override the callback for a configuration option of a base class
    + create a component
    + create an alias for a component
    + create an alias for a sub-component

    calling constructor:
    + set an option
    + set an option of a base class
    + set an option of a component created in the constructor
    + set an option of an aliased component or sub-component created in
        the constructor
    + set an option of one or more components via their group name
    + use the default value of an option
    + use the default value of an option of a base class
    + use the default value of an option of a base class where the default
        value is redefined in the derived class

    calling configure:
    + set a configuration option
    + set a configuration option of a base class
    + set a configuration option of a component
    + set a configuration option of an aliased component or sub-component
    + set a configuration option of one or more components via their group name
    + set a configuration option with a callback
    + set a configuration option of a base class with a callback in the
        derived class
"""

class Simple(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        optiondefs = (
            ('initsimple1', 'initsimple1', Pmw.INITOPT),
            ('initsimple2', 'initsimple2', Pmw.INITOPT),
            ('optsimple1', 'optsimple1', None),
            ('optsimple2', 'optsimple2', None),
        )
        self.defineoptions(kw, optiondefs)
        Pmw.MegaWidget.__init__(self, parent)

        interior = self.interior()
        self._widget = self.createcomponent('widget',
                (('widgy', 'widget'),), None,
                tkinter.Button, (interior,))
        self._widget.grid(column=0, row=0, sticky='nsew')

        self.initialiseoptions()

class Complex(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        optiondefs = (
            ('initcomplex1', 'initcomplex1', Pmw.INITOPT),
            ('initcomplex2', 'initcomplex2', Pmw.INITOPT),
            ('optcomplex1', 'optcomplex1', None),
            ('optcomplex2', 'optcomplex2', None),
        )
        self.defineoptions(kw, optiondefs)
        Pmw.MegaWidget.__init__(self, parent)

        interior = self.interior()
        self._simple = self.createcomponent('simple',
                (('widget', 'simple_widget'),), None,
                Simple, (interior,))
        self._simple.grid(column=0, row=0, sticky='nsew')

        self.initialiseoptions()

class Base(Pmw.MegaWidget):
    def __init__(self, parent = None, **kw):
        optiondefs = (
            ('initbase1', 'initbase1', Pmw.INITOPT),
            ('initbase2', 'initbase2', Pmw.INITOPT),
            ('initbase3', 'initbase3', Pmw.INITOPT),
            ('optbase1', 'optbase1', self._optbase1),
            ('optbase2', 'optbase2', None),
            ('optbase3', 'optbase3', None),
        )
        self.defineoptions(kw, optiondefs)
        Pmw.MegaWidget.__init__(self, parent)

        oldInterior = Pmw.MegaWidget.interior(self)
        self._widget = self.createcomponent('basesimple',
                (('widget', 'basesimple_widget'),), None,
                Simple, (oldInterior,))
        self._widget.grid(column=0, row=0, sticky='nsew')

        self._child = self.createcomponent('child',
                (), 'Mygroup',
                tkinter.Frame, (oldInterior,))
        self._child.grid(column=0, row=1, sticky='nsew')

        self._groupie = self.createcomponent('groupie',
                (), 'Mygroup',
                tkinter.Button, (oldInterior,), text = 'XXXXX')
        self._groupie.grid(column=0, row=2, sticky='nsew')

        self.basedummy = []

        self.initialiseoptions()

    def _optbase1(self):
        self.basedummy.append(self['optbase1'])

    def getbasedummy(self):
        return self.basedummy

    def interior(self):
        return self._child

class Derived(Base):
    def __init__(self, parent = None, **kw):
        # Define the options for this megawidget.
        optiondefs = (
            ('initbase2', 'initbase2inderived', Pmw.INITOPT),
            ('initderived1', 'initderived1', Pmw.INITOPT),
            ('initderived2', 'initderived2', Pmw.INITOPT),
            ('optbase1', 'optbase1', self._optbase1),
            ('optderived1', 'optderived1', None),
            ('optderived2', 'optderived2', None),
            ('groupie_text', 'YYYYY', None),
        )
        self.defineoptions(kw, optiondefs)

        # Initialise the base class (after defining my options).
        Base.__init__(self, parent)

        # Create components.
        interior = self.interior()
        self._widget = self.createcomponent('derivedcomplex',
                (('derivedsimple', 'derivedcomplex_simple'),), None,
                Complex, (interior,))
        self._widget.grid(column=0, row=0, sticky='nsew')

        # Initialise instance.

        # Initialise instance variables.
        self.deriveddummy = []

        # Check keywords and initialise options.
        self.initialiseoptions()

    def _optbase1(self):
        self.deriveddummy.append(self['optbase1'])

    def getderiveddummy(self):
        return self.deriveddummy

testData = ()

c = Simple
kw_1 = {
    'hull_borderwidth' :2,
    'hull_relief' :'sunken',
    'hull_background' :'red',
    'widget_text' :'simple',
    'widgy_foreground' :'red',
    'initsimple1' :'initsimple1_new',
}
tests = (
    (c.pack, ()),
    (c.components, (), ['hull', 'widget']),
    (c.componentaliases, (), [('widgy', 'widget'),]),
    (c.options, (), [('initsimple1', 'initsimple1', 1), ('initsimple2', 'initsimple2', 1), ('optsimple1', 'optsimple1', 0), ('optsimple2', 'optsimple2', 0)]),
    (c.cget, 'initsimple1', 'initsimple1_new'),
    (c.cget, 'initsimple2', 'initsimple2'),
    (c.cget, 'optsimple1', 'optsimple1'),
    (c.cget, 'optsimple2', 'optsimple2'),
    (c.cget, 'widget_foreground', 'red'),
    ('optsimple1', 'optsimple1_new'),
    (c.cget, 'optsimple1', 'optsimple1_new'),
)
testData = testData + ((c, ((tests, kw_1),)),)


c = Complex
kw_1 = {
    'hull_borderwidth' : 2,
    'hull_relief' : 'sunken',
    'hull_background' : 'red',
    'simple_widget_text' : 'complex',
    'widget_foreground' : 'yellow',
}
tests = (
    (c.pack, ()),
    (c.components, (), ['hull', 'simple']),
    (c.componentaliases, (), [('widget', 'simple_widget'),]),
    (c.options, (), [('initcomplex1', 'initcomplex1', 1), ('initcomplex2', 'initcomplex2', 1), ('optcomplex1', 'optcomplex1', 0), ('optcomplex2', 'optcomplex2', 0)]),
)
testData = testData + ((c, ((tests, kw_1),)),)

c = Base
kw_1 = {
    'hull_borderwidth' : 2,
    'hull_relief' : 'sunken',
    'hull_background' : 'red',
    'basesimple_widget_text' : 'base',
    'widget_foreground' : 'green',
    'initbase1' : 'initbase1_new',
}
tests = (
    (c.pack, ()),
    (c.components, (), ['basesimple', 'child', 'groupie', 'hull']),
    (c.componentaliases, (), [('widget', 'basesimple_widget'),]),
    (c.options, (), [('initbase1', 'initbase1', 1), ('initbase2', 'initbase2', 1), ('initbase3', 'initbase3', 1), ('optbase1', 'optbase1', 0), ('optbase2', 'optbase2', 0), ('optbase3', 'optbase3', 0)]),
    (c.cget, 'widget_foreground', 'green'),
    (c.cget, 'basesimple_widget_foreground', 'green'),
    (c.cget, 'basesimple_widgy_foreground', 'green'),
    ('widget_foreground', 'blue'),
    (c.cget, 'widget_foreground', 'blue'),
    (c.cget, 'basesimple_widget_foreground', 'blue'),
    (c.cget, 'basesimple_widgy_foreground', 'blue'),
    (c.cget, 'optbase1', 'optbase1'),
    (c.cget, 'groupie_text', 'XXXXX'),
    # When Test created the widget, it performed a test where it configured
    # each option. Hence, _optbase1() has been called twice:
    (c.getbasedummy, (), ['optbase1', 'optbase1']),
    ('optbase1', 'basedummy_new'),
    (c.getbasedummy, (), ['optbase1', 'optbase1', 'basedummy_new']),
)
testData = testData + ((c, ((tests, kw_1),)),)


c = Derived
kw_1 = {
    'hull_borderwidth' : 2,
    'hull_relief' : 'sunken',
    'hull_background' : 'red',
    'basesimple_widget_text' : 'base simple',
    'derivedcomplex_widget_text' : 'derived complex',
    'initderived1' : 'initderived1_new',
    'initbase1' : 'initbase1_new',
    'optbase3' : 'optbase3_new',
    'derivedcomplex_initcomplex1' : 'derivedcomplex_initcomplex1',
    'derivedsimple_initsimple1' : 'derivedsimple_initsimple1',
    'hull_cursor' : 'gumby',
    'Mygroup_borderwidth' : 2,
    'Mygroup_relief' : 'ridge',
}
tests = (
    (c.pack, ()),
    (c.components, (), ['basesimple', 'child', 'derivedcomplex', 'groupie', 'hull']),
    (c.componentaliases, (), [('derivedsimple', 'derivedcomplex_simple'), ('widget', 'basesimple_widget'),]),
    (c.options, (), [('initbase1', 'initbase1', 1), ('initbase2', 'initbase2inderived', 1), ('initbase3', 'initbase3', 1), ('initderived1', 'initderived1', 1), ('initderived2', 'initderived2', 1), ('optbase1', 'optbase1', 0), ('optbase2', 'optbase2', 0), ('optbase3', 'optbase3', 0), ('optderived1', 'optderived1', 0), ('optderived2', 'optderived2', 0), ]),
    (c.getbasedummy, (), []),
    (c.getderiveddummy, (), ['optbase1', 'optbase1']),
    ('optbase1', 'derivedbasedummy_new'),
    (c.getbasedummy, (), []),
    (c.getderiveddummy, (), ['optbase1', 'optbase1', 'derivedbasedummy_new']),
    (c.cget, 'optbase3', 'optbase3_new'),
    ('optbase3', 'optbase3_newer'),
    (c.cget, 'optbase3', 'optbase3_newer'),
    (c.cget, 'optderived1', 'optderived1'),
    (c.cget, 'initderived1', 'initderived1_new'),
    (c.cget, 'initbase2', 'initbase2inderived'),
    (c.cget, 'initbase1', 'initbase1_new'),
    (c.cget, 'initbase3', 'initbase3'),
    (c.cget, 'groupie_text', 'YYYYY'),
    ('groupie_text', 'ZZZZZ'),
    (c.cget, 'groupie_text', 'ZZZZZ'),
    (c.cget, 'derivedcomplex_optcomplex1', 'optcomplex1'),
    ('derivedcomplex_optcomplex1', 'optcomplex1_new'),
    (c.cget, 'derivedcomplex_optcomplex1', 'optcomplex1_new'),
    (c.cget, 'derivedsimple_optsimple2', 'optsimple2'),
    ('derivedsimple_optsimple2', 'optsimple2_new'),
    (c.cget, 'derivedcomplex_simple_optsimple2', 'optsimple2_new'),
    ('derivedcomplex_simple_optsimple2', 'optsimple2_newer'),
    (c.cget, 'derivedsimple_optsimple2', 'optsimple2_newer'),
    (c.cget, 'hull_cursor', 'gumby'),
    (c.cget, 'groupie_relief', 'ridge'),
    (c.cget, 'Mygroup_relief', 'ridge'),
    ('Mygroup_relief', 'sunken'),
    (c.cget, 'groupie_relief', 'sunken'),
    (c.cget, 'Mygroup_relief', 'sunken'),
    ('groupie_relief', 'groove'),
    (c.cget, 'groupie_relief', 'groove'),
)
testData = testData + ((c, ((tests, kw_1),)),)

if __name__ == '__main__':
    #Test.setverbose(1)
    Test.runTests(testData)
