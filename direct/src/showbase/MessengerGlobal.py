"""instantiate global Messenger object"""

import Messenger
import InputState

messenger = Messenger.Messenger()

# inputState is an optional add-on for the messenger, and
# that is why it is created here (See Also: InputState.py):
inputState = InputState.InputState()
