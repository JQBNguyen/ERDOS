#ifndef RATS_MAKE_CC_H
#define RATS_MAKE_CC_H
#include <map>
#include <vector>

using namespace std;

void bfs_cc(map<int, vector<int>>& face_adjL, vector<vector<int>>& double_edges, map<vector<int>, vector<int>>& edge_to_faces);
void makecc(vector<vector<int>> faces, set<vector<int>>& edges, vector<vector<int>>& double_edges);

#endif //RATS_MAKE_CC_H
