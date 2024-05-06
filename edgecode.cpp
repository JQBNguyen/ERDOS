#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <ostream>

#include "edgecode.h"

using namespace std;

/*
 * Creates vertex-to-edge adjacency list to represent mesh.
 *
 * @param vertices Container of vertices
 * @param faces Container of faces
 * @param edges Container of edges
 * @param double_edges Container of double edges
 * @param adjL Adjacency list to be made
 */
void create_adjL(vector<vector<double>>& vertices, vector<vector<int>>& faces, set<vector<int>>& edges, vector<vector<int>>& double_edges, map<int, vector<int>>& adjL) {

    vector<vector<int>> m;
    map<int, vector<vector<int>>> face_list;

    // Creates vertex-to-face adjacency list
    for_each(faces.begin(), faces.end(), [&face_list](vector<int> f) {
        for_each(f.begin(), f.end(), [&face_list, &f](int v) {
            face_list[v].push_back(f);
        }); //endfor
    }); //endfor

    // Creates edge matrix with vertex indices
    for (int i = 0; i < vertices.size(); ++i) {
        vector<int> a;
        for (int j = 0; j < vertices.size(); ++j) {
            a.push_back(-1);
        } //endfor
        m.push_back(a);
    } //endfor
    int j = 0;
    for_each(edges.begin(), edges.end(), [&m, &j](vector<int> e) {
        m.at(e.at(0)).at(e.at(1)) = j;
        m.at(e.at(1)).at(e.at(0)) = j++;
    }); //endfor

    for (int v = 0; v < vertices.size(); ++v) {
        // Inserts first two edges incident to current vertex of interest
        vector<int> curr_face = face_list[v].at(0);
        int vInd;
        for (int i = 0; i < curr_face.size(); ++i) {
            if (curr_face.at(i) == v) {
                vInd = i;
                break;
            } //endif
        } //endfor
        int uInd = vInd - 1;
        if (uInd == -1) {
            uInd = curr_face.size() - 1;
        } //endif
        int u = curr_face.at(uInd);
        int first_edge = m.at(u).at(v);
        int w = curr_face.at((vInd + 1) % curr_face.size());
        int curr_edge = m.at(v).at(w);
        adjL[v].push_back(first_edge);
        adjL[v].push_back(curr_edge);

        // Continues iterating iterating through faces to find every edge incident to current vertex of interest
        while (true) {
            bool new_face_found = false;
            for (int i = 0; i < face_list.at(v).size(); ++i) {
                for (int j = 0; j < face_list.at(w).size(); ++j) {
                    if (face_list.at(v).at(i) == face_list.at(w).at(j) && face_list.at(v).at(i) != curr_face) {
                        new_face_found = true;
                        curr_face = face_list.at(v).at(i);
                        break;
                    } //endif
                    if (new_face_found) {
                        break;
                    } //endif
                } //endfor
            } //endfor
            for (int i = 0; i < curr_face.size(); ++i) {
                if (curr_face.at(i) == v) {
                    vInd = i;
                    break;
                } //endif
            } //endfor
            w = curr_face.at((vInd + 1) % curr_face.size());
            int next_edge = m.at(v).at(w);;
            if (next_edge == first_edge) {
                break;
            } //endif
            adjL[v].push_back(next_edge);
        } //endwhile
    } //endfor
    
    // Inserts double edges in correct orientation into adjacency list
    for (int d = 0; d < double_edges.size(); ++d) {
        int u = double_edges.at(d).at(0);
        int v = double_edges.at(d).at(1);
        for (auto i = adjL[u].begin(); i < adjL[u].end(); ++i) {
            if (*i == m[u][v]) {
                adjL[u].insert(i + 1, d + edges.size());
                break;
            } //endif
        } //endfor
        for (auto i = adjL[v].end() - 1; i >= adjL[v].begin(); --i) {
            if (*i == m[u][v]) {
                adjL[v].insert(i, d + edges.size());
                break;
            } //endif
        } //endfor
    } //endfor
}