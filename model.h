#include <vector>
#include <string>
#include <iostream>
#include <array>

class Model {
private:
    std::vector<std::array<float, 3>> verts;
    std::vector<std::vector<int>> faces;

public:
    Model(std::string filename);
    std::vector<std::array<float, 3>> get_verts();
    std::vector<std::vector<int>> get_faces();
};