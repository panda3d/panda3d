from DirectSessionGlobal import *
import ParticleEffect
pe = ParticleEffect.ParticleEffect('particle-fx')
pe.reparentTo(render)
pe.setPos(0.0, 5.0, 4.0)
import ParticlePanel
p = pe.particles[0]
ParticlePanel.ParticlePanel(pe, p)
base.enableParticles()
pe.enable()
