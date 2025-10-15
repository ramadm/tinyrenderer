#include <cstdlib>
#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct RandomShader : IShader {
    const Model &model;
    TGAColor color = {};
    vec3 tri[3];  // triangle in eye coordinates

    RandomShader(const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri[vert] = gl_Position.xyz();                            // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        return {false, color};                                    // do not discard the pixel
    }
};

struct PhongShader: IShader {
    const Model &model;
    vec3 l; // light direction
    vec3 tri[3];  // triangle in eye coordinates
    vec3 norms[3]; // vertex normals (in eye coords?)
    vec2 uv_tri[3]; // uv-mapped coordinates of this triangle

    PhongShader(const Model &m, const vec3 light) : model(m), l(light) {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}).xyz()); // transform the light vector to view coordinates
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri[vert] = gl_Position.xyz();                            // in eye coordinates

        // vec3 v_n = model.vert_normal(face, vert);
        // vec4 gl_Normal = ModelView * vec4{v_n.x, v_n.y, v_n.z, 1.};
        // norms[vert] = gl_Normal.xyz(); // TODO: look into issues w/ using ModelView to transform normals

        uv_tri[vert] = model.vert_texture(face, vert); // uv-mapping for this triangle

        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
        vec2 uv_point = bar.x * uv_tri[0] + bar.y * uv_tri[1] + bar.z * uv_tri[2];

        vec3 n_pre = model.uv_normal(uv_point.x, uv_point.y);
        vec3 n = normalized((ModelView.invert_transpose() * vec4{n_pre.x, n_pre.y, n_pre.z, 0.}).xyz());

        vec3 r = normalized((2 * l * n) * n - l);

        double ambient = 0.4;
        double diffuse = 1. * std::max(0., l * n);
        double specular = (3. * model.uv_specular(uv_point.x, uv_point.y) / 255.) * pow(std::max(0., r.z), 35);

        TGAColor illumination = model.diffuse_color(uv_point.x, uv_point.y);
        for (int channel: {0, 1, 2}) {
            illumination[channel] = std::min(255., illumination[channel] * (ambient + diffuse + specular));
        }
        
        return {false, illumination};
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    lookat(eye, center, up);                                   // build the ModelView   matrix
    init_perspective(norm(eye-center));                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {0, 0, 0, 255});

    for (int m=1; m<argc; m++) {                    // iterate through all input objects
        Model model(argv[m]);                       // load the data
        PhongShader shader(model, vec3{1, 1, 1});
        for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

