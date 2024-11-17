#pragma once
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

bool skipBOM(ifstream &in);
string trim(const string& str);
std::string string_format(const std::string fmt_str, ...);