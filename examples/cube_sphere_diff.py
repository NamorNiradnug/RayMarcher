from raymarching import *

scene = Scene("AABBox and Sphere Difference")
sph = Sphere().scaled("1.25 + sin(TIME) / 4")
aabb = AABBox((1, 1, 1)).rotated((0, 1, 0), "TIME")
scene.set_sdf(Difference(aabb, sph))
scene.process()
