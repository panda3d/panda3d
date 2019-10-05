from direct.gui.DirectGuiBase import DirectGuiBase, DirectGuiWidget, toggleGuiGridSnap, setGuiGridSpacing
from direct.gui.OnscreenText import OnscreenText
from direct.gui import DirectGuiGlobals as DGG
from direct.showbase.ShowBase import ShowBase
from direct.showbase import ShowBaseGlobal
from direct.showbase.MessengerGlobal import messenger
from panda3d import core
import pytest


def test_create_DirectGuiBase():
    baseitem = DirectGuiBase()

def test_defineoptions():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test',0,None),)
    baseitem.defineoptions({}, testoptiondefs)
    assert baseitem['test'] == 0

def test_addoptions():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test',0,None),)

    # Those two will usually be set in the defineoptions function
    baseitem._optionInfo = {}
    baseitem._constructorKeywords = {}

    baseitem.addoptions(testoptiondefs, {})
    assert baseitem['test'] == 0

def test_initialiseoptions():
    baseitem = DirectGuiBase()

    class testWidget(DirectGuiBase):
        def __init__(self):
            pass

    tw = testWidget()
    baseitem.initialiseoptions(tw)

    testoptiondefs = (('test',0,None),)
    tw.defineoptions({}, testoptiondefs)
    baseitem.initialiseoptions(tw)

    assert tw['test'] == 0

def test_postInitialiseFunc():
    baseitem = DirectGuiBase()

    def func_a():
        global test_value_a
        test_value_a = 1
    def func_b():
        global test_value_b
        test_value_b = 1

    baseitem.postInitialiseFuncList.append(func_a)
    baseitem.postInitialiseFuncList.append(func_b)

    baseitem.postInitialiseFunc()

    global test_value_a
    assert test_value_a == 1

    global test_value_b
    assert test_value_b == 1

def test_isinitoption():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test-true',0,DGG.INITOPT),('test-false',0,None))
    baseitem.defineoptions({}, testoptiondefs)
    assert baseitem.isinitoption('test-true') == True
    assert baseitem.isinitoption('test-false') == False

def test_options():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test-1',0,DGG.INITOPT),('test-2',0,None),)
    baseitem.defineoptions({}, testoptiondefs)
    assert len(baseitem.options()) == 2

def test_get_options():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test-1',0,None),)
    baseitem.defineoptions({}, testoptiondefs)

    # Using the configure function
    # check if configuration have been set correctly
    assert baseitem.configure() == {'test-1': ('test-1', 0, 0),}
    # check for specific configurations
    assert baseitem.configure('test-1') == ('test-1', 0, 0)

    # using index style references __getItem__ and cget
    assert baseitem['test-1'] == 0
    assert baseitem.cget('test-1') == 0

def test_set_options():
    baseitem = DirectGuiBase()

    testoptiondefs = (('test-1',0,DGG.INITOPT),('test-2',0,None))
    baseitem.defineoptions({}, testoptiondefs)

    # try to set a value of an initopt option (shouldn't be possible)
    baseitem.configure('test-1', **{'test-1': 1})
    # check it's value. It shouldn't have changed
    assert baseitem['test-1'] == 0

    baseitem['test-1'] = 1
    # check it's value. It shouldn't have changed
    assert baseitem['test-1'] == 0

    # try changing using the configure function. As test-2 isn't an
    # initopt option, this should work fine
    baseitem.configure('test-2', **{'test-2': 2})
    # check it's value. It should be 2 now
    assert baseitem['test-2'] == 2

    # try changing using index style referencing
    baseitem['test-2'] = 1
    assert baseitem['test-2'] == 1

def test_component_handling():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test-1',0,None),)
    baseitem.defineoptions({}, testoptiondefs)

    assert len(baseitem.components()) == 0

    baseitem.createcomponent(
        'componentName',    # componentName
        (),                 # componentAliases
        'componentGroup',   # componentGroup
        OnscreenText,       # widgetClass
        (),                 # *widgetArgs
        text = 'Test',      # **kw
        parent = core.NodePath()
        )

    # check if we have exactly one component now
    assert len(baseitem.components()) == 1
    # check if we have a component named like the one we just created
    assert baseitem.hascomponent('componentName')

    # get our new component
    component = baseitem.component('componentName')
    # check some of its values
    assert component.text == 'Test'

    # destroy our recently created component
    baseitem.destroycomponent('componentName')
    # check if it really is gone
    assert baseitem.hascomponent('componentName') == False

def test_destroy():
    baseitem = DirectGuiBase()
    testoptiondefs = (('test-1',0,None),)
    baseitem.defineoptions({}, testoptiondefs)
    baseitem.destroy()
    # check if things got correctly removed
    assert not hasattr(baseitem, '_optionInfo')
    assert not hasattr(baseitem, '__componentInfo')
    assert not hasattr(baseitem, 'postInitialiseFuncList')

def test_bindings():
    baseitem = DirectGuiBase()

    # Our test function and variable to check
    global commandCalled
    commandCalled = False
    def command(**kw):
        global commandCalled
        commandCalled = True
        assert True

    # Bind an event to our item
    baseitem.bind(DGG.B1CLICK, command)

    # The function should be called now. Note that the guiId will be
    # automatically appended by the bind command!
    messenger.send(DGG.B1CLICK + baseitem.guiId)
    assert commandCalled

    # Try to unbind the event again
    baseitem.unbind(DGG.B1CLICK)

    # Now it shouldn't be called anymore
    commandCalled = False
    messenger.send(DGG.B1CLICK + baseitem.guiId)
    assert not commandCalled

def test_toggle_snap():
    try:
        DirectGuiWidget.snapToGrid = 0
        item = DirectGuiWidget()
        assert item.snapToGrid == 0
        toggleGuiGridSnap()
        assert item.snapToGrid == 1
    finally:
        DirectGuiWidget.snapToGrid = 0

def test_toggle_spacing():
    try:
        DirectGuiWidget.gridSpacing = 0
        item = DirectGuiWidget()
        setGuiGridSpacing(5)
        assert item.gridSpacing == 5
    finally:
        DirectGuiWidget.gridSpacing = 0.05

@pytest.mark.skipif(not ShowBaseGlobal.__dev__, reason="requires want-dev")
def test_track_gui_items():
    page = core.load_prc_file_data("", "track-gui-items true")
    try:
        item = DirectGuiWidget()
        id = item.guiId

        assert id in ShowBase.guiItems
        assert ShowBase.guiItems[id] == item

        item.destroy()

        assert id not in ShowBase.guiItems
    finally:
        core.unload_prc_file(page)
