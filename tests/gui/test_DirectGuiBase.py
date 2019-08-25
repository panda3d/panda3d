from direct.gui.DirectGuiBase import DirectGuiWidget
from direct.showbase.ShowBase import ShowBase
from direct.showbase import ShowBaseGlobal
from panda3d import core
import pytest


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
