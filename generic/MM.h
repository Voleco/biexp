//
//  MM.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 10/27/15.
//  Copyright © 2015 University of Denver. All rights reserved.
//

#ifndef MM_h
#define MM_h

#include "AStarOpenClosed.h"
#include "FPUtil.h"
#include "Timer.h"
#include <unordered_map>
#include <map>

#define EFFICIENT

template <class state, int epsilon = 1>
struct MMCompare {
	bool operator()(const AStarOpenClosedData<state> &i1, const AStarOpenClosedData<state> &i2) const
	{
		double p1 = std::max(i1.g+i1.h, i1.g*2+epsilon);
		double p2 = std::max(i2.g+i2.h, i2.g*2+epsilon);
//		double p1 = i1.g+i1.h;
//		double p2 = i2.g+i2.h;
		if (fequal(p1, p2))
		{
			//return (fgreater(i1.g, i2.g)); // low g-cost over high
			return (fless(i1.g, i2.g)); // high g-cost over low
		}
		return (fgreater(p1, p2)); // low priority over high
	}
};

namespace std {
	template <> struct hash<std::pair<double, double>>
	{
		size_t operator()(const std::pair<double, double> & x) const
		{
			return std::hash<double>()(x.first)^(std::hash<double>()(x.second)<<16);
		}
	};
}

//static bool operator< (const std::pair<double, double>& p1, const std::pair<double, double>& p2)
//{
//	return (p1.first + p1.second < p2.first + p2.second);
//}


template <class state, class action, class environment, class priorityQueue = AStarOpenClosed<state, MMCompare<state>> >
class MM {
public:
	MM(double epsilon = 1.0):epsilon(epsilon) { forwardHeuristic = 0; backwardHeuristic = 0; env = 0; ResetNodeCount(); }
	virtual ~MM() {}
	void GetPath(environment *env, const state& from, const state& to,
				 Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath);
	bool InitializeSearch(environment *env, const state& from, const state& to,
						  Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath);
	bool DoSingleSearchStep(std::vector<state> &thePath);
	
	virtual const char *GetName() { return "MM"; }
	
	void ResetNodeCount() { nodesExpanded = nodesTouched = uniqueNodesExpanded = 0; }
	
//	bool GetClosedListGCost(const state &val, double &gCost) const;
//	unsigned int GetNumOpenItems() { return openClosedList.OpenSize(); }
//	inline const AStarOpenClosedData<state> &GetOpenItem(unsigned int which) { return openClosedList.Lookat(openClosedList.GetOpenItem(which)); }
	inline const int GetNumForwardItems() { return forwardQueue.size(); }
	inline const AStarOpenClosedData<state> &GetForwardItem(unsigned int which) { return forwardQueue.Lookat(which); }
	inline const int GetNumBackwardItems() { return backwardQueue.size(); }
	inline const AStarOpenClosedData<state> &GetBackwardItem(unsigned int which) { return backwardQueue.Lookat(which); }
//	bool HaveExpandedState(const state &val)
//	{ uint64_t key; return openClosedList.Lookup(env->GetStateHash(val), key) != kNotFound; }
//	
//	void SetForwardHeuristic(Heuristic<state> *h) { forwardHeuristic = h; }
//	void SetBackwardHeuristic(Heuristic<state> *h) { backwardHeuristic = h; }
	
	uint64_t GetUniqueNodesExpanded() const { return uniqueNodesExpanded; }
	uint64_t GetNodesExpanded() const { return nodesExpanded; }
	uint64_t GetNodesTouched() const { return nodesTouched; }
	uint64_t GetNecessaryExpansions() const;
	//void FullBPMX(uint64_t nodeID, int distance);
	
