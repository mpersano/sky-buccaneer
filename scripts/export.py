import bpy
import struct

def write(filepath):
    scene = bpy.context.scene
    objects = [o for o in scene.objects if o.type == 'MESH']
    if not objects:
        print('No objects to export?')
        return
    with open(filepath, 'wb') as f: 
        f.write(struct.pack('<l', len(objects)))
        for obj in objects:
            matrix = obj.matrix_world.copy()
            mesh = obj.data
            vertices = mesh.vertices[:]
            polygons = mesh.polygons[:]
            f.write(struct.pack('<l', len(polygons)))
            print('Exporting %s (%d vertices, %d polygons)' % (mesh.name, len(vertices), len(polygons)))
            for poly in polygons:
                f.write(struct.pack('<l', len(poly.vertices)))
                for index in poly.vertices:
                    vertex = vertices[index]
                    f.write(struct.pack('<3f', *vertex.co))
                    f.write(struct.pack('<3f', *vertex.normal))

write('scene.bin')
