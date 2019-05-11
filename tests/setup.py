from setuptools import setup
import pytest

setup(
    name="panda3d-tester",
    options={
        'build_apps': {
            'gui_apps': {
                'tester': 'main.py',
            },
            'plugins': [
                'pandagl',
                'p3openal_audio',
            ],
        }
    }
)
