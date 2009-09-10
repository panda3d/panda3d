"""
This contains constants related with obj handling.
"""

# index for obj data structure
OBJ_UID = 0
OBJ_NP = 1
OBJ_DEF = 2
OBJ_MODEL = 3
OBJ_PROP = 4

# supported UI types
PROP_UI_ENTRY = '_PropUIEntry'
PROP_UI_COMBO = '_PropUIComboBox'
PROP_UI_RADIO = '_PropUIRadio'
PROP_UI_CHECK = '_PropUICheckBox'
PROP_UI_SLIDE = '_PropUISlider'
PROP_UI_SPIN = '_PropUISpinner'

# index for property definition
PROP_TYPE = 0
PROP_DATATYPE = 1
PROP_FUNC = 2
PROP_DEFAULT = 3
PROP_RANGE = 4

# index for slider UI
RANGE_MIN = 0
RANGE_MAX = 1
RANGE_RATE = 2

# index for create/update function declaration
FUNC_NAME = 0
FUNC_ARGS = 1

# data type of property value
PROP_INT = 0 # int type value
PROP_BOOL = 1 # bool type value
PROP_FLOAT = 2 # float type value
PROP_STR = 3 # string type value

TYPE_CONV = {PROP_INT: int, PROP_BOOL: bool, PROP_FLOAT: float, PROP_STR: str}

# these dynamic args should be used in update function declaration
ARG_VAL = '_arg_val' # value from UI
ARG_OBJ = '_arg_object' # obj information data structure
