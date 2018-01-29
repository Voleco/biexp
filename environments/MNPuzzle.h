/*
 *  MNPuzzle.h
 *  hog2
 *
 *  Created by Nathan Sturtevant on 5/9/07.
 *  Copyright 2007 Nathan Sturtevant, University of Alberta. All rights reserved.
 *
 */

#ifndef MNPUZZLE_H
#define MNPUZZLE_H

#include <stdint.h>
#include <iostream>
#include "SearchEnvironment.h"
#include "PermutationPuzzleEnvironment.h"
#include "UnitSimulation.h"
#include "GraphEnvironment.h"
#include "Graph.h"
#include "GraphEnvironment.h"
#include <sstream>

template <int width, int height>
class MNPuzzleState {
public:
	MNPuzzleState()
	{
		Reset();
	}
	void Reset()
	{
		for (unsigned int x = 0; x < size(); x++)
			puzzle[x] = x;
		blank = 0;
	}
	size_t size() const { return width*height; }
	void FinishUnranking(const MNPuzzleState<width, height> &s)
	{
		for (int x = 0; x < size(); x++)
		{
			if (puzzle[x] == 0)
			{
				blank = x;
				return;
			}
		}
	}

	unsigned int blank;
	int puzzle[width*height];
};

/**
 * Note, direction "kLeft" indicates that the blank is being moved to the left.
 * That is, a tile is being slid right into the blank position. The other
 * actions are defined similarly.
 */
enum slideDir {
	kLeft, kUp, kDown, kRight
};

template <int width, int height>
static std::ostream& operator <<(std::ostream & out, const MNPuzzleState<width, height> &loc)
{
	out << "(" << width << "x" << height << ")";
	for (unsigned int x = 0; x < loc.size(); x++)
		out << loc.puzzle[x] << " ";
	return out;
}

static std::ostream& operator <<(std::ostream & out, const slideDir &loc)
{
	switch (loc)
	{
		case kLeft: out << "Left"; break;
		case kRight: out << "Right"; break;
		case kUp: out << "Up"; break;
		case kDown: out << "Down"; break;
	}
	return out;
}


template <int width, int height>
static bool operator==(const MNPuzzleState<width, height> &l1, const MNPuzzleState<width, height> &l2)
{
	for (unsigned int x = 0; x < l1.size(); x++)
	{
		if (l1.puzzle[x] > 0 || l2.puzzle[x] > 0) // don't have to check the blank
			if (l1.puzzle[x] != l2.puzzle[x])
				return false;
	}
	return true;
}

template <int width, int height>
static bool operator!=(const MNPuzzleState<width, height> &l1, const MNPuzzleState<width, height> &l2)
{
	return !(l1 == l2);
}


template <int width, int height>
class MNPuzzle : public PermutationPuzzle::PermutationPuzzleEnvironment<MNPuzzleState<width, height>, slideDir> {
public:
	MNPuzzle();
	MNPuzzle(const std::vector<slideDir> op_order); // used to set action order
	~MNPuzzle();
	void SetWeighted(bool w) { weighted = w; }
	bool GetWeighted() const { return weighted; }
	void GetSuccessors(const MNPuzzleState<width, height> &stateID, std::vector<MNPuzzleState<width, height>> &neighbors) const;
	void GetActions(const MNPuzzleState<width, height> &stateID, std::vector<slideDir> &actions) const;
	slideDir GetAction(const MNPuzzleState<width, height> &s1, const MNPuzzleState<width, height> &s2) const;
	void ApplyAction(MNPuzzleState<width, height> &s, slideDir a) const;
	bool InvertAction(slideDir &a) const;
	static unsigned GetParity(MNPuzzleState<width, height> &state);

	OccupancyInterface<MNPuzzleState<width, height>, slideDir> *GetOccupancyInfo() { return 0; }
	double HCost(const MNPuzzleState<width, height> &state1, const MNPuzzleState<width, height> &state2) const;
	double HCost(const MNPuzzleState<width, height> &state1) const;
	double DefaultH(const MNPuzzleState<width, height> &s) const;

	double GCost(const MNPuzzleState<width, height> &state1, const MNPuzzleState<width, height> &state2) const;
	double GCost(const MNPuzzleState<width, height> &, const slideDir &) const;
	double AdditiveGCost(const MNPuzzleState<width, height> &, const slideDir &);
	bool GoalTest(const MNPuzzleState<width, height> &state, const MNPuzzleState<width, height> &goal) const;

	bool GoalTest(const MNPuzzleState<width, height> &s) const;

	void GetStateFromPDBHash(uint64_t hash, MNPuzzleState<width, height> &s,
							 int count, const std::vector<int> &pattern,
							 std::vector<int> &dual);
	//void LoadPDB(char *fname, const std::vector<int> &tiles, bool additive);

	uint64_t GetActionHash(slideDir act) const;
	void OpenGLDraw() const;
	void OpenGLDraw(const MNPuzzleState<width, height> &s) const;
	void OpenGLDraw(const MNPuzzleState<width, height> &l1, const MNPuzzleState<width, height> &l2, float v) const;
	void OpenGLDraw(const MNPuzzleState<width, height> &, const slideDir &) const { /* currently not drawing moves */ }
	void StoreGoal(MNPuzzleState<width, height> &); // stores the locations for the given goal state

