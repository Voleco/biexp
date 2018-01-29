//
//  HardPancake.cpp
//

#include "HardPancake.h"
#include "PancakePuzzle.h"
#include "TemplateAStar.h"
#include "IDAStar.h"
#include "PEAStar.h"
#include "NBS.h"
#include "WeightedVertexGraph.h"
//#include"WeightedHeuristic.h"
#include<set>
#include<map>
#include<algorithm>


//template <class state>
//class WeightedHeuristic : public Heuristic<state> {
//public:
//
//	WeightedHeuristic(Heuristic<state>* o, double w)
//		:original(o), weight(w)
//	{
//	}
//	double HCost(const state &a, const state &b) const
//	{
//		return weight * original->HCost(a, b);
//	}
//private:
//	double weight;
//	Heuristic<state>* original;
//};


const int N = 16;

using std::set;
using std::map;
using std::cout;

void Test1();
void Test2();
void Test3();
void Test4();
void Test5();
void Test6();
void Test7();
void Test8();

PancakePuzzleState<N> GetPancakeInstance(int which)
{
	int instances[3][10] =
	{
		{ 0,1,3,5,2,4,7,8,6,9},
		{2,0,1,3,6,4,5,9,7,8},
		{0,2,4,1,3,5,7,9,6,8}
	};
	PancakePuzzleState<N> result;
	for (int i = 0; i < 10; i++)
	{
		result.puzzle[i] = instances[which][i];
	}
	return result;
}

