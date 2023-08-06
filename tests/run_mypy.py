import os
import pathlib
import shutil
import subprocess
import tempfile


def main():
    root = pathlib.Path(__file__).parent.parent
    direct_src = root / 'direct' / 'src'
    mypy_config = root / 'mypy.ini'
    with tempfile.TemporaryDirectory() as temp_dir:
        os.environ['MYPYPATH'] = temp_dir
        direct_copy = pathlib.Path(temp_dir, 'direct')
        shutil.copytree(direct_src, direct_copy)
        subprocess.run([
            'mypy',
            str(direct_copy),
            '--config-file',
            str(mypy_config),
        ])


if __name__ == '__main__':
    main()
