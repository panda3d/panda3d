from direct.gui.DirectOptionMenu import DirectOptionMenu
import pytest


def test_menu_destroy():
    menu = DirectOptionMenu(items=["item1", "item2"])
    menu.destroy()


def test_showPopupMenu():
    menu = DirectOptionMenu()

    # Showing an option menu without items will raise an exception
    with pytest.raises(Exception):
        menu.showPopupMenu()

    menu["items"] = ["item1", "item2"]
    menu.showPopupMenu()
    assert not menu.popupMenu.isHidden()
    assert not menu.cancelFrame.isHidden()

    menu.hidePopupMenu()
    assert menu.popupMenu.isHidden()
    assert menu.cancelFrame.isHidden()


def test_index():
    menu = DirectOptionMenu(items=["item1", "item2"])
    assert menu.index("item1") == 0
    assert menu.index("item2") == 1


def test_set_get():
    menu = DirectOptionMenu(items=["item1", "item2"])
    menu.set(1, False)
    assert menu.selectedIndex == 1
    assert menu.get() == "item2"
    assert menu["text"] == "item2"


def test_initialitem():
    # initialitem by string
    menuByStr = DirectOptionMenu(items=["item1", "item2"], initialitem="item2")
    assert menuByStr.get() == "item2"
    assert menuByStr["text"] == "item2"

    # initialitem by Index
    menuByIdx = DirectOptionMenu(items=["item1", "item2"], initialitem=1)
    assert menuByIdx.get() == "item2"
    assert menuByIdx["text"] == "item2"


def test_item_text_scale():
    highlightScale = (2, 2)
    unhighlightScale = (0.5, 0.5)
    menu = DirectOptionMenu(
        items=["item1", "item2"],
        item_text_scale=unhighlightScale,
        highlightScale=highlightScale)

    # initial scale
    item = menu.component("item0")

    item_text_scale = 0.8
    assert item["text_scale"] == unhighlightScale

    # highlight scale
    menu._highlightItem(item, 0)
    assert item["text_scale"] == highlightScale

    # back to initial scale
    menu._unhighlightItem(item, item["frameColor"])
    assert item["text_scale"] == unhighlightScale

def test_static_and_dynamic_text():

    # Setup

    itemListList = [
        ["mew", "meow"],
        ["cat", "kitten"]
                    ]

    staticText = "No kitties here!"

    menuNormal = DirectOptionMenu(items = itemListList[0])
    menuStatic = DirectOptionMenu(items = itemListList[0],
                                  text = staticText)

    # Check that the normal menu works as expected,
    # even when changing selection or item-list

    assert menuNormal["text"] == itemListList[0][0]

    menuNormal.set(1)

    assert menuNormal["text"] == itemListList[0][1]

    menuNormal["items"] = itemListList[1]
    menuNormal.set(0)

    assert menuNormal["text"] == itemListList[1][0]

    menuNormal.set(1)

    assert menuNormal["text"] == itemListList[1][1]

    # Check that the static menu doesn't change its text,
    # regardless of item -list or -selection

    assert menuStatic["text"] == staticText

    menuStatic.set(1)

    assert menuStatic["text"] == staticText

    menuStatic["items"] = itemListList[1]
    menuStatic.set(0)

    assert menuStatic["text"] == staticText

    menuStatic.set(1)

    assert menuStatic["text"] == staticText