/*
 * ERDOS: Eulerian Routing of DNA origami Scaffolds
 * This program implements the automated design of toroidal DNA polyhedra for A-trail scaffold routings.
 */

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <cmath>
#include "ply_to_embedding.h"
#include "make_cc.h"
#include "edgecode.h"
#include "cc_embedded_graph.h"
#include "bb_covering_tree.h"
#include "find_a_trail.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif

using namespace std;

int main(int argc, char *argv[]) {
    string file_name;                   // Target ply file
    string shape;                       // Desired shape name
    int first_color;                    // First covering tree to search
    vector<vector<double>> vertices;    // Vertex coordinates
    vector<vector<int>> faces;          // Faces as vector of vertices
    set<vector<int>> edges;             // Edges as pair of vertices
    vector<vector<int>> double_edges;   // Double edges as pair of vertices
    map<int, vector<int>> adjL;         // Vertex-to-edge adjacency list
    vector<int> ver_stack;              // Covering tree vertices
    vector<int> v_order;                // BFS ordering of graph vertices
    vector<Edge> a_trail;               // A-trail as a vector of edges
    int branches;                       // Number of branches/threads
    bool useCheckPoints = false;        // Whether or not to produce checkpoint files
    bool checkPointGiven = false;       // Whether or not checkpoint file is given
    vector<string> checkpoints;         // Checkpoint files

    // Iteration tracking variables
    unsigned long long int iterationCount = 0;
    auto start = chrono::high_resolution_clock::now();

    // Program Welcome Message
    cout << string(50, '=') << endl;
    cout << "Welcome to ERDOS (Eulerian Routing of DNA Origami Scaffolds)! This program is used for automating the design of finding A-trail scaffold routings for toroidal DNA polyhedra. Please refer to https://github.com/JQBNguyen/ERDOS for more information and general-use guidelines." << endl;
    cout << string(50, '=') << endl;

    // All Command Arguments Given
    if (argc >= 5) {
        file_name = argv[1];
        shape = argv[2];
        branches = pow(2, (int)log2(stoi(argv[3])));
        useCheckPoints = stoi(argv[4]);
    } //endif
    if (argc >= 6) {
        checkPointGiven = true;
        for (int i = 5; i < argc; ++i) {
            checkpoints.emplace_back(argv[i]);
        } //endfor
    } //endif

    // Insufficient Arguments Given
    if (argc < 5) {
        cout << "Insufficient arguments given.\n";
        cout << "Sample expected input: ./ERDOS <input_PLY_file> <given_shape_name> <number_of_parallel_branches> <produce_checkpoint_files> <list_of_given_checkpoint_files>(optional).\n";
        cout << "Proceed with manual input of files and options below." << endl;

        cerr << "Please input file path name (provided file should be of PLY format)." << endl;
        cin >> file_name;

        cerr << "Please input desired shape name for output files. (any output files will use this as an identifier)" << endl;
        cin >> shape;

        cerr << "Please input number of branches (number of parallel processing branches to speed up covering tree search)." << endl;
        int b;
        cin >> b;
        while (cin.fail()) {
            cin.clear();
            cin.ignore(9999, '\n');
            cerr << "Try again. Please input an integer." << endl;
            cin >> b;
        } //endwhile
        branches = pow(2, (int)log2(b));

        cerr << "Do you want to produce checkpoint files (periodically store covering tree search information to save progress and rerun ERDOS at later times)? (Y/N)" << endl;
        char cp;
        cin >> cp;
        if (cp == 'y' || cp == 'Y') {
            useCheckPoints = true;
        }

        cerr << "Are you providing checkpoint files? (Y/N)" << endl;
        cin >> cp;
        string checkpointFile;
        if (cp == 'y' || cp == 'Y') {
            while (checkpointFile != "q" && checkpointFile != "Q") {
                cerr << "Please input checkpoint file path name. (Input q/Q to stop inputting)" << endl;
                cin >> checkpointFile;
                if (checkpointFile != "q" && checkpointFile != "Q") {
                    checkpoints.push_back(checkpointFile);
                    checkPointGiven = true;
                } //endif
            } //endwhile
        } //endif
    }

    // Reads ply file
    cout << string(50, '=') << endl;
    cout << "Parsing through provided PLY file to read polyhedron information (vertex, edge, face information) ..." << endl;
    if(!ply_to_embedding(file_name, vertices, faces)) {
        cerr << "Failed to read ply file." << endl;
        exit(1);
    } //endif
    cout << "FINISHED" << endl;
    cout << string(50, '=') << endl;

    // Makes mesh checkerboard-colorable
    cout << string(50, '=') << endl;
    cout << "Checking whether or not the provided mesh is checkerboard-colorable (faces can each be colored one of either 2 colors without same-colored faces being adjacent to one another). If not, add \"double edges\" to fulfill requirement ..." << endl;
    makecc(faces, edges, double_edges);
    cout << "FINISHED" << endl;
    cout << string(50, '=') << endl;

    // Creates vertex-to-edge adjacency list to represent mesh
    cout << string(50, '=') << endl;
    cout << "Creating adjacency list representation of mesh for processing purposes ..." << endl;
    create_adjL(vertices, faces, edges, double_edges, adjL);
    cout << "There were " << double_edges.size() << " double edges added to the mesh." << endl;
    cout << "FINISHED" << endl;
    cout << string(50, '=') << endl;

    // Creates embedded graph object
    cout << string(50, '=') << endl;
    cout << "Creating embedded graph representation of mesh for processing purposes ..." << endl;
    CC_Embedded_Graph eg = CC_Embedded_Graph(adjL);
    cout << "FINISHED" << endl;
    cout << string(50, '=') << endl;

    // BFS ordering of graph vertices
    cout << string(50, '=') << endl;
    cout << "Performing BFS (breadth-first-search) on mesh to determine a vertex ordering for processing purposes ..." << endl;
    v_order = eg.getVertexOrdering();
    cout << "FINISHED" << endl;
    cout << string(50, '=') << endl;

    // Covering tree search
    cout << string(50, '=') << endl;
    cout << "Beginning covering tree search ..." << endl;

#ifdef USE_OPENMP
    omp_set_num_threads(branches);
    if (!checkPointGiven) { // Covering tree search from beginning
        if (branches <= 1) { // Serial search
            // User chooses which color face to search first
            cout << "Input 1 (red) or 0 (blue)" << endl;
            cin >> first_color;
            while (first_color != 1 && first_color != 0) {
                cout << "Input 1 (red) or 0 (blue)";
                cin >> first_color;
            } //endwhile

            vector<int> ver_choice;
            int color = first_color;
            start = chrono::high_resolution_clock::now();

            // First face color search
            bool has_covering_tree = bb_covering_tree(eg, -1, 1, ver_stack, first_color, v_order,
                                                      iterationCount, start, 0, shape, useCheckPoints);
            if (has_covering_tree) {
                // Covering tree vertices
                for (int j = 0; j < ver_stack.size(); ++j) {
                    ver_choice.push_back(v_order[ver_stack[j]]);
                } //endfor

                // A-trail
                if (!ver_choice.empty()) {
                    find_ATrail(eg, a_trail, ver_choice, first_color, shape + "_1");
                    bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                    write_atrail_to_file(a_trail, first_color, shape + "_1", has_crossing_staples);
                } //endif

                // Output vertex stack and covering tree vertices to console
                cout << endl;
                cout << "Covering tree found ..." << endl;
                cout << "Vertex stack (" << (color ? "red" : "blue") << ") :";
                for (auto v: ver_stack) {
                    cout << v << " ";
                } //endfor
                cout << endl;
                cout << "Covering tree vertices (" << (color ? "red" : "blue") << ") :";
                for (auto v: ver_choice) {
                    cout << v << " ";
                } //endfor
                cout << endl;
            }
            else {
                // Search second face color
                ver_stack.clear();
                ver_choice.clear();
                iterationCount = 0;
                start = chrono::high_resolution_clock::now();
                has_covering_tree = bb_covering_tree(eg, -1, 1, ver_stack, (first_color + 1) % 2, v_order,
                                                     iterationCount, start, 0, shape, useCheckPoints);
                if (has_covering_tree) {
                    color = (first_color + 1) % 2;
                    // Covering tree vertices
                    for (int j = 0; j < ver_stack.size(); ++j) {
                        ver_choice.push_back(v_order[ver_stack[j]]);
                    } //endfor

                    // A-trail
                    if (!ver_choice.empty()) {
                        find_ATrail(eg, a_trail, ver_choice, color, shape + "_0");
                        bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                        write_atrail_to_file(a_trail, color, shape + "_0", has_crossing_staples);
                    } //endfor

                    // Output vertex stack and covering tree vertices to console
                    cout << endl;
                    cout << "Covering tree found ..." << endl;
                    cout << "Vertex stack (" << (color ? "red" : "blue") << ") :";
                    for (auto v: ver_stack) {
                        cout << v << " ";
                    } //endfor
                    cout << endl;
                    cout << "Covering tree vertices (" << (color ? "red" : "blue") << ") :";
                    for (auto v: ver_choice) {
                        cout << v << " ";
                    } //endfor
                    cout << endl;
                }
            }
        } else {
            // Searching for covering tree (parallelized)
            #pragma omp parallel for shared(v_order, branches, eg) private(ver_stack, a_trail, iterationCount, start)
            for (int i = 0; i < branches; ++i) {
                int color = i % 2;

                // Populate different starting ver_stack based on branch
                int binary = i / 2;
                for (int j = 0; j < (branches / 2); ++j) {
                    if ((binary >> j) & 1) {
                        ver_stack.push_back(j);
                    } //endif
                } //endfor

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
                } //endif

                int ver_stack_initial_count = ver_stack.size();

                // Find covering tree
                if (cont) {
                    // Start search at next vertex
                    start = chrono::high_resolution_clock::now();
                    bool has_covering_tree = bb_covering_tree(eg, log2(branches / 2), 1, ver_stack, color, v_order,
                                                              iterationCount, start, i, shape, useCheckPoints);
                    if (!has_covering_tree)
                        bb_covering_tree(eg, log2(branches / 2), 0, ver_stack, color, v_order, iterationCount, start, i,
                                         shape, useCheckPoints);

                    // Clear initial vertex stack if no covering tree found
                    if (ver_stack.size() == ver_stack_initial_count) {
                        ver_stack.clear();
                    } //endif
                } //endif

                // Covering tree vertices
                vector<int> ver_choice;
                for (int j = 0; j < ver_stack.size(); ++j) {
                    ver_choice.push_back(v_order[ver_stack[j]]);
                } //endfor

                // A-trail
                if (!ver_choice.empty()) {
                    find_ATrail(eg, a_trail, ver_choice, color, shape + "_" + to_string(i));
                    bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                    write_atrail_to_file(a_trail, color, shape + "_" + to_string(i), has_crossing_staples);
                } //endif

                // Output vertex stack and covering tree vertices to console
                #pragma omp critical
                {
                    cout << endl;
                    cout << "Covering tree found ..." << endl;
                    cout << "Vertex stack (" << (color ? "red" : "blue") << ") branch " << i << ": ";
                    for (auto v: ver_stack) {
                        cout << v << " ";
                    } //endfor
                    cout << endl;
                    cout << "Covering tree vertices (" << (color ? "red" : "blue") << ") branch " << i << ": ";
                    for (auto v: ver_choice) {
                        cout << v << " ";
                    } //endfor
                    cout << endl;
                }
            } //endfor
        } //endif
    }
    else if (checkPointGiven) { // Start search from checkpoint file(s)
        // Loop through given checkpoint files
        #pragma omp parallel for shared(v_order, eg) private(ver_stack, a_trail, iterationCount, start)
        for (int i = 0; i < checkpoints.size(); ++i) {
            string line, n;
            ifstream myFile;
            myFile.open(checkpoints[i]);

            int v, choice, face_color, branchNum;

            // Read checkpoint file
            if (myFile.is_open()) {
                if (getline(myFile, line)) {
                    istringstream stream(line);
                    while (stream >> n) {
                        if (n == "|") {
                            break;
                        } //endif
                        ver_stack.push_back(stoi(n));
                    } //endwhile
                    stream >> n;
                    v = stoi(n);
                    stream >> n;
                    choice = stoi(n);
                    stream >> n;
                    face_color = stoi(n);
                    stream >> n;
                    branchNum = stoi(n);
                } //endif
            } //endif
            myFile.close();

            start = chrono::high_resolution_clock::now();
            bool has_covering_tree = bb_covering_tree(eg, v, choice, ver_stack, face_color, v_order,
                                                          iterationCount, start, branchNum, shape, useCheckPoints);

            // Covering tree vertices
            vector<int> ver_choice;
            for (int j = 0; j < ver_stack.size(); ++j) {
                ver_choice.push_back(v_order[ver_stack[j]]);
            } //endfor

            // A-trail
            if (!ver_choice.empty()) {
                find_ATrail(eg, a_trail, ver_choice, face_color, shape + "_" + to_string(branchNum));
                bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                write_atrail_to_file(a_trail, face_color, shape + "_" + to_string(branchNum), has_crossing_staples);

            } //endif

            // Output vertex stack and covering tree vertices to console
            #pragma omp critical
            {
                cout << endl;
                cout << "Covering tree found ..." << endl;
                cout << "Vertex stack (" << (face_color ? "red" : "blue") << ") branch " << i << ": ";
                for (auto v: ver_stack) {
                    cout << v << " ";
                } //endfor
                cout << endl;
                cout << "Covering tree vertices (" << (face_color ? "red" : "blue") << ") branch " << i << ": ";
                for (auto v: ver_choice) {
                    cout << v << " ";
                } //endfor
                cout << endl;
            }
        } //endfor
    } //endif