	/** Returns stored goal state if it is stored.**/
	MNPuzzleState<width, height> Get_Goal(){
		if (!goal_stored) {
			fprintf(stderr, "ERROR: Call to Get_Goal when no goal stored\n");
			exit(1);
		}
		return goal;
	}

	virtual const std::string GetName();

	void ClearGoal(); // clears the current stored information of the goal

	bool IsGoalStored() const {return goal_stored;} // returns if a goal is stored or not
	Graph *GetGraph();

	/**
	Changes the ordering of operators to the new inputted order
	**/
	void Change_Op_Order(const std::vector<slideDir> op_order);

	/**
	Creates num_puzzles random MN puzzles of the specified size and stores them in
	puzzle-vector. All random puzzles are unique and solvable for the standard goal
	of 0, 1, 2, ..., MN - 1. The method assumes that num_puzzles is no bigger than
	the total number of solvable problems for that size.
	**/
	static void Create_Random_MN_Puzzles(MNPuzzleState<width, height> &goal, std::vector<MNPuzzleState<width, height>> &puzzle_vector, unsigned num_puzzles);

	/**
	Reads in the the desired number of puzzles from the given filename with the
	given dimensions and stores them in puzzle_vector. Only the first max_puzzles
	are stored. first_counter should be set to true if the first element on every
	row is the index of the puzzle. The input format is the entries in each of
	the puzzle positions separated by a space, with the optional index entry
	described above.
	**/
	static int read_in_mn_puzzles(const char *filename, bool first_counter, unsigned max_puzzles, std::vector<MNPuzzleState<width, height>> &puzzle_vector);

	/**
	Returns a possible ordering of the operators. The orders are in a "lexicographic"
	with the original ordering being Up, Left, Right, Down. This is therefore the order
	returned with a call of order_num=0. This initial ordering is that used by Korf in
	his original IDA* experiments. The ordering originally used in HOG is returned with a
	call of order_num=15.
	**/
	static std::vector<slideDir> Get_Op_Order_From_Hash(int order_num);
	std::vector<slideDir> Get_Op_Order(){return ops_in_order;}

	static MNPuzzleState<width, height> Generate_Random_Puzzle();

//	virtual void GetStateFromHash(MNPuzzleState<width, height> &s, uint64_t hash) const;
//	uint64_t GetStateHash(const MNPuzzleState<width, height> &s) const;
	uint64_t GetMaxStateHash() const;
	
	bool State_Check(const MNPuzzleState<width, height> &to_check);

	unsigned Get_Num_Of_Columns(){return width;}
	unsigned Get_Num_Of_Rows(){return height;}

	void Set_Use_Manhattan_Heuristic(bool to_use){use_manhattan = to_use;}
private:
//	double DoPDBLookup(const MNPuzzleState<width, height> &state);
//	std::vector<std::vector<uint8_t> > PDB;
//	std::vector<std::vector<int> > PDBkey;
	std::vector<std::vector<slideDir> > operators; // stores the operators applicable at each blank position
	std::vector<slideDir> ops_in_order;
	bool goal_stored; // whether a goal is stored or not
	bool use_manhattan;
	bool weighted;
	
	// stores the heuristic value of each tile-position pair indexed by the tile value (0th index is empty)
	std::vector<std::vector<unsigned> > h_increment;
	MNPuzzleState<width, height> goal;
};

template <int width, int height>
class GraphPuzzleDistanceHeuristic : public GraphDistanceHeuristic {
public:
	GraphPuzzleDistanceHeuristic(MNPuzzle<width, height> &mnp, Graph *graph, int count);
	double HCost(const graphState &state1, const graphState &state2) const;
private:
	MNPuzzle<width, height> puzzle;
};

//typedef UnitSimulation<MNPuzzleState, slideDir, MNPuzzle> PuzzleSimulation;

template <int width, int height>
MNPuzzle<width, height>::MNPuzzle()
{
	weighted = false;
	// stores applicable operators at each of the width*height positions
	Change_Op_Order(Get_Op_Order_From_Hash(15)); // Right, Left, Down, Up is default operator ordering
	goal_stored = false;
	use_manhattan = true;
}

template <int width, int height>
MNPuzzle<width, height>::MNPuzzle(const std::vector<slideDir> op_order)
{
	Change_Op_Order(op_order);
	goal_stored = false;
	use_manhattan = true;
	weighted = false;
}

template <int width, int height>
MNPuzzle<width, height>::~MNPuzzle()
{
	ClearGoal();
}

