from DirectSessionGlobal import *

import ParticleEffect
import ParticlePanel
import Forces

# Showbase
base.enableParticles()

# Forces
gravity = LinearVectorForce(Vec3(0.0, 0.0, -10.0))
f = Forces.Forces()
f.addForce(gravity)

# Particle effect
pe = ParticleEffect.ParticleEffect('particle-fx')
pe.reparentTo(render)
pe.setPos(0.0, 5.0, 4.0)
pe.addForces(f)
pe.enable()

# Particle Panel
ParticlePanel.ParticlePanel(pe)
