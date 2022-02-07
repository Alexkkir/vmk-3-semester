#include <iostream>
#include <string>
#include <fstream>
#include <limits>
#include <tuple>


using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::max;
using std::numeric_limits;
using std::tuple;
using std::get;

struct TreeNode {
    int id; // id of element (used when initializing a tree)
    long long value; // stored value
    long long sum; // sum of all elements in current sub-tree
    TreeNode *Left, *Right;

    TreeNode(int id, long long value) :
            id(id),
            value(value),
            sum(0),
            Left(nullptr),
            Right(nullptr) {}
};

TreeNode *FindById(TreeNode *node, int id) {
    if (node == nullptr)
        return nullptr;
    if (node->id == id)
        return node;
    else {
        TreeNode *tmp;
        tmp = FindById(node->Left, id);
        if (tmp != nullptr)
            return tmp;
        tmp = FindById(node->Right, id);
        if (tmp != nullptr)
            return tmp;
    }
    return nullptr;
}

void PrintTree(TreeNode *node, int n_separators = 0) {
    if (node == nullptr)
        return;
    PrintTree(node->Right, n_separators + 1);
    for (int i = 0; i < n_separators; i++) {
        cout << "\t";
    }
    cout << node->id << ": " << node->value << " " << node->sum << endl;
    PrintTree(node->Left, n_separators + 1);
}

long long CalsSums(TreeNode *node) {
    if (node == nullptr)
        return 0;
    long long prod_left, prod_right;
    prod_left = CalsSums(node->Left);
    prod_right = CalsSums(node->Right);
    node->sum = node->value + prod_left + prod_right;
    return node->sum;
}

tuple<long long, int, int> Process(TreeNode *node, long long total_sum) {
    if (node == nullptr || (node->Left == nullptr && node->Right == nullptr))
        return {numeric_limits<long long>::min(), 0, 0};

    // calculating for current node
    long long max_prod = numeric_limits<long long>::min(), cur_prod;
    int best_vert_1 = node->id, best_vert_2 = 0;
    if (node->Left != nullptr) {
        long long subtree_sum = node->Left->sum;
        cur_prod = subtree_sum * (total_sum - subtree_sum);
        if (cur_prod > max_prod) {
            max_prod = cur_prod;
            best_vert_2 = node->Left->id;
        }
    }
    if (node->Right != nullptr) {
        long long subtree_sum = node->Right->sum;
        cur_prod = subtree_sum * (total_sum - subtree_sum);
        if (cur_prod > max_prod) {
            max_prod = cur_prod;
            best_vert_2 = node->Right->id;
        }
    }

    tuple<long long, int, int>
            result_for_left = Process(node->Left, total_sum),
            result_for_right = Process(node->Right, total_sum);
    if (get<0>(result_for_left) > max_prod) {
        max_prod = get<0>(result_for_left);
        best_vert_1 = get<1>(result_for_left);
        best_vert_2 = get<2>(result_for_left);
    }
    if (get<0>(result_for_right) > max_prod) {
        max_prod = get<0>(result_for_right);
        best_vert_1 = get<1>(result_for_right);
        best_vert_2 = get<2>(result_for_right);
    }
    return {max_prod, best_vert_1, best_vert_2};
}

int main() {
    TreeNode *tree;

    /*
    Write tree in format:
        [father id] [L | R // for left and right] [current node id] [value]
    Checking of correct input not provided. Root node has node father, hence for it's father use "-"
    Example:
        - - 1 100
        1 L 2 200
        1 R 3 300
     */

    string filename = "square.txt"; // if you want to use file for input
    std::ifstream in(filename);
    cin.rdbuf(in.rdbuf());
    if (!in.is_open()) {
        cout << "File not found" << endl;
        return 0;
    }

    int father_id, id;
    long long val;
    string tmp, dir;

    cin >> tmp >> tmp >> id >> val;
    tree = new TreeNode(id, val);

    while (cin >> father_id >> dir >> id >> val) {
        TreeNode *father = FindById(tree, father_id);
        if (dir == "L") {
            father->Left = new TreeNode(id, val);
        } else if (dir == "R") {
            father->Right = new TreeNode(id, val);
        }
    }
    CalsSums(tree);
    cout << "Tree:" << endl;
    PrintTree(tree);
    tuple<long long, int, int> result = Process(tree, tree->sum);
    long long max_prod = get<0>(result);
    int best_vert_1 = get<1>(result), best_vert_2 = get<2>(result);
    cout << "Result: " << max_prod << endl <<
         "Corresponding vertices (id): " << best_vert_1 << " " << best_vert_2 << endl;
    return 0;
}
