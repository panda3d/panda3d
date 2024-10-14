import pytest
from direct.directnotify import RotatingLog

LOG_TEXT = 'Arbitrary log text'


@pytest.fixture
def log_dir(tmp_path):
    log_dir = tmp_path / 'logs'
    log_dir.mkdir()
    return log_dir


@pytest.fixture
def rotating_log(log_dir):
    log_filename = str(log_dir / 'log')
    rotating_log = RotatingLog.RotatingLog(log_filename)
    yield rotating_log
    rotating_log.close()


def test_rotation(rotating_log, log_dir):
    rotating_log.sizeLimit = -1
    rotating_log.write('1')
    rotating_log.write('2')
    written = [f.read_text() for f in log_dir.iterdir()]
    assert written == ['1', '2'] or written == ['2', '1']


def test_wrapper_methods(rotating_log, log_dir):
    rotating_log.write('')
    log_file, = log_dir.iterdir()

    assert rotating_log.fileno() == rotating_log.file.fileno()
    assert rotating_log.isatty() == rotating_log.file.isatty()

    rotating_log.writelines([LOG_TEXT] * 10)
    assert not log_file.read_text()
    rotating_log.flush()
    assert log_file.read_text() == LOG_TEXT * 10

    assert rotating_log.tell() == len(LOG_TEXT) * 10
    rotating_log.seek(len(LOG_TEXT))
    rotating_log.truncate(None)
    assert log_file.read_text() == LOG_TEXT
