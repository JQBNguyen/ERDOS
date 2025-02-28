/*
 * ERDOS: Eulerian Routing of DNA origami Scaffolds
 * This program implements the automated design of toroidal DNA polyhedra for A-trail scaffold routings.
 */

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <omp.h>
#include <bits/stdc++.h>
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
    string first_color;                    // First covering tree to search
    vector<vector<double>> vertices;    // Vertex coordinates
    vector<vector<int>> faces;          // Faces as vector of vertices
    set<vector<int>> edges;             // Edges as pair of vertices
    vector<vector<int>> double_edges;   // Double edges as pair of vertices
    map<int, vector<int>> adjL;         // Vertex-to-edge adjacency list
    vector<int> ver_stack;              // Covering tree vertices
    vector<int> v_order;                // BFS ordering of graph vertices
    vector<Edge> a_trail;               // A-trail as a vector of edges
    int branches;

    // Checking whether arguments are given
    if (argc <= 1) { // No arguments given
        cerr << "No file given. Please input file name." << endl;
        cin >> file_name;
        cerr << "No shape name given. Please input desired shape name." << endl;
        cin >> shape;
        cerr << "Branch number not specified. Please input number of branches." << endl;
        int b;
        cin >> b;
        branches = pow(2, (int)log2(b));
    }
    else if (argc <= 2) { // Only one argument given
        file_name = argv[1];
        cerr << "No shape name given. Please input desired shape name." << endl;
        cin >> shape;
        cerr << "Branch number not specified. Please input number of branches." << endl;
        int b;
        cin >> b;
        branches = pow(2, (int)log2(b));
    }
    else if (argc <= 3) {
        file_name = argv[1];
        shape = argv[2];
        cerr << "Branch number not specified. Please input number of branches." << endl;
        int b;
        cin >> b;
        branches = pow(2, (int)log2(b));
    }
    else { // Arguments given
        file_name = argv[1];
        shape = argv[2];
        branches = pow(2, (int)log2(stoi(argv[3])));
    } //endif

    // Detects if excessive arguments are given
    if (argc >= 5) {
        cerr << "WARNING - More arguments given then necessary." << endl;
    } //endif

    // Reads ply file
    cout << "Reading ply file ..." << endl;
    if(!ply_to_embedding(file_name, vertices, faces)) {
        exit(1);
    }

    // Makes mesh checkerboard-colorable
    cout << "Running makecc ..." << endl;
    makecc(faces, edges, double_edges);

    // Creates vertex-to-edge adjacency list to represent mesh
    cout << "Running create_adjL ..." << endl;
    create_adjL(vertices, faces, edges, double_edges, adjL);
    cout << "Double Edge Count: " << double_edges.size() << endl;

    // Creates embedded graph object
    cout << "Creating embedded graph ..." << endl;
    CC_Embedded_Graph eg = CC_Embedded_Graph(adjL);

    // BFS ordering of graph vertices
    cout << "Getting vertex ordering ..." << endl;
    v_order = eg.getVertexOrdering();

    // Searching for covering tree
    cout << "Searching covering tree(s) ..." << endl;
    #pragma omp parallel for shared(v_order, branches, eg) private(ver_stack, a_trail)
    for (int i = 0; i < branches; ++i) {
        int color = i % 2;

        // Populate different starting ver_stack based on branch
        int binary = i / 2;
        for (int j = 0; j < (branches / 2); ++j) {
            if ((binary >> j) & 1) {
                ver_stack.push_back(j);
            }
        }

        // Check starting covering tree validity
        vector<vector<int>> covering_tree;
        bool covers_all_colored_faces = full_tree_test(eg, ver_stack, color, covering_tree, v_order);
        bool has_cycle = test_for_cycle(covering_tree);

        // Stop branch condition
        bool cont = true;
        if (!has_cycle && covers_all_colored_faces) {
            cont = false;
        }
        else if (has_cycle || eg.getVertexCount() <= ver_stack.size()) {
            cont = false;
            ver_stack.clear();
        }

        int ver_stack_initial_count = ver_stack.size();

        // Find covering tree
        if (cont) {
            // Start search at next vertex
            bool has_covering_tree = bb_covering_tree(eg, log2(branches / 2), 1, ver_stack, color, v_order);
            if (!has_covering_tree) bb_covering_tree(eg, log2(branches / 2), 0, ver_stack, color, v_order);

            // Clear initial vertex stack if no covering tree found
            if (ver_stack.size() == ver_stack_initial_count) {
                ver_stack.clear();
            }
        }

        // Covering tree vertices
        vector<int> ver_choice;
        for (int j = 0; j < ver_stack.size(); ++j) {
            ver_choice.push_back(v_order[ver_stack[j]]);
        }

        // A-trail
        if (!ver_choice.empty()) {
            find_ATrail(eg, a_trail, ver_choice, color, shape + "_" + to_string(i));
        }

        #pragma omp critical
        {
            cout << endl;
            cout << "Vertex stack (" << (color ? "red" : "blue") << ") branch " << i << ": ";
            for (auto v: ver_stack) {
                cout << v << " ";
            }
            cout << endl;
            cout << "Covering tree vertices (" << (color ? "red" : "blue") << ") branch " << i << ": ";
            for (auto v: ver_choice) {
                cout << v << " ";
            }
            cout << endl;
        }
    }

    return 0;
}
