from raymarching import *

scene = Scene("Random Example")
scene.set_sdf(Intersection(
            Sphere().scaled(3).translated("vec3(0, sin(TIME), cos(TIME)) * 3"),
            AABBox((1, 1, 1))
         ).rotated(u=(0, 0, 0), angle="TIME"))
scene.process()
