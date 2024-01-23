"""
Base class for all DirectGui items.  Handles composite widgets and
command line argument parsing.

Code overview:

1)  Each widget defines a set of options (optiondefs) as a list of tuples
    of the form ``('name', defaultValue, handler)``.
    'name' is the name of the option (used during construction of configure)
    handler can be: None, method, or INITOPT.  If a method is specified, it
    will be called during widget construction (via initialiseoptions), if the
    Handler is specified as an INITOPT, this is an option that can only be set
    during widget construction.

2)  :func:`~DirectGuiBase.defineoptions` is called.  defineoption creates:

    self._constructorKeywords = { keyword: [value, useFlag] }
        A dictionary of the keyword options specified as part of the
        constructor keywords can be of the form 'component_option', where
        component is the name of a widget's component, a component group or a
        component alias.

    self._dynamicGroups
        A list of group names for which it is permissible to specify options
        before components of that group are created.
        If a widget is a derived class the order of execution would be::

          foo.optiondefs = {}
          foo.defineoptions()
            fooParent()
               fooParent.optiondefs = {}
               fooParent.defineoptions()

3)  :func:`~DirectGuiBase.addoptions` is called.  This combines options
    specified as keywords to the widget constructor (stored in
    self._constructorKeywords) with the default options (stored in optiondefs).
    Results are stored in
    ``self._optionInfo = { keyword: [default, current, handler] }``.
    If a keyword is of the form 'component_option' it is left in the
    self._constructorKeywords dictionary (for use by component constructors),
    otherwise it is 'used', and deleted from self._constructorKeywords.

    Notes:

    - constructor keywords override the defaults.
    - derived class default values override parent class defaults
    - derived class handler functions override parent class functions

4)  Superclass initialization methods are called (resulting in nested calls
    to define options (see 2 above)

5)  Widget components are created via calls to
    :func:`~DirectGuiBase.createcomponent`.  User can specify aliases and groups
    for each component created.

    Aliases are alternate names for components, e.g. a widget may have a
    component with a name 'entryField', which itself may have a component
    named 'entry', you could add an alias 'entry' for the 'entryField_entry'
    These are stored in self.__componentAliases.  If an alias is found,
    all keyword entries which use that alias are expanded to their full
    form (to avoid conversion later)

    Groups allow option specifications that apply to all members of the group.
    If a widget has components: 'text1', 'text2', and 'text3' which all belong
    to the 'text' group, they can be all configured with keywords of the form:
    'text_keyword' (e.g. ``text_font='comic.rgb'``).  A component's group
    is stored as the fourth element of its entry in self.__componentInfo.

    Note: the widget constructors have access to all remaining keywords in
    _constructorKeywords (those not transferred to _optionInfo by
    define/addoptions).  If a component defines an alias that applies to
    one of the keywords, that keyword is replaced with a new keyword with
    the alias expanded.

    If a keyword (or substituted alias keyword) is used during creation of the
    component, it is deleted from self._constructorKeywords.  If a group
    keyword applies to the component, that keyword is marked as used, but is
    not deleted from self._constructorKeywords, in case it applies to another
    component.  If any constructor keywords remain at the end of component
    construction (and initialisation), an error is raised.

5)  :func:`~DirectGuiBase.initialiseoptions` is called.  This method calls any
    option handlers to respond to any keyword/default values, then checks to
    see if any keywords are left unused.  If so, an error is raised.
"""

__all__ = ['DirectGuiBase', 'DirectGuiWidget']


from panda3d.core import *
from direct.showbase import ShowBaseGlobal
from direct.showbase.ShowBase import ShowBase
from . import DirectGuiGlobals as DGG
from .OnscreenText import *
from .OnscreenGeom import *
from .OnscreenImage import *
from direct.directtools.DirectUtil import ROUND_TO
from direct.showbase import DirectObject
from direct.task import Task
import sys

if sys.version_info >= (3, 0):
    stringType = str
else:
    stringType = basestring

guiObjectCollector = PStatCollector("Client::GuiObjects")