template <int width, int height>
void MNPuzzle<width, height>::Change_Op_Order(std::vector<slideDir> op_order)
{
	operators.clear();
	ops_in_order.clear();
	
	bool up_act = false;
	bool down_act = false;
	bool left_act = false;
	bool right_act = false;
	
	if (op_order.size() != 4)
	{
		fprintf(stderr, "ERROR: Not enough operators in operator sequence for construction of MNPuzzle\n");
		exit(1);
	}
	
	for (unsigned int op_num = 0; op_num < 4; op_num++)
	{
		if (op_order[op_num] == kUp)
		{
			up_act = true;
		}
		else if (op_order[op_num] == kDown)
		{
			down_act = true;
		}
		else if (op_order[op_num] == kLeft)
		{
			left_act = true;
		}
		else if (op_order[op_num] == kRight)
		{
			right_act = true;
		}
	}
	
	if (!up_act || !down_act || !left_act || !right_act)
	{
		fprintf(stderr, "ERROR: Invalid operator sequence for construction of MNPuzzle\n");
		exit(1);
	}
	
	for (unsigned i = 0; i < op_order.size(); i++)
	{
		ops_in_order.push_back(op_order[i]);
	}
	
	// stores applicable operators at each of the width*height positions
	std::vector<slideDir> ops(4);
	for (unsigned int blank = 0; blank < width*height; blank++) {
		ops.resize(0);
		
		for (unsigned int op_num = 0; op_num < 4; op_num++)
		{
			if (op_order[op_num] == kUp && blank > width - 1)
			{
				ops.push_back(kUp);
			}
			if (op_order[op_num] == kLeft && blank % width > 0)
			{
				ops.push_back(kLeft);
			}
			if (op_order[op_num] == kRight && (blank % width) < width - 1)// && (blank != 0))
			{
				ops.push_back(kRight);
			}
			if (op_order[op_num] == kDown && blank < width * height - width)// && (blank != 1))
			{
				ops.push_back(kDown);
			}
		}
		
		operators.push_back(ops);
	}
}

template <int width, int height>
const std::string MNPuzzle<width, height>::GetName(){
	std::string s = "STP(";
	s += std::to_string(width);
	s += ",";
	s += std::to_string(height);
	s += ")";
	return s;
//	std::stringstream name;
//	name << width;
//	name << "x";
//	name << height;
//	name << " Sliding Tile Puzzle";
//	
////	if (PDB_distincts.size() > 0)
////	{
////		name << ", PDBS:";
////		for (unsigned i = 0; i < PDB_distincts.size(); i++)
////		{
////			name << " <";
////			for (unsigned j = 0; j < PDB_distincts[i].size() - 1; j++)
////			{
////				name << PDB_distincts[i][j];
////				name << ", ";
////			}
////			name << PDB_distincts[i].back();
////			name << ">";
////		}
////		name << ", Manhattan Distance";
////	}
//	{
//		name << ", Manhattan Distance";
//	}
//	
//	name << ", Op Order: ";
//	for (unsigned op_num = 0; op_num < ops_in_order.size() - 1; op_num++){
//		name << ops_in_order[op_num];
//		name << ", ";
//	}
//	name << ops_in_order.back();
//	return name.str();
}

template <int width, int height>
Graph *MNPuzzle<width, height>::GetGraph()
{
	Graph *g = new Graph();
	//Factorial(width*height);
	
	for (int x = 0; x < 362880; x++)
	{
		node *n;
		g->AddNode(n = new node(""));
		n->SetLabelF(GraphSearchConstants::kXCoordinate, (32768.0-random()%65536)/32768.0);
		n->SetLabelF(GraphSearchConstants::kYCoordinate, (32768.0-random()%65536)/32768.0);
		n->SetLabelF(GraphSearchConstants::kZCoordinate, (32768.0-random()%65536)/32768.0);
	}
	for (int x = 0; x < 362880; x++)
	{
		MNPuzzleState<width, height> t;
		GetStateFromHash(t, x);
		std::vector<slideDir> moves;
		GetActions(t, moves);
		g->GetNode(x)->SetLabelF(GraphSearchConstants::kZCoordinate, GetParity(t));
		for (unsigned int y = 0; y < moves.size(); y++)
		{
			ApplyAction(t, moves[y]);
			uint64_t hash = GetStateHash(t);
			InvertAction(moves[y]);
			ApplyAction(t, moves[y]);
			if (!g->FindEdge(x, hash))
				g->AddEdge(new edge(x, hash, 1));
		}
	}
	// 362880 states
	// 2x2 area -- roughly 600x600 states, or distance 1 = 1/600
	//	for (int t = 0; t < 362880; t++)
	//	{
	//		node *n = g->GetNode(t);
	//		neighbor_iterator ni = n->getNeighborIter();
	//		for (long tmp = n->nodeNeighborNext(ni); tmp != -1; tmp = n->nodeNeighborNext(ni))
	//		{
	//			double x, y, z;
	//			x = n->GetLabelF(GraphSearchConstants::kXCoordinate);
	//			y = n->GetLabelF(GraphSearchConstants::kYCoordinate);
	//			z = n->GetLabelF(GraphSearchConstants::kZCoordinate);
	//
	//			double x1, y1, z1;
	//			node *nb = g->GetNode(tmp);
	//			x1 = nb->GetLabelF(GraphSearchConstants::kXCoordinate);
	//			y1 = nb->GetLabelF(GraphSearchConstants::kYCoordinate);
	//			z1 = nb->GetLabelF(GraphSearchConstants::kZCoordinate);
	//			// now move n to be 1/600 away from nb!
	//			n->SetLabelF(GraphSearchConstants::kXCoordinate, 0.5*x + 0.5*x1);
	//			n->SetLabelF(GraphSearchConstants::kYCoordinate, 0.5*y + 0.5*y1);
	//			n->SetLabelF(GraphSearchConstants::kZCoordinate, 0.5*z + 0.5*z1);
	//
	//		}
	//	}
	return g;
}

