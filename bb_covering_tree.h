#ifndef RATS_BB_COVERING_TREE_H
#define RATS_BB_COVERING_TREE_H

bool dfs_cycle(int v, vector<bool>& visited, int parent, vector<vector<int>>& covering_tree);
bool test_for_cycle(vector<vector<int>>& covering_tree);
bool is_connected(vector<vector<int>>& covering_tree, int v);
bool full_tree_test(CC_Embedded_Graph &eg, vector<int>& ver_stack, int face_color, vector<vector<int>>& covering_tree, vector<int>& v_order);
bool bb_covering_tree(CC_Embedded_Graph &eg, int v, int choice, vector<int>& ver_stack, int face_color, vector<int>& v_order);

#endif //RATS_BB_COVERING_TREE_H
