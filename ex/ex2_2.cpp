#include<iostream>
#include<vector>

using namespace std;

int pentagonal (int n)
{
  return n * (3 * n - 1) / 2;
}
void print_vec (const vector<int>& vec, string type) {
  cout << "wtf trans " << type << endl;
  for(auto v:vec) {
    cout << v << " ";
  }
  cout << endl;
}
inline bool isValid(int n) {
  return n < 100;
}
const vector<int>* get_vec (int n) {
  static vector<int> res;
  size_t cur_size = res.size() + 1;
  if (cur_size < n) {
     for (int i = cur_size; i <= n; i++) {
       res.push_back(pentagonal(i));
     }
  }
  return &res;
}
int last_num (int n) {
  auto vec = get_vec(n);
  return vec->at(n - 2);
}

int main () {
  cout << "Pls enter n: \n";
  int n;
  cin >> n;
  if (!isValid(n)) {
    cout << "invalid num\n";
    return 1;
  }
  const vector<int>* v_res = get_vec(n);
  print_vec(*v_res, "int");

  cout << "Pls enter n: \n";
  cin >> n;
  if (!isValid(n)) {
    cout << "invalid num\n";
    return 1;
  }
  cout << "last is " << last_num(n) << endl;
}