from panda3d.core import AudioSound
from .extension_native_helpers import Dtool_funcToMethod

def getCommentDict(self):
    comments = {}
    for row in self.getComment().split("\n"):
        if "=" not in row:
            continue
        key, value = row.split("=", 1)
        comments[key.upper()] = value
    # include some commonly used comments
    comments.setdefault("DATE", "")
    comments.setdefault("TITLE", "")
    comments.setdefault("ARTIST", "")
    return comments
    
Dtool_funcToMethod(getCommentDict, AudioSound)
del getCommentDict
