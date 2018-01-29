//
//  HardPancake.h
//
//

#ifndef HardPancake_h
#define HardPancake_h

#include<vector>
#include<stdio.h>
#include<string>

using std::vector;
using std::string;

/*
puzzle is the arrangement of the created problem instance
method:
0: self-inverse
1: short cycles
2: bootstrapping
*/
void CreateHardPancakeInstance(vector<int>& puzzle, int size, int method);
string PuzzleToString(const vector<int>& puzzle);

void TestHardPancake();
#endif /* HardPancake_h */