#else
    if (!checkPointGiven) { // Covering tree search from beginning
        // User chooses which color face to search first
        cout << "Input 1 (red) or 0 (blue)";
        cin >> first_color;
        while (first_color != 1 && first_color != 0) {
            cout << "Input 1 (red) or 0 (blue)";
            cin >> first_color;
        } //endwhile

        vector<int> ver_choice;
        int color = first_color;
        start = chrono::high_resolution_clock::now();

        // First face color search
        bool has_covering_tree = bb_covering_tree(eg, -1, 1, ver_stack, first_color, v_order,
                                                      iterationCount, start, 0, shape, useCheckPoints);

        // Search second face color
        if (!has_covering_tree) {
            has_covering_tree = bb_covering_tree(eg, -1, 1, ver_stack, (first_color + 1) % 2, v_order,
                                                     iterationCount, start, 0, shape, useCheckPoints);
            if (has_covering_tree) color = (first_color + 1) % 2;
            // Covering tree vertices
            for (int j = 0; j < ver_stack.size(); ++j) {
                ver_choice.push_back(v_order[ver_stack[j]]);
            } //endfor

            // A-trail
            if (!ver_choice.empty()) {
                find_ATrail(eg, a_trail, ver_choice, first_color, shape + "_0");
                bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                write_atrail_to_file(a_trail, first_color, shape + "_0", has_crossing_staples);
            } //endfor
        } else {
            // Covering tree vertices
            for (int j = 0; j < ver_stack.size(); ++j) {
                ver_choice.push_back(v_order[ver_stack[j]]);
            } //endfor

            // A-trail
            if (!ver_choice.empty()) {
                find_ATrail(eg, a_trail, ver_choice, (first_color + 1) % 2, shape + "_1");
                bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                write_atrail_to_file(a_trail, (first_color + 1) % 2, shape + "_1", has_crossing_staples);
            } //endif
        } //endif

        // Output vertex stack and covering tree vertices to console
        cout << endl;
        cout << "Covering tree found ..." << endl;
        cout << "Vertex stack (" << (color ? "red" : "blue") << ") :";
        for (auto v: ver_stack) {
            cout << v << " ";
        } //endfor
        cout << endl;
        cout << "Covering tree vertices (" << (color ? "red" : "blue") << ") :";
        for (auto v: ver_choice) {
            cout << v << " ";
        } //endfor
        // cout << endl;
    }
    else if (checkPointGiven) { // Start search from checkpoint file(s)
        // Loop through given checkpoint files
        for (int i = 0; i < checkpoints.size(); ++i) {
            string line, n;
            ifstream myFile;
            myFile.open(checkpoints[i]);

            int v, choice, face_color, branchNum;

            // Read checkpoint file
            if (myFile.is_open()) {
                if (getline(myFile, line)) {
                    istringstream stream(line);
                    while (stream >> n) {
                        if (n == "|") {
                            break;
                        } //endif
                        ver_stack.push_back(stoi(n));
                    } //endwhile
                    stream >> n;
                    v = stoi(n);
                    stream >> n;
                    choice = stoi(n);
                    stream >> n;
                    face_color = stoi(n);
                    stream >> n;
                    branchNum = stoi(n);
                } //endif
            } //endif
            myFile.close();

            start = chrono::high_resolution_clock::now();
            bool has_covering_tree = bb_covering_tree(eg, v, choice, ver_stack, face_color, v_order,
                                                          iterationCount, start, branchNum, shape, useCheckPoints);

            // Covering tree vertices
            vector<int> ver_choice;
            for (int j = 0; j < ver_stack.size(); ++j) {
                ver_choice.push_back(v_order[ver_stack[j]]);
            } //endfor

            // A-trail
            if (!ver_choice.empty()) {
                find_ATrail(eg, a_trail, ver_choice, face_color, shape + "_" + to_string(branchNum));
                bool has_crossing_staples = check_crossing_staples(adjL, a_trail);
                write_atrail_to_file(a_trail, face_color, shape + "_" + to_string(branchNum), has_crossing_staples);
            } //endif

            // Output vertex stack and covering tree vertices to console
            #pragma omp critical
            {
                cout << endl;
                cout << "Covering tree found ..." << endl;
                cout << "Vertex stack (" << (face_color ? "red" : "blue") << ") branch " << i << ": ";
                for (auto v: ver_stack) {
                    cout << v << " ";
                } //endfor
                cout << endl;
                cout << "Covering tree vertices (" << (face_color ? "red" : "blue") << ") branch " << i << ": ";
                for (auto v: ver_choice) {
                    cout << v << " ";
                } //endfor
                cout << endl;
            }
        } //endfor
    } //endif
#endif

    cout << "FINISHED" << endl;
    cout << string(50, '=') << endl;
    cout << string(50, '=') << endl;

    return 0;
}
