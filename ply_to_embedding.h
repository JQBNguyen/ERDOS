#ifndef RATS_PLY_TO_EMBEDDING_H
#define RATS_PLY_TO_EMBEDDING_H
#include <vector>
#include <fstream>
using namespace std;

bool ply_to_embedding(string file_name, vector<vector<double>>& vertices, vector<vector<int>>& faces);

#endif //RATS_PLY_TO_EMBEDDING_H