template <int width, int height>
void MNPuzzle<width, height>::StoreGoal(MNPuzzleState<width, height> &s)
{
//	assert(s.height == height);
//	assert(s.width == width);
	
	// makes sure goal contains all legal
	bool goal_tester[width*height];
	for (unsigned int i = 0; i < width*height; i++)
	{
		goal_tester[i] = false;
	}
	
	for (unsigned int i = 0; i < width*height; i++)
	{
		goal_tester[s.puzzle[i]] = true;
	}
	
	for (unsigned int i = 0; i < width*height; i++)
	{
		if (!goal_tester[i])
		{
			printf("INVALID GOAL\n");
			assert(goal_tester[i]);
		}
	}
	
	h_increment.resize(width*height);
	//h_increment = new unsigned*[width*height];
	
	for (unsigned int i = 1; i < width*height; i++)
	{
		h_increment[i].resize(width*height);
		//h_increment[i] = new unsigned [width*height];
	}
	
	for (unsigned goal_pos = 0; goal_pos < width*height; goal_pos++)
	{
		unsigned tile = s.puzzle[goal_pos];
		if (tile == 0)
			continue;
		
		for (unsigned pos = 0; pos < width*height; pos++)
		{
			h_increment[tile][pos] = abs((int) (goal_pos % width) - (int)(pos % width)); // difference in column
			h_increment[tile][pos] += abs((int) (goal_pos / width) - (int)(pos / width)); // difference in row
		}
	}
	
	goal_stored = true;
	
	goal = s;
}

template <int width, int height>
bool MNPuzzle<width, height>::GoalTest(const MNPuzzleState<width, height> &s) const
{
	return (s == goal);
}

template <int width, int height>
void MNPuzzle<width, height>::ClearGoal()
{
	if (goal_stored)
	{
		goal_stored = false;
		//		// clears memory allocated for h_increment
		//		for (unsigned int i = 1; i < width*height; i++)
		//		{
		//			delete h_increment[i];
		//		}
		//		delete h_increment;
	}
}

template <int width, int height>
void MNPuzzle<width, height>::GetSuccessors(const MNPuzzleState<width, height> &stateID,
							 std::vector<MNPuzzleState<width, height>> &neighbors) const
{
	neighbors.resize(0);
	
	for (unsigned int i = 0; i < operators[stateID.blank].size(); i++)
	{
		neighbors.push_back(stateID);
		ApplyAction(neighbors.back(), operators[stateID.blank][i]);
	}
}

template <int width, int height>
void MNPuzzle<width, height>::GetActions(const MNPuzzleState<width, height> &stateID, std::vector<slideDir> &actions) const
{
	actions.resize(0);
	for (unsigned int i = 0; i < operators[stateID.blank].size(); i++)
	{
		actions.push_back(operators[stateID.blank][i]);
	}
}

template <int width, int height>
slideDir MNPuzzle<width, height>::GetAction(const MNPuzzleState<width, height> &a, const MNPuzzleState<width, height> &b) const
{
	int row1 = a.blank%width;
	int col1 = a.blank/height;
	int row2 = b.blank%width;
	int col2 = b.blank/height;
	if (row1 == row2)
	{
		if (col1 > col2)
			return kUp;
		return kDown;
	}
	if (row1 > row2)
		return kLeft;
	return kRight;
}

template <int width, int height>
void MNPuzzle<width, height>::ApplyAction(MNPuzzleState<width, height> &s, slideDir a) const
{
	// we actually do the swap to maintain consistency when using abstract states
	// (these contain -1 in some positions, including possibly the blank position.)
	switch (a)
	{
		case kUp:
			if (s.blank >= width)
			{
				int tmp = s.puzzle[s.blank];
				s.puzzle[s.blank] = s.puzzle[s.blank-width];
				s.blank -= width;
				s.puzzle[s.blank] = tmp;
			}
			else {
				printf("Invalid up operator\n");
				assert(false);
				exit(0);
			}
			break;
		case kDown:
			if (s.blank < s.size() - width)
			{
				int tmp = s.puzzle[s.blank];
				s.puzzle[s.blank] = s.puzzle[s.blank+width];
				s.blank += width;
				s.puzzle[s.blank] = tmp;
			}
			else {
				printf("Invalid down operator\n");
				assert(false);
				exit(0);
			}
			break;
		case kRight:
			if ((s.blank%width) < width-1)
			{
				int tmp = s.puzzle[s.blank];
				s.puzzle[s.blank] = s.puzzle[s.blank+1];
				s.blank += 1;
				s.puzzle[s.blank] = tmp;
			}
			else {
				printf("Invalid right operator\n");
				assert(false);
				exit(0);
			}
			break;
		case kLeft:
			if ((s.blank%width) > 0)
			{
				int tmp = s.puzzle[s.blank];
				s.puzzle[s.blank] = s.puzzle[s.blank-1];
				s.blank -= 1;
				s.puzzle[s.blank] = tmp;
			}
			else {
				printf("Invalid left operator\n");
				assert(false);
				exit(0);
			}
			break;
	}
}

template <int width, int height>
bool MNPuzzle<width, height>::InvertAction(slideDir &a) const
{
	switch (a)
	{
		case kLeft: a = kRight; break;
		case kUp: a = kDown; break;
		case kDown: a = kUp; break;
		case kRight: a = kLeft; break;
	}
	return true;
}