template <int N>
bool GetPancakeInstance(PancakePuzzleState<N> &s, int instance)
{
	int problems[100][16] = {
		{ 11,9,10,8,15,14,13,12,6,7,4,5,2,3,0,1 },
		{ 8,10,9,15,12,11,14,13,6,7,4,5,2,3,0,1 },
		{ 0,2,1,4,3,6,5,7,8,11,10,9,13,12,15,14 },
		{ 15,12,13,14,8,10,9,11,0,2,1,3,5,4,7,6 },
		{ 8,10,9,12,11,14,13,15,7,5,6,3,4,1,2,0 },
		{ 0,2,1,4,3,6,5,7,8,13,10,9,12,11,15,14 },
		{ 8,11,10,9,13,12,15,14,3,1,2,0,7,6,5,4 },
		{ 8,9,15,13,14,11,12,10,4,2,3,1,0,7,6,5 },
		{ 1,0,3,2,5,4,7,6,11,12,9,10,8,15,14,13 },
		{ 7,5,6,3,4,1,2,0,12,10,11,8,9,15,14,13 },
		{ 2,4,3,5,0,1,7,6,8,10,9,12,11,14,13,15 },
		{ 0,1,3,2,5,4,7,6,12,13,9,10,11,8,15,14 },
		{ 1,2,0,4,3,7,6,5,14,15,11,12,13,9,10,8 },
		{ 4,6,5,7,1,2,3,0,13,10,11,12,8,9,15,14 },
		{ 8,11,10,13,12,9,15,14,0,1,7,5,6,3,4,2 },
		{ 14,15,11,12,13,9,10,8,7,4,5,6,1,2,3,0 },
		{ 10,11,8,9,13,12,15,14,0,2,1,4,3,6,5,7 },
		{ 2,4,3,6,5,7,0,1,8,11,10,13,12,15,14,9 },
		{ 12,10,11,9,8,15,14,13,2,4,3,5,0,1,7,6 },
		{ 14,15,8,10,9,12,11,13,2,0,1,5,4,3,7,6 },
		{ 7,5,6,2,3,4,0,1,11,9,10,8,15,14,13,12 },
		{ 10,8,9,13,12,11,15,14,6,7,4,5,1,2,3,0 },
		{ 15,13,14,11,12,9,10,8,6,7,0,2,1,4,3,5 },
		{ 14,15,11,12,13,9,10,8,3,4,0,2,1,7,6,5 },
		{ 8,9,15,13,14,11,12,10,4,1,2,3,0,7,6,5 },
		{ 4,2,3,0,1,7,6,5,9,8,12,11,10,13,15,14 },
		{ 4,5,1,2,3,0,7,6,13,10,11,12,8,9,15,14 },
		{ 8,15,10,9,12,11,14,13,0,5,2,1,4,3,7,6 },
		{ 3,1,2,0,7,6,5,4,10,8,9,12,11,15,14,13 },
		{ 0,1,3,2,5,4,7,6,8,10,9,12,11,14,13,15 },
		{ 8,10,9,12,11,15,14,13,4,2,3,0,1,7,6,5 },
		{ 3,1,2,0,5,4,7,6,11,9,10,8,15,14,13,12 },
		{ 4,5,2,3,0,1,7,6,10,8,9,12,11,15,14,13 },
		{ 0,1,3,2,5,4,7,6,8,13,10,9,12,11,15,14 },
		{ 11,12,8,10,9,15,14,13,1,2,0,4,3,7,6,5 },
		{ 1,0,3,2,5,4,7,6,15,12,13,14,9,10,11,8 },
		{ 0,2,1,4,3,7,6,5,9,8,12,11,10,13,15,14 },
		{ 1,0,4,3,2,5,7,6,14,15,11,12,13,9,10,8 },
		{ 8,10,9,15,12,11,14,13,4,5,2,3,0,1,7,6 },
		{ 11,9,10,8,14,13,12,15,7,4,5,6,1,2,3,0 },
		{ 12,9,10,11,8,15,14,13,2,4,3,6,5,7,0,1 },
		{ 13,11,12,9,10,8,15,14,5,2,3,4,0,1,7,6 },
		{ 0,2,1,7,4,3,6,5,8,10,9,12,11,14,13,15 },
		{ 12,13,10,11,8,9,15,14,4,1,2,3,0,7,6,5 },
		{ 0,5,2,1,4,3,7,6,11,9,10,8,14,13,12,15 },
		{ 0,3,2,5,4,7,6,1,11,9,10,8,15,14,13,12 },
		{ 7,4,5,6,0,2,1,3,11,12,9,10,8,15,14,13 },
		{ 0,2,1,4,3,7,6,5,8,9,15,13,14,11,12,10 },
		{ 4,5,1,2,3,0,7,6,9,10,8,12,11,15,14,13 },
		{ 4,6,5,7,1,2,3,0,10,12,11,8,9,15,14,13 },
		{ 0,7,2,1,4,3,6,5,9,10,8,12,11,15,14,13 },
		{ 4,6,5,7,1,2,3,0,8,10,9,11,13,12,15,14 },
		{ 0,3,2,1,5,4,7,6,15,12,13,14,9,10,11,8 },
		{ 1,0,4,3,2,5,7,6,8,10,9,15,12,11,14,13 },
		{ 14,15,8,10,9,12,11,13,0,1,3,2,5,4,7,6 },
		{ 0,2,1,4,3,6,5,7,8,10,9,12,11,14,13,15 },
		{ 6,7,4,5,1,2,3,0,8,10,9,12,11,15,14,13 },
		{ 1,2,0,4,3,7,6,5,10,11,8,9,13,12,15,14 },
		{ 0,2,1,5,4,3,7,6,12,13,9,10,11,8,15,14 },
		{ 6,7,0,2,1,4,3,5,9,8,11,10,13,12,15,14 },
		{ 0,1,3,2,5,4,7,6,14,15,11,12,13,9,10,8 },
		{ 0,1,7,5,6,3,4,2,9,10,8,13,12,11,15,14 },
		{ 1,2,0,4,3,7,6,5,9,8,11,10,13,12,15,14 },
		{ 9,8,11,10,13,12,15,14,0,2,1,3,5,4,7,6 },
		{ 8,15,10,9,12,11,14,13,6,7,4,5,1,2,3,0 },
		{ 10,11,8,9,13,12,15,14,6,7,3,4,5,1,2,0 },
		{ 7,4,5,6,2,3,0,1,14,15,8,10,9,12,11,13 },
		{ 11,12,8,10,9,15,14,13,2,3,0,1,5,4,7,6 },
		{ 0,2,1,5,4,7,6,3,15,12,13,14,10,11,8,9 },
		{ 8,10,9,13,12,11,15,14,1,0,3,2,5,4,7,6 },
		{ 11,12,8,10,9,15,14,13,4,5,0,2,1,3,7,6 },
		{ 4,5,0,2,1,3,7,6,10,12,11,14,13,15,8,9 },
		{ 2,0,1,5,4,3,7,6,8,13,10,9,12,11,15,14 },
		{ 11,12,9,10,8,15,14,13,6,7,4,5,2,3,0,1 },
		{ 5,2,3,4,0,1,7,6,11,9,10,8,14,13,12,15 },
		{ 7,4,5,6,2,3,0,1,10,12,11,14,13,15,8,9 },
		{ 2,4,3,0,1,7,6,5,15,12,13,14,10,11,8,9 },
		{ 8,9,11,10,13,12,15,14,0,1,3,2,5,4,7,6 },
		{ 0,2,1,3,5,4,7,6,14,15,12,13,9,10,11,8 },
		{ 8,9,15,13,14,11,12,10,4,6,5,7,1,2,3,0 },
		{ 1,0,3,2,5,4,7,6,15,13,14,11,12,9,10,8 },
		{ 5,3,4,1,2,0,7,6,8,11,10,13,12,9,15,14 },
		{ 3,4,1,2,0,7,6,5,15,12,13,14,9,10,11,8 },
		{ 0,1,3,2,5,4,7,6,12,10,11,9,8,15,14,13 },
		{ 9,8,12,11,10,13,15,14,5,3,4,1,2,0,7,6 },
		{ 2,4,3,5,0,1,7,6,12,10,11,9,8,15,14,13 },
		{ 7,4,5,6,2,3,0,1,8,13,10,9,12,11,15,14 },
		{ 3,1,2,0,7,6,5,4,10,12,11,8,9,15,14,13 },
		{ 14,15,12,13,9,10,11,8,4,5,0,2,1,3,7,6 },
		{ 0,1,7,5,6,3,4,2,10,11,8,9,13,12,15,14 },
		{ 8,11,10,13,12,15,14,9,2,3,0,1,5,4,7,6 },
		{ 8,10,9,12,11,13,15,14,0,5,2,1,4,3,7,6 },
		{ 8,15,10,9,12,11,14,13,6,7,4,5,2,3,0,1 },
		{ 10,11,8,9,13,12,15,14,6,7,4,5,2,3,0,1 },
		{ 4,3,1,2,0,7,6,5,11,12,8,10,9,15,14,13 },
		{ 0,1,3,2,5,4,7,6,14,15,12,13,9,10,11,8 },
		{ 8,15,10,9,12,11,14,13,2,4,3,6,5,7,0,1 },
		{ 12,14,13,15,9,10,11,8,4,1,2,3,0,7,6,5 },
		{ 0,1,3,2,5,4,7,6,8,10,9,11,13,12,15,14 },
		{ 10,8,9,13,12,11,15,14,0,2,1,3,5,4,7,6 }
	};
	if (instance < 0 || instance >= 100)
		return false;
	for (int x = 0; x < 16; x++)
		s.puzzle[x] = problems[instance][x];
	return true;
}

