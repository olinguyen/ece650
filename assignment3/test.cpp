#include <iostream>
#include <vector>

using namespace std;

int main(int arg, char** argv) {

  cout << "Hello World!" << endl;
  vector<int> some_vector;
  for (int i = 0; i < 5; ++i) {
    some_vector.push_back(i);
    cout << some_vector[i] << endl;
  }

  return 0;
}
