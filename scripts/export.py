import bpy
from bpy_extras.io_utils import ExportHelper
from mathutils import Euler
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

        def write_float(value):
            outfile.write(struct.pack('<f', value))

        def write_vec3(value):
            outfile.write(struct.pack('<3f', *value))

        def write_vec4(value):
            outfile.write(struct.pack('<4f', *value))

        def write_quat(value):
            outfile.write(struct.pack('<4f', value.x, value.y, value.z, value.w))

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
            print('Exporting mesh `%s` (%d vertices, %d polygons, %d triangles)' % (mesh.name, len(vertices), len(polygons), triangles))
            for poly in polygons:
                for i in range(1, len(poly.vertices) - 1):
                    write_int32(poly.vertices[0])
                    write_int32(poly.vertices[i])
                    write_int32(poly.vertices[i + 1])

        def write_action(target_node, action):
            RotationChannel = 0
            TranslationChannel = 1
            ScaleChannel = 2

            translation_fcurves = [fcurve for fcurve in action.fcurves if fcurve.data_path == 'location']
            rotation_fcurves = [fcurve for fcurve in action.fcurves if fcurve.data_path == 'rotation_euler']
            scale_curves = [fcurve for fcurve in action.fcurves if fcurve.data_path == 'scale']

            channel_count = 0
            if len(translation_fcurves) > 0:
                channel_count += 1
            if len(rotation_fcurves) > 0:
                channel_count += 1
            if len(scale_curves) > 0:
                channel_count += 1

            print('Exporting action `%s` (%d channels)' % (action.name, channel_count))
            write_int32(channel_count)

            def write_channel(path_type, fcurves, write_sample):
                write_int32(target_node)
                write_int8(path_type)
                start_frame = int(min(fcurve.range()[0] for fcurve in fcurves))
                end_frame = int(max(fcurve.range()[1] for fcurve in fcurves))
                write_int32(start_frame)
                write_int32(end_frame)
                for frame in range(start_frame, end_frame + 1):
                    sample = [fcurve.evaluate(frame) for fcurve in fcurves]
                    print('frame=%d, sample=%s' % (frame, sample))
                    write_sample(sample)

            if len(translation_fcurves) > 0:
                write_channel(TranslationChannel, translation_fcurves, write_vec3)
            if len(rotation_fcurves) > 0:
                write_channel(RotationChannel, rotation_fcurves, lambda sample:write_quat(Euler(sample).to_quaternion()))
            if len(scale_curves) > 0:
                write_channel(ScaleChannel, scale_fcurves, write_vec3)

        write_int32(len(objects))
        for obj in objects:
            is_mesh = obj.type == 'MESH'
            write_int8(is_mesh)
            translation, rotation, scale = obj.matrix_local.decompose()
            write_vec3(translation)
            write_quat(rotation)
            write_vec3(scale)
            children = [o for o in obj.children if o in objects]
            write_int32(len(children))
            for child in children:
                write_int32(objects.index(child))
            if is_mesh:
                write_mesh(obj.data)

        actions = [(o, o.animation_data.action) for o in objects if o.animation_data is not None and o.animation_data.action is not None]
        print('Exporting %d actions' % len(actions))

        write_int32(len(actions))
        for obj, action in actions:
            write_action(objects.index(obj), action)

class Area13EntityExporter(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.a13"
    bl_label = "Export Area-13 Entity"
    bl_description = "Export scene to Area-13 entity"

    filename_ext = ".entity"

    def execute(self, context):
        write_entity(self.filepath, context)
        return {'FINISHED'}

def register():
    print('registered!')
    bpy.utils.register_class(Area13EntityExporter)

def unregister():
    bpy.utils.unregister_class(Area13EntityExporter)
