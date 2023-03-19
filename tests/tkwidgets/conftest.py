import tkinter as tk
import pytest


@pytest.fixture
def tk_toplevel():
    try:
        root = tk.Toplevel()
    except tk.TclError as e:
        pytest.skip(str(e))
    yield root
    root.destroy()
