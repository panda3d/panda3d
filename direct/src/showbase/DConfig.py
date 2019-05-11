"This module contains a deprecated shim emulating the old DConfig API."

__all__ = []

from panda3d.core import (ConfigFlags, ConfigVariableBool, ConfigVariableInt,
                          ConfigVariableDouble, ConfigVariableString)


def GetBool(sym, default=False):
    return ConfigVariableBool(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetInt(sym, default=0):
    return ConfigVariableInt(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetDouble(sym, default=0.0):
    return ConfigVariableDouble(sym, default, "DConfig", ConfigFlags.F_dconfig).value


def GetString(sym, default=""):
    return ConfigVariableString(sym, default, "DConfig", ConfigFlags.F_dconfig).value


GetFloat = GetDouble
