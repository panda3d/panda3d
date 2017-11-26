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
            'include_modules': {
                '*': [
                    'pkg_resources.*.*',
                    # TODO why does the above not get these modules too?
                    'pkg_resources._vendor.packaging.*',
                    'pkg_resources._vendor.packaging.version',
                    'pkg_resources._vendor.packaging.specifiers',
                    'pkg_resources._vendor.packaging.requirements',
                ] + pytest.freeze_includes(),
            },
            'deploy_platforms': [
                'manylinux1_x86_64',
                'macosx_10_6_x86_64',
                'win32',
                'win_amd64',
            ],
        }
    }
)
