#NAME "Spheres Intersection"
#SUN sin(TIME * 0.2) (sin(TIME * 0.1) / 2 + 0.75) cos(TIME * 0.2)
#SDF s1 Sphere radius=1.5  1.0 0.0 0.0 0.0  -1.0 0.0 0.0  1.0
#SDF s2 Sphere radius=1.0  1.0 0.0 0.0 0.0  0.0 0.0 0.0  1.0
#SDF i1 Intersection o1=s1 o2=s2  0.0 1.0 0.0 TIME  0.0 0.0 0.0  1.0
#SCENE i1
from raymarching import *

scene = Scene("Sheres Intersection")
# scene.set_sun(...) or something, will added later
scene.set_sdf(Intersection(Sphere().scaled(1.5).translated((-1, 0, 0)), Sphere())
                            .rotated(u=(0, 1, 0), angle="TIME"))
scene.process()
