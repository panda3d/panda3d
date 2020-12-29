"This module contains a deprecated shim emulating the old DConfig API."

__all__ = []

from panda3d.core import (ConfigFlags, ConfigVariableBool, ConfigVariableInt,
                          ConfigVariableDouble, ConfigVariableString)
import warnings

def GetBool(sym, default=False):
    #I was not sure about these warnings so I only put in one for now. --Paustian
    if __debug__:
        assert not warnings.warn("This is deprecated. use ConfigVriableBool instead", DeprecationWarning, stacklevel=2)
    return ConfigVariableBool(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetInt(sym, default=0):
    return ConfigVariableInt(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetDouble(sym, default=0.0):
    return ConfigVariableDouble(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetString(sym, default=""):
    return ConfigVariableString(sym, default, "DConfig", ConfigFlags.F_dconfig).value


GetFloat = GetDouble
