from direct.showbase.ShowBaseGlobal import *

import ParticleEffect
from direct.tkpanels import ParticlePanel
import ForceGroup

# Showbase
base.enableParticles()

# ForceGroup
gravity = LinearVectorForce(Vec3(0.0, 0.0, -10.0))
fg = ForceGroup.ForceGroup()
fg.addForce(gravity)

# Particle effect
pe = ParticleEffect.ParticleEffect('particle-fx')
pe.reparentTo(render)
pe.setPos(0.0, 5.0, 4.0)
pe.addForceGroup(fg)

# Particle Panel
pp = ParticlePanel.ParticlePanel(pe)
