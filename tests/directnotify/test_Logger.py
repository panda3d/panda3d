import re
from direct.directnotify import Logger

LOG_TEXT = 'Arbitrary log text'


def test_logging(tmp_path):
    log_filename = str(tmp_path / 'log')
    logger = Logger.Logger(log_filename)

    assert logger.getTimeStamp()
    logger.log(LOG_TEXT)
    logger.setTimeStamp(False)
    assert not logger.getTimeStamp()
    logger.log(LOG_TEXT)
    logger._Logger__closeLogFile()

    log_file, = tmp_path.iterdir()
    log_text = log_file.read_text()
    pattern = rf'\d\d:\d\d:\d\d:\d\d: {LOG_TEXT}\n{LOG_TEXT}\n'
    assert re.match(pattern, log_text)
