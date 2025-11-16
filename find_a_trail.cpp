#include <vector>
#include <algorithm>
#include <fstream>
#include <unordered_map>

#include "cc_embedded_graph.h"

using namespace std;

void write_atrail_to_file(vector<Edge>& a_trail, int color, string shape, bool crossing_staples) {
    // Output file name
    string o = "";
    string o1 = "";
    if (color) {
        o = shape + "_red";
        o1 = shape + "_red";
    } else {
        o = shape + "_blue";
        o1 = shape + "_blue";
    } //endif

    if (crossing_staples) {
        o += "_crossing_staples.txt";
        o1 += "_crossing_staples.ntrail";
    }
    else {
        o += "_no_crossing_staples.txt";
        o1 += "_no_crossing_staples.ntrail";
    } //endif

    // Write A-trail to output file
#pragma omp critical
    {
        ofstream myFile(o);
        for (auto i: a_trail) {
            myFile << i.getV1() + 1 << " ";
        } //endfor
        myFile << a_trail.at(a_trail.size() - 1).getV2() + 1;
        myFile << flush;
        myFile.close();

        ofstream myFile1(o1);
        for (auto i: a_trail) {
            myFile1 << i.getV1() << " ";
        } //endfor
        myFile1 << a_trail.at(a_trail.size() - 1).getV2();
        myFile1 << flush;
        myFile1.close();
    }
}

bool check_crossing_staples(map<int, vector<int>>& adjL, vector<Edge>& a_trail) {
    vector<unordered_map<int, int>> adjL_in_out;

    // Vertex-edge adjacency list with in-out information
    for (int i = 0; i < adjL.size(); ++i) {
        unordered_map<int, int> curr_v;
        for (int j = 0; j < adjL[i].size(); ++j) {
            curr_v[adjL[i][j]] = -1; // -1 as placeholder
        }
        adjL_in_out.push_back(curr_v);
    }

    // Assign in-out values for each edge in a-trail to the 2 corresponding vertices
    for (int i = 0; i < a_trail.size(); ++i) {
        adjL_in_out[a_trail[i].getV1()][a_trail[i].getID()] = 1; // out
        adjL_in_out[a_trail[i].getV2()][a_trail[i].getID()] = 0; // in
    }
}

    // Iterate through adjaceny list in order orientation to check in-out pattern
    for (int i = 0; i < adjL.size(); ++i) {
        for (int j = 0; j < adjL[i].size() - 1; ++j) {
            if (adjL_in_out[i][adjL[i][j]] == adjL_in_out[i][adjL[i][j + 1]]) {
                return true;
            }
        }
        // Edge case check for first and last edge in adjacency list order
        if (adjL_in_out[i][adjL[i][0]] == adjL_in_out[i][adjL[i][adjL[i].size() - 1]]) {
            return true;
        }
    }

    return false;
}

/*
 * Finds A-trail based on given covering tree of graph.
 *
 * @param eg Embedded graph
 * @param a_trail A-trail to be found
 * @param ver_choice Covering tree vertices
 * @param color Color of covering tree
 * @param shape Name of shape to find_atrail for
 */
void find_ATrail(CC_Embedded_Graph& eg, vector<Edge>& a_trail, vector<int>& ver_choice, int color, string shape) {
    vector<Face> faces_1;
    vector<Face> faces_2;

    // Checks for which color of face we are finding A-trail for
    if (color) {
        faces_1 = eg.getRedFaces();
        faces_2 = eg.getBlueFaces();
    } else {
        faces_1 = eg.getBlueFaces();
        faces_2 = eg.getRedFaces();
    } //endif

    // Determines starting edge and face
    Edge curr_e = Edge(-1, -1, -1);
    bool flag = false;
    for (int i = 0; i < faces_1[0].getEdges().size(); ++i) {
        for (int j = 0; j < ver_choice.size(); ++j) {
            if (faces_1[0].getEdges()[i].getV1() == ver_choice[j]) {
                curr_e = faces_1[0].getEdges()[i];
                a_trail.push_back(curr_e);
                flag = true;
                break;
            } //endif
        } //endfor
        if (flag) {
            break;
        } //endif
    } //endfor
    Face curr_f = faces_1[0];

    while (a_trail.size() != eg.getEdgeCount()) {
        if (curr_f.getColor() == color) { // If current face is color of interest
            if (find_if(ver_choice.begin(), ver_choice.end(), [&curr_e](int v) {
                return curr_e.getV2() == v;
            }) != ver_choice.end()) { // Wraps around opposing colored face if current vertex is in covering tree
                for (int i = 0; i < faces_2.size(); ++i) {
                    bool flag = false;
                    for (int j = 0; j < faces_2[i].getEdges().size(); ++j) {
                        if (curr_e == faces_2[i].getEdges()[j]) {
                            curr_f = faces_2[i];
                            if (j == 0) {
                                j = curr_f.getEdges().size();
                            } //endif
                            curr_e = curr_f.getEdges()[j - 1];
                            a_trail.push_back(Edge(curr_e.getV2(), curr_e.getV1(), curr_e.getID()));
                            flag = true;
                            break;
                        } //endif
                    } //endfor
                    if (flag) {
                        break;
                    } //endif
                } //endfor
            } else { // Wraps around current face if current vertex is NOT in covering tree
                int eInd = -1;
                for (int i = 0; i < curr_f.getEdges().size(); ++i) {
                    if (curr_f.getEdges()[i] == curr_e) {
                        eInd = i;
                        break;
                    } //endif
                } //endfor
                curr_e = curr_f.getEdges()[(eInd + 1) % curr_f.getEdges().size()];
                a_trail.push_back(curr_e);
            } //endif
        } else if (curr_f.getColor() != color) { // If face is NOT color of interest
            if (find_if(ver_choice.begin(), ver_choice.end(), [&curr_e](int v) {
                return curr_e.getV1() == v;
            }) != ver_choice.end()) { // Wraps around current face if current vertex is in covering tree
                int eInd = -1;
                for (int i = 0; i < curr_f.getEdges().size(); ++i) {
                    if (curr_f.getEdges()[i] == curr_e) {
                        eInd = i;
                        break;
                    } //endif
                } //endfor
                if (eInd == 0) {
                    eInd = curr_f.getEdges().size();
                } //endif
                curr_e = curr_f.getEdges()[eInd - 1];
                a_trail.push_back(Edge(curr_e.getV2(), curr_e.getV1(), curr_e.getID()));
            } else { // Wraps around opposing colored face if current vertex is NOT in covering tree
                for (int i = 0; i < faces_1.size(); ++i) {
                    bool flag = false;
                    for (int j = 0; j < faces_1[i].getEdges().size(); ++j) {
                        if (curr_e == faces_1[i].getEdges()[j]) {
                            curr_f = faces_1[i];
                            curr_e = curr_f.getEdges()[(j + 1) % curr_f.getEdges().size()];
                            a_trail.push_back(curr_e);
                            flag = true;
                            break;
                        } //endif
                    } //endfor
                    if (flag) {
                        break;
                    } //endif
                } //endfor
            } //endif
        } //endif
    } //endwhile
}