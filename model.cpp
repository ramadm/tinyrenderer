#include "model.h"
#include <fstream>


std::vector<std::array<float, 3>> Model::get_verts() {
    return verts;
}

std::vector<std::vector<int>> Model::get_faces() {
    return faces;
}

// Load a model from a wavefront file (.obj)
Model::Model(std::string filename) {
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