//template <int N>
//bool GetPancakeInstance(PancakePuzzleState<N> &s, int instance)
//{
//	int problems[100][20] = {
//		{ 8,10,9,12,11,14,13,16,15,17,19,18,4,2,3,0,1,7,6,5 },
//		{ 8,9,11,10,13,12,15,14,17,16,19,18,6,7,3,4,5,1,2,0 },
//		{ 12,14,13,17,16,19,18,15,0,1,4,3,2,6,5,9,8,7,11,10 },
//		{ 10,8,9,12,11,14,13,16,15,19,18,17,3,1,2,0,5,4,7,6 },
//		{ 8,10,9,12,11,14,13,16,15,17,19,18,6,7,4,5,1,2,3,0 },
//		{ 0,2,1,4,3,6,5,8,7,11,10,9,16,17,14,15,12,13,19,18 },
//		{ 16,17,12,14,13,15,19,18,2,4,3,0,1,6,5,9,8,7,11,10 },
//		{ 8,11,10,9,13,12,15,14,17,16,19,18,5,2,3,4,0,1,7,6 },
//		{ 0,9,2,1,4,3,6,5,8,7,11,10,12,19,14,13,16,15,18,17 },
//		{ 2,0,1,5,4,3,7,6,9,8,11,10,15,13,14,12,17,16,19,18 },
//		{ 0,2,1,4,3,6,5,9,8,7,11,10,19,17,18,15,16,13,14,12 },
//		{ 13,12,15,14,17,16,19,18,0,2,1,4,3,8,7,6,5,11,10,9 },
//		{ 3,4,0,2,1,7,6,5,9,8,12,11,10,14,13,15,17,16,19,18 },
//		{ 8,10,9,11,14,13,12,17,16,15,19,18,4,1,2,3,0,7,6,5 },
//		{ 17,15,16,13,14,12,19,18,8,9,6,7,4,5,2,3,0,1,11,10 },
//		{ 0,1,3,2,5,4,7,6,9,8,11,10,16,17,13,14,15,12,19,18 },
//		{ 13,14,12,17,16,15,19,18,2,3,0,1,5,4,7,6,9,8,11,10 },
//		{ 10,11,8,9,6,7,4,5,1,2,3,0,13,14,12,16,15,19,18,17 },
//		{ 12,19,14,13,16,15,18,17,3,4,0,2,1,6,5,9,8,7,11,10 },
//		{ 8,10,9,11,14,13,12,17,16,15,19,18,6,7,4,5,1,2,3,0 },
//		{ 8,11,10,13,12,15,14,17,16,9,19,18,0,1,3,2,5,4,7,6 },
//		{ 2,4,3,6,5,7,0,1,9,10,8,12,11,14,13,16,15,19,18,17 },
//		{ 5,2,3,4,0,1,7,6,19,17,18,15,16,12,13,14,10,11,8,9 },
//		{ 8,9,6,7,4,5,2,3,0,1,11,10,12,14,13,16,15,17,19,18 },
//		{ 8,9,11,10,13,12,16,15,14,19,18,17,5,3,4,1,2,0,7,6 },
//		{ 12,19,14,13,16,15,18,17,0,2,1,4,3,6,5,9,8,7,11,10 },
//		{ 8,10,9,12,11,14,13,16,15,19,18,17,0,2,1,4,3,6,5,7 },
//		{ 0,2,1,4,3,6,5,8,7,11,10,9,12,14,13,16,15,18,17,19 },
//		{ 15,13,14,12,18,17,16,19,4,5,2,3,0,1,7,6,9,8,11,10 },
//		{ 5,2,3,4,0,1,7,6,14,15,12,13,10,11,8,9,17,16,19,18 },
//		{ 0,3,2,5,4,1,7,6,8,10,9,11,13,12,15,14,17,16,19,18 },
//		{ 12,14,13,19,16,15,18,17,3,4,0,2,1,6,5,9,8,7,11,10 },
//		{ 8,10,9,11,14,13,12,17,16,15,19,18,0,1,7,5,6,3,4,2 },
//		{ 1,2,0,5,4,3,7,6,8,9,11,10,13,12,15,14,17,16,19,18 },
//		{ 16,17,12,14,13,15,19,18,0,3,2,5,4,7,6,9,8,1,11,10 },
//		{ 6,7,0,2,1,4,3,5,18,19,16,17,13,14,15,11,12,9,10,8 },
//		{ 18,19,12,14,13,16,15,17,10,11,8,9,5,6,7,3,4,1,2,0 },
//		{ 14,12,13,17,16,15,19,18,1,2,0,4,3,6,5,9,8,7,11,10 },
//		{ 13,12,15,14,17,16,19,18,0,2,1,4,3,7,6,5,10,9,8,11 },
//		{ 1,2,0,4,3,6,5,9,8,7,11,10,15,13,14,12,18,17,16,19 },
//		{ 1,2,0,4,3,6,5,9,8,7,11,10,12,14,13,16,15,18,17,19 },
//		{ 0,5,2,1,4,3,7,6,8,9,11,10,13,12,16,15,14,19,18,17 },
//		{ 3,1,2,0,5,4,7,6,8,10,9,12,11,16,15,14,13,19,18,17 },
//		{ 0,2,1,4,3,6,5,8,7,9,11,10,18,19,16,17,13,14,15,12 },
//		{ 4,6,5,7,1,2,3,0,10,11,8,9,13,12,15,14,17,16,19,18 },
//		{ 9,10,8,12,11,14,13,16,15,19,18,17,0,3,2,5,4,1,7,6 },
//		{ 2,0,1,5,4,3,7,6,9,8,11,10,13,12,16,15,14,17,19,18 },
//		{ 14,15,12,13,10,11,8,9,17,16,19,18,6,7,4,5,2,3,0,1 },
//		{ 0,7,2,1,4,3,6,5,9,10,8,12,11,14,13,16,15,19,18,17 },
//		{ 4,1,2,3,0,7,6,5,19,16,17,18,14,15,12,13,10,11,8,9 },
//		{ 1,0,4,3,2,6,5,8,7,9,11,10,12,15,14,17,16,13,19,18 },
//		{ 16,15,13,14,12,19,18,17,11,8,9,10,6,7,4,5,2,3,0,1 },
//		{ 13,11,12,9,10,8,15,14,17,16,19,18,0,5,2,1,4,3,7,6 },
//		{ 4,5,2,3,0,1,7,6,8,10,9,12,11,15,14,13,18,17,16,19 },
//		{ 0,2,1,4,3,6,5,7,8,10,9,12,11,14,13,16,15,19,18,17 },
//		{ 12,14,13,16,15,19,18,17,1,0,3,2,5,4,7,6,9,8,11,10 },
//		{ 2,3,0,1,5,4,7,6,9,8,11,10,12,19,14,13,16,15,18,17 },
//		{ 16,18,17,19,13,14,15,12,0,2,1,4,3,6,5,8,7,9,11,10 },
//		{ 12,14,13,16,15,18,17,19,0,2,1,4,3,6,5,8,7,10,9,11 },
//		{ 4,5,2,3,0,1,7,6,9,10,8,12,11,14,13,16,15,19,18,17 },
//		{ 12,14,13,15,17,16,19,18,10,11,8,9,6,7,4,5,1,2,3,0 },
//		{ 3,4,1,2,0,7,6,5,10,11,8,9,13,12,15,14,17,16,19,18 },
//		{ 8,10,9,11,13,12,15,14,17,16,19,18,2,4,3,5,0,1,7,6 },
//		{ 0,3,2,5,4,7,6,1,13,11,12,9,10,8,15,14,17,16,19,18 },
//		{ 5,3,4,1,2,0,7,6,16,17,14,15,12,13,10,11,8,9,19,18 },
//		{ 11,9,10,6,7,8,4,5,2,3,0,1,14,16,15,17,12,13,19,18 },
//		{ 13,11,12,9,10,8,15,14,17,16,19,18,4,2,3,0,1,7,6,5 },
//		{ 12,14,13,19,16,15,18,17,1,0,3,2,6,5,4,8,7,9,11,10 },
//		{ 8,9,12,11,10,14,13,17,16,15,19,18,7,4,5,6,1,2,3,0 },
//		{ 6,7,0,2,1,4,3,5,8,11,10,9,13,12,15,14,17,16,19,18 },
//		{ 11,12,8,10,9,14,13,17,16,15,19,18,0,2,1,4,3,6,5,7 },
//		{ 2,3,0,1,5,4,7,6,10,11,8,9,13,12,15,14,17,16,19,18 },
//		{ 7,5,6,2,3,4,0,1,18,19,16,17,14,15,11,12,13,9,10,8 },
//		{ 6,7,4,5,2,3,0,1,9,8,11,10,12,14,13,17,16,15,19,18 },
//		{ 4,6,5,7,1,2,3,0,8,9,11,10,13,12,16,15,14,19,18,17 },
//		{ 0,1,3,2,5,4,7,6,9,8,11,10,12,13,15,14,17,16,19,18 },
//		{ 8,10,9,11,14,13,12,17,16,15,19,18,4,2,3,0,1,7,6,5 },
//		{ 17,14,15,16,12,13,19,18,1,0,4,3,2,6,5,7,9,8,11,10 },
//		{ 5,3,4,1,2,0,7,6,9,8,11,10,16,17,13,14,15,12,19,18 },
//		{ 19,16,17,18,12,14,13,15,1,0,3,2,6,5,4,8,7,9,11,10 },
//		{ 0,1,3,2,6,5,4,9,8,7,11,10,19,17,18,15,16,13,14,12 },
//		{ 4,5,0,2,1,3,7,6,18,19,16,17,14,15,11,12,13,9,10,8 },
//		{ 0,2,1,4,3,6,5,9,8,7,11,10,12,13,15,14,17,16,19,18 },
//		{ 12,13,15,14,17,16,19,18,2,4,3,0,1,6,5,9,8,7,11,10 },
//		{ 1,2,0,4,3,7,6,5,9,8,11,10,15,16,13,14,12,19,18,17 },
//		{ 1,2,0,4,3,6,5,9,8,7,11,10,14,16,15,18,17,19,12,13 },
//		{ 4,2,3,1,0,7,6,5,8,9,11,10,13,12,16,15,14,19,18,17 },
//		{ 0,3,2,5,4,7,6,9,8,1,11,10,13,12,15,14,17,16,19,18 },
//		{ 0,2,1,4,3,7,6,5,9,8,11,10,12,14,13,16,15,18,17,19 },
//		{ 9,10,8,12,11,14,13,17,16,15,19,18,6,7,4,5,1,2,3,0 },
//		{ 2,0,1,4,3,6,5,8,7,11,10,9,16,18,17,19,13,14,15,12 },
//		{ 11,9,10,8,13,12,15,14,17,16,19,18,3,4,1,2,0,7,6,5 },
//		{ 13,12,15,14,17,16,19,18,1,2,0,4,3,7,6,5,9,8,11,10 },
//		{ 0,3,2,1,5,4,7,6,8,10,9,11,13,12,16,15,14,19,18,17 },
//		{ 0,2,1,4,3,8,7,6,5,11,10,9,16,15,13,14,12,19,18,17 },
//		{ 0,3,2,1,5,4,7,6,9,8,11,10,19,16,17,18,14,15,12,13 },
//		{ 1,0,3,2,5,4,8,7,6,9,11,10,19,16,17,18,13,14,15,12 },
//		{ 0,9,2,1,4,3,6,5,8,7,11,10,18,19,15,16,17,13,14,12 },
//		{ 0,2,1,3,5,4,7,6,10,8,9,13,12,11,15,14,17,16,19,18 },
//		{ 18,19,16,17,14,15,11,12,13,9,10,8,3,4,1,2,0,7,6,5 }
//	};
//	if (instance < 0 || instance >= 100)
//		return false;
//	for (int x = 0; x < 20; x++)
//		s.puzzle[x] = problems[instance][x];
//	return true;
//}


