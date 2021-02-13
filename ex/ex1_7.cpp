#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>

using namespace std;
int main () {
  vector<string> v_str;  
  ifstream ifile("./ex/file.txt");
  if (!ifile) {
    cout << "open error! \n";
  } else {
    string line;
    while(ifile >> line) {
      v_str.push_back(line);
    }
  }
  sort(v_str.begin(), v_str.end());
  for(auto str:v_str) {
    cout << str << endl;
  } 
  ofstream ofile("./ex/out_file.txt", ios_base::app);
  if (!ofile) {
    cout << "create file error";
  } else {
    for(int i = 0; i < v_str.size(); i++) {
      ofile << v_str[i] << endl;
    } 
    ofile.close();
  }
}