template <int width, int height>
double MNPuzzle<width, height>::HCost(const MNPuzzleState<width, height> &state1) const
{
	return DefaultH(state1);
}

template <int width, int height>
double MNPuzzle<width, height>::HCost(const MNPuzzleState<width, height> &state1, const MNPuzzleState<width, height> &state2) const
{
	if (goal_stored)
		return HCost(state1);
//	if (state1.height != height || state1.width != width)
//	{
//		fprintf(stderr, "ERROR: HCost called with a state with wrong size.\n");
//		exit(1);
//	}
//	if (state2.height != height || state2.width != width)
//	{
//		fprintf(stderr, "ERROR: HCost called with a state with wrong size.\n");
//		exit(1);
//	}
	
	double hval = 0;
	//	if (PDB.size() != 0)
	//		hval = std::max(hval, PDB_Lookup(state1));
	
	if (use_manhattan)
	{
		double man_dist = 0;
		std::vector<int> xloc(width*height);
		std::vector<int> yloc(width*height);
		
		for (unsigned int x = 0; x < width; x++)
		{
			for (unsigned int y = 0; y < height; y++)
			{
				xloc[state2.puzzle[x + y*width]] = x;
				yloc[state2.puzzle[x + y*width]] = y;
			}
		}
		for (unsigned int x = 0; x < width; x++)
		{
			for (unsigned int y = 0; y < height; y++)
			{
				if (state1.puzzle[x + y*width] != 0)
				{
					man_dist += (abs((int)(xloc[state1.puzzle[x + y*width]] - x))
								 + abs((int)(yloc[state1.puzzle[x + y*width]] - y)));
				}
			}
		}
		hval = std::max(hval, man_dist);
	}
//	// if no heuristic
//	else if (PDB.size()==0)
//	{
//		if (state1 == state2)
//			return 0;
//		else
//			return 1;
//	}
	
	return hval;
}

template <int width, int height>
double MNPuzzle<width, height>::DefaultH(const MNPuzzleState<width, height> &state) const
{
	double man_dist = 0;
	// increments amound for each tile location
	// this calculates the Manhattan distance
	for (unsigned loc = 0; loc < width*height; loc++)
	{
		if (state.puzzle[loc] > 0)
			man_dist += h_increment[state.puzzle[loc]][loc];
	}
	return man_dist;
}

static int costs[25] =
{
	3, 5, 4, 7, 10, 5, 3, 3, 8, 9, 2, 10, 10, 1, 2, 1, 1, 4, 7, 9, 6, 10, 2, 8, 8
};

template <int width, int height>
double MNPuzzle<width, height>::AdditiveGCost(const MNPuzzleState<width, height> &s, const slideDir &d)
{
	int tile;
	switch (d)
	{
		case kLeft: tile = s.puzzle[s.blank-1]; break;
		case kUp: tile = s.puzzle[s.blank-height]; break;
		case kDown: tile = s.puzzle[s.blank+height]; break;
		case kRight: tile = s.puzzle[s.blank+1]; break;
	}
	if (tile == -1)
		return 0;
	if (weighted)
		return costs[s.blank];
	return 1;
}

template <int width, int height>
double MNPuzzle<width, height>::GCost(const MNPuzzleState<width, height> &a, const MNPuzzleState<width, height> &b) const
{
	//	int diff = a.blank - b.blank;
	//
	if (weighted)
		return costs[a.blank];
	return 1;
}

template <int width, int height>
double MNPuzzle<width, height>::GCost(const MNPuzzleState<width, height> &s, const slideDir &d) const
{
	//	double cost;
	//	switch (d)
	//	{
	//		case kLeft: cost = s.puzzle[s.blank-1]; break;
	//		case kUp: cost = s.puzzle[s.blank-s.height]; break;
	//		case kDown: cost = s.puzzle[s.blank+s.height]; break;
	//		case kRight: cost = s.puzzle[s.blank+1]; break;
	//	}
	//	if (cost < 1) // in PDB might have negative costs
	//		cost = 1;
	//	return cost;
	if (weighted)
		return costs[s.blank];
	return 1;
}


template <int width, int height>
bool MNPuzzle<width, height>::GoalTest(const MNPuzzleState<width, height> &state, const MNPuzzleState<width, height> &theGoal) const
{
	return (state == theGoal);
}

template <int width, int height>
uint64_t MNPuzzle<width, height>::GetActionHash(slideDir act) const
{
	switch (act)
	{
		case kUp: return 0;
		case kDown: return 1;
		case kRight: return 2;
		case kLeft: return 3;
	}
	return 4;
}

template <int width, int height>
void MNPuzzle<width, height>::GetStateFromPDBHash(uint64_t hash, MNPuzzleState<width, height> &s,
								   int count, const std::vector<int> &pattern,
								   std::vector<int> &dual)
{
	uint64_t hashVal = hash;
	dual.resize(pattern.size());
	
	int numEntriesLeft = count-pattern.size()+1;
	for (int x = pattern.size()-1; x >= 0; x--)
	{
		dual[x] = hashVal%numEntriesLeft;
		hashVal /= numEntriesLeft;
		numEntriesLeft++;
		for (int y = x+1; y < pattern.size(); y++)
		{
			if (dual[y] >= dual[x])
				dual[y]++;
		}
	}
	s.puzzle.resize(count);
	std::fill(s.puzzle.begin(), s.puzzle.end(), -1);
	for (int x = 0; x < dual.size(); x++)
	{
		s.puzzle[dual[x]] = pattern[x];
		if (pattern[x] == 0)
			s.blank = dual[x];
	}
}


