import _pytest
if not hasattr(_pytest, '__file__'):
    import os
    import _pytest._pluggy
    import py

    _pytest.__file__ = os.getcwd()
    _pytest._pluggy.__file__ = os.getcwd()
    py.__file__ = os.getcwd()

import pytest

pytest.main()
