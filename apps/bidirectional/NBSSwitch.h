//
//	NBSSwitch.h
//	This file derived from MM.h by Nathan Sturtevant
//	The following is the original claim
//
//  MM.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 10/27/15.
//  Copyright © 2015 University of Denver. All rights reserved.
//

#ifndef NBSSWITCH_H
#define NBSSWITCH_H

#include "BDOpenClosed.h"
#include "FPUtil.h"

#include<map>

#define EPSILON 1

using std::cout;
//low g -> low f
template <class state>
struct NBSSwitchCompareOpenReady {
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + i1.h;
		double f2 = i2.g + i2.h;

		if (fequal(i1.g, i2.g))
		{
			return (!fless(f1, f2)); //equal g, low f over high
		}
		return (fgreater(i1.g, i2.g)); // low g over high
	}
};

template <class state>
struct NBSSwitchCompareOpenWaiting {
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + i1.h;
		double f2 = i2.g + i2.h;

		if (fequal(f1, f2))
		{
		    return (!fgreater(i1.g, i2.g)); // high g-cost over low
			//return (!fgreater(i2.g, i1.g)); // low g-cost over low
		}
		return (fgreater(f1, f2)); // low f over high
	}
};

template <class state, class action, class environment,  class priorityQueue = BDOpenClosed<state, NBSSwitchCompareOpenReady<state>, NBSSwitchCompareOpenWaiting<state>>>
class NBSSwitch {
public:
	NBSSwitch()
	{
		forwardHeuristic = 0; backwardHeuristic = 0; env = 0; ResetNodeCount();
	}
	virtual ~NBSSwitch() {}
	void GetPath(environment *env, const state& from, const state& to,
				 Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath);
	bool InitializeSearch(environment *env, const state& from, const state& to,
						  Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath);
	bool ExpandAPair(std::vector<state> &thePath);
	bool DoSingleSearchStep(std::vector<state> &thePath);
	

	void SetTrueSolutionCost(double c) { trueCost = c; }
	
	virtual const char *GetName() { return "NBSSwitch"; }
	
	void ResetNodeCount() { nodesExpanded = nodesTouched = nodesExpandedOfTies = nodesExpandedToProve= 0; }
	
//	bool GetClosedListGCost(const state &val, double &gCost) const;
//	unsigned int GetNumOpenItems() { return openClosedList.OpenSize(); }
//	inline const AStarOpenClosedData<state> &GetOpenItem(unsigned int which) { return openClosedList.Lookat(openClosedList.GetOpenItem(which)); }
	inline const int GetNumForwardItems() { return forwardQueue.size(); }
	inline const BDOpenClosedData<state> &GetForwardItem(uint64_t which) { return forwardQueue.Lookat(which); }
	inline const int GetNumBackwardItems() { return backwardQueue.size(); }
	inline const BDOpenClosedData<state> &GetBackwardItem(uint64_t which) { return backwardQueue.Lookat(which); }
//	bool HaveExpandedState(const state &val)
//	{ uint64_t key; return openClosedList.Lookup(env->GetStateHash(val), key) != kNotFound; }
//	
	void SetForwardHeuristic(Heuristic<state> *h) { forwardHeuristic = h; }
	void SetBackwardHeuristic(Heuristic<state> *h) { backwardHeuristic = h; }
	stateLocation GetNodeForwardLocation(const state &s) 
	{
		uint64_t childID;
		auto l = forwardQueue.Lookup(env->GetStateHash(s), childID);
			return l;
	}
	stateLocation GetNodeBackwardLocation(const state &s)
	{
		uint64_t childID;
		return backwardQueue.Lookup(env->GetStateHash(s), childID);
	}
	double GetNodeForwardG(const state& s)
	{

		uint64_t childID;
		auto l = forwardQueue.Lookup(env->GetStateHash(s), childID);
		if (l != kUnseen)
			return forwardQueue.Lookat(childID).g;
		return -1;
	}
	double GetNodeBackwardG(const state& s)
	{

		uint64_t childID;
		auto l = backwardQueue.Lookup(env->GetStateHash(s), childID);
		if (l != kUnseen)
			return backwardQueue.Lookat(childID).g;
		return -1;
	}
	double GetNodeForwardH(const state& s)
	{

		uint64_t childID;
		auto l = forwardQueue.Lookup(env->GetStateHash(s), childID);
		if (l != kUnseen)
			return forwardQueue.Lookat(childID).h;
		return -1;
	}
	double GetNodeBackwardH(const state& s)
	{

		uint64_t childID;
		auto l = backwardQueue.Lookup(env->GetStateHash(s), childID);
		if (l != kUnseen)
			return backwardQueue.Lookat(childID).h;
		return -1;
	}
	uint64_t GetNodesExpanded() const { return nodesExpanded; }
	uint64_t GetNodesExpandedToProve() const { return nodesExpandedToProve; }
	uint64_t GetNodesExpandedOfTies() const { return nodesExpandedOfTies; }
	uint64_t GetNodesTouched() const { return nodesTouched; }
	double GetSolutionCost() const { return currentCost; }
	//void FullBPMX(uint64_t nodeID, int distance);
	
