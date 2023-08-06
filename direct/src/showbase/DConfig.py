"This module contains a deprecated shim emulating the old DConfig API."

__all__ = ()

from panda3d.core import (ConfigFlags, ConfigVariableBool, ConfigVariableInt,
                          ConfigVariableDouble, ConfigVariableString)
import warnings


def GetBool(sym, default=False):
    if __debug__:
        warnings.warn("This is deprecated. Use ConfigVariableBool instead", DeprecationWarning, stacklevel=2)
    return ConfigVariableBool(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetInt(sym, default=0):
    if __debug__:
        warnings.warn("This is deprecated. Use ConfigVariableInt instead", DeprecationWarning, stacklevel=2)
    return ConfigVariableInt(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetDouble(sym, default=0.0):
    if __debug__:
        warnings.warn("This is deprecated. Use ConfigVariableDouble instead", DeprecationWarning, stacklevel=2)
    return ConfigVariableDouble(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetString(sym, default=""):
    if __debug__:
        warnings.warn("This is deprecated. Use ConfigVariableString instead", DeprecationWarning, stacklevel=2)
    return ConfigVariableString(sym, default, "DConfig", ConfigFlags.F_dconfig).value


GetFloat = GetDouble