class DirectGuiBase(DirectObject.DirectObject):
    """Base class of all DirectGUI widgets."""

    def __init__(self):
        # Default id of all gui object, subclasses should override this
        self.guiId = 'guiObject'
        # List of all post initialization functions
        self.postInitialiseFuncList = []
        # To avoid doing things redundantly during initialisation
        self.fInit = 1
        # Mapping from each megawidget option to a list of information
        # about the option
        #   - default value
        #   - current value
        #   - function to call when the option is initialised in the
        #     call to initialiseoptions() in the constructor or
        #     modified via configure().  If this is INITOPT, the
        #     option is an initialisation option (an option that can
        #     be set by the call to the constructor but can not be
        #     used with configure).
        # This mapping is not initialised here, but in the call to
        # defineoptions() which precedes construction of this base class.
        #
        # self._optionInfo = {}

        # Mapping from each component name to a tuple of information
        # about the component.
        #   - component widget instance
        #   - configure function of widget instance
        #   - the class of the widget (Frame, EntryField, etc)
        #   - cget function of widget instance
        #   - the name of the component group of this component, if any
        self.__componentInfo = {}

        # Mapping from alias names to the names of components or
        # sub-components.
        self.__componentAliases = {}

        # Contains information about the keywords provided to the
        # constructor.  It is a mapping from the keyword to a tuple
        # containing:
        #    - value of keyword
        #    - a boolean indicating if the keyword has been used.
        # A keyword is used if, during the construction of a megawidget,
        #    - it is defined in a call to defineoptions() or addoptions(), or
        #    - it references, by name, a component of the megawidget, or
        #    - it references, by group, at least one component
        # At the end of megawidget construction, a call is made to
        # initialiseoptions() which reports an error if there are
        # unused options given to the constructor.
        #
        # self._constructorKeywords = {}

        # List of dynamic component groups.  If a group is included in
        # this list, then it not an error if a keyword argument for
        # the group is given to the constructor or to configure(), but
        # no components with this group have been created.
        # self._dynamicGroups = ()

    def defineoptions(self, keywords, optionDefs, dynamicGroups = ()):
        """ defineoptions(keywords, optionDefs, dynamicGroups = {}) """
        # Create options, providing the default value and the method
        # to call when the value is changed.  If any option created by
        # base classes has the same name as one in <optionDefs>, the
        # base class's value and function will be overriden.

        # keywords is a dictionary of keyword/value pairs from the constructor
        # optionDefs is a dictionary of default options for the widget
        # dynamicGroups is a tuple of component groups for which you can
        # specify options even though no components of this group have
        # been created

        # This should be called before the constructor of the base
        # class, so that default values defined in the derived class
        # override those in the base class.
        if not hasattr(self, '_constructorKeywords'):
            tmp = {}
            for option, value in keywords.items():
                tmp[option] = [value, 0]
            self._constructorKeywords = tmp
            self._optionInfo = {}
        # Initialize dictionary of dynamic groups
        if not hasattr(self, '_dynamicGroups'):
            self._dynamicGroups = ()
        self._dynamicGroups = self._dynamicGroups + tuple(dynamicGroups)
        # Reconcile command line and default options
        self.addoptions(optionDefs, keywords)

    def addoptions(self, optionDefs, optionkeywords):
        """ addoptions(optionDefs) - add option def to option info """
        # Add additional options, providing the default value and the
        # method to call when the value is changed.  See
        # "defineoptions" for more details

        # optimisations:
        optionInfo = self._optionInfo
        optionInfo_has_key = optionInfo.__contains__
        keywords = self._constructorKeywords
        keywords_has_key = keywords.__contains__
        FUNCTION = DGG._OPT_FUNCTION

        for name, default, function in optionDefs:
            if '_' not in name:
                default = optionkeywords.get(name, default)
                # The option will already exist if it has been defined
                # in a derived class.  In this case, do not override the
                # default value of the option or the callback function
                # if it is not None.
                if not optionInfo_has_key(name):
                    if keywords_has_key(name):
                        # Overridden by keyword, use keyword value
                        value = keywords[name][0]
                        optionInfo[name] = [default, value, function]
                        # Delete it from self._constructorKeywords
                        del keywords[name]
                    else:
                        # Use optionDefs value
                        value = default
                        if isinstance(value, list):
                            value = list(value)
                        elif isinstance(value, dict):
                            value = dict(value)
                        optionInfo[name] = [default, value, function]
                elif optionInfo[name][FUNCTION] is None:
                    # Only override function if not defined by derived class
                    optionInfo[name][FUNCTION] = function
            else:
                # This option is of the form "component_option".  If this is
                # not already defined in self._constructorKeywords add it.
                # This allows a derived class to override the default value
                # of an option of a component of a base class.
                if not keywords_has_key(name):
                    keywords[name] = [default, 0]

    def initialiseoptions(self, myClass):
        """
        Call all initialisation functions to initialize widget
        options to default of keyword value
        """
        # This is to make sure this method class is only called by
        # the most specific class in the class hierarchy
        if self.__class__ is myClass:
            # Call the configuration callback function for every option.
            FUNCTION = DGG._OPT_FUNCTION
            self.fInit = 1
            for info in self._optionInfo.values():
                func = info[FUNCTION]
                if func is not None and func is not DGG.INITOPT:
                    func()
            self.fInit = 0

            # Now check if anything is left over
            unusedOptions = []
            keywords = self._constructorKeywords
            for name in keywords:
                used = keywords[name][1]
                if not used:
                    # This keyword argument has not been used.  If it
                    # does not refer to a dynamic group, mark it as
                    # unused.
                    index = name.find('_')
                    if index < 0 or name[:index] not in self._dynamicGroups:
                        unusedOptions.append(name)
            self._constructorKeywords = {}
            if len(unusedOptions) > 0:
                if len(unusedOptions) == 1:
                    text = 'Unknown option "'
                else:
                    text = 'Unknown options "'
                raise KeyError(text + ', '.join(unusedOptions) + \
                        '" for ' + myClass.__name__)
            # Can now call post init func
            self.postInitialiseFunc()

    def postInitialiseFunc(self):
        for func in self.postInitialiseFuncList:
            func()

    def isinitoption(self, option):
        """
        Is this opition one that can only be specified at construction?
        """
        return self._optionInfo[option][DGG._OPT_FUNCTION] is DGG.INITOPT

    def options(self):
        """
        Print out a list of available widget options.
        Does not include subcomponent options.
        """
        options = []
        if hasattr(self, '_optionInfo'):
            for option, info in self._optionInfo.items():
                isinit = info[DGG._OPT_FUNCTION] is DGG.INITOPT
                default = info[DGG._OPT_DEFAULT]
                options.append((option, default, isinit))
            options.sort()
        return options

    def configure(self, option=None, **kw):
        """
        configure(option = None)
        Query or configure the megawidget options.
        """
        #
        # If not empty, *kw* is a dictionary giving new
        # values for some of the options of this gui item
        # For options defined for this widget, set
        # the value of the option to the new value and call the
        # configuration callback function, if any.
        #
        # If *option* is None, return all gui item configuration
        # options and settings.  Options are returned as standard 3
        # element tuples
        #
        # If *option* is a string, return the 3 element tuple for the
        # given configuration option.

        # First, deal with the option queries.
        if len(kw) == 0:
            # This configure call is querying the values of one or all options.
            # Return 3-tuples:
            #     (optionName, default, value)
            if option is None:
                rtn = {}
                for option, config in self._optionInfo.items():
                    rtn[option] = (option,
                                   config[DGG._OPT_DEFAULT],
                                   config[DGG._OPT_VALUE])
                return rtn
            else:
                config = self._optionInfo[option]
                return (option, config[DGG._OPT_DEFAULT], config[DGG._OPT_VALUE])

        # optimizations:
        optionInfo = self._optionInfo
        optionInfo_has_key = optionInfo.__contains__
        componentInfo = self.__componentInfo
        componentInfo_has_key = componentInfo.__contains__
        componentAliases = self.__componentAliases
        componentAliases_has_key = componentAliases.__contains__
        VALUE = DGG._OPT_VALUE
        FUNCTION = DGG._OPT_FUNCTION

        # This will contain a list of options in *kw* which
        # are known to this gui item.
        directOptions = []

        # This will contain information about the options in
        # *kw* of the form <component>_<option>, where
        # <component> is a component of this megawidget.  It is a
        # dictionary whose keys are the configure method of each
        # component and whose values are a dictionary of options and
        # values for the component.
        indirectOptions = {}
        indirectOptions_has_key = indirectOptions.__contains__

        for option, value in kw.items():
            if optionInfo_has_key(option):
                # This is one of the options of this gui item.
                # Check it is an initialisation option.
                if optionInfo[option][FUNCTION] is DGG.INITOPT:
                    print('Cannot configure initialisation option "' \
                          + option + '" for ' + self.__class__.__name__)
                    break
                    #raise KeyError, \
                #           'Cannot configure initialisation option "' \
                #           + option + '" for ' + self.__class__.__name__
                optionInfo[option][VALUE] = value
                directOptions.append(option)
            else:
                index = option.find('_')
                if index >= 0:
                    # This option may be of the form <component>_<option>.
                    # e.g. if alias ('efEntry', 'entryField_entry')
                    # and option = efEntry_width
                    # component = efEntry, componentOption = width
                    component = option[:index]
                    componentOption = option[(index + 1):]

                    # Expand component alias
                    if componentAliases_has_key(component):
                        # component = entryField, subcomponent = entry
                        component, subComponent = componentAliases[component]
                        if subComponent is not None:
                            # componentOption becomes entry_width
                            componentOption = subComponent + '_' \
                                    + componentOption

                        # Expand option string to write on error
                        # option = entryField_entry_width
                        option = component + '_' + componentOption

                    # Does this component exist
                    if componentInfo_has_key(component):
                        # Get the configure func for the named component
                        # component = entryField
                        componentConfigFuncs = [componentInfo[component][1]]
                    else:
                        # Check if this is a group name and configure all
                        # components in the group.
                        componentConfigFuncs = []
                        # For each component
                        for info in componentInfo.values():
                            # Check if it is a member of this group
                            if info[4] == component:
                                # Yes, append its config func
                                componentConfigFuncs.append(info[1])

                        if len(componentConfigFuncs) == 0 and \
                                component not in self._dynamicGroups:
                            raise KeyError('Unknown option "' + option + \
                                    '" for ' + self.__class__.__name__)

                    # Add the configure method(s) (may be more than
                    # one if this is configuring a component group)
                    # and option/value to dictionary.
                    for componentConfigFunc in componentConfigFuncs:
                        if not indirectOptions_has_key(componentConfigFunc):
                            indirectOptions[componentConfigFunc] = {}
                        # Create a dictionary of keyword/values keyed
                        # on configuration function
                        indirectOptions[componentConfigFunc][componentOption] \
                                = value
                else:
                    raise KeyError('Unknown option "' + option + \
                            '" for ' + self.__class__.__name__)

        # Call the configure methods for any components.
        # Pass in the dictionary of keyword/values created above
        for func, options in indirectOptions.items():
            func(**options)

        # Call the configuration callback function for each option.
        for option in directOptions:
            info = optionInfo[option]
            func = info[DGG._OPT_FUNCTION]
            if func is not None:
              func()

    # Allow index style references
    def __setitem__(self, key, value):
        self.configure(**{key: value})

    def cget(self, option):
        """
        Get current configuration setting for this option
        """
        # Return the value of an option, for example myWidget['font'].
        if option in self._optionInfo:
            return self._optionInfo[option][DGG._OPT_VALUE]
        else:
            index = option.find('_')
            if index >= 0:
                component = option[:index]
                componentOption = option[(index + 1):]

                # Expand component alias
                if component in self.__componentAliases:
                    component, subComponent = self.__componentAliases[
                        component]
                    if subComponent is not None:
                        componentOption = subComponent + '_' + componentOption

                    # Expand option string to write on error
                    option = component + '_' + componentOption

                if component in self.__componentInfo:
                    # Call cget on the component.
                    componentCget = self.__componentInfo[component][3]
                    return componentCget(componentOption)
                else:
                    # If this is a group name, call cget for one of
                    # the components in the group.
                    for info in self.__componentInfo.values():
                        if info[4] == component:
                            componentCget = info[3]
                            return componentCget(componentOption)

        # Option not found
        raise KeyError('Unknown option "' + option + \
                '" for ' + self.__class__.__name__)

    # Allow index style refererences
    __getitem__ = cget

    def createcomponent(self, componentName, componentAliases, componentGroup,
                        widgetClass, *widgetArgs, **kw):
        """
        Create a component (during construction or later) for this widget.
        """
        # Check for invalid component name
        if '_' in componentName:
            raise ValueError('Component name "%s" must not contain "_"' % componentName)

        # Get construction keywords
        if hasattr(self, '_constructorKeywords'):
            keywords = self._constructorKeywords
        else:
            keywords = {}

        for alias, component in componentAliases:
            # Create aliases to the component and its sub-components.
            index = component.find('_')
            if index < 0:
                # Just a shorter name for one of this widget's components
                self.__componentAliases[alias] = (component, None)
            else:
                # An alias for a component of one of this widget's components
                mainComponent = component[:index]
                subComponent = component[(index + 1):]
                self.__componentAliases[alias] = (mainComponent, subComponent)

            # Remove aliases from the constructor keyword arguments by
            # replacing any keyword arguments that begin with *alias*
            # with corresponding keys beginning with *component*.
            alias = alias + '_'
            aliasLen = len(alias)
            for option in keywords.copy():
                if len(option) > aliasLen and option[:aliasLen] == alias:
                    newkey = component + '_' + option[aliasLen:]
                    keywords[newkey] = keywords[option]
                    del keywords[option]

        # Find any keyword arguments for this component
        componentPrefix = componentName + '_'
        nameLen = len(componentPrefix)

        # First, walk through the option list looking for arguments
        # than refer to this component's group.

        for option in keywords:
            # Check if this keyword argument refers to the group
            # of this component.  If so, add this to the options
            # to use when constructing the widget.  Mark the
            # keyword argument as being used, but do not remove it
            # since it may be required when creating another
            # component.
            index = option.find('_')
            if index >= 0 and componentGroup == option[:index]:
                rest = option[(index + 1):]
                kw[rest] = keywords[option][0]
                keywords[option][1] = 1

        # Now that we've got the group arguments, walk through the
        # option list again and get out the arguments that refer to
        # this component specifically by name.  These are more
        # specific than the group arguments, above; we walk through
        # the list afterwards so they will override.

        for option in keywords.copy():
            if len(option) > nameLen and option[:nameLen] == componentPrefix:
                # The keyword argument refers to this component, so add
                # this to the options to use when constructing the widget.
                kw[option[nameLen:]] = keywords[option][0]
                # And delete it from main construction keywords
                del keywords[option]

        # Return None if no widget class is specified
        if widgetClass is None:
            return None
        # Get arguments for widget constructor
        if len(widgetArgs) == 1 and type(widgetArgs[0]) == tuple:
            # Arguments to the constructor can be specified as either
            # multiple trailing arguments to createcomponent() or as a
            # single tuple argument.
            widgetArgs = widgetArgs[0]
        # Create the widget
        widget = widgetClass(*widgetArgs, **kw)
        componentClass = widget.__class__.__name__
        self.__componentInfo[componentName] = (widget, widget.configure,
                componentClass, widget.cget, componentGroup)
        return widget

    def component(self, name):
        # Return a component widget of the megawidget given the
        # component's name
        # This allows the user of a megawidget to access and configure
        # widget components directly.

        # Find the main component and any subcomponents
        index = name.find('_')
        if index < 0:
            component = name
            remainingComponents = None
        else:
            component = name[:index]
            remainingComponents = name[(index + 1):]

        # Expand component alias
        # Example entry which is an alias for entryField_entry
        if component in self.__componentAliases:
            # component = entryField, subComponent = entry
            component, subComponent = self.__componentAliases[component]
            if subComponent is not None:
                if remainingComponents is None:
                    # remainingComponents = entry
                    remainingComponents = subComponent
                else:
                    remainingComponents = subComponent + '_' \
                            + remainingComponents
        # Get the component from __componentInfo dictionary
        widget = self.__componentInfo[component][0]
        if remainingComponents is None:
            # Not looking for subcomponent
            return widget
        else:
            # Recursive call on subcomponent
            return widget.component(remainingComponents)

    def components(self):
        # Return a list of all components.
        names = list(self.__componentInfo.keys())
        names.sort()
        return names

    def hascomponent(self, component):
        return component in self.__componentInfo

    def destroycomponent(self, name):
        # Remove a megawidget component.
        # This command is for use by megawidget designers to destroy a
        # megawidget component.
        self.__componentInfo[name][0].destroy()
        del self.__componentInfo[name]

    def destroy(self):
        # Clean out any hooks
        self.ignoreAll()
        del self._optionInfo
        del self.__componentInfo
        del self.postInitialiseFuncList

    def bind(self, event, command, extraArgs = []):
        """
        Bind the command (which should expect one arg) to the specified
        event (such as ENTER, EXIT, B1PRESS, B1CLICK, etc.)
        See DirectGuiGlobals for possible events
        """
        # Need to tack on gui item specific id
        gEvent = event + self.guiId
        if ShowBaseGlobal.config.GetBool('debug-directgui-msgs', False):
            from direct.showbase.PythonUtil import StackTrace
            print(gEvent)
            print(StackTrace())
        self.accept(gEvent, command, extraArgs = extraArgs)

    def unbind(self, event):
        """
        Unbind the specified event
        """
        # Need to tack on gui item specific id
        gEvent = event + self.guiId
        self.ignore(gEvent)

