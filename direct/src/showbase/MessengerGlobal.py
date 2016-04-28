"""instantiate global Messenger object"""

__all__ = ['messenger']

from . import Messenger

messenger = Messenger.Messenger()
