import os
import subprocess
import sys

SAMPLES_TO_BUILD = [
    'asteroids',
]
SAMPLES_DIR = os.path.join(os.path.dirname(__file__), '..', 'samples')

def main():
    for sample in SAMPLES_TO_BUILD:
        sampledir = os.path.join(SAMPLES_DIR, sample)
        os.chdir(sampledir)

        # This will raise a CalledProcessError if the build fails, which will cause
        # this script to fail
        subprocess.check_call([sys.executable, 'setup.py', 'bdist_apps'])

if __name__ == '__main__':
    main()