int factorial(int n)
{
	static int table[13] =
	{ 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600};
	return table[n];
}

void RankToPermutation(int size, int rank, vector<int>& pm)
{
	pm.resize(size);
	vector<int> sorted;
	sorted.resize(0);
	for (int i = 0; i < size; i++)
		sorted.push_back(i);
	int countRight = 0;
	int r = rank;
	for (int i = 0; i < size; i++)
	{
		countRight = r / factorial(size - 1 - i);
		r = r%factorial(size - 1 - i);
		pm[i] = sorted[countRight];
		sorted.erase(sorted.begin() + countRight);
	}
}

string PuzzleToString(const vector<int>& puzzle)
{
	string result = "{";
	for (int i = 0; i < puzzle.size()-1; i++)
		result = result + std::to_string(puzzle[i]) + ", ";
	result = result + std::to_string(puzzle[puzzle.size() - 1]) + "},";
	return result;
}


void CreateHardPancakeInstance(vector<int>& puzzle, int size, int method)
{
	srandom(time(0));
	puzzle.clear();
	puzzle.resize(size);
	//self-inverse
	if (method == 0)
	{
		vector<int> setS;
		for (int i = 0; i < size; i++)
			setS.push_back(i);
		while (setS.size() > 1)
		{
			//set π[e1] = e2 and π[e2] = e1
			if (random() % 2 == 0)
			{
				int index1 = random() % setS.size();
				int index2 = index1;
				while (index2 == index1)
					index2 = random() % setS.size();
				if (index2 < index1)
				{
					int tmp = index1;
					index1 = index2;
					index2 = tmp;
				}

				int e1 = setS[index1];
				int e2 = setS[index2];

				puzzle[e1] = e2;
				puzzle[e2] = e1;
				//guaranteed index1<index2
				setS.erase(setS.begin() + index2);
				setS.erase(setS.begin() + index1);
			}
			//set π[e] = e
			else
			{
				int index = random() % setS.size();
				int e = setS[index];
				puzzle[e] = e;
				setS.erase(setS.begin() + index);
			}
		}
		if (setS.size() == 1)
			puzzle[setS[0]] = setS[0];
	}
	//short cycles
	if (method == 1)
	{
		//number of remaining elements
		int eleNum = size;
		while (eleNum >= 4)
		{
			int k = random() % 4 + 1;
			
			int offset = size - eleNum;
			if (k == 1)
			{
				puzzle[k-1 + offset] = k-1 + offset;
			}
			else
			{
				//generate a random permutation of size k
				int rank = random() % factorial(k);
				vector<int> p;
				RankToPermutation(k, rank, p);

				for (int i = 0; i < k; i++)
					p[i] = p[i] + offset;

				//π[ek−1] = π[ek]
				for (int i = 0; i < k-1; i++)
					puzzle[p[i]] = p[i + 1];
				//π[ek] = π[e1]
				puzzle[p[k - 1]] = p[0];
			}			
			eleNum = eleNum - k;
		}

		//deal with the remainder

		if (eleNum == 0)
			;
		else if (eleNum == 1)
		{
			puzzle[size-1] = size-1;
		}
		else
		{
			//generate a random permutation of size k

			int rank = random() % factorial(eleNum);
			vector<int> p;
			RankToPermutation(eleNum, rank, p);

			for (int i = 0; i < eleNum; i++)
				p[i] = p[i] + size - eleNum;
			

			//π[ek−1] = π[ek]
			for (int i = 0; i < eleNum - 1; i++)
				puzzle[p[i]] = p[i + 1];
			//π[ek] = π[e1]
			puzzle[p[eleNum - 1]] = p[0];
		}

	}


	//bootstrapping
	//concatenate small problems
	if (method == 2)
	{

	}
}



