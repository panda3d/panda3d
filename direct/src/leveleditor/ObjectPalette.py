"""
This is just a sample code.

LevelEditor, ObjectHandler, ObjectPalette should be rewritten
to be game specific.

You can define object template class inheriting ObjectBase
to define properties shared by multiple object types.
When you are defining properties
you should specify their name, UI type, data type,
update function, default value, and value range.

Then you need implement ObjectPalette class inheriting ObjectPaletteBase,
and in the populate function you can define ObjectPalette tree structure.
"""

from .ObjectPaletteBase import *

class ObjectProp(ObjectBase):
    def __init__(self, *args, **kw):
        ObjectBase.__init__(self, *args, **kw)
        self.properties['Abc'] =[OG.PROP_UI_RADIO, # UI type
                                 OG.PROP_STR,      # data type
                                 None,             # update function
                                 'a',              # default value
                                 ['a', 'b', 'c']]  # value range


class ObjectSmiley(ObjectProp):
    def __init__(self, *args, **kw):
        ObjectProp.__init__(self, *args, **kw)
        self.properties['123'] = [OG.PROP_UI_COMBO,
                                  OG.PROP_INT,
                                  None,
                                  1,
                                  [1, 2, 3]]


class ObjectDoubleSmileys(ObjectProp):
    def __init__(self, *args, **kw):
        ObjectProp.__init__(self, *args, **kw)
        self.properties['Distance'] = [OG.PROP_UI_SLIDE,
                                                    OG.PROP_FLOAT,
                                                    ('.updateDoubleSmiley',
                                                     {'val':OG.ARG_VAL, 'obj':OG.ARG_OBJ}),
                                                    # In this case, an update function for property is defined
                                                    # so whenever you change the value of this property from UI
                                                    # this update function will be called with these arguments.
                                                    # OG.ARG_VAL will be replaced by the value from UI.
                                                    # OG.ARG_OBJ will be replaced by object data structure.
                                                    # When an update function is starting with .
                                                    # it means this function belongs to the default objectHandler.
                                                    1.0, [0, 10, 0.1]]


class ObjectPalette(ObjectPaletteBase):
    def __init__(self):
        ObjectPaletteBase.__init__(self)

    def populate(self):
        # Create a group called 'Prop' in the ObjectPalette tree
        self.add('Prop')

        # Create a group called 'Double Smileys' under 'Prop' group
        self.add('Double Smileys', 'Prop')

        # Add an object type 'Smiley' which is inheriting ObjectSmiley template
        # and have following properties.
        self.add(ObjectSmiley(name='Smiley',
                              model='models/smiley.egg',
                              models=['models/smiley.egg',
                                      'models/frowney.egg',
                                      'models/jack.egg'],
                              # when an object is just a simple geometry, you can define
                              # model, and models like this
                              # instead of defining createFunction
                              properties={'Happy':[OG.PROP_UI_CHECK,
                                                   OG.PROP_BOOL,
                                                   None,
                                                   True],
                                          'Number':[OG.PROP_UI_SPIN,
                                                    OG.PROP_INT,
                                                    ('.updateSmiley',
                                                     {'val':OG.ARG_VAL, 'obj':OG.ARG_OBJ}),
                                                    1, [1, 10]],
                                        }),
                 'Prop') # This object type will be added under the 'Prop' group.
        self.add(ObjectDoubleSmileys(name='H Double Smiley',
                                     createFunction = ('.createDoubleSmiley', {})),
                                     # When the createFunction is defined like this,
                                     # this function will be called to create the object.
                                     # When a create function is starting with .
                                     # it means this function belongs to the default objectHandler.
                 'Double Smileys')

        self.add(ObjectDoubleSmileys(name='V Double Smiley',
                                     createFunction = ('.createDoubleSmiley', {'horizontal':False})),
                                     # You can specify argument for the create function, too
                 'Double Smileys')

        self.add('Animal')
        self.add(ObjectBase(name='Panda',
                            createFunction = ('.createPanda', {}),
                            anims = ['models/panda-walk4.egg',],
                            properties = {}),
                 'Animal')

        self.add('BG')
        self.add(ObjectBase(name='Grass',
                            createFunction = ('.createGrass', {}),
                            properties = {}),
                 'BG')