	bool switched = false;
	void RebucketNodes();
	void SwitchVC();
	void ProveUnidirectionally(std::vector<state> &thePath);
	void OpenGLDraw() const;
	
//	void SetWeight(double w) {weight = w;}
private:
	void ExtractPathToGoal(state &node, std::vector<state> &thePath)
	{ uint64_t theID; backwardQueue.Lookup(env->GetStateHash(node), theID); ExtractPathToGoalFromID(theID, thePath); }
	void ExtractPathToGoalFromID(uint64_t node, std::vector<state> &thePath)
	{
		do {
			thePath.push_back(backwardQueue.Lookup(node).data);
			node = backwardQueue.Lookup(node).parentID;
		} while (backwardQueue.Lookup(node).parentID != node);
		thePath.push_back(backwardQueue.Lookup(node).data);
	}
	
	void ExtractPathToStart(state &node, std::vector<state> &thePath)
	{ uint64_t theID; forwardQueue.Lookup(env->GetStateHash(node), theID); ExtractPathToStartFromID(theID, thePath); }
	void ExtractPathToStartFromID(uint64_t node, std::vector<state> &thePath)
	{
		do {
			thePath.push_back(forwardQueue.Lookup(node).data);
			node = forwardQueue.Lookup(node).parentID;
		} while (forwardQueue.Lookup(node).parentID != node);
		thePath.push_back(forwardQueue.Lookup(node).data);
	}

	void OpenGLDraw(const priorityQueue &queue) const;
	
	void Expand(priorityQueue &current,
				priorityQueue &opposite,
				Heuristic<state> *heuristic, const state &target);
	//direction ==0 forward; 1 backward
	//void Expand(int direction);
	uint64_t nodesTouched, nodesExpanded, nodesExpandedOfTies, nodesExpandedToProve;
	state middleNode;
	double currentCost;
	double currentSolutionEstimate;
	std::vector<state> neighbors;
	environment *env;
	

	priorityQueue forwardQueue, backwardQueue;
	//priorityQueue2 forwardQueue, backwardQueue;

	state goal, start;

	Heuristic<state> *forwardHeuristic;
	Heuristic<state> *backwardHeuristic;


	std::map<double, int> forwardCounts, backwardCounts;
	std::map<double, std::vector<uint64_t>> forwardBuckets, backwardBuckets;
	//keep track of whether we expand a node or put it back to open
	bool expand;

	double currentPr;
	double trueCost;

};

template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::GetPath(environment *env, const state& from, const state& to,
			 Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath)
{
	if (InitializeSearch(env, from, to, forward, backward, thePath) == false)
		return;
	
	//while (!ExpandAPair(thePath))
	//{ }

	while (currentCost == DBL_MAX)
	{
		if (ExpandAPair(thePath) == true)//in this case, the problem has no solution
			return;
	}
	ProveUnidirectionally(thePath);
	//RebucketNodes();
	//SwitchVC();
}

