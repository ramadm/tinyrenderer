#include <cmath>
#include "tgaimage.h"
#include <iostream>
#include <array>

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};


void draw_line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }

    int y = ay;
    int ierror = 0;
    
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        if (ierror > bx - ax) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}

struct Point2D {
    int x;
    int y;
};

void draw_polygon(std::vector<Point2D> points, TGAImage &framebuffer, TGAColor color) {
    for (size_t i = 0; i+1 < points.size(); i++) {
        Point2D point1 = points[i];
        Point2D point2 = points[i+1];
        draw_line(point1.x, point1.y, point2.x, point2.y, framebuffer, color);
    }
    Point2D end = points[points.size() - 1];
    Point2D start = points[0];
    draw_line(end.x, end.y, start.x, start.y, framebuffer, color);
}

class WavefrontObj {
private:
    std::vector<std::array<float, 3>> verts;
    std::vector<std::vector<int>> faces;

public:

    WavefrontObj(std::string filename) {
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cerr << "Failed to open file: " << filename;
            return;
        }

        std::string line;
        while(std::getline(inputFile, line)) {
            // parse the line by spaces
            std::string delim = " ";
            std::vector<std::string> tokens;
            size_t pos = 0;
            std::string token;
            while ((pos = line.find(delim)) != std::string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + delim.length());
            }
            tokens.push_back(line);

            // vertices
            if (tokens[0] == "v") {
                std::array<float, 3> triple;
                for (size_t i = 0; i < 3; i++) {
                    triple[i] = std::stof(tokens[i+1]);
                }
                verts.push_back(triple);
            }

            // faces
            if (tokens[0] == "f") {
                std::vector<int> polygon;
                for (size_t i = 1; i < tokens.size(); i++) {
                    // first number before / is the index of a coord in the verts array
                    std::string s = tokens[i];
                    int ind = std::stoi(s.substr(0, s.find("/"))) - 1; // decremented because wavefront file format uses 1-indexing.
                    polygon.push_back(ind);
                }
                faces.push_back(polygon);
            }
        }
    }

    // for now this method works by flattening the image on the z-axis
    void render(TGAImage &framebuffer) {
        const int width = framebuffer.width();
        const int height = framebuffer.height();

        std::vector<Point2D> realVerts;
        for (size_t i = 0; i < verts.size(); i++) {
            Point2D vert;
            vert.x = width / 2 + verts[i][0] * width / 2;
            vert.y = height / 2 + verts[i][1] * height / 2;
            framebuffer.set(vert.x, vert.y, white);
            realVerts.push_back(vert);
        }

        for (size_t i = 0; i < faces.size(); i++) {
            std::vector<int> face = faces[i];
            std::vector<Point2D> polygon;
            for (size_t j = 0; j < face.size(); j++) {
                int ind = face[j];
                polygon.push_back(realVerts[ind]);
            }
            draw_polygon(polygon, framebuffer, red);
        }
    }

};

int main(int argc, char** argv) {
    constexpr int width  = 1024;
    constexpr int height = 1024;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    // load the diablo image file
    std::string filename = "obj/diablo3_pose/diablo3_pose.obj";
    WavefrontObj model(filename);
    model.render(framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

