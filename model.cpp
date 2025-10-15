#include <fstream>
#include <sstream>
#include "model.h"
#include <algorithm>

Model::Model(const std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            vec3 v;
            for (int i : {0,1,2}) iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "vn")) {
            iss >> trash >> trash;
            vec3 v_n;
            for (int i : {0,1,2}) iss >> v_n[i];
            vert_normals.push_back(v_n);
        } else if (!line.compare(0, 2, "vt")) {
            iss >> trash >> trash;
            vec2 v_t;
            for (int i : {0,1}) iss >> v_t[i];
            vert_textures.push_back(v_t);

        } else if (!line.compare(0, 2, "f ")) {
            int f,t,n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n) {
                facet_vrt.push_back(--f);
                facet_nrm.push_back(--n);
                facet_txt.push_back(--t);
                cnt++;
            }
            if (3!=cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# "  << nfaces() << std::endl;

    // normal map
    std::string stripped = filename.substr(0, filename.length() - 4);
    std::string nm_filename = stripped + "_nm.tga";
    if (!normal_map.read_tga_file(nm_filename)) {
        return;
    }

    std::string diff_filename = stripped + "_diffuse.tga";
    if (!diffuse_map.read_tga_file(diff_filename)) {
        return;
    }

    std::string spec_filename = stripped + "_spec.tga";
    if (!specular_map.read_tga_file(spec_filename)) {
        return;
    }

}

int Model::nverts() const { return verts.size(); }
int Model::nfaces() const { return facet_vrt.size()/3; }

vec3 Model::vert(const int i) const {
    return verts[i];
}

vec3 Model::vert(const int iface, const int nthvert) const {
    return verts[facet_vrt[iface*3+nthvert]];
}

vec3 Model::vert_normal(const int iface, const int nthvert) const {
    return vert_normals[facet_nrm[iface*3+nthvert]];
}

vec2 Model::vert_texture(const int iface, const int nthvert) const {
    return vert_textures[facet_txt[iface*3+nthvert]];
}

vec3 Model::uv_normal(const double x, const double y) const {
    const int x_coord = x * normal_map.width();
    const int y_coord = normal_map.height() - y * normal_map.height();
    TGAColor as_color = normal_map.get(x_coord, y_coord);
    vec3 n {};
    for (int i : {0, 1, 2}) {
        int j = 2 - i;
        n[j] = (static_cast<double>(as_color[i]) - 127) / 255.;
    }
    return normalized(n);
}

TGAColor Model::diffuse_color(const double x, const double y) const {
    const int x_coord = x * diffuse_map.width();
    const int y_coord = diffuse_map.height() - y * diffuse_map.height();
    return diffuse_map.get(x_coord, y_coord);
}

double Model::uv_specular(const double x, const double y) const {
    const int x_coord = x * specular_map.width();
    const int y_coord = specular_map.height() - y * specular_map.height();
    TGAColor as_color = specular_map.get(x_coord, y_coord);
    return static_cast<double>(as_color[0]);
}