	void OpenGLDraw() const;
	void PrintHDist()
	{
		std::vector<uint64_t> d;
		for (auto i = dist.begin(); i != dist.end(); i++)
		{
			if (i->first.first < i->first.second)
			{
				int h = (int)i->first.second;
				if (h >= d.size())
					d.resize(h+1);
				d[h] += i->second;
			}
		}
		printf("MM Dynamic Distribution\n");
		for (int x = 0; x < d.size(); x++)
		{
			if (d[x] != 0)
				printf("%d\t%llu\n", x, d[x]);
		}
	}
	void PrintOpenStats(std::unordered_map<std::pair<double, double>, int>  &s)
	{
		printf("Search distributions: (%s)\n", ((&s)==(&f))?"forward":"backward");
		for (auto i = s.begin(); i != s.end(); i++)
		{
			if (i->second > 0)
			{
				bool ignore = false;
				ignore = (i->first.first+i->first.second >= currentCost);
				printf("%c g: %1.1f h: %1.1f count: %d\n", ignore?'*':' ',
					   i->first.first, i->first.second, i->second);
			}
		}
	}

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
		Heuristic<state> *heuristic,
		const state &target,
#ifdef EFFICIENT
		std::map<double, int>& fcount,
		std::map<double, int>& gcount);
#else
		std::unordered_map<std::pair<double, double>, int> &count);
#endif // EFFICIENT
//				std::unordered_map<double, int> &ming,
//				std::unordered_map<double, int> &minf);
	priorityQueue forwardQueue, backwardQueue;
	state goal, start;
	std::unordered_map<std::pair<double, double>, int> dist;
	std::unordered_map<std::pair<double, double>, int> f, b;

#ifdef EFFICIENT
	std::map<double, int> fGCounts, bGCounts;
	std::map<double, int> fFCounts, bFCounts;
#endif // EFFICIENT
	uint64_t nodesTouched, nodesExpanded, uniqueNodesExpanded;
	state middleNode;
	double currentCost;
	double lastMinForwardG;
	double lastMinBackwardG;
	double epsilon;

	std::vector<state> neighbors;
	environment *env;
	Timer t;
	Heuristic<state> *forwardHeuristic;
	Heuristic<state> *backwardHeuristic;

	double oldp1;
	double oldp2;
	bool recheckPath;
};

template <class state, class action, class environment, class priorityQueue>
void MM<state, action, environment, priorityQueue>::GetPath(environment *env, const state& from, const state& to,
			 Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath)
{
	if (InitializeSearch(env, from, to, forward, backward, thePath) == false)
		return;
	t.StartTimer();
	while (!DoSingleSearchStep(thePath))
	{ }
}

template <class state, class action, class environment, class priorityQueue>
bool MM<state, action, environment, priorityQueue>::InitializeSearch(environment *env, const state& from, const state& to,
																	 Heuristic<state> *forward, Heuristic<state> *backward,
																	 std::vector<state> &thePath)
{
	this->env = env;
	forwardHeuristic = forward;
	backwardHeuristic = backward;
	currentCost = DBL_MAX;
	forwardQueue.Reset();
	backwardQueue.Reset();
	ResetNodeCount();
	thePath.resize(0);
	start = from;
	goal = to;
	if (start == goal)
		return false;
	oldp1 = oldp2 = 0;
	lastMinForwardG = 0;
	lastMinBackwardG = 0;
	forwardQueue.AddOpenNode(start, env->GetStateHash(start), 0, forwardHeuristic->HCost(start, goal));
	backwardQueue.AddOpenNode(goal, env->GetStateHash(goal), 0, backwardHeuristic->HCost(goal, start));
	f.clear();
	b.clear();

#ifdef EFFICIENT
	fFCounts.clear();
	bFCounts.clear();
	fGCounts.clear();
	bGCounts.clear();
#endif
	recheckPath = false;
	return true;
}