template <int width, int height>
void MNPuzzle<width, height>::OpenGLDraw() const
{
}

void DrawTile(float x, float y, char c1, char c2, int w, int h)
{
	//glLineWidth(10.0);
	int textWidth = 0;
	if (c1 != 0)
		textWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, c1);
	if (c2 != 0)
		textWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, c2);
	if (textWidth == 0)
		return;
	//printf("%d\n", textWidth);
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0);
	glTranslatef(x*2.0/w-1.0, (1+y)*2.0/h-1.0-0.15, -0.001);
	glScalef(1.0/(w*120.0), 1.0/(h*120.0), 1);
	glRotatef(180, 0.0, 0.0, 1.0);
	glRotatef(180, 0.0, 1.0, 0.0);
	glTranslatef(120-textWidth/2, 0, 0);
	if (c1 != 0)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c1);
	if (c2 != 0)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c2);
	//glTranslatef(-x/width+0.5, -y/height+0.5, 0);
	glPopMatrix();
	
	glLineWidth(1.0);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glVertex3f(x*2.0/w-1+.05/w, (y)*2.0/h-1+.05/h, 0.002);
	glVertex3f((x+1)*2.0/w-1-.05/w, (y)*2.0/h-1+.05/h, 0.002);
	glVertex3f((x+1)*2.0/w-1-.05/w, (y+1)*2.0/h-1-.05/h, 0.002);
	glVertex3f(x*2.0/w-1+.05/w, (y+1)*2.0/h-1-.05/h, 0.002);
	glEnd();
}

void DrawFrame(int w, int h)
{
	// frame
	glLineWidth(3.0);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-1-.05/w, -1-.05/h, 0.002);
	glVertex3f(1+.05/w, -1-.05/h, 0.002);
	glVertex3f(1+.05/w, 1+.05/h, 0.002);
	glVertex3f(-1-.05/w, 1+.05/h, 0.002);
	glEnd();
	
	glColor3f(0.25, 0.25, 0.25);
	glBegin(GL_QUADS);
	glVertex3f(-1-.05/w, -1-.05/h, 0.003);
	glVertex3f(1+.05/w, -1-.05/h, 0.003);
	glVertex3f(1+.05/w, 1+.05/h, 0.003);
	glVertex3f(-1-.05/w, 1+.05/h, 0.003);
	glEnd();
	glLineWidth(1.0);
}

template <int width, int height>
void MNPuzzle<width, height>::OpenGLDraw(const MNPuzzleState<width, height> &s) const
{
	glEnable(GL_LINE_SMOOTH);
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			char c1=0, c2=0;
			if (s.puzzle[x+y*width] > 9)
				c1 = '0'+(((s.puzzle[x+y*width])/10)%10);
			if (s.puzzle[x+y*width] > 0)
				c2 = '0'+((s.puzzle[x+y*width])%10);
			if (s.puzzle[x+y*width] == -1)
				c1 = ' ';
			DrawTile(x, y, c1, c2, width, height);
		}
	}
	DrawFrame(width, height);
}

template <int width, int height>
void MNPuzzle<width, height>::OpenGLDraw(const MNPuzzleState<width, height> &s1, const MNPuzzleState<width, height> &s2, float v) const
{
	glEnable(GL_LINE_SMOOTH);
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			char c1=0, c2=0;
			if (s2.puzzle[x+y*width] > 9)
				c1 = '0'+(((s2.puzzle[x+y*width])/10)%10);
			if (s2.puzzle[x+y*width] > 0)
				c2 = '0'+((s2.puzzle[x+y*width])%10);
			if (s2.puzzle[x+y*width] == -1)
				c1 = ' ';
			
			if (s1.puzzle[x+y*width] == s2.puzzle[x+y*width])
			{
				DrawTile(x, y, c1, c2, width, height);
			}
			else {
				switch (GetAction(s1, s2))
				{
					case kUp: DrawTile(x, (y-1)*v + (y)*(1-v), c1, c2, width, height); break;
					case kDown: DrawTile(x, (y+1)*v + (y)*(1-v), c1, c2, width, height); break;
					case kLeft: DrawTile((x)*(1-v)+(x-1)*v, y, c1, c2, width, height); break;
					case kRight: DrawTile((x+1)*v+(x)*(1-v), y, c1, c2, width, height); break;
					default: assert(!"action not found");
				}
			}
		}
	}
	DrawFrame(width, height);
}

/**
 Reads in MNPuzzle states from the given filename. Each line of the
 input file contains a puzzle state of the given width and height.
 The state is written in the form of listing the tile in each puzzle
 position from 1 to width*height. The blank is represented by the 0
 character.
 
 If any line is found to not be legitimate, either because it does
 not have the proper number of tiles, or the proper tiles, it is
 simply not added to the list of puzzles.
 
 The function will return 1 if reading from the given filename failed,
 and 0 otherwise.
 
 puzz_num_start - should be set as true if the first non-whitespace
 element on each line is the puzzle number, otherwise should be set
 to false.
 
 max_puzzles - the maximum number of puzzles that will be added to
 the puzzle list.
 **/
