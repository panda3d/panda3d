from setuptools import setup
import pytest

setup(
    name="panda3d-tester",
    options = {
        'build_apps': {
            'gui_apps': {
                'tester': 'main.py',
            },
            'plugins': [
                'pandagl',
                'p3openal_audio',
            ],
            },
            'platforms': [
                'manylinux1_x86_64',
                'macosx_10_6_x86_64',
                'win32',
                'win_amd64',
            ],
        }
    }
)
