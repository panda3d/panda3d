"""instantiate global DirectNotify used in Direct"""

__all__ = ['directNotify', 'giveNotify']

import DirectNotify

directNotify = DirectNotify.DirectNotify()
giveNotify = directNotify.giveNotify
