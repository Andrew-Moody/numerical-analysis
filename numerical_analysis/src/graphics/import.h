#pragma once

struct Mesh;

// STL files should only contain a single mesh
void load_stl(const char* path, struct Mesh* mesh);