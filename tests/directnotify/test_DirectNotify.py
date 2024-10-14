import pytest
from panda3d import core
from direct.directnotify import DirectNotify, Logger, Notifier

CATEGORY_NAME = 'test'


@pytest.fixture
def notify():
    notify = DirectNotify.DirectNotify()
    notify.newCategory(CATEGORY_NAME)
    return notify


def test_categories():
    notify = DirectNotify.DirectNotify()
    assert len(notify.getCategories()) == 0
    assert notify.getCategory(CATEGORY_NAME) is None
    notifier = notify.newCategory(CATEGORY_NAME, logger=Logger.Logger())
    assert isinstance(notifier, Notifier.Notifier)
    assert notify.getCategories() == [CATEGORY_NAME]


def test_setDconfigLevels(notify):
    config = core.ConfigVariableString('notify-level-' + CATEGORY_NAME, '')
    notifier = notify.getCategory(CATEGORY_NAME)
    config.value = 'error'
    notify.setDconfigLevels()
    assert notifier.getSeverity() == core.NS_error
    config.value = 'warning'
    notify.setDconfigLevels()
    assert notifier.getSeverity() == core.NS_warning
    config.value = 'info'
    notify.setDconfigLevels()
    assert notifier.getSeverity() == core.NS_info
    config.value = 'debug'
    notify.setDconfigLevels()
    assert notifier.getSeverity() == core.NS_debug


def test_setVerbose(notify):
    notifier = notify.getCategory(CATEGORY_NAME)
    notifier.setWarning(False)
    notifier.setInfo(False)
    notifier.setDebug(False)
    notify.setVerbose()
    assert notifier.getWarning()
    assert notifier.getInfo()
    assert notifier.getDebug()


def test_giveNotify(notify):
    class HasNotify:
        notify = None

    notify.giveNotify(HasNotify)
    assert isinstance(HasNotify.notify, Notifier.Notifier)