void TestHardPancake()
{
	Test8();
}

void Test1()
{
	vector<int> instance;
	cout << "selfinverse\n";
	//for (int i = 0; i < 50; i++)
	//{
	//	createhardpancakeinstance(instance, 10, 0);
	//	cout << "ok3\n";
	//	cout << puzzletostring(instance) << "\n";
	//}
	cout << "shortcycles\n";
	for (int i = 0; i < 1; i++)
	{
		CreateHardPancakeInstance(instance, 10, 1);
		//cout << "ok3\n";
		cout << PuzzleToString(instance) << "\n";
	}
}


void Test2()
{
	TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;

	vector<PancakePuzzleState<N>> astarPath;

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;
	PancakePuzzle<N> pancake(0);
	ZeroHeuristic<PancakePuzzleState<N>> z;

	goal.Reset();
	start = GetPancakeInstance(2);
	astar.SetHeuristic(&z);
	astar.InitializeSearch(&pancake, goal, start, astarPath);
	
	bool first_find = false;
	bool astarFinished = false;
	while (true)
	{
		auto item = astar.openClosedList.Lookat(astar.openClosedList.Peek());
		if (!fequal(item.g, pancake.HCost(item.data, goal)) && first_find == false)
		{
			first_find = true;
			cout << "first do not match\n"
				<< item.data << "\n"
				<< "g: " << item.g << "h" << pancake.HCost(item.data, goal) << "\n"
				<< astar.GetNodesExpanded() << "\n";
			//break;
		}
		if (astarFinished == true)
		{
			cout << "A* finished, cost: "<< pancake.GetPathLength(astarPath)<<"\n" ;
			break;
		}
		astarFinished = astar.DoSingleSearchStep(astarPath);


	}

}