template <class state, class action, class environment, class priorityQueue>
bool MM<state, action, environment, priorityQueue>::DoSingleSearchStep(std::vector<state> &thePath)
{
	if (forwardQueue.OpenSize() == 0 || backwardQueue.OpenSize() == 0)
	{
		return true;
	}
	
//	if (forwardQueue.OpenSize() == 0)
//		//Expand(backwardQueue, forwardQueue, backwardHeuristic, start, g_b, f_b);
//		Expand(backwardQueue, forwardQueue, backwardHeuristic, start, b);
//	
//	if (backwardQueue.OpenSize() == 0)
//		//Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, g_f, f_f);
//		Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, f);

	uint64_t forward = forwardQueue.Peek();
	uint64_t backward = backwardQueue.Peek();
	
	const AStarOpenClosedData<state> &nextForward = forwardQueue.Lookat(forward);
	const AStarOpenClosedData<state> &nextBackward = backwardQueue.Lookat(backward);

	double p1 = std::max(nextForward.g+nextForward.h, nextForward.g*2 + epsilon);
	double p2 = std::max(nextBackward.g+nextBackward.h, nextBackward.g*2 + epsilon);
	bool done1 = false;
	if (p1 > oldp1)
	{
//		printf("Forward priority to %1.2f [%llu expanded - %1.2fs]\n", p1, GetNodesExpanded(), t.EndTimer());
		oldp1 = p1;

//#ifdef EFFICIENT
		if (currentCost < DBL_MAX && p1>=currentCost)
			done1 = true;
//#endif // EFFICIENT

		//PrintOpenStats(f);
	}
	if (p2 > oldp2)
	{
//		printf("Backward priority to %1.2f [%llu expanded - %1.2fs]\n", p2, GetNodesExpanded(), t.EndTimer());
		oldp2 = p2;
//#ifdef EFFICIENT
		if (currentCost < DBL_MAX && p2>=currentCost)
			done1 = true;
//#endif // EFFICIENT
		//PrintOpenStats(b);
	}

	if (done1)
	{
		//			PrintOpenStats(f);
		//			PrintOpenStats(b);

		std::vector<state> pFor, pBack;
		ExtractPathToGoal(middleNode, pBack);
		ExtractPathToStart(middleNode, pFor);
		reverse(pFor.begin(), pFor.end());
		thePath = pFor;
		thePath.insert(thePath.end(), pBack.begin() + 1, pBack.end());

		return true;
	}
	
	if (fless(p1, p2))
	{
		//Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, g_f, f_f);
#ifdef EFFICIENT
		Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, fFCounts,fGCounts);
#else
		Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, f);
#endif
	}
	else if (fless(p2, p1))
	{
		//Expand(backwardQueue, forwardQueue, backwardHeuristic, start, g_b, f_b);
#ifdef EFFICIENT
		Expand(backwardQueue, forwardQueue, backwardHeuristic, start, bFCounts,bGCounts);
#else
		Expand(backwardQueue, forwardQueue, backwardHeuristic, start, b);
#endif
	}
	else { // equal priority
		if (fless(nextForward.g, nextBackward.g))
		{
			//Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, g_f, f_f);
#ifdef EFFICIENT
			Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, fFCounts,fGCounts);
#else
			Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, f);
#endif
		}
		else if (fless(nextBackward.g, nextForward.g))
		{
			//Expand(backwardQueue, forwardQueue, backwardHeuristic, start, g_b, f_b);
#ifdef EFFICIENT
			Expand(backwardQueue, forwardQueue, backwardHeuristic, start, bFCounts,bGCounts);
#else
			Expand(backwardQueue, forwardQueue, backwardHeuristic, start, b);
#endif
		}
		else {
#ifdef EFFICIENT
			Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, fFCounts,fGCounts);
#else
			Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, f);
