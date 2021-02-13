#include<iostream>
#include<string>
#include<cstring>
// #include<cstdio>

using namespace std;
int main () {
  string user_name, last_name;
  char name[20];
  cout << "Please enter your first name: ";
  cin >> user_name;
  while (last_name.size() <= 2)
  {
    cout << "Pls enter ur last name: (>2 chars)";
    cin >> last_name;
  }
  fgets(name, 10, stdin);  // clear stdin buffer \n
  while (strlen(name) <= 2)
  {
    cout << "name: (>2 chars)";
    fgets(name, 10, stdin);
    // rewind(stdin);
    // scanf("%s", name);
  }
   

  cout << "\n" << "hello, " << user_name << " " << last_name << " " << name << endl;
}