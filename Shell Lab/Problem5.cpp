// Copyright 2017 Joseph Liba joiemoie@bu.edu

#include<iostream>
#include<string>

using namespace std;

int main(int argc, char ** argv) {
  int size = 10;
  int N = stoi(argv[1]);
  for (int row = size-1; row >=0; row--) {
    for (int col = 0; col < size; col++) {
      if ((row == col || row == col + 1) && col <= N) std::cout << "X";
      else if (((row == col||col == row+2) && (col == N+2 && N%2 == 1)) || (row==col && (col == N+1 && N%2 == 0))) std::cout << "O";
      else std::cout << "_";
    }
    std::cout << "\n";
  }
  return 0;
}
