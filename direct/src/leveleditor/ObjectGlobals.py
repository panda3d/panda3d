"""
This contains constants related with obj handling.
"""

# index for obj data structure
OBJ_UID = 0
OBJ_NP = 1
OBJ_DEF = 2
OBJ_MODEL = 3
OBJ_ANIM = 4
OBJ_PROP = 5
OBJ_RGBA = 6

# supported UI types
PROP_UI_ENTRY = '_PropUIEntry'
PROP_UI_COMBO = '_PropUIComboBox'
PROP_UI_RADIO = '_PropUIRadio'
PROP_UI_CHECK = '_PropUICheckBox'
PROP_UI_SLIDE = '_PropUISlider'
PROP_UI_SPIN = '_PropUISpinner'
PROP_UI_BLIND = '_PropUIBlind'
PROP_UI_COMBO_DYNAMIC = '_PropUIComboBoxDynamic'
PROP_UI_TIME = '_PropUITime'

# index for property definition
PROP_TYPE = 0
PROP_DATATYPE = 1
PROP_FUNC = 2
PROP_DEFAULT = 3
PROP_RANGE = 4
PROP_DYNAMIC_KEY = 5

# key value constant for dynamic props
PROP_MODEL = '_PropModel'

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
PROP_BLIND = 4 # blind type value

TYPE_CONV = {PROP_INT: int, PROP_BOOL: bool, PROP_FLOAT: float, PROP_STR: str}

# these dynamic args should be used in create / update function declaration
ARG_NAME = '_arg_name'
ARG_VAL = '_arg_val' # value from UI
ARG_OBJ = '_arg_object' # obj information data structure
ARG_NOLOADING = '_arg_noloading' # to indicate this call is not from loading a scene
ARG_PARENT = '_arg_parent' # parent object to be passed
