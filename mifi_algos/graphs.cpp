#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <algorithm>

using namespace std;
using std::vector;
using std::queue;
using std::map;
using std::set;

/// Topological sort
void
dfs_topo(map<char, set<char>> &g, char v, map<char, bool> &used, vector<char> &ans)
{
    used[v] = true;
    for (auto to : g[v]) {
        if (!used[to])
            dfs_topo(g, to, used, ans);
    }
    ans.push_back(v);
}

void
topo_sort(map<char, set<char>> &g, vector<char> &ans)
{
    int n = g.size();
    map<char, bool> used;

    for (auto v : g)
        used[v.first] = false;

    for (auto v : g)
        if (!used[v.first])
            dfs_topo(g, v.first, used, ans);
    reverse(ans.begin(), ans.end());
}

/// Minimal


void MST(vector < vector < int >>
const &g,
vector <vector<int>> &ans
) {
int n = g.size();
const int INF = 1000000000; // значение "бесконечность"
vector<bool> used(n);
vector<int> min_e(n, INF), sel_e(n, -1);
min_e[0] = 0;
for (
int i = 0;
i<n;
++i) {
int v = -1;
for (
int j = 0;
j<n;
++j)
if (!used[j] && (v == -1 || min_e[j] < min_e[v]))
v = j;
//        if (min_e[v] == INF) {
//            cout << "No MST!";
//            exit(0);
//        }

used[v] = true;
if (sel_e[v] != -1)
ans.push_back({
v, sel_e[v], min_e[v]});

for (
int to = 0;
to<n;
++to)
if (g[v][to] != 0 && g[v][to] < min_e[to]) {
min_e[to] = g[v][to];
sel_e[to] =
v;
}
}
}


/// Dijkstra algorithm
void
dijkstra(vector<vector<pair<int, int>>> const g, int s, vector<int> &p, vector<int> &d)
{
    int n = g.size();
    p.clear();
    d.clear();
    p = vector<int>(n);
    d = vector<int>(n, INF);
    set <pair<int, int>> q;

    d[s] = 0;
    q.insert({d[s], s});

    while (!q.empty()) {
        int v = q.begin()->second;
        q.erase(q.begin());

        for (size_t j = 0; j < g[v].size(); j++) {
            int to = g[v][j].first,
                    len = g[v][j].second;
            if (d[v] + len < d[to]) {
                q.erase({d[to], to});
                d[to] = d[v] + len;
                p[to] = v;
                q.insert({d[to], to});
            }
        }
    }
}


/// Transitive closure
void transitive_closure(bit_set **g, int n) {
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
//            g[i] |= g[i].in(k) & g[k];
            if (g[i]->in(k)) {
                *g[i] |= *g[k];
            }
//            for (int j = 0; j < n; j++) {
//                g[i][j] = g[i][j] | (g[i][k] & g[k][j]);
//            }
        }
    }
}
