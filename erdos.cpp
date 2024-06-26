/*
 * ERDOS: Eulerian Routing of DNA origami Scaffolds
 * This program implements the automated design of toroidal DNA polyhedra for A-trail scaffold routings.
 */

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include "ply_to_embedding.h"
#include "make_cc.h"
#include "edgecode.h"
#include "cc_embedded_graph.h"
#include "bb_covering_tree.h"
#include "find_a_trail.h"

using namespace std;

int main(int argc, char *argv[]) {
    string file_name;                   // Target ply file
    string shape;                       // Desired shape name
    vector<vector<double>> vertices;    // Vertex coordinates
    vector<vector<int>> faces;          // Faces as vector of vertices
    set<vector<int>> edges;             // Edges as pair of vertices
    vector<vector<int>> double_edges;   // Double edges as pair of vertices
    map<int, vector<int>> adjL;         // Vertex-to-edge adjacency list
    vector<int> ver_stack;              // Covering tree vertices
    vector<int> v_order;                // BFS ordering of graph vertices
    vector<Edge> a_trail;               // A-trail as a vector of edges

    // Checking whether arguments are given
    if (argc <= 1) { // No arguments given
        cerr << "No file given. Please input file name." << endl;
        cin >> file_name;
        cerr << "No shape name given. Please input desired shape name." << endl;
        cin >> shape;
    }
    else if (argc <= 2){ // Only one argument given
        cerr << "No shape name given. Please input desired shape name." << endl;
        cin >> shape;
    }
    else { // No arguments given
        file_name = argv[1];
        shape = argv[2];
    } //endif

    // Detects if excessive arguments are given
    if (argc >= 4) {
        cerr << "WARNING - More arguments given then necessary." << endl;
    } //endif

    // Reads ply file
    if(!ply_to_embedding(file_name, vertices, faces)) {
        exit(1);
    }

    // Makes mesh checkerboard-colorable
    makecc(faces, edges, double_edges);

    // Creates vertex-to-edge adjacency list to represent mesh
    create_adjL(vertices, faces, edges, double_edges, adjL);

    // Creates embedded graph object
    CC_Embedded_Graph eg = CC_Embedded_Graph(adjL);

    // BFS ordering of graph vertices
    v_order = eg.getVertexOrdering();

    // Branch bound search for "red" covering tree vertices 
    bb_covering_tree(eg, -1, 1, ver_stack, 1, v_order); 

    // Checks whether "red" covering tree exists
    if (ver_stack.size() == 0) {
        // Branch bound search for "blue" covering tree vertices
        bb_covering_tree(eg, -1, 1, ver_stack, 0, v_order); 

        if (ver_stack.size() == 0) { // No covering tree was found
            cout << "No covering tree was found. Exiting program." << endl;
            return 0;
        } //endif

        // Finds A-trail for blue covering tree
        find_ATrail(eg, a_trail, ver_stack, 0, shape);
    }
    else {
        // Finds A-trail for red covering tree
        find_ATrail(eg, a_trail, ver_stack, 1, shape);
    } //endif

    return 0;
}
