from raymarching import *
from math import pi

scene = Scene("Smooth Union")
scene.set_sdf(SmoothUnion(1, Cylinder(radius=0.5, height2=1.5).rotated(u=(1, 0, 0), angle=pi/2),
                             Sphere().scaled(0.3).translated(("sin(TIME) * 2", 0, 0.2))))
scene.process()