template <int width, int height>
int MNPuzzle<width, height>::read_in_mn_puzzles(const char *filename, bool puzz_num_start, unsigned int max_puzzles,
												std::vector<MNPuzzleState<width, height>> &puzzles)
{
	
	std::vector<std::vector<int> > permutations;
	PermutationPuzzle::PermutationPuzzleEnvironment<MNPuzzleState<width, height>, slideDir>::Read_In_Permutations(filename, width*height, max_puzzles, permutations, puzz_num_start);
	
	// convert permutations into MNPuzzleStates
	for (unsigned i = 0; i < permutations.size(); i++)
	{
		MNPuzzleState<width, height> new_state;
		
		for (unsigned j = 0; j < width*height; j++)
		{
			new_state.puzzle[j] = permutations[i][j];
			if (new_state.puzzle[j] == 0)
				new_state.blank = j;
		}
		puzzles.push_back(new_state);
	}
	return 0;
}

template <int width, int height>
GraphPuzzleDistanceHeuristic<width, height>::GraphPuzzleDistanceHeuristic(MNPuzzle<width, height> &mnp, Graph *graph, int count)
:GraphDistanceHeuristic(graph), puzzle(mnp)
{
	for (int x = 0; x < count /*10*/; x++)
	{
		AddHeuristic();
	}
}

template <int width, int height>
double GraphPuzzleDistanceHeuristic<width, height>::HCost(const graphState &state1, const graphState &state2) const
{
	MNPuzzleState<3, 3> a, b;
	puzzle.GetStateFromHash(a, state1);
	puzzle.GetStateFromHash(b, state2);
	double val = puzzle.HCost(a, b);
	
	for (unsigned int i=0; i < heuristics.size(); i++)
	{
		double hval = heuristics[i][state1]-heuristics[i][state2];
		if (hval < 0)
			hval = -hval;
		if (fgreater(hval,val))
			val = hval;
	}
	
	return val;
}


/**
 Randomly generates a puzzle of the specified dimensions
 and returns that puzzle.
 **/
template <int width, int height>
MNPuzzleState<width, height> MNPuzzle<width, height>::Generate_Random_Puzzle()
{
	MNPuzzleState<width, height> new_puzz;
	std::vector<int> permutation = PermutationPuzzle::PermutationPuzzleEnvironment<MNPuzzleState<width, height>, slideDir>::Get_Random_Permutation(width*height);
	
	for (unsigned i = 0; i < permutation.size(); i++)
	{
		new_puzz.puzzle[i] = permutation[i];
		if (permutation[i] == 0)
			new_puzz.blank = i;
	}
	
	return new_puzz;
}

//template <int width, int height>
//void MNPuzzle<width, height>::GetStateFromHash(MNPuzzleState<width, height> &s, uint64_t hash) const
//{
//	s.puzzle.resize(width*height);
//	int count = s.puzzle.size();
//	int countm2 = s.puzzle.size()-2;
//	uint64_t hashVal = hash;
//	std::vector<int> dual(s.puzzle.size()-2);
//	
//	// unrank the locations of the first 10 tiles
//	int numEntriesLeft = count-countm2+1;
//	for (int x = countm2-1; x >= 0; x--)
//	{
//		dual[x] = hashVal%numEntriesLeft;
//		hashVal /= numEntriesLeft;
//		numEntriesLeft++;
//		for (int y = x+1; y < countm2; y++)
//		{
//			if (dual[y] >= dual[x])
//				dual[y]++;
//		}
//	}
//	// clear puzzle locations
//	for (int x = 0; x < count; x++)
//	{
//		s.puzzle[x] = -1;
//	}
//	// revert locations of tiles into positions in the puzzle
//	for (int x = 0; x < countm2; x++)
//	{
//		s.puzzle[dual[x]] = x;
//	}
//	// reset the cache of the blanks location
//	s.blank = dual[0];
//	
//	// now find the two -1's and assign them
//	// to ensure the right parity
//	int x = 0;
//	int loc1 = -1, loc2 = -1;
//	for (; x < count; x++)
//	{
//		if (s.puzzle[x] == -1)
//		{
//			loc1 = x;
//			x++;
//			break;
//		}
//	}
//	for (; x < count; x++)
//	{
//		if (s.puzzle[x] == -1)
//		{
//			loc2 = x;
//			break;
//		}
//	}
//	assert(loc1 != -1 && loc2 != -1);
//	// Choose an arbitrary ordering and then
//	// check the parity. If it's wrong, we just
//	// swap them and are guaranteed to get the right
//	// parity.
//	s.puzzle[loc1] = countm2;
//	s.puzzle[loc2] = countm2+1;
//	if (GetParity(s) == 1)
//	{
//		s.puzzle[loc1] = countm2+1;
//		s.puzzle[loc2] = countm2;
//	}
//}

