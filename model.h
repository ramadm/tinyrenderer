#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
    std::vector<vec3> verts = {};    // array of vertices
    std::vector<int> facet_vrt = {}; // per-triangle index in the above array
    std::vector<vec3> vert_normals = {};
    std::vector<int> facet_nrm = {};
    std::vector<vec2> vert_textures = {};
    std::vector<int> facet_txt = {};
    TGAImage normal_map {};
    TGAImage diffuse_map {};
    TGAImage specular_map {};

public:
    Model(const std::string filename);
    int nverts() const; // number of vertices
    int nfaces() const; // number of triangles
    vec3 vert(const int i) const;                          // 0 <= i < nverts()
    vec3 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
    vec3 vert_normal(const int iface, const int nthvert) const;
    vec2 vert_texture(const int iface, const int nthvert) const;
    vec3 uv_normal(const double x, const double y) const;
    TGAColor diffuse_color(const double x, const double y) const;
    double uv_specular(const double x, const double y) const;
};

