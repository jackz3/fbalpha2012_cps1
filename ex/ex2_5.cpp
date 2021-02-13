#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;

template<typename T>
T _max(T t1, T t2) {
  return t1 > t2 ? t1 : t2;
}
template<typename T>
T _max(const vector<T>& vec) {
  T res = vec.at(0);
  for(T v:vec) {
    res = _max(res, v);
  }
  // *max_element(vec.begin(), vec.end());
  return res;
}
template<typename T>
T _max(T arr[], int size) {
  T res = *arr;
  for (int i = 1; i < size; i++) {
    res = _max(res, *(arr + i));
  }
  return res;
}

int main() {
  int res = _max(11, 22);

  cout << res << endl;
  cout << _max(0.3f, 0.29f) << endl;
  cout << _max("fwef", "23f2e") << endl;

  vector<int> v1 = {11, 22, 33};
  vector<float> v2 = {0.3f, 0.2f, 0.4f};
  vector<string> v3 = {"fae", "bv", "23r"};

  cout << _max(v1) << endl;
  cout << _max(v2) << endl;
  cout << _max(v3) << endl;

  int a1[] = {11, 22, 33};
  float a2[] = {0.3f, 0.2f, 0.4f};
  string a3[] = {"fae", "bv", "23r"};
  cout << _max(a1, 3) << endl;
  cout << _max(a2, 3) << endl;
  cout << _max(a3, 3) << endl;
}