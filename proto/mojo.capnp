@0xa019c1c3a519880e;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("proto");

struct Vec3 {
    x @0 :Float32;
    y @1 :Float32;
    z @2 :Float32;
}

struct Color {
    r @0 :Float32;
    g @1 :Float32;
    b @2 :Float32;
    a @3 :Float32;
}

struct Pencil {
    layers @0 :List(Layer);

    struct Layer {
        name @0 :Text;
        frames @1 :List(Frame);

        struct Frame {
            frame @0 :Int16;
            strokes @1 :List(Stroke);

            struct Stroke {
                points @0: List(Point);
                color @1 :Color;
                triangles @2: List(UInt32);
                struct Point {
                    position @0 :Vec3;
                    pressure @1 :Float32;
                }
            }
        }
    }
}

struct Mesh {
    points @0 :List(Vec3);
    counts @1 :List(UInt32);
    connection @2 :List(UInt32);
}