import bpy
from bpy_extras.io_utils import ExportHelper
import struct

bl_info = {
    "name": "Area-13 Entity Exporter",
    "blender": (2, 82, 0),
    "category": "Import-Export"
}

def write_entity(filepath, context):
    scene = context.scene
    objects = [o for o in scene.objects if o.type == 'MESH' or o.type == 'EMPTY']
    with open(filepath, 'wb') as outfile:
        def write_int8(value):
            outfile.write(struct.pack('<B', value))

        def write_int32(value):
            outfile.write(struct.pack('<L', value))

        def write_vec3(value):
            outfile.write(struct.pack('<3f', *value))

        def write_vec4(value):
            outfile.write(struct.pack('<4f', *value))

        def write_mat4(value):
            for v in value:
                write_vec4(v)

        def write_mesh(mesh):
            vertices = mesh.vertices[:]
            write_int32(len(vertices))
            for vertex in vertices:
                write_vec3(vertex.co)
                write_vec3(vertex.normal)
            polygons = mesh.polygons[:]
            triangles = 0
            for poly in polygons:
                triangles += len(poly.vertices) - 2
            write_int32(triangles)
            print('Exporting %s (%d vertices, %d polygons, %d triangles)' % (mesh.name, len(vertices), len(polygons), triangles))
            for poly in polygons:
                for i in range(1, len(poly.vertices) - 1):
                    write_int32(poly.vertices[0])
                    write_int32(poly.vertices[i])
                    write_int32(poly.vertices[ii + 1])

        write_int32(len(objects))
        for obj in objects:
            is_mesh = obj.type == 'MESH'
            write_int8(is_mesh)
            write_mat4(obj.matrix_local)
            children = [o for o in obj.children if o in objects]
            write_int32(len(children))
            for child in children:
                write_int32(objects.index(child))
            if is_mesh:
                write_mesh(obj.data)

class Area13EntityExporter(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.a13"
    bl_label = "Export Area-13 Entity"
    bl_description = "Export scene to Area-13 entity"

    filename_ext = ".entity"

    def execute(self, context):
        write_entity(self.filepath, context)
        return {'FINISHED'}

def register():
    bpy.utils.register_class(Area13EntityExporter)

def unregister():
    bpy.utils.unregister_class(Area13EntityExporter)
