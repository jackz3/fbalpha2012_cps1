#include<iostream>

using namespace std;

bool fibon_elem(int, int &);
void calc_once() {
  int pos;
  cout << "pls enter a pos: ";
  cin >> pos;
  int elem;
  if (fibon_elem(pos, elem))
    cout << "elem # " << pos << " is " << elem << endl;
  else
    cout << "Sorry, Could not calc elem # " << pos << endl;
}

int main() {
  char again;
  do {
    calc_once(); 
    cout << "Again ? (Y/N)\n";
    cin >> again;
  }
  while (again == 'Y' || again == 'y');
  cout << "Tks!\n";
}

bool fibon_elem (int pos, int& elem) {
  if (pos <= 0 | pos > 1024) {
    elem = 0;
    return false;
  }
  elem = 1;
  int n_2 = 1, n_1 = 1;
  for (int ix = 3; ix <= pos; ++ix) {
    elem = n_2 + n_1;
    n_2 = n_1;
    n_1 = elem;
  }
  return true;
}