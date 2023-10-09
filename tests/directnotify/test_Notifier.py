import io
import re
import time
import pytest
from panda3d import core
from direct.directnotify import Logger, Notifier

NOTIFIER_NAME = 'Test notifier'
DEBUG_LOG = 'Debug log'
INFO_LOG = 'Info log'
WARNING_LOG = 'Warning log'
ERROR_LOG = 'Error log'


@pytest.fixture
def log_io():
    return io.StringIO()


@pytest.fixture
def notifier(log_io):
    logger = Logger.Logger()
    logger.setTimeStamp(False)
    logger._Logger__logFile = log_io
    notifier = Notifier.Notifier(NOTIFIER_NAME, logger)
    notifier.setLogging(True)
    return notifier


def test_setServerDelta():
    notifier = Notifier.Notifier(NOTIFIER_NAME)
    notifier.setServerDelta(4.2, time.timezone)
    assert Notifier.Notifier.serverDelta == 4
    Notifier.Notifier.serverDelta = 0


def test_logging(notifier, log_io):
    notifier.setLogging(False)
    assert not notifier.getLogging()
    notifier.info(INFO_LOG)
    assert log_io.getvalue() == ''

    notifier.setLogging(True)
    assert notifier.getLogging()
    notifier.info(INFO_LOG)
    assert log_io.getvalue() == f':{NOTIFIER_NAME}: {INFO_LOG}\n'


@pytest.mark.parametrize('severity', (core.NS_debug, core.NS_info, core.NS_warning, core.NS_error))
def test_severity(severity, notifier, log_io):
    notifier.setSeverity(severity)
    assert notifier.getSeverity() == severity

    with pytest.raises(Notifier.NotifierException):
        notifier.error(ERROR_LOG)
    warning_return = notifier.warning(WARNING_LOG)
    info_return = notifier.info(INFO_LOG)
    debug_return = notifier.debug(DEBUG_LOG)
    assert warning_return and info_return and debug_return

    expected_logs = [
        f'{Notifier.NotifierException}: {NOTIFIER_NAME}(error): {ERROR_LOG}',
        f':{NOTIFIER_NAME}(warning): {WARNING_LOG}',
        f':{NOTIFIER_NAME}: {INFO_LOG}',
        f':{NOTIFIER_NAME}(debug): {DEBUG_LOG}',
    ]
    del expected_logs[6-severity:]
    assert log_io.getvalue() == '\n'.join(expected_logs) + '\n'


def test_custom_exception(notifier):
    class CustomException(Exception):
        pass

    with pytest.raises(CustomException):
        notifier.error(ERROR_LOG, CustomException)


def test_debugCall(notifier, log_io):
    notifier.setDebug(False)
    return_value = notifier.debugCall(DEBUG_LOG)
    assert return_value
    assert log_io.getvalue() == ''
    notifier.setDebug(True)
    notifier.debugCall(DEBUG_LOG)
    pattern = rf':\d\d:\d\d:\d\d:{NOTIFIER_NAME} "{DEBUG_LOG}" test_debugCall\(.*\)\n'
    assert re.match(pattern, log_io.getvalue())
