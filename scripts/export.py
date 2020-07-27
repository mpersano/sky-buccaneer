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
            f.write(struct.pack('<l', len(vertices)))
            for vertex in vertices:
                f.write(struct.pack('<3f', *vertex.co))
                f.write(struct.pack('<3f', *vertex.normal))

            polygons = mesh.polygons[:]
            triangles = 0
            for poly in polygons:
                triangles += len(poly.vertices) - 2
            f.write(struct.pack('<l', triangles))
            print('Exporting %s (%d vertices, %d polygons, %d triangles)' % (mesh.name, len(vertices), len(polygons), triangles))
            for poly in polygons:
                for i in range(1, len(poly.vertices) - 1):
                    def write_vertex(index):
                        f.write(struct.pack('<l', poly.vertices[index]))
                    write_vertex(0)
                    write_vertex(i)
                    write_vertex(i + 1)

write('scene.bin')
