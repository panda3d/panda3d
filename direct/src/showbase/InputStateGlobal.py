"""instantiate global InputState object"""

# This file had to be separated from MessengerGlobal to resolve a
# circular include dependency with DirectObject.

import InputState

inputState = InputState.InputState()
