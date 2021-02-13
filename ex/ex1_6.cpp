#include<iostream>
#include<array>
#include<vector>

using namespace std;
int main() {
  int num = -1;
  int narr[100];
  vector<int> v_int;
  int i = 0;
  int total;
  cout << "pls enter a int: (0 end)";
  while (cin >> num) {
    cout << "pls enter a int: (0 end)";
    narr[i] = num;
    v_int.push_back(num);
    i++;
  }
  for (int j = 0; j < i; j++) {
    cout << narr[j] << " ";
    total += narr[j];
  }
  cout << "array sum is " << total << endl;
  total = 0;
  for (auto val : v_int) {
    cout << val << " ";
    total += val;
  }
  cout << "vector sum is " << total << endl;
}