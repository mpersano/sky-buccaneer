import bpy
from bpy_extras.io_utils import ExportHelper
from bpy_extras import node_shader_utils
from mathutils import Euler
import struct
import os

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

        def write_vec2(value):
            outfile.write(struct.pack('<2f', *value))

        def write_vec3(value):
            outfile.write(struct.pack('<3f', *value))

        def write_vec4(value):
            outfile.write(struct.pack('<4f', *value))

        def write_quat(value):
            outfile.write(struct.pack('<4f', value.x, value.y, value.z, value.w))

        def write_mat4(value):
            for v in value:
                write_vec4(v)

        def write_string(value):
            write_int8(len(value))
            outfile.write(value.encode('ascii'))

        def write_mesh(mesh):
            mesh_vertices = mesh.vertices[:]
            mesh_polygons = mesh.polygons[:]
            materials = mesh.materials[:]

            uv_layer = None
            if len(mesh.uv_layers) > 0:
                uv_layer = mesh.uv_layers.active.data

            # write meshes (one per material)
            write_int32(len(materials))
            for mat_index, mat in enumerate(materials):
                # write material

                write_string(mat.name)
                mat_wrap = node_shader_utils.PrincipledBSDFWrapper(mat)
                base_color_texture = mat_wrap.base_color_texture.image.filepath
                write_string(os.path.basename(base_color_texture))

                polygons = [p for p in mesh_polygons if p.material_index == mat_index]
                vertex_dict = {}
                vertices = []
                triangles = []

                for poly in polygons:
                    if uv_layer is not None:
                        texcoords = [uv_layer[i].uv for i in range(poly.loop_start, poly.loop_start + poly.loop_total)]
                    else:
                        texcoords = [[0, 0] * len(poly.vertices)]

                    def vec3_to_key(v):
                        return round(v[0], 6), round(v[1], 6), round(v[2], 6)

                    def vec2_to_key(v):
                        return round(v[0], 6), round(v[1], 6)

                    def vertex_index(i):
                        vertex = mesh_vertices[poly.vertices[i]]
                        position = vertex.co
                        normal = vertex.normal
                        texcoord = texcoords[i]
                        key = vec3_to_key(position), vec3_to_key(normal), vec2_to_key(texcoord)
                        vertex_index = vertex_dict.get(key)
                        if vertex_index is None:
                            vertex_index = vertex_dict[key] = len(vertices)
                            vertices.append((position, normal, texcoord))
                        return vertex_index

                    for i in range(1, len(poly.vertices) - 1):
                        triangles.append((vertex_index(0), vertex_index(i), vertex_index(i + 1)))

                print('Exporting submesh (material: `%s`, vertices: %d, triangles: %d)' % (mat.name, len(vertices), len(triangles)))

                write_int32(len(vertices))
                for vertex in vertices:
                    write_vec3(vertex[0]) # position
                    write_vec3(vertex[1]) # normal
                    write_vec2(vertex[2]) # texcoord

                write_int32(len(triangles))
                for tri in triangles:
                    write_int32(tri[0])
                    write_int32(tri[1])
                    write_int32(tri[2])

        def write_action(action):
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

            write_string(action.name)
            write_int32(channel_count)

            def write_channel(path_type, fcurves, write_sample):
                write_int8(path_type)
                start_frame = int(min(fcurve.range()[0] for fcurve in fcurves))
                end_frame = int(max(fcurve.range()[1] for fcurve in fcurves))
                write_int32(start_frame)
                write_int32(end_frame)
                for frame in range(start_frame, end_frame + 1):
                    sample = [0] * 3
                    for fcurve in fcurves:
                        sample[fcurve.array_index] = fcurve.evaluate(frame)
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
            print('Exporting object `%s`' % obj.name)
            is_mesh = obj.type == 'MESH'
            write_string(obj.name)
            write_int8(is_mesh)
            translation, rotation, scale = obj.matrix_local.decompose()
            write_vec3(translation)
            write_quat(rotation)
            write_vec3(scale)
            children = [o for o in obj.children if o in objects]
            write_int32(len(children))
            for child in children:
                write_int32(objects.index(child))
            actions = []
            if obj.animation_data is not None:
                if obj.animation_data.action is not None:
                    actions.append(obj.animation_data.action)
                for track in obj.animation_data.nla_tracks:
                    for strip in track.strips:
                        if strip.action is not None and strip.mute is False:
                            actions.append(strip.action)
                actions = list(set(actions))
            write_int32(len(actions))
            for action in actions:
                write_action(action)
            if is_mesh:
                mesh = obj.to_mesh()
                write_mesh(mesh)
                obj.to_mesh_clear()

class ExportEntity(bpy.types.Operator, ExportHelper):
    bl_idname = "export_mesh.z3"
    bl_label = "Export Entity"
    bl_description = "Export scene to game entity"

    filename_ext = ".z3"

    def execute(self, context):
        write_entity(self.filepath, context)
        return {'FINISHED'}

def register():
    print('registered!')
    bpy.utils.register_class(EntityExporter)

def unregister():
    bpy.utils.unregister_class(EntityExporter)