#endif
			//Expand(forwardQueue, backwardQueue, forwardHeuristic, goal, g_f, f_f);
		}
	}
	// check if we can terminate
	if (recheckPath)
	{
		recheckPath = false;
		// TODO: make this more efficient
		double minForwardG = DBL_MAX;
		double minBackwardG = DBL_MAX;
		double minForwardF = DBL_MAX;
		double minBackwardF =  DBL_MAX;
		double forwardP;
		double backwardP;
	
#ifdef EFFICIENT
		while (!fGCounts.empty()) 
		{
			auto iFG = fGCounts.begin();
			if (iFG->second != 0)
			{
				minForwardG = iFG->first;
				break;
			}
			else
				fGCounts.erase(iFG);

		}

		while (!bGCounts.empty())
		{
			auto iBG = bGCounts.begin();
			if (iBG->second != 0)
			{
				minBackwardG = iBG->first;
				break;
			}
			else
				bGCounts.erase(iBG);

		}

		while (!fFCounts.empty())
		{
			auto iFF = fFCounts.begin();
			if (iFF->second != 0)
			{
				minForwardF = iFF->first;
				break;
			}
			else
				 fFCounts.erase(iFF);

		}

		while (!bFCounts.empty())
		{
			auto iBF = bFCounts.begin();
			if (iBF->second != 0)
			{
				minBackwardF = iBF->first;
				break;
			}
			else
				bFCounts.erase(iBF);

		}


#else
		for (auto i = f.begin(); i != f.end(); i++)
		{
			if (i->second > 0) // some elements
			{
				if ((i->first.first + i->first.second < currentCost) && // termination only stopped by lower f-cost
					(i->first.first + lastMinBackwardG + 1.0 < currentCost))
				{
					minForwardG = std::min(minForwardG, i->first.first);
					minForwardF = std::min(minForwardF, i->first.first+i->first.second);
				}
			}
		}
		for (auto i = b.begin(); i != b.end(); i++)
		{
			if (i->second > 0) // some elements
			{
				if ((i->first.first + i->first.second < currentCost) && // termination only stopped by lower f-cost
					(i->first.first + lastMinForwardG + 1.0 < currentCost))
				{
					minBackwardG = std::min(minBackwardG, i->first.first);
					minBackwardF = std::min(minBackwardF, i->first.first+i->first.second);
				}
			}
		}
#endif
		
		{
			auto iB = backwardQueue.Lookat(backwardQueue.Peek());
			backwardP = std::max(iB.g+iB.h, iB.g*2+epsilon);
			auto iF = forwardQueue.Lookat(forwardQueue.Peek());
			forwardP = std::max(iF.g+iF.h, iF.g*2+epsilon);
		}
		bool done = false;
		if (minForwardF == DBL_MAX)
		{
			minForwardF = minForwardG = currentCost+1;
		}
		if (minBackwardF == DBL_MAX)
		{
			minBackwardF = minBackwardG = currentCost+1;
		}
		if (!fgreater(currentCost, minForwardF))
		{
//			printf("Terminated on forwardf (%f >= %f)\n", minForwardF, currentCost);
			done = true;
		}
		if (!fgreater(currentCost, minBackwardF))
		{
//			printf("Terminated on backwardf (%f >= %f)\n", minBackwardF, currentCost);
			done = true;
		}
		if (!fgreater(currentCost, minForwardG+minBackwardG+epsilon)) // TODO: epsilon
		{
//			printf("Terminated on g+g+epsilon (%f+%f+%f >= %f)\n", minForwardG, minBackwardG, epsilon, currentCost);
			done = true;
		}
		if (!fgreater(currentCost, std::min(forwardP, backwardP)))
		{
//			printf("Terminated on forwardP/backwardP (min(%f, %f) >= %f)\n", forwardP, backwardP, currentCost);
			done = true;
		}
//		if (!fgreater(currentCost, backwardP))
//		{
//			printf("Terminated on backwardP\n");
//			done = true;
//		}
		// for now, always terminate
		lastMinBackwardG = minBackwardG;
		lastMinForwardG = minForwardG;
		if (done)
		{
//			PrintOpenStats(f);
//			PrintOpenStats(b);
			
			std::vector<state> pFor, pBack;
			ExtractPathToGoal(middleNode, pBack);
			ExtractPathToStart(middleNode, pFor);
			reverse(pFor.begin(), pFor.end());
			thePath = pFor;
			thePath.insert( thePath.end(), pBack.begin()+1, pBack.end() );
			
			return true;
		}
	}
	return false;
}

template <class state, class action, class environment, class priorityQueue>
void MM<state, action, environment, priorityQueue>::Expand(priorityQueue &current,
	priorityQueue &opposite,
	Heuristic<state> *heuristic, const state &target,
#ifdef EFFICIENT
	std::map<double, int>& fcount,
	std::map<double, int>& gcount)
#else
														   std::unordered_map<std::pair<double, double>, int> &count)
