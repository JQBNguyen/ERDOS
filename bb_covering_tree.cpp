#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <stack>

#include "cc_embedded_graph.h"

/*
 * Depth-first search on graph to check for cycles
 *
 * @param v Current vertex
 * @param visited Visited information of vertices
 * @param parent Parent vertex of current vertex
 * @param covering_tree Graph to search
 * @return Whether a cycle is present.
 */
bool dfs_cycle(int v, map<int, bool>& visited, int parent, map<int, vector<int>>& covering_tree) {
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
bool test_for_cycle(map<int, vector<int>>& covering_tree) {
    map<int, bool> visited;
    for_each(covering_tree.begin(), covering_tree.end(), [&visited](pair<int, vector<int>> p) {
        visited[p.first] = false;
    }); //endfor
    vector<int> covering_tree_v;
    for (auto i : covering_tree) {
        covering_tree_v.push_back(i.first);
    } //endfor
    for (int i = 0; i < covering_tree_v.size(); ++i) {
        if (!visited[covering_tree_v[i]]) {
            if (dfs_cycle(covering_tree_v[i], visited, -1, covering_tree)) {
                return true;
            } //endif
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
bool is_connected(map<int, vector<int>>& covering_tree, int v) {
    set<int> visited;
    stack<int> stack;
    stack.push(v);

    while (!stack.empty()) {
        int ver = stack.top();
        stack.pop();
        if(find_if(visited.begin(), visited.end(), [&ver](int i) {
            return ver == i;
        }) == visited.end()) {
            visited.insert(ver);
        } //endif
        for_each(covering_tree[ver].begin(), covering_tree[ver].end(), [&visited, &stack](int j) {
            if (find_if(visited.begin(), visited.end(), [&j](int i) {
                return j == i;
            }) == visited.end())  {
                stack.push(j);
            } //endif
        }); //endfor
    } //endwhile
    return visited.size() == covering_tree.size();
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
bool full_tree_test(CC_Embedded_Graph &eg, vector<int>& ver_stack, int face_color, map<int, vector<int>>& covering_tree) {
    int vertex_face_num = eg.getVertexCount();
    vector<Face> color_face_list;
    if (face_color) {
        color_face_list = eg.getRedFaces();
    }
    else {
        color_face_list = eg.getBlueFaces();
    } //endif
    for (int i = 0; i < color_face_list.size(); ++i) {
        bool covered = false;
        for (int j = 0; j < ver_stack.size(); ++j) {
            if (color_face_list[i].containsV(ver_stack[j])) {
                covering_tree[ver_stack[j]].push_back(vertex_face_num);
                covering_tree[vertex_face_num].push_back(ver_stack[j]);
                covered = true;
            } //endif
        } //endfor
        if (covered) {
//            cout << "Covers" << color_face_list[i];
            ++vertex_face_num;
        } //endif
    } //endfor

    if ((vertex_face_num == (eg.getVertexCount() + color_face_list.size())) && is_connected(covering_tree, ver_stack[0])) {
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
 * @return bool Facilitates  branch-bound decision making.
 */
bool bb_covering_tree(CC_Embedded_Graph &eg, int v, int choice, vector<int>& ver_stack, int face_color, vector<int>& v_order) {
    // Push or pop vertex onto stack
    if (v != -1) {
        if (choice == 1) {
            ver_stack.push_back(v_order[v]);
        }
        else if (choice == 0) {
            ver_stack.pop_back();
        } //endif
    } //endif

    map<int, vector<int>> covering_tree;
    int covers_all_colored_faces = full_tree_test(eg, ver_stack, face_color, covering_tree);

    bool has_cycle = test_for_cycle(covering_tree);

    // Checks if covering tree is found
    if (!has_cycle && covers_all_colored_faces) {
        return true;
    }
    else if (has_cycle || (eg.getVertexCount() - 1) == v) {
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