void Test3()
{
	TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;

	vector<PancakePuzzleState<N>> astarPath;

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;
	PancakePuzzle<N> pancake(0);
	ZeroHeuristic<PancakePuzzleState<N>> z;

	goal.Reset();
	cout << goal << "\n";
	map<int, int> counts;

	int h2g3 = 0;
	//int h_10 = 0;
	//for (int i = 0; i < factorial(N); i++)
	for (int i = factorial(N) - 1; i >= 0; i--)
	{
		//if (10*i % factorial(N) == 0)
		//	cout << (10*i / factorial(N) + 1) * 10 << "% finished\n";
		vector<int> p;
		RankToPermutation(N, i, p);
		start.Reset();
		for (int j = 0; j < N; j++)
			start.puzzle[j] = p[j];

		int h = (int)pancake.HCost(start, goal);
		//if (h == N && h_10 < 10)
		//{
		//	cout << start << "\n";
		//	h_10++;
		//}
		//cout << "h: " << h << "\n";

		if (h == 2)
		{
			astarPath.clear();
			astar.SetHeuristic(&pancake);
			astar.GetPath(&pancake, goal, start, astarPath);
			int g = (int)pancake.GetPathLength(astarPath);
			if (g == 3)
			{
				cout << start << "\n";
				h2g3++;
			}

		}
		if (counts.find(h) != counts.end())
			counts[h]++;
		else
			counts[h] = 1;
	
		//astar.SetHeuristic(&z);
		//astar.InitializeSearch(&pancake, goal, start, astarPath);

		//bool astarFinished = false;
		//while (true)
		//{
		//	auto item = astar.openClosedList.Lookat(astar.openClosedList.Peek());
		//	if (!fequal(item.g, pancake.HCost(item.data, goal)))
		//	{
		//		cout << "first do not match\n"
		//			<< "g: " << item.g << "h" << pancake.HCost(item.data, goal) << "\n"
		//			<< astar.GetNodesExpanded() << "\n";
		//		break;
		//	}
		//	if (astarFinished == true)
		//	{
		//		cout << "A* finished\n";
		//		break;
		//	}
		//	astarFinished = astar.DoSingleSearchStep(astarPath);


		//}
	}
	for (auto i = counts.begin(); i != counts.end(); i++)
		cout << "h: " << i->first << " counts:\t" << i->second<<"\n";
	cout << "h=2, g=3 counts:\t" << h2g3 << "\n";
}

