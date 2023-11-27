import pathlib
import shutil
import subprocess
import sys
import tempfile


def main():
    root = pathlib.Path(__file__).parent.parent
    direct_src = root / 'direct' / 'src'
    mypy_config = root / 'mypy.ini'
    with tempfile.TemporaryDirectory() as temp_dir:
        direct_copy = pathlib.Path(temp_dir, 'direct')
        shutil.copytree(direct_src, direct_copy)
        command = [
            sys.executable,
            '-m',
            'mypy',
            str(direct_copy.resolve()),
            '--config-file',
            str(mypy_config.resolve()),
        ]
        result = subprocess.run(command, cwd=temp_dir)
    sys.exit(result.returncode)


if __name__ == '__main__':
    main()
