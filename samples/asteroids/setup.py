from direct.showutil.dist import *

setup(
    name="asteroids",
    directories=['.'],
    exclude_paths=['build', 'setup.py', 'requirements.txt', 'wheels'],
    applications=[Application('main.py', 'asteroids')],
    deploy_platforms=['linux_x86_64'],
)
