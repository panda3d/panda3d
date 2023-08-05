from direct.showbase.ShowBaseGlobal import base
from .WxPandaShell import WxPandaShell
base.app = WxPandaShell()  # type: ignore[attr-defined]