template <class state, class action, class environment, class priorityQueue>
bool NBSSwitch<state, action, environment, priorityQueue>::InitializeSearch(environment *env, const state& from, const state& to,
																	 Heuristic<state> *forward, Heuristic<state> *backward,
																	 std::vector<state> &thePath)
{
	this->env = env;
	forwardHeuristic = forward;
	backwardHeuristic = backward;
	currentSolutionEstimate = 0;
	currentCost = DBL_MAX;
	trueCost = DBL_MAX;
	forwardQueue.Reset();
	backwardQueue.Reset();
	ResetNodeCount();
	thePath.resize(0);
	start = from;
	goal = to;
	if (start == goal)
		return false;

	forwardQueue.AddOpenNode(start, env->GetStateHash(start), 0, forwardHeuristic->HCost(start, goal));
	backwardQueue.AddOpenNode(goal, env->GetStateHash(goal), 0, forwardHeuristic->HCost(goal, start));

	return true;
}

template <class state, class action, class environment, class priorityQueue>
bool NBSSwitch<state, action, environment, priorityQueue>::ExpandAPair(std::vector<state> &thePath)
{
	
	if (forwardQueue.OpenSize() == 0 || backwardQueue.OpenSize() == 0)
	{
		//		printf("No more pairs, terminate! (%f)\n", currentCost);
		if (currentCost != DBL_MAX)
		{
			std::vector<state> pFor, pBack;
			ExtractPathToGoal(middleNode, pBack);
			ExtractPathToStart(middleNode, pFor);
			reverse(pFor.begin(), pFor.end());
			thePath = pFor;
			thePath.insert( thePath.end(), pBack.begin()+1, pBack.end() );
		}
		return true;
	}
	
	uint64_t nextIDForward;
	uint64_t nextIDBackward;
	BDOpenClosedData<state> iFReady, iBReady, iFWaiting, iBWaiting;
	
	if (forwardQueue.OpenReadySize() == 0)
		forwardQueue.PutToReady();
	if (backwardQueue.OpenReadySize() == 0)
		backwardQueue.PutToReady();
	
	if (forwardQueue.OpenWaitingSize() > 0)
	{
		iFWaiting = forwardQueue.Lookat(forwardQueue.Peek(kOpenWaiting));
		while (iFWaiting.g+iFWaiting.h < currentSolutionEstimate)
		{
			forwardQueue.PutToReady();
			if (forwardQueue.OpenWaitingSize()  == 0)
				break;
			iFWaiting = forwardQueue.Lookat(forwardQueue.Peek(kOpenWaiting));
		}
	}
	if (backwardQueue.OpenWaitingSize() > 0)
	{
		iBWaiting = backwardQueue.Lookat(backwardQueue.Peek(kOpenWaiting));
		while (iBWaiting.g+iBWaiting.h < currentSolutionEstimate)
		{
			backwardQueue.PutToReady();
			if (backwardQueue.OpenWaitingSize()  == 0)
				break;
			iBWaiting = backwardQueue.Lookat(backwardQueue.Peek(kOpenWaiting));
		}
	}

	iFReady = forwardQueue.Lookat(forwardQueue.Peek(kOpenReady));
	iBReady = backwardQueue.Lookat(backwardQueue.Peek(kOpenReady));
	
	if (forwardQueue.OpenWaitingSize() != 0 || backwardQueue.OpenWaitingSize() != 0)
	{
		double fBound, gBound;
		
		gBound = iFReady.g + iBReady.g + EPSILON;
		if (forwardQueue.OpenWaitingSize() == 0)
		{
			iBWaiting = backwardQueue.Lookat(backwardQueue.Peek(kOpenWaiting));
			fBound = iBWaiting.g + iBWaiting.h;
		}
		else if (backwardQueue.OpenWaitingSize() == 0)
		{
			iFWaiting = forwardQueue.Lookat(forwardQueue.Peek(kOpenWaiting));
			fBound = iFWaiting.g + iFWaiting.h;
		}
		else//forwardQueue.OpenWaitingSize() > 0 && backwardQueue.OpenWaitingSize() > 0
		{
			iFWaiting = forwardQueue.Lookat(forwardQueue.Peek(kOpenWaiting));
			iBWaiting = backwardQueue.Lookat(backwardQueue.Peek(kOpenWaiting));
			fBound = std::min(iFWaiting.g + iFWaiting.h, iBWaiting.g + iBWaiting.h);
		}
		
		while (fgreater(gBound, fBound))
		{
			if (forwardQueue.OpenWaitingSize() == 0)
				backwardQueue.PutToReady();
			else if (backwardQueue.OpenWaitingSize() == 0)
				forwardQueue.PutToReady();
			else
			{
				nextIDForward = forwardQueue.Peek(kOpenWaiting);
				nextIDBackward = backwardQueue.Peek(kOpenWaiting);
				auto iF = forwardQueue.Lookat(nextIDForward);
				auto iB = backwardQueue.Lookat(nextIDBackward);
				if (fless(iF.g + iF.h, iB.g + iB.h))
					forwardQueue.PutToReady();
				else
					backwardQueue.PutToReady();
			}
			
			if (forwardQueue.OpenWaitingSize() == 0 && backwardQueue.OpenWaitingSize() == 0)
				break;
			else if (forwardQueue.OpenWaitingSize() == 0)
			{
				iBWaiting = backwardQueue.Lookat(backwardQueue.Peek(kOpenWaiting));
				
				fBound = iBWaiting.g + iBWaiting.h;
			}
			else if (backwardQueue.OpenWaitingSize() == 0)
			{
				iFWaiting = forwardQueue.Lookat(forwardQueue.Peek(kOpenWaiting));
				fBound = iFWaiting.g + iFWaiting.h;
			}
			else //forwardQueue.OpenWaitingSize() > 0 && backwardQueue.OpenWaitingSize() > 0
			{
				iFWaiting = forwardQueue.Lookat(forwardQueue.Peek(kOpenWaiting));
				iBWaiting = backwardQueue.Lookat(backwardQueue.Peek(kOpenWaiting));
				
				fBound = std::min(iFWaiting.g + iFWaiting.h, iBWaiting.g + iBWaiting.h);
				//TODO epsilon
			}
			iFReady = forwardQueue.Lookat(forwardQueue.Peek(kOpenReady));
			iBReady = backwardQueue.Lookat(backwardQueue.Peek(kOpenReady));
			gBound = iFReady.g + iBReady.g + EPSILON;
		}
		currentSolutionEstimate = fBound;
		iFReady = forwardQueue.Lookat(forwardQueue.Peek(kOpenReady));
		iBReady = backwardQueue.Lookat(backwardQueue.Peek(kOpenReady));
	}
	//assert(forwardQueue.ValidateOpenReady());
	//assert(forwardQueue.ValidateOpenWaiting());
	//assert(backwardQueue.ValidateOpenReady());
	//assert(backwardQueue.ValidateOpenWaiting());

	if (iFReady.data == iBReady.data) // terminate - fronts equal
	{
		std::cout << "terminate - fronts equal\n";
		if (currentCost != DBL_MAX)
		{
			std::vector<state> pFor, pBack;
			ExtractPathToGoal(middleNode, pBack);
			ExtractPathToStart(middleNode, pFor);
			reverse(pFor.begin(), pFor.end());
			thePath = pFor;
			thePath.insert( thePath.end(), pBack.begin()+1, pBack.end() );
		}
		return true;
	}
	double minPr = std::max(iFReady.g + iFReady.h, iBReady.g + iBReady.h);
	minPr = std::max(minPr, iFReady.g + iBReady.g + EPSILON);

	if (!fless(minPr, currentCost)) // terminate - priority >= incumbant solution
	{
		if (currentCost != DBL_MAX)
		{
			std::vector<state> pFor, pBack;
			ExtractPathToGoal(middleNode, pBack);
			ExtractPathToStart(middleNode, pFor);
			reverse(pFor.begin(), pFor.end());
			thePath = pFor;
			thePath.insert( thePath.end(), pBack.begin()+1, pBack.end() );
		}
		return true;
	}

	if(fequal(minPr,trueCost))
		nodesExpandedOfTies += 2;
	Expand(forwardQueue, backwardQueue, forwardHeuristic, goal);
	Expand(backwardQueue, forwardQueue, backwardHeuristic, start);
	return false;
	
	
}




