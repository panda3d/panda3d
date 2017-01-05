from direct.showutil.dist import *

setup(
    name="asteroids",
    directories=['.'],
    exclude_paths=['build', 'setup.py'],
    applications=[Application('main.py', 'asteroids')],
    deploy_platforms=['linux_x86_64'],
)
