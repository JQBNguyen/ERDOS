#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include "make_cc.h"

using namespace std;

/*
 * Adjusted breadth-first search algorithm to insert any double edges to make graph checkerboard-colorable
 *
 * @param face_adjL Face adjacency list
 * @param double_edges Double edges to be inserted if necessary
 * @param edge_to_faces Edge-to-face adjacency list
 */
void bfs_cc(map<int, vector<int>>& face_adjL, vector<vector<int>>& double_edges, map<vector<int>, vector<int>>& edge_to_faces) {

    vector<bool> visited;
    vector<int> q;
    vector<int> level_faces;
    vector<vector<int>> same_level_faces;
    int s = 0;

    visited.resize(face_adjL.size(), false);
    visited[s] = true;
    q.push_back(s);
    level_faces.push_back(s);

    while(!level_faces.empty()) {
        // Searches vertices one level at a time
        q = level_faces;
        level_faces.clear();

        // Standard BFS
        while (!q.empty()) {
            s = q.at(q.size() - 1);
            q.pop_back();

            for (int i = 0; i < face_adjL.at(s).size(); ++i) {
                if (visited.at(face_adjL.at(s)[i]) == false) {
                    visited.at(face_adjL.at(s)[i]) = true;
                    level_faces.push_back(face_adjL.at(s)[i]);
                } //endif
            } //endfor
        } //endwhile

        // Checks for adjacent same-colored faces and tracks which faces to be subdivided
        for (int i = 0; i < level_faces.size(); ++i) {
            for (int j = i + 1; j < level_faces.size(); ++j) {
                if (find(face_adjL.at(level_faces.at(i)).begin(), face_adjL.at(level_faces.at(i)).end(), level_faces[j]) != face_adjL.at(i).end()) {
                    same_level_faces.push_back({level_faces.at(i), level_faces.at(j)});
                } //endif
            } //endfor
        } //endfor

    } //endwhile

    // Inserts double edges where necessary
    for_each(same_level_faces.begin(), same_level_faces.end(), [edge_to_faces, &double_edges](vector<int> faces) {
        for_each(edge_to_faces.begin(), edge_to_faces.end(), [&faces, &double_edges](pair<vector<int>, vector<int>> p) {;
            sort(faces.begin(), faces.end());
            if (p.second == faces) {
                double_edges.push_back(p.first);
            }
        });
    });

}

/*
 * Makes a mesh checkerboard-colorable.
 *
 * @param faces Faces of the mesh
 * @param edges Edges of the mesh
 * @param double_edges Double edges to be inserted if necessary
 */
void makecc(vector<vector<int>> faces, set<vector<int>>& edges, vector<vector<int>>& double_edges) {

    map<vector<int>, vector<int>> edge_to_faces;

    // Creates edge-to-face adjacency list
    for (int i = 0; i < faces.size(); ++i) {
        vector<int> f = faces[i];
        for (int j = 0; j < f.size(); ++j) {
            vector<int> e = {f.at(j), f.at((j + 1) % f.size())};
            sort(e.begin(), e.end());
            edges.insert(e);
            edge_to_faces[e].push_back(i);
        }
    }

    map<int, vector<int>> face_adjL;

    // Creates face adjacency list
    for_each(edge_to_faces.begin(), edge_to_faces.end(), [&face_adjL](pair<vector<int>, vector<int>> p) {
        int f1 = p.second.at(0);
        int f2 = p.second.at(1);
        face_adjL[f1].push_back(f2);
        face_adjL[f2].push_back(f1);
    });

    // Adjusted BFS to insert double edges
    bfs_cc(face_adjL, double_edges, edge_to_faces);

}
