#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <queue>

#include "cc_embedded_graph.h"

using namespace std;

/*
 * Constructor
 *
 * @param adjL Vertex-to-edge adjacency list representing embedded graph
 */
CC_Embedded_Graph::CC_Embedded_Graph(map<int, vector<int>> &adjL) {
    this->adjL = adjL;
    create_graph(this->adjL);
    color_dfs();
    bfs();
    calcRedFaces(red_faces);
    calcBlueFaces(blue_faces);
}

/*
 * Creates embedded graph object with vertex-to-edge adjacency list
 *
 * @param adjL Vertex-to-edge adjacency list representing embedded graph
 */
void CC_Embedded_Graph::create_graph(map<int, vector<int>> adjL) {
    int face_id = 0;
    
    // Iterate through each vertex
    for (int i = 0; i < adjL.size(); ++i) {
        // Iterate through each incident edge
        for (int j = 0; j < adjL[i].size(); ++j) {
            vector<Edge> face_edges;
            int next_edge_id = adjL[i][j];
            int curr_v = i;
            while (true) {
                // Forms face information by going through adjacency list
                for (int k = 0; k < adjL.size(); ++k) {
                    if (k != curr_v) {
                        if (find(adjL[k].begin(), adjL[k].end(), next_edge_id) != adjL[k].end()) {
                            Edge e = Edge(curr_v, k, next_edge_id);
                            face_edges.push_back(e);
                            if (find_if(this->edges.begin(), this->edges.end(), [&e](Edge edge) {
                                return edge == e;
                            }) != this->edges.end()) {
                                // empty
                            }
                            else {
                                this->edges.push_back(e);
                            } //endif
                            curr_v = k;
                            int curr_e_ind;
                            for (int i = 0; i < adjL[k].size(); ++i) {
                                if (adjL[k].at(i) == next_edge_id) {
                                    curr_e_ind = i;
                                    break;
                                } //endif
                            } //endfor
                            int curr_e = (curr_e_ind + 1) % adjL[k].size();
                            next_edge_id = adjL[curr_v][curr_e];
                            break;
                        } //endif
                    } //endif
                } //endfor
                if (next_edge_id == adjL[i][j]) {
                    break;
                } //endif
            } //endwhile
            Face f = Face(face_edges, face_id);
            if (find_if(faces.begin(), faces.end(), [&f](Face face) {
                return face == f;
            }) != faces.end()) {
                //empty
            }
            else {
                faces.push_back(f);
                ++face_id;
            } //endif
        } //endfor
    } //endfor
    face_count = face_id;
    edge_count = edges.size();
    vertex_count = adjL.size();

    // Creates face adjacency list
    for (int i = 0; i < faces.size(); ++i) {
        for (int j = i + 1; j < faces.size(); ++j) {
            for (auto e1 : faces[i].getEdges()) {
                for (auto e2 : faces[j].getEdges()) {
                    if (e1 == e2) {
                        face_adjL[faces[j].getID()].push_back(faces[i].getID());
                        face_adjL[faces[i].getID()].push_back(faces[j].getID());
                    } //endif
                } //endfor
            } //endfor
        } //endfor
    } //endfor
    for (int i = 0; i < faces.size(); ++i) {
        sort(face_adjL[i].begin(), face_adjL[i].end());
        auto itr = unique(face_adjL[i].begin(), face_adjL[i].end());
        face_adjL[i].resize(distance(face_adjL[i].begin(), itr));
    } //endfor

    // Creates vertex adjacency list
    for_each(edges.begin(), edges.end(), [this](Edge e) {
        int u = e.getV1();
        int v = e.getV2();
        v_adjL[u].push_back(v);
        v_adjL[v].push_back(u);
    }); //endfor
    for (int i = 0; i < v_adjL.size(); ++i) {
        sort(v_adjL[i].begin(), v_adjL[i].end());
        auto itr = unique(v_adjL[i].begin(), v_adjL[i].end());
        v_adjL[i].resize(distance(v_adjL[i].begin(), itr));
    } //endfor
}

/*
 * Depth-first search coloring of embedded graph.
 */
void CC_Embedded_Graph::color_dfs() {
    bool visited[face_count];
    fill_n(visited, face_count, false);
    color_dfs_util(0, 0, visited);
}

/*
 * Utility function for recursive implementation of DFS.
 *
 * @param face Current face being searched
 * @param color Current color of face
 * @param visited DFS visited array.
 */
void CC_Embedded_Graph::color_dfs_util(int face, int color, bool visited[]) {
    visited[face] = true;
    faces[face].setColor(color);
    for (int i = 0; i < face_adjL[face].size(); ++i) {
        if(!visited[face_adjL[face][i]]) {
            color_dfs_util(face_adjL[face][i], 1 - color, visited);
        } //endif
    } //endfor

}

/*
 * Standard breadth-first search for establishing vertex ordering in graph.
 */
void CC_Embedded_Graph::bfs() {
    vector<int> visited;
    visited.resize(v_adjL.size(), false);
    queue<int> q;
    visited[0] = true;
    q.push(0);

    while (!q.empty()) {
        int s = q.front();
        vertex_ordering.push_back(s);
        q.pop();

        for (int i = 0; i < v_adjL[s].size(); ++i) {
            if (!visited[v_adjL[s][i]]) {
                q.push(v_adjL[s][i]);
                visited[v_adjL[s][i]] = true;
            } //endif
        } //endfor
    } //endwhile
}


/*
 * Calculate for "red" faces of graph.
 */
void CC_Embedded_Graph::calcRedFaces(vector<Face>& red_faces) {
    for_each(faces.begin(), faces.end(), [&red_faces](Face f) {
        if (f.getColor()) {
            red_faces.push_back(f);
        }
    });

    red_face_vertices = vector<vector<int>>(vertex_count);

    for (int i = 0; i < red_faces.size(); ++i) {
        for (int j = 0; j < vertex_count; ++j) {
            if (red_faces[i].containsV(j)) red_face_vertices[j].push_back(i);
        }
    }
}

/*
 * Calculate for "blue" faces of graph.
 */
void CC_Embedded_Graph::calcBlueFaces(vector<Face>& blue_faces) {
    for_each(faces.begin(), faces.end(), [&blue_faces](Face f) {
        if (!f.getColor()) {
            blue_faces.push_back(f);
        }
    });

    blue_face_vertices = vector<vector<int>>(vertex_count);

    for (int i = 0; i < blue_faces.size(); ++i) {
        for (int j = 0; j < vertex_count; ++j) {
            if (blue_faces[i].containsV(j)) blue_face_vertices[j].push_back(i);
        }
    }
}

/*
 * Checks whether a Face contains a specified vertex
 *
 * @param v Vertex to check
 * @return bool Whether or not vertex is present in face
 */
bool Face::containsV(int v) {
    for (auto i : edges) {
        if (v == i.getV1() || v == i.getV2()) {
            return true;
        }
    }
    return false;
}