#endif
//														   std::unordered_map<double, int> &ming,
//														   std::unordered_map<double, int> &minf)
{
	uint64_t nextID = current.Close();
	nodesExpanded++;
	if (current.Lookup(nextID).reopened == false)
		uniqueNodesExpanded++;

	// decrease count from parent
	{
		auto &parentData = current.Lookup(nextID);
#ifdef EFFICIENT
		gcount[parentData.g]--;
		fcount[parentData.g + parentData.h]--;
#else
		count[{parentData.g,parentData.h}]--;
#endif

#ifdef EFFICIENT
		if ((fcount[parentData.g + parentData.h]==0 || gcount[parentData.g]==0) && currentCost < DBL_MAX)
#else
		if (count[{parentData.g, parentData.h}] == 0 && currentCost < DBL_MAX)
#endif
		{
			recheckPath = true;
		}
#ifdef EFFICIENT
		//if (gcount[parentData.g] == 0)
		//	gcount.erase(gcount.find(parentData.g));
		//if (fcount[parentData.g+ parentData.h] == 0)
		//	fcount.erase(fcount.find(parentData.g+ parentData.h));
#endif

	}

	env->GetSuccessors(current.Lookup(nextID).data, neighbors);
	for (auto &succ : neighbors)
	{
		nodesTouched++;
		uint64_t childID;
		uint64_t hash = env->GetStateHash(succ);
		auto loc = current.Lookup(hash, childID);
		auto &childData = current.Lookup(childID);
		auto &parentData = current.Lookup(nextID);

		double edgeCost = env->GCost(parentData.data, succ);
		switch (loc)
		{
			case kClosedList: // ignore
				if (fless(parentData.g+edgeCost, childData.g))
				{
					childData.h = std::max(childData.h, parentData.h-edgeCost);
					childData.parentID = nextID;
					childData.g = parentData.g+edgeCost;
#ifdef EFFICIENT
					gcount[childData.g]++;
					fcount[childData.g + childData.h]++;
#else
					count[{childData.g, childData.h}]++;
#endif
					dist[{childData.g,childData.h}]++;
					current.Reopen(childID);
				}
				break;
			case kOpenList: // update cost if needed
			{
				// 1-step BPMX
				parentData.h = std::max(childData.h-edgeCost, parentData.h);

				if (fgreater(parentData.h-edgeCost, childData.h))
				{
					//gcount does not change
#ifdef EFFICIENT
					fcount[childData.g+childData.h]--;
					//if (fcount[childData.g + childData.h] == 0)
					//	fcount.erase(fcount.find(childData.g + childData.h));
#else
					count[{childData.g, childData.h}]--;
#endif
					dist[{childData.g,childData.h}]--;
					//minf[childData.g+childData.h]--;
					childData.h = parentData.h-edgeCost;
					//minf[childData.g+childData.h]++;
					
#ifdef EFFICIENT
					fcount[childData.g + childData.h]++;
#else
					count[{childData.g, childData.h}]++;
#endif
					dist[{childData.g,childData.h}]++;
				}
				if (fless(parentData.g+edgeCost, childData.g))
				{
					//gcount changes
#ifdef EFFICIENT
					gcount[childData.g]--;
					fcount[childData.g + childData.h]--;
					//if (gcount[childData.g] == 0)
					//	gcount.erase(gcount.find(childData.g));
					//if (fcount[childData.g + childData.h] == 0)
					//	fcount.erase(fcount.find(childData.g + childData.h));
#else
					count[{childData.g, childData.h}]--;
#endif
					dist[{childData.g,childData.h}]--;
					childData.parentID = nextID;
					childData.g = parentData.g+edgeCost;
					current.KeyChanged(childID);
					
#ifdef EFFICIENT
					gcount[childData.g]++;
					fcount[childData.g + childData.h]++;
#else
					count[{childData.g, childData.h}]++;
#endif
					dist[{childData.g,childData.h}]++;
					
					
					// TODO: check if we improved the current solution?
					uint64_t reverseLoc;
					auto loc = opposite.Lookup(hash, reverseLoc);
					if (loc == kOpenList)
					{
						if (fless(parentData.g+edgeCost + opposite.Lookup(reverseLoc).g, currentCost))
						{
							recheckPath = true;
							// TODO: store current solution
//							printf("Potential updated solution found, cost: %1.2f + %1.2f = %1.2f\n",
//								   parentData.g+edgeCost,
//								   opposite.Lookup(reverseLoc).g,
//								   parentData.g+edgeCost+opposite.Lookup(reverseLoc).g);
							currentCost = parentData.g+edgeCost + opposite.Lookup(reverseLoc).g;
							middleNode = succ;
//							PrintOpenStats(f);
//							PrintOpenStats(b);
						}
					}
				}
			}
				break;
			case kNotFound:
			{
				double g = parentData.g+edgeCost;
				double h = std::max(heuristic->HCost(succ, target), parentData.h-edgeCost);

				// Ignore nodes that don't have lower f-cost than the incumbant solution
				if (!fless(g+h, currentCost))
					break;
//				ming[g]++;
//				minf[g+h]++;

#ifdef EFFICIENT
				gcount[g]++;
				fcount[g + h]++;
#else
				count[{g,h}]++;
#endif
				dist[{g,h}]++;
				// 1-step BPMX
				parentData.h = std::max(h-edgeCost, parentData.h);

				current.AddOpenNode(succ, // This may invalidate our references
									hash,
									g,
									h,
									nextID);
				
				// check for solution
				uint64_t reverseLoc;
				auto loc = opposite.Lookup(hash, reverseLoc);
				if (loc == kOpenList)
				{
					if (fless(current.Lookup(nextID).g+edgeCost + opposite.Lookup(reverseLoc).g, currentCost))
					{
						recheckPath = true;
						// TODO: store current solution
//						printf("Potential solution found, cost: %1.2f + %1.2f = %1.2f\n",
//							   current.Lookup(nextID).g+edgeCost,
//							   opposite.Lookup(reverseLoc).g,
//							   current.Lookup(nextID).g+edgeCost+opposite.Lookup(reverseLoc).g);
						currentCost = current.Lookup(nextID).g+edgeCost + opposite.Lookup(reverseLoc).g;
						middleNode = succ;
//						PrintOpenStats(f);
//						PrintOpenStats(b);
					}
				}
			}
		}
	}
}