void Test4()
{
	TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;

	vector<PancakePuzzleState<N>> astarPath;

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;
	PancakePuzzle<N> pancake(0);
	ZeroHeuristic<PancakePuzzleState<N>> z;

	goal.Reset();

	vector<int> instance;
	cout << "shortcycles\n";
	//CreateHardPancakeInstance(instance, N, 1);
	for (int i = 0; i < N; i++)
		instance.push_back(i);
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(instance.begin(), instance.end(), std::default_random_engine(seed));
	start.Reset();
	for (int i = 0; i < N;i++)
		start.puzzle[i] = instance[i];
	astar.SetHeuristic(&z);
	astar.InitializeSearch(&pancake, goal, start, astarPath);

	bool first_find = false;
	bool astarFinished = false;
	while (true)
	{
		auto item = astar.openClosedList.Lookat(astar.openClosedList.Peek());
		if (!fequal(item.g, pancake.HCost(item.data, goal)) && first_find == false)
		{
			first_find = true;
			cout << "first do not match\n"
				<< item.data << "\n"
				<< "g: " << item.g << "h" << pancake.HCost(item.data, goal) << "\n"
				<< astar.GetNodesExpanded() << "\n";
			//break;
		}
		if (astarFinished == true)
		{
			cout << "A* finished, cost: " << pancake.GetPathLength(astarPath) << "\n";
			break;
		}
		astarFinished = astar.DoSingleSearchStep(astarPath);


	}

}


void Test5()
{
	TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;
	PEAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> peastar;
	
	vector<PancakePuzzleState<N>> astarPath;
	vector<PancakePuzzleState<N>> peastarPath;

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;
	PancakePuzzle<N> pancake(0);
	ZeroHeuristic<PancakePuzzleState<N>> z;

	goal.Reset();


	for (int i = factorial(N) - 1; i >= 0; i--)
	//for (int i = 114434; i >= 0; i--)
	{

		vector<int> p;
		RankToPermutation(N, i, p);
		start.Reset();
		for (int j = 0; j < N; j++)
			start.puzzle[j] = p[j];


		astar.SetHeuristic(&pancake);
		astar.GetPath(&pancake, goal, start, astarPath);
		int g1 = (int)pancake.GetPathLength(astarPath);
		
		peastar.SetHeuristic(&pancake);
		peastar.GetPath(&pancake, goal, start, peastarPath);
		int g2 = (int)pancake.GetPathLength(peastarPath);
		
		if (g1 != g2)
		{
			cout << "error, g1:\t" << g1 << "\tg2:\t" << g2 << "\n";
			cout << "a* node exp:" << astar.GetNodesExpanded() << "\n";
			cout << "pea* node exp:" << peastar.GetNodesExpanded()<<"\n";
			cout << "start:\t" << start << "\n";
			break;
		}

	}
	cout << "all good\n";

}

