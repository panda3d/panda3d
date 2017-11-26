from setuptools import setup

setup(
    name="asteroids",
    options = {
        'build_apps': {
            'include_patterns': [
                '*.png',
                '*.jpg',
                '*.egg',
            ],
            'gui_apps': {
                'asteroids': 'main.py',
            },
            'plugins': [
                'pandagl',
                'p3openal_audio',
            ],
            'platforms': [
                'manylinux1_x86_64',
                'macosx_10_6_x86_64',
                'win32',
                'win_amd64',
            ],
        }
    }
)
