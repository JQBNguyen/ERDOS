#ifndef RATS_FIND_A_TRAIL_H
#define RATS_FIND_A_TRAIL_H

using namespace std;

void find_ATrail(CC_Embedded_Graph& eg, vector<Edge>& a_trail, vector<int>& ver_choice, int color, string shape);

bool check_crossing_staples(map<int, vector<int>>& adjL, vector<Edge>& a_trail);

void write_atrail_to_file(vector<Edge>& a_trail, int color, string shape, bool crossing_staples);

#endif //RATS_FIND_A_TRAIL_H
