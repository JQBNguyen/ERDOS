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
    int first_color;                 // First covering tree to search
    vector<vector<double>> vertices;    // Vertex coordinates
    vector<vector<int>> faces;          // Faces as vector of vertices
    set<vector<int>> edges;             // Edges as pair of vertices
    vector<vector<int>> double_edges;   // Double edges as pair of vertices
    map<int, vector<int>> adjL;         // Vertex-to-edge adjacency list
    vector<int> ver_stack;              // Covering tree vertices
    vector<int> v_order;                // BFS ordering of graph vertices
    vector<Edge> a_trail;               // A-trail as a vector of edges
    int branches;
    bool checkPointGiven = false;

    unsigned long long int iterationCount = 0;
    auto start = chrono::high_resolution_clock::now();

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
    else if (argc <= 4){ // Arguments given
        file_name = argv[1];
        shape = argv[2];
        branches = pow(2, (int)log2(stoi(argv[3])));
    }
    else if (argc >= 5) {
        file_name = argv[1];
        shape = argv[2];
        checkPointGiven = true;
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

    if (!checkPointGiven) {
        if (branches <= 1) {
            cout << "Input 1 (red) or 0 (blue)";
            cin >> first_color;
            while (first_color != 1 && first_color != 0) {
                cout << "Input 1 (red) or 0 (blue)";
                cin >> first_color;
            }
            vector<int> ver_choice;
            int color = first_color;
            start = chrono::high_resolution_clock::now();
            bool has_covering_tree = bb_covering_tree(eg, -1, 1, ver_stack, first_color, v_order,
                                                      iterationCount, start, 0, shape);
            if (!has_covering_tree) {
                has_covering_tree = bb_covering_tree(eg, -1, 1, ver_stack, (first_color + 1) % 2, v_order,
                                                     iterationCount, start, 0, shape);
                if (has_covering_tree) color = (first_color + 1) % 2;
                // Covering tree vertices
                for (int j = 0; j < ver_stack.size(); ++j) {
                    ver_choice.push_back(v_order[ver_stack[j]]);
                }

                // A-trail
                if (!ver_choice.empty()) {
                    find_ATrail(eg, a_trail, ver_choice, first_color, shape + "_0");
                }
            } else {
                // Covering tree vertices
                for (int j = 0; j < ver_stack.size(); ++j) {
                    ver_choice.push_back(v_order[ver_stack[j]]);
                }

                // A-trail
                if (!ver_choice.empty()) {
                    find_ATrail(eg, a_trail, ver_choice, (first_color + 1) % 2, shape + "_1");
                }
            }

            cout << endl;
            cout << "Vertex stack (" << (color ? "red" : "blue") << ") :";
            for (auto v: ver_stack) {
                cout << v << " ";
            }
            cout << endl;
            cout << "Covering tree vertices (" << (color ? "red" : "blue") << ") :";
            for (auto v: ver_choice) {
                cout << v << " ";
            }
            cout << endl;
        } else {
            // Searching for covering tree
            cout << "Searching covering tree(s) ..." << endl;
            #pragma omp parallel for shared(v_order, branches, eg) private(ver_stack, a_trail, iterationCount, start)
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
                } else if (has_cycle || eg.getVertexCount() <= ver_stack.size()) {
                    cont = false;
                    ver_stack.clear();
                }

                int ver_stack_initial_count = ver_stack.size();

                // Find covering tree
                if (cont) {
                    // Start search at next vertex
                    start = chrono::high_resolution_clock::now();
                    bool has_covering_tree = bb_covering_tree(eg, log2(branches / 2), 1, ver_stack, color, v_order,
                                                              iterationCount, start, i, shape);
                    if (!has_covering_tree)
                        bb_covering_tree(eg, log2(branches / 2), 0, ver_stack, color, v_order, iterationCount, start, i,
                                         shape);

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
        }
    }
    else if (checkPointGiven) {
        // Loop through given checkpoint files
        #pragma omp parallel for shared(v_order, eg) private(ver_stack, a_trail, iterationCount, start)
        {
            for (int i = 4; i < argc + 1; ++i) {
                string n;
                ifstream myFile;
                myFile.open(argv[i]);

                int v, choice, face_color, branchNum;

                if (myFile.is_open()) {
                    while (getline(myFile, n, ' ')) {
                        if (n == "|") {
                            break;
                        }
                        ver_stack.push_back(stoi(n));
                    }
                    getline(myFile, n, ' ');
                    v = stoi(n);
                    getline(myFile, n, ' ');
                    choice = stoi(n);
                    getline(myFile, n, ' ');
                    face_color = stoi(n);
                    getline(myFile, n, ' ');
                    branchNum = stoi(n);
                }
                myFile.close();

                start = chrono::high_resolution_clock::now();
                bool has_covering_tree = bb_covering_tree(eg, v, choice, ver_stack, face_color, v_order,
                                                          iterationCount, start, branchNum, shape);

                // Covering tree vertices
                vector<int> ver_choice;
                for (int j = 0; j < ver_stack.size(); ++j) {
                    ver_choice.push_back(v_order[ver_stack[j]]);
                }

                // A-trail
                if (!ver_choice.empty()) {
                    find_ATrail(eg, a_trail, ver_choice, face_color, shape + "_" + to_string(branchNum));
                }

                #pragma omp critical
                {
                    cout << endl;
                    cout << "Vertex stack (" << (face_color ? "red" : "blue") << ") branch " << i << ": ";
                    for (auto v: ver_stack) {
                        cout << v << " ";
                    }
                    cout << endl;
                    cout << "Covering tree vertices (" << (face_color ? "red" : "blue") << ") branch " << i << ": ";
                    for (auto v: ver_choice) {
                        cout << v << " ";
                    }
                    cout << endl;
                }

            }
        }
    }

    return 0;
}
