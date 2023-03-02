import MojoBlender


def export(GP_pencil):
    pencil = MojoBlender.Pencil()

    styles = [
        material.grease_pencil for material in GP_pencil.materials]
    for GP_layer in GP_pencil.layers:
        layer = MojoBlender.PencilLayer()
        layer.name = GP_layer.info
        for GP_frame in GP_layer.frames:
            frame = MojoBlender.PencilFrame()
            frame.frame = GP_frame.frame_number
            for GP_stroke in GP_frame.strokes:
                style = styles[GP_stroke.material_index]
                points = []
                for GP_point in GP_stroke.points:
                    points.append(GP_point.co.x)
                    points.append(GP_point.co.z)
                    points.append(-GP_point.co.y)
                    points.append(GP_point.pressure * GP_stroke.line_width)
                if style.show_stroke:
                    frame.add_stroke(style.color, points)
                if style.show_fill:
                    indices = []
                    for tri in GP_stroke.triangles:
                        indices.append(tri.v1)
                        indices.append(tri.v2)
                        indices.append(tri.v3)
                    frame.add_filled(style.fill_color, points, indices)
            layer.push_frame(frame)
        pencil.push_layer(layer)

    return pencil
