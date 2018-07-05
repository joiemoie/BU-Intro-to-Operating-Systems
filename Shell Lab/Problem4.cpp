// Copyright 2017 Joseph Liba joiemoie@bu.edu

#include<iostream>
#include<fstream>

int main(int argc, char ** argv) {
  std::ifstream myReadFile;
  myReadFile.open(argv[1]);
  if (myReadFile.is_open()) {
    std::string str;
    std::string result;
    int maxLength = 0;
    while (std::getline(myReadFile, str)) {
      if (str.length() > maxLength) result = str;
      maxLength = str.length();
    }
    std::cout << result;
  }
  myReadFile.close();
  return 0;
}
