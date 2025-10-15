#include <cmath>
#include <tuple>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

mat<4,4> ModelView, Viewport, Perspective;

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalized(eye-center);
    vec3 l = normalized(cross(up,n));
    vec3 m = normalized(cross(n, l));
    ModelView = mat<4,4>{{{l.x,l.y,l.z,0}, {m.x,m.y,m.z,0}, {n.x,n.y,n.z,0}, {0,0,0,1}}} *
                mat<4,4>{{{1,0,0,-center.x}, {0,1,0,-center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void perspective(const double f) {
    Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer, TGAColor color) {
    int bbminx = std::max(0, std::min(std::min(ax, bx), cx)); // bounding box for the triangle clipped by the screen
    int bbminy = std::max(0, std::min(std::min(ay, by), cy)); // defined by its top left and bottom right corners
    int bbmaxx = std::min(framebuffer.width() -1, std::max(std::max(ax, bx), cx));
    int bbmaxy = std::min(framebuffer.height()-1, std::max(std::max(ay, by), cy));
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            double z_calc = (alpha * az + beta * bz + gamma * cz);
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            if (z <= zbuffer.get(x, y)[0]) continue;
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

// the viewport transformation is an affine transformation which takes clip coordinates to x in [0, w], y in [0, h] and discards z
void viewport(const int x, const int y, const int w, const int h) {
    mat<4, 4> Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

// the perspective deformation takes [x, y, z] -> (1 - z/f)[x, y, z] for focal distance f
void perspective(const double f) {
    mat<4, 4> Perspective = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0, -1/f,1}}};
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;
    constexpr int height = 800;
    constexpr vec3    eye{-1,0,2}; // camera position
    constexpr vec3 center{0,0,0};  // camera direction
    constexpr vec3     up{0,1,0};  // camera up vector

    // TODO: look at each of these
    lookat(eye, center, up);                              // build the ModelView   matrix
    perspective(norm(eye-center));                        // build the Perspective matrix
    viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport

    TGAImage framebuffer(width, height, TGAImage::RGB);
    Model model(argv[1]);

    // TODO, why is this changed to a different data type
    TGAImage     zbuffer(width, height, TGAImage::GRAYSCALE);

    // now we render can render multiple input objects in one go
    for (int m = 1; m < argc; m++) {
        Model model(argv[m]);
        for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
            // a real triangle in 3D space is represented as 3 points
            // which are 4-vectors because of our homogenous embedding
            vec4 clip[3];
            // now we will embed our 3D points in 4D to handle affine transformations
            // before we go back to 3D
            for (int j=0; j<3; j++) {
                vec3 v = model.vert(i, j);
                // TODO: go over the order of transformations + why don't we multiply by viewport here?
                clip[j] = Perspective * ModelView * vec4{v.x, v.y, v.z, 1.};
            }
            TGAColor rnd;
            for (int c=0; c<3; c++) rnd[c] = std::rand()%255;
            // TODO, replace with new rasterize function
            //triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
        }
    }



    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
