#include <iostream>
using namespace std;
int main() {
  const int b = 10;
  int* p = &b;
  b = 0; // error
  const string s = "helloworld";
  const int i, j = 0;
}
