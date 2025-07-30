#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "ply_to_embedding.h"

using namespace std;

/*
 * Reads and parses ply file for vertex and face information.
 *
 * @param file_name String representing the ply file to be read.
 * @param vertices Container to hold vertex information.
 * @param faces Container to hold face information.
 * @return bool Whether or not file was successfully read.
 */
bool ply_to_embedding(string file_name, vector<vector<double>>& vertices, vector<vector<int>>& faces) {

    string line;
    ifstream my_file;
    size_t vertex_count, face_count;

    my_file.open(file_name); // Opens file

    if (my_file.is_open()) {
        
        // Checks whether or not file is a ply file
        getline(my_file, line);
        if (line.substr(0, 3).compare("ply")) {
            cerr << "ERROR - File is not a PLY file." << endl;
            return false;
        }//endif

        // Reads file line by line
        while (my_file.good()) {

            getline(my_file, line);

            // // Checks ply file format
            // if (!line.substr(0, 6).compare("format")) {
            //     if (line.substr(7, 12).compare("ascii")) {
            //         cerr << "ERROR - The PLY file is not ascii format." << endl;
            //         return false;
            //     }
            // }//endif

            // Checks vertex count
            if (!line.substr(0, 14).compare("element vertex")) {
                vertex_count = stoi(line.substr(15));
                cout << "There are " << vertex_count << " vertices" << endl;
            }//endif

            // Checks face count
            if (!line.substr(0, 12).compare("element face")) {
                face_count = stoi(line.substr(13));
                cout << "There are " << face_count << " faces." << endl;
            }//endif

            // Starts reading for vertex and face information if "end_header" reached
            if (!line.substr(0, 10).compare("end_header")) {

                // Reads vertices (coordinates)
                for (int i = 0; i < vertex_count; ++i) {
                    getline(my_file, line);
                    double coord;
                    stringstream s;
                    string temp;
                    vector<double> coords;
                    s << line;
                    while (!s.eof()) {
                        s >> temp;
                        if (stringstream(temp) >> coord) {
                            coords.push_back(coord);
                        }
                        temp = "";
                    }//endwhile
                    vertices.push_back(coords);
                }//endfor

                // Reads faces (list of vertices)
                for (int i = 0; i < face_count; ++i) {
                    getline(my_file, line);
                    int vertex;
                    stringstream s;
                    string temp;
                    vector<int> vertices;
                    s << line;
                    s >> temp;
                    while (!s.eof()) {
                        s >> temp;
                        if (stringstream(temp) >> vertex) {
                            vertices.push_back(vertex);
                        }
                        temp = "";
                    }//endwhile
                    faces.push_back(vertices);
                }//endfor
            } //endif
        }
        my_file.close();
    }
    else {
        cerr << "ERROR - Cannot open specified file: " << file_name << endl;
        return false;
    }//endif

    return true;
}
