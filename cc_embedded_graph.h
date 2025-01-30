#ifndef RATS_CC_EMBEDDED_GRAPH_H
#define RATS_CC_EMBEDDED_GRAPH_H

#include <ostream>
#include <iostream>
#include <map>
#include <algorithm>

using namespace std;

/*
 * Class to represent Edge of polyhedra.
 */
class Edge {
private:
    int v1, v2, id;

public:
    Edge();
    Edge(int v1, int v2, int id) {
        this->v1 = v1;
        this->v2 = v2;
        this->id = id;
    }
    int getV1() { return this->v1; }
    int getV2() { return this->v2; }
    int getID() { return this->id; }
    bool operator == (const Edge& e) const {
        return e.id == this->id;
    }
    friend ostream& operator<<(ostream& os, const Edge& e) {
        os << "Edge " << e.id << ": (" << e.v1 << ", " << e.v2 << ")";
        return os;
    }
};

/*
 * Class to represent Face of polyhedra.
 */
class Face {
private:
    vector<Edge> edges;
    int id;
    int color;

public:
    Face();
    Face(vector<Edge> edges, int id) {
        this->edges = edges;
        this->id = id;
        this-> color = -1;
    };
    vector<Edge> getEdges() { return this->edges; }
    int getID() { return this->id; }
    int getColor() { return this->color; }
    void setColor(int color) { this->color = color; }
    bool containsV(int v);
    bool operator == (const Face& f) const {
        bool equal = true;
        for (auto e : this->edges) {
            if (find_if(f.edges.begin(), f.edges.end(), [&e](Edge ep) {
                return ep == e;
            }) != f.edges.end()) {
                //empty
            }
            else {
                equal = false;
            }
        }
        return equal;
    }
    friend ostream& operator<<(ostream& os, const Face& f) {
        os << "Face " << f.id << "(Color:" << f.color << "): ";
        for (auto i : f.edges) {
            os << i << " ; ";
        }
        os << endl;
        return os;
    }
};

/*
 * Class to represent checkerboard-colorable embedded graph.
 */
class CC_Embedded_Graph {
private:
    int face_count;
    int edge_count;
    int vertex_count;
    map<int, vector<int>> adjL;
    vector<Face> faces;
    vector<Edge> edges;
    vector<int> vertex_ordering;
    map<Edge, vector<Face>> edge_to_face;
    map<int, vector<int>> face_adjL;
    map<int, vector<int>> v_adjL;
    vector<Face> red_faces;
    vector<Face> blue_faces;
    vector<vector<int>> red_face_vertices;
    vector<vector<int>> blue_face_vertices;
    void create_graph(map<int, vector<int>> adjL);
    void bfs();
    void color_dfs();
    void color_dfs_util(int face, int color, bool visited[]);

public:
    CC_Embedded_Graph();
    CC_Embedded_Graph(map<int, vector<int>>& adjL);
    int getFaceCount() const { return face_count; }
    int getEdgeCount() const { return edge_count; }
    int getVertexCount() const { return vertex_count; }
    vector<Face> getFaces() { return faces; }
    vector<int> getVertexOrdering() { return vertex_ordering; }
    vector<Face> getRedFaces() { return red_faces; }
    vector<Face> getBlueFaces() { return blue_faces; }
    vector<vector<int>> getRedFaceV() { return red_face_vertices; }
    vector<vector<int>> getBlueFaceV() { return blue_face_vertices; }
    void calcRedFaces(vector<Face>&);
    void calcBlueFaces(vector<Face>&);

};

#endif //RATS_CC_EMBEDDED_GRAPH_H