def toggleGuiGridSnap():
    DirectGuiWidget.snapToGrid = 1 - DirectGuiWidget.snapToGrid

def setGuiGridSpacing(spacing):
    DirectGuiWidget.gridSpacing = spacing


class DirectGuiWidget(DirectGuiBase, NodePath):
    # Toggle if you wish widget's to snap to grid when draggin
    snapToGrid = 0
    gridSpacing = 0.05

    # Determine the default initial state for inactive (or
    # unclickable) components.  If we are in edit mode, these are
    # actually clickable by default.
    guiEdit = ShowBaseGlobal.config.GetBool('direct-gui-edit', False)
    if guiEdit:
        inactiveInitState = DGG.NORMAL
    else:
        inactiveInitState = DGG.DISABLED

    guiDict = {}

    def __init__(self, parent = None, **kw):
        # Direct gui widgets are node paths
        # Direct gui widgets have:
        # -  stateNodePaths (to hold visible representation of widget)
        # State node paths can have:
        # -  a frame of type (None, FLAT, RAISED, GROOVE, RIDGE)
        # -  arbitrary geometry for each state
        # They inherit from DirectGuiWidget
        # -  Can create components (with aliases and groups)
        # -  Can bind to mouse events
        # They inherit from NodePath
        # -  Can position/scale them
        optiondefs = (
            # Widget's constructor
            ('pgFunc',         PGItem,       None),
            ('numStates',      1,            None),
            ('invertedFrames', (),           None),
            ('sortOrder',      0,            None),
            # Widget's initial state
            ('state',          DGG.NORMAL,   self.setState),
            # Widget's frame characteristics
            ('relief',         DGG.FLAT,     self.setRelief),
            ('borderWidth',    (.1, .1),     self.setBorderWidth),
            ('borderUvWidth',  (.1, .1),     self.setBorderUvWidth),
            ('frameSize',      None,         self.setFrameSize),
            ('frameColor',     (.8, .8, .8, 1), self.setFrameColor),
            ('frameTexture',   None,         self.setFrameTexture),
            ('frameVisibleScale', (1, 1),     self.setFrameVisibleScale),
            ('pad',            (0, 0),        self.resetFrameSize),
            # Override button id (beware! your name may not be unique!)
            ('guiId',          None,         DGG.INITOPT),
            # Initial pos/scale of the widget
            ('pos',            None,         DGG.INITOPT),
            ('hpr',            None,         DGG.INITOPT),
            ('scale',          None,         DGG.INITOPT),
            ('color',          None,         DGG.INITOPT),
            # Do events pass through this widget?
            ('suppressMouse',  1,            DGG.INITOPT),
            ('suppressKeys',   0,            DGG.INITOPT),
            ('enableEdit',     1,            DGG.INITOPT),
            )
        # Merge keyword options with default options
        self.defineoptions(kw, optiondefs)

        # Initialize the base classes (after defining the options).
        DirectGuiBase.__init__(self)
        NodePath.__init__(self)
        # Create a button
        self.guiItem = self['pgFunc']('')
        # Override automatically generated guiId
        if self['guiId']:
            self.guiItem.setId(self['guiId'])
        self.guiId = self.guiItem.getId()

        if ShowBaseGlobal.__dev__:
            guiObjectCollector.addLevel(1)
            guiObjectCollector.flushLevel()
            # track gui items by guiId for tracking down leaks
            if ShowBaseGlobal.config.GetBool('track-gui-items', False):
                if not hasattr(ShowBase, 'guiItems'):
                    ShowBase.guiItems = {}
                if self.guiId in ShowBase.guiItems:
                    ShowBase.notify.warning('duplicate guiId: %s (%s stomping %s)' %
                                            (self.guiId, self,
                                             ShowBase.guiItems[self.guiId]))
                ShowBase.guiItems[self.guiId] = self

        # Attach button to parent and make that self
        if parent is None:
            parent = ShowBaseGlobal.aspect2d

        self.assign(parent.attachNewNode(self.guiItem, self['sortOrder']))
        # Update pose to initial values
        if self['pos']:
            self.setPos(self['pos'])
        if self['hpr']:
            self.setHpr(self['hpr'])
        if self['scale']:
            self.setScale(self['scale'])
        if self['color']:
            self.setColor(self['color'])
        # Initialize names
        # Putting the class name in helps with debugging.
        self.setName("%s-%s" % (self.__class__.__name__, self.guiId))
        # Create
        self.stateNodePath = []
        for i in range(self['numStates']):
            self.stateNodePath.append(NodePath(self.guiItem.getStateDef(i)))
        # Initialize frame style
        self.frameStyle = []
        for i in range(self['numStates']):
            self.frameStyle.append(PGFrameStyle())
        # For holding bounds info
        self.ll = Point3(0)
        self.ur = Point3(0)

        # Is drag and drop enabled?
        if self['enableEdit'] and self.guiEdit:
            self.enableEdit()

        # Set up event handling
        suppressFlags = 0
        if self['suppressMouse']:
            suppressFlags |= MouseWatcherRegion.SFMouseButton
            suppressFlags |= MouseWatcherRegion.SFMousePosition
        if self['suppressKeys']:
            suppressFlags |= MouseWatcherRegion.SFOtherButton
        self.guiItem.setSuppressFlags(suppressFlags)

        # Bind destroy hook
        self.guiDict[self.guiId] = self
        # self.bind(DGG.DESTROY, self.destroy)

        # Update frame when everything has been initialized
        self.postInitialiseFuncList.append(self.frameInitialiseFunc)

        # Call option initialization functions
        self.initialiseoptions(DirectGuiWidget)

    def frameInitialiseFunc(self):
        # Now allow changes to take effect
        self.updateFrameStyle()
        if not self['frameSize']:
            self.resetFrameSize()

    def enableEdit(self):
        self.bind(DGG.B2PRESS, self.editStart)
        self.bind(DGG.B2RELEASE, self.editStop)
        self.bind(DGG.PRINT, self.printConfig)
        # Can we move this to showbase
        # Certainly we don't need to do this for every button!
        #mb = base.mouseWatcherNode.getModifierButtons()
        #mb.addButton(KeyboardButton.control())
        #base.mouseWatcherNode.setModifierButtons(mb)

    def disableEdit(self):
        self.unbind(DGG.B2PRESS)
        self.unbind(DGG.B2RELEASE)
        self.unbind(DGG.PRINT)
        #mb = base.mouseWatcherNode.getModifierButtons()
        #mb.removeButton(KeyboardButton.control())
        #base.mouseWatcherNode.setModifierButtons(mb)

    def editStart(self, event):
        taskMgr.remove('guiEditTask')
        vWidget2render2d = self.getPos(render2d)
        vMouse2render2d = Point3(event.getMouse()[0], 0, event.getMouse()[1])
        editVec = Vec3(vWidget2render2d - vMouse2render2d)
        if base.mouseWatcherNode.getModifierButtons().isDown(
            KeyboardButton.control()):
            t = taskMgr.add(self.guiScaleTask, 'guiEditTask')
            t.refPos = vWidget2render2d
            t.editVecLen = editVec.length()
            t.initScale = self.getScale()
        else:
            t = taskMgr.add(self.guiDragTask, 'guiEditTask')
            t.editVec = editVec

    def guiScaleTask(self, state):
        mwn = base.mouseWatcherNode
        if mwn.hasMouse():
            vMouse2render2d = Point3(mwn.getMouse()[0], 0, mwn.getMouse()[1])
            newEditVecLen = Vec3(state.refPos - vMouse2render2d).length()
            self.setScale(state.initScale * (newEditVecLen/state.editVecLen))
        return Task.cont

    def guiDragTask(self, state):
        mwn = base.mouseWatcherNode
        if mwn.hasMouse():
            vMouse2render2d = Point3(mwn.getMouse()[0], 0, mwn.getMouse()[1])
            newPos = vMouse2render2d + state.editVec
            self.setPos(render2d, newPos)
            if DirectGuiWidget.snapToGrid:
                newPos = self.getPos()
                newPos.set(
                    ROUND_TO(newPos[0], DirectGuiWidget.gridSpacing),
                    ROUND_TO(newPos[1], DirectGuiWidget.gridSpacing),
                    ROUND_TO(newPos[2], DirectGuiWidget.gridSpacing))
                self.setPos(newPos)
        return Task.cont

    def editStop(self, event):
        taskMgr.remove('guiEditTask')

    def setState(self):
        if type(self['state']) == type(0):
            self.guiItem.setActive(self['state'])
        elif (self['state'] == DGG.NORMAL) or (self['state'] == 'normal'):
            self.guiItem.setActive(1)
        else:
            self.guiItem.setActive(0)

    def resetFrameSize(self):
        if not self.fInit:
            self.setFrameSize(fClearFrame = 1)

    def setFrameSize(self, fClearFrame = 0):
        # Use ready state to determine frame Type
        frameType = self.getFrameType()
        if self['frameSize']:
            # Use user specified bounds
            self.bounds = self['frameSize']
            #print "%s bounds = %s" % (self.getName(), self.bounds)
            bw = (0, 0)

        else:
            if fClearFrame and (frameType != PGFrameStyle.TNone):
                self.frameStyle[0].setType(PGFrameStyle.TNone)
                self.guiItem.setFrameStyle(0, self.frameStyle[0])
                # To force an update of the button
                self.guiItem.getStateDef(0)
            # Clear out frame before computing bounds
            self.getBounds()
            # Restore frame style if necessary
            if (frameType != PGFrameStyle.TNone):
                self.frameStyle[0].setType(frameType)
                self.guiItem.setFrameStyle(0, self.frameStyle[0])

            if ((frameType != PGFrameStyle.TNone) and
                (frameType != PGFrameStyle.TFlat)):
                bw = self['borderWidth']
            else:
                bw = (0, 0)

        # Set frame to new dimensions
        self.guiItem.setFrame(
            self.bounds[0] - bw[0],
            self.bounds[1] + bw[0],
            self.bounds[2] - bw[1],
            self.bounds[3] + bw[1])


    def getBounds(self, state = 0):
        self.stateNodePath[state].calcTightBounds(self.ll, self.ur)
        # Scale bounds to give a pad around graphics
        vec_right = Vec3.right()
        vec_up = Vec3.up()
        left = (vec_right[0] * self.ll[0]
              + vec_right[1] * self.ll[1]
              + vec_right[2] * self.ll[2])
        right = (vec_right[0] * self.ur[0]
               + vec_right[1] * self.ur[1]
               + vec_right[2] * self.ur[2])
        bottom = (vec_up[0] * self.ll[0]
                + vec_up[1] * self.ll[1]
                + vec_up[2] * self.ll[2])
        top = (vec_up[0] * self.ur[0]
             + vec_up[1] * self.ur[1]
             + vec_up[2] * self.ur[2])
        self.ll = Point3(left, 0.0, bottom)
        self.ur = Point3(right, 0.0, top)
        self.bounds = [self.ll[0] - self['pad'][0],
                       self.ur[0] + self['pad'][0],
                       self.ll[2] - self['pad'][1],
                       self.ur[2] + self['pad'][1]]
        return self.bounds

    def getWidth(self):
        return self.bounds[1] - self.bounds[0]

    def getHeight(self):
        return self.bounds[3] - self.bounds[2]

    def getCenter(self):
        x = self.bounds[0] + (self.bounds[1] - self.bounds[0])/2.0
        y = self.bounds[2] + (self.bounds[3] - self.bounds[2])/2.0
        return (x, y)

    def getFrameType(self, state = 0):
        return self.frameStyle[state].getType()

    def updateFrameStyle(self):
        if not self.fInit:
            for i in range(self['numStates']):
                self.guiItem.setFrameStyle(i, self.frameStyle[i])

    def setRelief(self, fSetStyle = 1):
        relief = self['relief']
        # Convert None, and string arguments
        if relief == None:
            relief = PGFrameStyle.TNone
        elif isinstance(relief, stringType):
            # Convert string to frame style int
            relief = DGG.FrameStyleDict[relief]
        # Set style
        if relief == DGG.RAISED:
            for i in range(self['numStates']):
                if i in self['invertedFrames']:
                    self.frameStyle[1].setType(DGG.SUNKEN)
                else:
                    self.frameStyle[i].setType(DGG.RAISED)
        elif relief == DGG.SUNKEN:
            for i in range(self['numStates']):
                if i in self['invertedFrames']:
                    self.frameStyle[1].setType(DGG.RAISED)
                else:
                    self.frameStyle[i].setType(DGG.SUNKEN)
        else:
            for i in range(self['numStates']):
                self.frameStyle[i].setType(relief)
        # Apply styles
        self.updateFrameStyle()

    def setFrameColor(self):
        # this might be a single color or a list of colors
        colors = self['frameColor']
        if type(colors[0]) == int or \
           type(colors[0]) == float:
            colors = (colors,)
        for i in range(self['numStates']):
            if i >= len(colors):
                color = colors[-1]
            else:
                color = colors[i]
            self.frameStyle[i].setColor(color[0], color[1], color[2], color[3])
        self.updateFrameStyle()

    def setFrameTexture(self):
        # this might be a single texture or a list of textures
        textures = self['frameTexture']
        if textures == None or \
           isinstance(textures, Texture) or \
           isinstance(textures, stringType):
            textures = (textures,) * self['numStates']
        for i in range(self['numStates']):
            if i >= len(textures):
                texture = textures[-1]
            else:
                texture = textures[i]
            if isinstance(texture, stringType):
                texture = loader.loadTexture(texture)
            if texture:
                self.frameStyle[i].setTexture(texture)
            else:
                self.frameStyle[i].clearTexture()
        self.updateFrameStyle()

    def setFrameVisibleScale(self):
        scale = self['frameVisibleScale']
        for i in range(self['numStates']):
            self.frameStyle[i].setVisibleScale(scale[0], scale[1])
        self.updateFrameStyle()

    def setBorderWidth(self):
        width = self['borderWidth']
        for i in range(self['numStates']):
            self.frameStyle[i].setWidth(width[0], width[1])
        self.updateFrameStyle()

    def setBorderUvWidth(self):
        uvWidth = self['borderUvWidth']
        for i in range(self['numStates']):
            self.frameStyle[i].setUvWidth(uvWidth[0], uvWidth[1])
        self.updateFrameStyle()

    def destroy(self):
        if hasattr(self, "frameStyle"):
            if ShowBaseGlobal.__dev__:
                guiObjectCollector.subLevel(1)
                guiObjectCollector.flushLevel()
                if hasattr(ShowBase, 'guiItems'):
                    ShowBase.guiItems.pop(self.guiId, None)

            # Destroy children
            for child in self.getChildren():
                childGui = self.guiDict.get(child.getName())
                if childGui:
                    childGui.destroy()
                else:
                    # RAU since we added the class to the name, try
                    # it with the original name
                    parts = child.getName().split('-')
                    simpleChildGui = self.guiDict.get(parts[-1])
                    if simpleChildGui:
                        simpleChildGui.destroy()
                # messenger.send(DESTROY + child.getName())
            del self.guiDict[self.guiId]
            del self.frameStyle
            # Get rid of node path
            self.removeNode()
            for nodePath in self.stateNodePath:
                nodePath.removeNode()
            del self.stateNodePath
            del self.guiItem
            # Call superclass destruction method (clears out hooks)
            DirectGuiBase.destroy(self)

    def printConfig(self, indent = 0):
        space = ' ' * indent
        print('%s%s - %s' % (space, self.guiId, self.__class__.__name__))
        print('%sPos:   %s' % (space, tuple(self.getPos())))
        print('%sScale: %s' % (space, tuple(self.getScale())))
        # Print out children info
        for child in self.getChildren():
            messenger.send(DGG.PRINT + child.getName(), [indent + 2])

    def copyOptions(self, other):
        """
        Copy other's options into our self so we look and feel like other
        """
        for key, value in other._optionInfo.items():
            self[key] = value[1]

    def taskName(self, idString):
        return (idString + "-" + str(self.guiId))

    def uniqueName(self, idString):
        return (idString + "-" + str(self.guiId))

    def setProp(self, propString, value):
        """
        Allows you to set a property like frame['text'] = 'Joe' in
        a function instead of an assignment.
        This is useful for setting properties inside function intervals
        where must input a function and extraArgs, not an assignment.
        """
        self[propString] = value