template <class state, class action, class environment, class priorityQueue>
uint64_t MM<state, action, environment, priorityQueue>::GetNecessaryExpansions() const
{
	uint64_t count = 0;
	for (unsigned int x = 0; x < forwardQueue.size(); x++)
	{
		const AStarOpenClosedData<state> &data = forwardQueue.Lookat(x);
		if ((data.where == kClosedList) && (fless(data.g+data.h, currentCost)))
			count++;
	}
	for (unsigned int x = 0; x < backwardQueue.size(); x++)
	{
		const AStarOpenClosedData<state> &data = backwardQueue.Lookat(x);
		if ((data.where == kClosedList) && (fless(data.g+data.h, currentCost)))
			count++;
	}
	return count;
}

template <class state, class action, class environment, class priorityQueue>
void MM<state, action, environment, priorityQueue>::OpenGLDraw() const
{
	OpenGLDraw(forwardQueue);
	OpenGLDraw(backwardQueue);
}

template <class state, class action, class environment, class priorityQueue>
void MM<state, action, environment, priorityQueue>::OpenGLDraw(const priorityQueue &queue) const
{
	double transparency = 0.9;
	if (queue.size() == 0)
		return;
	uint64_t top = -1;
	//	double minf = 1e9, maxf = 0;
	if (queue.OpenSize() > 0)
	{
		top = queue.Peek();
	}
	for (unsigned int x = 0; x < queue.size(); x++)
	{
		const AStarOpenClosedData<state> &data = queue.Lookat(x);
		if (x == top)
		{
			env->SetColor(1.0, 1.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
		if ((data.where == kOpenList) && (data.reopened))
		{
			env->SetColor(0.0, 0.5, 0.5, transparency);
			env->OpenGLDraw(data.data);
		}
		else if (data.where == kOpenList)
		{
			env->SetColor(0.0, 1.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
		else if ((data.where == kClosedList) && (data.reopened))
		{
			env->SetColor(0.5, 0.0, 0.5, transparency);
			env->OpenGLDraw(data.data);
		}
		else if (data.where == kClosedList)
		{
			env->SetColor(1.0, 0.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
	}
}

#endif /* MM_h */
