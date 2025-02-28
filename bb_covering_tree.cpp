#include <vector>
#include <algorithm>
#include <set>
#include <stack>
#include <chrono>
#include "cc_embedded_graph.h"

int iterationCount = 0;
auto start = chrono::high_resolution_clock::now();
const int interval = 10000000;

/*
 * Depth-first search on graph to check for cycles
 *
 * @param v Current vertex
 * @param visited Visited information of vertices
 * @param parent Parent vertex of current vertex
 * @param covering_tree Graph to search
 * @return Whether a cycle is present.
 */
bool dfs_cycle(int v, vector<bool>& visited, int parent, vector<vector<int>>& covering_tree) {
    visited[v] = true;
    for (int i = 0; i < covering_tree[v].size(); ++i) {
        if (!visited[covering_tree[v][i]]) {
            if (dfs_cycle(covering_tree[v][i], visited, v, covering_tree)) {
                return true;
            } //endif
        }
        else if (parent != covering_tree[v][i]) {
            return true;
        } //endif
    } //endfor
    return false;
}

/*
 * Graph test to detect cycles
 *
 * @param covering_tree Graph to search
 * @return Whether a cycle is present.
 */
bool test_for_cycle(vector<vector<int>>& covering_tree) {
    vector<bool> visited(covering_tree.size(), false);
    for (int i = 0; i < covering_tree.size(); ++i) {
        if (!visited[i]) {
            if (dfs_cycle(i, visited, -1, covering_tree)) return true;
        } //endif
    } //endfor

    return false;
}

/*
 * Test for whether graph is connected by use of depth-first search
 *
 * @param covering_tree Graph to search through
 * @param v Starting vertex
 * @return Whether graph is connected
 */
bool is_connected(vector<vector<int>>& covering_tree, int v) {
    vector<bool> visited(covering_tree.size(), false);
    stack<int> stack;
    stack.push(v);

    while (!stack.empty()) {
        int ver = stack.top();
        stack.pop();

        if (!visited[ver]) {
            visited[ver] = true;
        } //endif

        for (int i = 0; i < covering_tree[ver].size(); ++i) {
            if (!visited[covering_tree[ver][i]]) stack.push(covering_tree[ver][i]);
        } //endfor

    } //endwhile

    return count(visited.begin(), visited.end(), true) == covering_tree.size();
}

/*
 * Tests whether covering tree covers all colored faces of interest
 *
 * @param eg Embedded graph
 * @param ver_stack Current selection of covering tree vertices
 * @param face_color Face color to test
 * @param covering_tree Covering tree to create
 * @return Whether or not the covering tree covers all colored faces of interest
 */
bool full_tree_test(CC_Embedded_Graph &eg, vector<int>& ver_stack, int face_color, vector<vector<int>>& covering_tree, vector<int>& v_order) {
    int vertex_face_num = ver_stack.size();

    vector<vector<int>> face_vertices;
    int color_face_count;
    if (face_color) {
        face_vertices = eg.getRedFaceV();
        vector<Face> red_faces = eg.getRedFaces();
        color_face_count = red_faces.size();
    }
    else {
        face_vertices = eg.getBlueFaceV();
        vector<Face> blue_faces = eg.getBlueFaces();
        color_face_count = blue_faces.size();
    }

    covering_tree = vector<vector<int>>(vertex_face_num + color_face_count);

    for (int i = 0; i < ver_stack.size(); ++i) {
        for (int j = 0; j < face_vertices[v_order[ver_stack[i]]].size(); ++j) {
            covering_tree[i].push_back(vertex_face_num + face_vertices[v_order[ver_stack[i]]][j]);
            covering_tree[vertex_face_num + face_vertices[v_order[ver_stack[i]]][j]].push_back(i);
        }
    }

    covering_tree.erase(remove_if(covering_tree.begin(), covering_tree.end(), [](const vector<int>& v) {
        return v.empty();
    }), covering_tree.end());

    for (int i = 0; i < ver_stack.size(); ++i) {
        covering_tree[i].clear(); // Clear Vertex-to-Face lists for refactoring
    }

    for (int i = ver_stack.size(); i < covering_tree.size(); ++i) {
        for (int j = 0; j < covering_tree[i].size(); ++j) {
            covering_tree[covering_tree[i][j]].push_back(i);
        }
    }

    if ((covering_tree.size() == (ver_stack.size() + color_face_count)) && is_connected(covering_tree, ver_stack[0])) {
        return true;
    }
    else {
        return false;
    } //endif
}

/*
 * Branch-bound algorithm to decide which vertices make up a covering tree
 *
 * @param eg Embedded graph
 * @param v Current vertex ordering index to consider
 * @param choice "Yes" or "No" choice on whether to include vertex
 * @param ver_stack Covering tree vertices
 * @param face_color Face color to consider
 * @param v_order BFS vertex ordering
 * @return bool Facilitates  branch-bound decision-making.
 */
bool bb_covering_tree(CC_Embedded_Graph &eg, int v, int choice, vector<int>& ver_stack, int face_color, vector<int>& v_order) {
    // Time
    ++iterationCount;
    if (iterationCount % interval == 0) {
        cout << iterationCount << ": ";
        auto end = chrono::high_resolution_clock::now();
        auto time = chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << time.count() << "ms" << endl;
        start = chrono::high_resolution_clock::now();
    }

    // Push or pop vertex onto stack
    if (v != -1) {
        if (choice == 1) {
            ver_stack.push_back(v);
        }
        else if (choice == 0) {
            ver_stack.pop_back();
        } //endif
    } //endif

    vector<vector<int>> covering_tree;
    bool covers_all_colored_faces = full_tree_test(eg, ver_stack, face_color, covering_tree, v_order);
    bool has_cycle = test_for_cycle(covering_tree);

    // Checks if covering tree is found
    if (!has_cycle && covers_all_colored_faces) {
        cout << "found" << endl;
        return true;
    }
    else if (has_cycle || (eg.getVertexCount() - 1) <= v) {
        return false;
    }
    else {
        int next_v = v + 1;
        bool has_covering_tree = bb_covering_tree(eg, next_v, 1, ver_stack, face_color, v_order); // "Yes" to next vertex
        if (!has_covering_tree) {
            bb_covering_tree(eg, next_v, 0, ver_stack, face_color, v_order); // "No" to next vertex
        }
        else {
            return true;
        } //endif
    } //endif

    return false;
}

