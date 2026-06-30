from direct.showbase.ShowBase import ShowBase
from direct.showbase import ShowBaseGlobal
from direct.gui.DirectSpinBox import DirectSpinBox
from direct.gui import DirectGuiGlobals as DGG
import pytest

from direct.showbase.Loader import Loader
import sys
if sys.version_info >= (3, 0):
    import builtins
else:
    import __builtin__ as builtins
builtins.loader = Loader(base=None)

from direct.showbase.MessengerGlobal import messenger

def test_set_value():
    spinner = DirectSpinBox(value=5)
    assert spinner.getValue() == 5

    # Assigning a non nummeric value shouldn't crash but set it to 0
    spinner = DirectSpinBox(value="A")
    assert spinner.getValue() == 0

    spinner.setValue(1)
    assert spinner.getValue() == 1

    spinner.setValue("2")
    assert spinner.getValue() == 2

def test_step():
    spinner = DirectSpinBox(value=1)

    spinner.doStep(1)
    assert spinner.getValue() == 2

    spinner.doStep(-1)
    assert spinner.getValue() == 1

def test_step_size():
    spinner = DirectSpinBox(stepSize=2)
    messenger.send(DGG.B1PRESS + spinner.incButton.guiId, [None])
    messenger.send(DGG.B1RELEASE + spinner.incButton.guiId, [None])
    assert spinner.getValue() == 2

    messenger.send(DGG.B1PRESS + spinner.decButton.guiId, [None])
    messenger.send(DGG.B1RELEASE + spinner.decButton.guiId, [None])
    assert spinner.getValue() == 0

def test_format():
    value = 2.8
    textFormat = '{:0.2f}'
    valueType = float

    spinner = DirectSpinBox(value=value, textFormat=textFormat, valueType=valueType)
    assert spinner.get() == textFormat.format(value)

def test_max_value():
    spinner = DirectSpinBox(maxValue=200)
    spinner.setValue(300)
    assert spinner.getValue() == 200

def test_min_value():
    spinner = DirectSpinBox(minValue = 5)
    spinner.setValue(3)
    assert spinner.getValue() == 5

def test_get_value():
    spinner = DirectSpinBox(value=10)
    assert spinner.getValue() == 10
    assert spinner.get() == "10"

def test_callbacks():
    global test_value_a
    test_value_a = False
    global test_value_b
    test_value_b= False

    def func_a():
        global test_value_a
        test_value_a = True
    def func_b():
        global test_value_b
        test_value_b = True

    spinner = DirectSpinBox(incButtonCallback=func_a, decButtonCallback=func_b)

    # The function should be called now. Note that the guiId will be
    # automatically appended by the bind command!
    messenger.send(DGG.B1PRESS + spinner.incButton.guiId, [None])
    messenger.send(DGG.B1RELEASE + spinner.incButton.guiId, [None])

    messenger.send(DGG.B1PRESS + spinner.decButton.guiId, [None])
    messenger.send(DGG.B1RELEASE + spinner.decButton.guiId, [None])

    assert test_value_a
    assert test_value_b

def test_click():
    global test_value_a
    test_value_a = False

    def func_a(text, extraArg):
        global test_value_a
        test_value_a = extraArg

    spinner = DirectSpinBox(command=func_a, extraArgs=[True])

    messenger.send(DGG.ACCEPT + spinner.valueEntry.guiId, [None])

    assert test_value_a

def test_suppress():
    spinner = DirectSpinBox(suppressKeys=True, suppressMouse=True)

    assert spinner.valueEntry["suppressKeys"]
    assert spinner.valueEntry["suppressMouse"]
    assert spinner.incButton["suppressKeys"]
    assert spinner.incButton["suppressMouse"]
    assert spinner.decButton["suppressKeys"]
    assert spinner.decButton["suppressMouse"]

#pytest.main()