void Test6()
{
	PEAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> peastar;
	TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;


	vector<PancakePuzzleState<N>> astarPath;
	vector<PancakePuzzleState<N>> peastarPath;

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;
	
	goal.Reset();

	//for (int gap = 0; gap < 1; gap++)
	//{
	//	PancakePuzzle<N> pancake(gap);

	//	cout << "GAP-" << gap << "\n";
	//	for (int i = 0; i <100; i++)
	//	{

	//		GetPancakeInstance(start, i);

	//		astar.SetHeuristic(&pancake);
	//		astar.GetPath(&pancake, goal, start, astarPath);

	//		peastar.SetHeuristic(&pancake);
	//		peastar.GetPath(&pancake, goal, start, peastarPath);

	//		cout << "instance:\t"<<i<<"\n"
	//			<<"pea* node exp:\t" << peastar.GetNodesExpanded() << "nece exp : \t" << peastar.GetNecessaryExpansions() << "\n"
	//		 << "a* node exp:\t" << astar.GetNodesExpanded() << "nece exp : \t" << astar.GetNecessaryExpansions() << "\n";
	//	}

	//}
	
	for (int factor = 1; factor < 4; factor++)
	{
		PancakePuzzle<N> pancake(0);
		WeightedHeuristic<PancakePuzzleState<N>> wh(&pancake, 1.0 - factor*0.1);

		cout << "GAP*" << 1.0-factor*0.1 << "\n";
		for (int i = 0; i <100; i++)
		{

			GetPancakeInstance(start, i);
			//cout << "ok0\n";
			if (!peastar.InitializeSearch(&pancake, goal, start, peastarPath))
			{
				cout << "error, fail to intialize\n";
				return;
			}
			//cout << "ok1\n";
			peastar.SetHeuristic(&wh);
			//cout << "ok2\n";
			while (!peastar.DoSingleSearchStep(peastarPath)) {}
			
			cout << "instance:\t" << i << "\tpea* node exp:\t" << peastar.GetNodesExpanded() << "\n";
		}
	}


}

void Test7()
{

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;

	goal.Reset();

	//for (int gap = 0; gap < 4; gap++)
	//{
	//	PancakePuzzle<N> pancake(gap);

	//	cout << "GAP-" << gap << "\n";
	//	for (int i = 0; i <100; i++)
	//	{

	//		GetPancakeInstance(start, i);

	//		peastar.SetHeuristic(&pancake);
	//		peastar.GetPath(&pancake, goal, start, peastarPath);

	//		cout << "instance:\t"<<i<<"\tpea* node exp:\t" << peastar.GetNodesExpanded() << "\n";
	//	}

	//}


		PancakePuzzle<N> pancake(0);


		for (int i = 0; i <100; i++)
		{
			GetPancakeInstance(start, i);
			BidirectionalProblemAnalyzer<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>>
				ana(start, goal, &pancake, &pancake, &pancake);
		}

}

void Test8()
{
	NBS<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> nbs;
	TemplateAStar<PancakePuzzleState<N>, PancakePuzzleAction, PancakePuzzle<N>> astar;


	vector<PancakePuzzleState<N>> astarPath;
	vector<PancakePuzzleState<N>> nbsPath;

	PancakePuzzleState<N> start;
	PancakePuzzleState<N> goal;

	goal.Reset();

	for (int gap = 0; gap < 1; gap++)
	{
		PancakePuzzle<N> pancake(gap);

		cout << "GAP-" << gap << "\n";
		for (int i = 0; i <100; i++)
		{

			GetPancakeInstance(start, i);

			astar.SetHeuristic(&pancake);
			astar.GetPath(&pancake, start, goal, astarPath);

			nbs.GetPath(&pancake, start,goal, &pancake,&pancake,nbsPath);

			if (pancake.GetPathLength(nbsPath) != pancake.GetPathLength(astarPath))
			{
				cout << pancake.GetPathLength(nbsPath) << ", " << pancake.GetPathLength(astarPath) << "\n";
				return;
			}
			else
				cout << pancake.GetPathLength(nbsPath)<<"\n";
			cout << "\ninstance:\t" << i << "\n"
			<<"nbs node exp:\t" << nbs.GetNodesExpanded() << "nece exp : \t" << nbs.GetNecessaryExpansions() << "\n"
						 << "a* node exp:\t" << astar.GetNodesExpanded() << "nece exp : \t" << astar.GetNecessaryExpansions() << "\n";
		}

	}
}