#ifndef RATS_EDGECODE_H
#define RATS_EDGECODE_H

using namespace std;

void create_adjL(vector<vector<double>>& vertices, vector<vector<int>>& faces, set<vector<int>>& edges, vector<vector<int>>& double_edges, map<int, vector<int>>& adjL);

#endif //RATS_EDGECODE_H
