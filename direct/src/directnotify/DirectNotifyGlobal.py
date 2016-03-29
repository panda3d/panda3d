"""instantiate global DirectNotify used in Direct"""

__all__ = ['directNotify', 'giveNotify']

from . import DirectNotify

directNotify = DirectNotify.DirectNotify()
giveNotify = directNotify.giveNotify