template <class state, class action, class environment, class priorityQueue>
bool NBSSwitch<state, action, environment, priorityQueue>::DoSingleSearchStep(std::vector<state> &thePath)
{
	ExpandAPair();
}


template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::Expand(priorityQueue &current,
														   priorityQueue &opposite,
														   Heuristic<state> *heuristic, const state &target)
{
	expand = false;
	uint64_t nextID = current.Peek(kOpenReady);
	
	nextID = current.Close();

	//this can happen when we expand a single node instead of a pair
	if (!fless(current.Lookup(nextID).g + current.Lookup(nextID).h, currentCost))
		return;

	expand = true;
	nodesExpanded++;

	env->GetSuccessors(current.Lookup(nextID).data, neighbors);
	for (auto &succ : neighbors)
	{
		nodesTouched++;
		uint64_t childID;
		auto loc = current.Lookup(env->GetStateHash(succ), childID);
		switch (loc)
		{
			case kClosed: // ignore
				break;
			case kOpenReady: // update cost if needed
			case kOpenWaiting:
			{
				double edgeCost = env->GCost(current.Lookup(nextID).data, succ);
				if (fless(current.Lookup(nextID).g+edgeCost, current.Lookup(childID).g))
				{
					double oldGCost = current.Lookup(childID).g;
					current.Lookup(childID).parentID = nextID;
					current.Lookup(childID).g = current.Lookup(nextID).g+edgeCost;
					current.KeyChanged(childID);

					// TODO: check if we improved the current solution?
					uint64_t reverseLoc;
					auto loc = opposite.Lookup(env->GetStateHash(succ), reverseLoc);
					if (loc == kOpenReady || loc == kOpenWaiting)
					{
						if (fless(current.Lookup(nextID).g+edgeCost + opposite.Lookup(reverseLoc).g, currentCost))
						{
							// TODO: store current solution
//							printf("NBSSwitch Potential updated solution found, cost: %1.2f + %1.2f = %1.2f (%llu nodes)\n",
//								   current.Lookup(nextID).g+edgeCost,
//								   opposite.Lookup(reverseLoc).g,
//								   current.Lookup(nextID).g+edgeCost+opposite.Lookup(reverseLoc).g,
//								nodesExpanded);
							currentCost = current.Lookup(nextID).g+edgeCost + opposite.Lookup(reverseLoc).g;

							middleNode = succ;
						}
					}
					else if (loc == kClosed)
					{
						current.Remove(childID);
					}
				}
			}
				break;
			case kUnseen:
			{
				uint64_t reverseLoc;
				auto loc = opposite.Lookup(env->GetStateHash(succ), reverseLoc);
				if (loc == kClosed)// then 
				{
					break;			//do nothing. do not put this node to open
				}
				else//loc == kUnseen
				{
					double edgeCost = env->GCost(current.Lookup(nextID).data, succ);
					double newNodeF = current.Lookup(nextID).g + edgeCost + heuristic->HCost(succ, target);
					if (fless(newNodeF , currentCost))
					{
						if (fless(newNodeF , currentSolutionEstimate))
							current.AddOpenNode(succ,
												env->GetStateHash(succ),
												current.Lookup(nextID).g + edgeCost,
												heuristic->HCost(succ, target),
												nextID, kOpenReady);
						else
							current.AddOpenNode(succ,
											env->GetStateHash(succ),
											current.Lookup(nextID).g + edgeCost,
											heuristic->HCost(succ, target),
											nextID, kOpenWaiting);

					}
					if (loc == kOpenReady || loc == kOpenWaiting)
					{
						double edgeCost = env->GCost(current.Lookup(nextID).data, succ);
						if (fless(current.Lookup(nextID).g + edgeCost + opposite.Lookup(reverseLoc).g, currentCost))
						{
							// TODO: store current solution
//							printf("NBSSwitch Potential solution found, cost: %1.2f + %1.2f = %1.2f (%llu nodes)\n",
//								current.Lookup(nextID).g + edgeCost,
//								opposite.Lookup(reverseLoc).g,
//								current.Lookup(nextID).g + edgeCost + opposite.Lookup(reverseLoc).g,
//								nodesExpanded);
							currentCost = current.Lookup(nextID).g + edgeCost + opposite.Lookup(reverseLoc).g;

							middleNode = succ;
						}

					}
				}

			}
			break;
		}
	}
}