//template <int width, int height>
//uint64_t MNPuzzle<width, height>::GetStateHash(const MNPuzzleState<width, height> &s) const
//{
//	std::vector<int> locs(s.puzzle.size()-2); // We only rank n-2 of n items; last two are fixed by the parity
//	std::vector<int> dual(s.puzzle.size());
//	
//	// build the representation containing the item locations
//	for (unsigned int x = 0; x < s.puzzle.size(); x++)
//	{
//		dual[s.puzzle[x]] = x;
//	}
//	// build an array with the locations of the first 10 items
//	for (int x = 0; x < s.puzzle.size()-2; x++)
//	{
//		locs[x] = dual[x];
//	}
//	
//	uint32_t hashVal = 0;
//	int numEntriesLeft = s.puzzle.size();
//	
//	// compute the lexographical ranking of the locations
//	// of the first 10 tiles
//	for (unsigned int x = 0; x < s.puzzle.size()-2; x++)
//	{
//		hashVal += locs[x]*Factorial(numEntriesLeft-1)/2;
//		numEntriesLeft--;
//		
//		// decrement locations of remaining items
//		// to keep the numbering compact
//		for (unsigned y = x; y < s.puzzle.size()-2; y++)
//		{
//			if (locs[y] > locs[x])
//				locs[y]--;
//		}
//	}
//	return hashVal;
//}

template <int width, int height>
uint64_t MNPuzzle<width, height>::GetMaxStateHash() const
{
	int val = width*height;
	return PermutationPuzzle::PermutationPuzzleEnvironment<MNPuzzleState<width, height>, slideDir>::Factorial(val)/2;
}


//	if q is odd, the invariant is the parity (odd or even) of the number of pairs of pieces in reverse order (the parity of the permutation). For the order of the 15 pieces consider line 2 after line 1, etc., like words on a page.
//	if q is even, the invariant is the parity of the number of pairs of pieces in reverse order plus the row number of the empty square counting from bottom and starting from 0.
template <int width, int height>
unsigned MNPuzzle<width, height>::GetParity(MNPuzzleState<width, height> &state)
{
	unsigned swaps = 0; // counts number of swaps
	for (unsigned x = 0; x < state.puzzle.size(); x++)
	{
		if (state.puzzle[x] == 0) // skip blank
			continue;
		for (unsigned y = x + 1; y < state.puzzle.size(); y++)
		{
			if (state.puzzle[y] == 0) // skip blank
				continue;
			if (state.puzzle[y] < state.puzzle[x])
				swaps++;
		}
	}
	// if odd num of columns
	if ((state.width % 2) == 1)
	{
		return swaps % 2;
	}
	
	// if even num of columns
	return (swaps + (state.puzzle.size()-state.blank-1)/state.width)%2;
}

template <int width, int height>
void MNPuzzle<width, height>::Create_Random_MN_Puzzles(MNPuzzleState<width, height> &goal, std::vector<MNPuzzleState<width, height>> &puzzle_vector, unsigned num_puzzles)
{
	std::map<uint64_t, uint64_t> puzzle_map; // used to ensure uniqueness
	
	MNPuzzle my_puzz(width, height);
	
	unsigned count = 0;
	unsigned goal_parity = GetParity(goal);
	
	while (count < num_puzzles)
	{
		MNPuzzleState<width, height> next = Generate_Random_Puzzle(width, height);
		uint64_t next_hash = my_puzz.GetStateHash(next);
		
		if (puzzle_map.find(next_hash) != puzzle_map.end())
		{
			continue;
		}
		
		// checks parity to make sure problem is solvable
		if (GetParity(next) == goal_parity)
		{
			puzzle_map[next_hash] = next_hash;
			puzzle_vector.push_back(next);
			count++;
		}
		
	}
}

template <int width, int height>
bool MNPuzzle<width, height>::State_Check(const MNPuzzleState<width, height> &to_check)
{
//	if (to_check.size() != width*height)
//		return false;
//	
//	if (to_check.width != width)
//		return false;
//	
//	if (to_check.height != width)
//		return false;
	
	if (to_check.blank >= width*height)
		return false;
	
	if (to_check.puzzle[to_check.blank] != 0)
		return false;
	
	return true;
}

template <int width, int height>
std::vector<slideDir> MNPuzzle<width, height>::Get_Op_Order_From_Hash(int order_num)
{
	std::vector<slideDir> ops;
	assert(order_num <= 23);
	assert(order_num >= 0);
	
	
	std::vector<int> op_nums(4);
	
	int num_left = 1;
	for (int x = 3; x >= 0; x--)
	{
		op_nums[x] = order_num % num_left;
		order_num /= num_left;
		num_left++;
		
		for (int y = x+1; y < 4; y++)
		{
			if (op_nums[y] >= op_nums[x])
			{
				op_nums[y]++;
			}
		}
	}
	
	bool up_act = false;
	bool down_act = false;
	bool left_act = false;
	bool right_act = false;
	
	for (unsigned i = 0; i < 4; i++)
	{
		if (op_nums[i] == 0)
		{
			ops.push_back(kUp);
			up_act = true;
		}
		else if (op_nums[i] == 1)
		{
			ops.push_back(kLeft);
			left_act = true;
		}
		else if (op_nums[i] == 2)
		{
			ops.push_back(kRight);
			right_act = true;
		}
		else if (op_nums[i] == 3)
		{
			ops.push_back(kDown);
			down_act = true;
		}
	}
	
	assert(up_act);
	assert(left_act);
	assert(right_act);
	assert(down_act);
	
	return ops;
}



#endif
