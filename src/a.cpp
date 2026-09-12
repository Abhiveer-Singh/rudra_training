#include <iostream> 
using namespace std;
int main(){
  struct me {
    int age = 10;
  };
  me person; 
  int marks = 12; 
  //person.date = marks;
  cout << person.age << endl;
  return 0; 
}