template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::OpenGLDraw() const
{
	OpenGLDraw(forwardQueue);
	OpenGLDraw(backwardQueue);
}

template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::OpenGLDraw(const priorityQueue &queue) const
{
	double transparency = 0.9;
	if (queue.size() == 0)
		return;
	uint64_t top = -1;
	//	double minf = 1e9, maxf = 0;
	if (queue.OpenReadySize() > 0)
	{
		top = queue.Peek(kOpenReady);
	}
	for (unsigned int x = 0; x < queue.size(); x++)
	{
		const auto &data = queue.Lookat(x);
		if (x == top)
		{
			env->SetColor(1.0, 1.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
		if (data.where == kOpenWaiting)
		{
			env->SetColor(0.0, 0.5, 0.5, transparency);
			env->OpenGLDraw(data.data);
		}
		else if (data.where == kOpenReady)
		{
			env->SetColor(0.0, 1.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
		else if (data.where == kClosed)
		{
			env->SetColor(1.0, 0.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
	}
}

template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::RebucketNodes()
{
	if (switched)//pull nodes from buckets
	{

	}
	else//pull nodes from priority queues
	{
		for (uint64_t i = 0; i < forwardQueue.size(); i++)
		{
			auto item = GetForwardItem(i);
			if (!fless(item.g + item.h, currentCost))
				forwardQueue.Remove(i);
		}
	}
}

template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::SwitchVC()
{

}

template <class state, class action, class environment, class priorityQueue>
void NBSSwitch<state, action, environment, priorityQueue>::ProveUnidirectionally(std::vector<state> &thePath)
{
	int fcount = 0;
	int bcount = 0;
	for (uint64_t i = 0; i < forwardQueue.size(); i++)
	{
		auto item = GetForwardItem(i);
		if (fless(item.g + item.h, currentCost))
			fcount++;
	}
	for (uint64_t i = 0; i < backwardQueue.size(); i++)
	{
		auto item = GetBackwardItem(i);
		if (fless(item.g + item.h, currentCost))
			bcount++;
	}
	if (fcount <= bcount)
	{
		while (forwardQueue.OpenWaitingSize() > 0)
			forwardQueue.PutToReady();
		while (forwardQueue.OpenSize() > 0)
		{
			if (forwardQueue.OpenReadySize() == 0)
				forwardQueue.PutToReady();
			Expand(forwardQueue, backwardQueue, forwardHeuristic, goal);
			if (expand)
				nodesExpandedToProve++;
		}
	}
	else
	{
		while (backwardQueue.OpenWaitingSize() > 0)
			backwardQueue.PutToReady();
		while (backwardQueue.OpenSize() > 0)
		{
			if (backwardQueue.OpenReadySize() == 0)
				backwardQueue.PutToReady();
			Expand(backwardQueue, forwardQueue, backwardHeuristic, start);
			if (expand)
				nodesExpandedToProve++;
		}
	}

	if (forwardQueue.OpenSize() == 0 || backwardQueue.OpenSize() == 0)
	{
		//		printf("No more pairs, terminate! (%f)\n", currentCost);
		if (currentCost != DBL_MAX)
		{
			std::vector<state> pFor, pBack;
			ExtractPathToGoal(middleNode, pBack);
			ExtractPathToStart(middleNode, pFor);
			reverse(pFor.begin(), pFor.end());
			thePath = pFor;
			thePath.insert(thePath.end(), pBack.begin() + 1, pBack.end());
		}
	}
	std::cout<<"nodes to prove: "<<nodesExpandedToProve<<"\n";
}


#endif /* NBSSwitch_h */
