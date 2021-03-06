//
//	NBSFF.h
//	This file derived from MM.h by Nathan Sturtevant
//	The following is the original claim
//
//  MM.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 10/27/15.
//  Copyright © 2015 University of Denver. All rights reserved.
//

#ifndef NBSFF_H
#define NBSFF_H

#include "BDFFOpenClosed.h"
#include "FPUtil.h"
#include <unordered_map>
#include "NBSFFQueue.h"
//#include "NBSFFQueueGF.h"
//#include "Graphics.h"
//#define EPSILON 1

// ADMISSIBLE is defined at "BDOpenClosed.h"


using std::cout;


template <class state, class action, class environment, class dataStructure = NBSFFQueue<state>,
          class priorityQueue = BDFFOpenClosed<state, NBSFFCompareOpenReady<state>, NBSFFCompareOpenWaiting<state>>>
class NBSFF {
public:
	NBSFF()
	{
		forwardHeuristic = 0; backwardHeuristic = 0; env = 0; ResetNodeCount(); 		
	}
	virtual ~NBSFF() {}
	void GetPath(environment *env, const state& from, const state& to,
				 Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath);
	bool InitializeSearch(environment *env, const state& from, const state& to,
						  Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath);
	bool ExpandAPair(std::vector<state> &thePath);
	bool DoSingleSearchStep(std::vector<state> &thePath);
	
	
	
	
	virtual const char *GetName() { return "NBSFF"; }
	
	void ResetNodeCount() { nodesExpanded = nodesTouched = 0; counts.clear(); }
	
	inline const int GetNumForwardItems() { return queue.forwardQueue.size(); }
	inline const BDOpenClosedData<state> &GetForwardItem(unsigned int which) { return queue.forwardQueue.Lookat(which); }
	inline const int GetNumBackwardItems() { return queue.backwardQueue.size(); }
	inline const BDOpenClosedData<state> &GetBackwardItem(unsigned int which) { return queue.backwardQueue.Lookat(which); }
	
	void SetForwardHeuristic(Heuristic<state> *h) { forwardHeuristic = h; }
	void SetBackwardHeuristic(Heuristic<state> *h) { backwardHeuristic = h; }
	stateLocation GetNodeForwardLocation(const state &s)
	{
		uint64_t childID;
		auto l = queue.forwardQueue.Lookup(env->GetStateHash(s), childID);
		return l;
	}
	stateLocation GetNodeBackwardLocation(const state &s)
	{
		uint64_t childID;
		return queue.backwardQueue.Lookup(env->GetStateHash(s), childID);
	}
	double GetNodeForwardG(const state& s)
	{
		uint64_t childID;
		auto l = queue.forwardQueue.Lookup(env->GetStateHash(s), childID);
		if (l != kUnseen)
			return queue.forwardQueue.Lookat(childID).g;
		return -1;
	}
	double GetNodeBackwardG(const state& s)
	{
		uint64_t childID;
		auto l = queue.backwardQueue.Lookup(env->GetStateHash(s), childID);
		if (l != kUnseen)
			return queue.backwardQueue.Lookat(childID).g;
		return -1;
	}
	uint64_t GetNodesExpanded() const { return nodesExpanded; }
	uint64_t GetNodesTouched() const { return nodesTouched; }
	uint64_t GetDoubleExpansions() const;
	uint64_t GetNecessaryExpansions() const
	{
		uint64_t necessary = 0;
		for (const auto &i : counts)
		{
			if (i.first < currentCost)
				necessary+=i.second;
		}
		return necessary;
	}
	// returns 0...1 for the percentage of the optimal path length on each frontier
	float GetMeetingPoint()
	{
		uint64_t fID, bID;
		queue.backwardQueue.Lookup(env->GetStateHash(middleNode), bID);
		queue.forwardQueue.Lookup(env->GetStateHash(middleNode), fID);
		assert (fequal(queue.backwardQueue.Lookup(bID).g+queue.forwardQueue.Lookup(fID).g, currentCost));
		return queue.backwardQueue.Lookup(bID).g/currentCost;
	}
	double GetSolutionCost() const { return currentCost; }
	
	void OpenGLDraw() const;
	void OpenGLDrawPlot() const;
	//void Draw(Graphics::Display &d) const;
	//void DrawBipartiteGraph(Graphics::Display &d) const;
	

private:
	void ExtractFromMiddle(std::vector<state> &thePath);
	void ExtractPathToGoal(state &node, std::vector<state> &thePath)
	{ uint64_t theID; queue.backwardQueue.Lookup(env->GetStateHash(node), theID); ExtractPathToGoalFromID(theID, thePath); }
	void ExtractPathToGoalFromID(uint64_t node, std::vector<state> &thePath)
	{
		do {
			thePath.push_back(queue.backwardQueue.Lookup(node).data);
			node = queue.backwardQueue.Lookup(node).parentID;
		} while (queue.backwardQueue.Lookup(node).parentID != node);
		thePath.push_back(queue.backwardQueue.Lookup(node).data);
	}
	
	void ExtractPathToStart(state &node, std::vector<state> &thePath)
	{
		uint64_t theID;
		auto loc = queue.forwardQueue.Lookup(env->GetStateHash(node), theID);
		ExtractPathToStartFromID(theID, thePath);
	}
	void ExtractPathToStartFromID(uint64_t node, std::vector<state> &thePath)
	{
		do {
			thePath.push_back(queue.forwardQueue.Lookup(node).data);
			node = queue.forwardQueue.Lookup(node).parentID;
		} while (queue.forwardQueue.Lookup(node).parentID != node);
		thePath.push_back(queue.forwardQueue.Lookup(node).data);
	}
	
	void OpenGLDraw(const priorityQueue &queue) const;
	//void Draw(Graphics::Display &d, const priorityQueue &queue) const;
	
	void Expand(uint64_t nextID,
				priorityQueue &current,
				priorityQueue &opposite,
				Heuristic<state> *fheuristic, Heuristic<state> *bheuristic,
		const state &target);
	//direction ==0 forward; 1 backward
	//void Expand(int direction);
	uint64_t nodesTouched, nodesExpanded;
	state middleNode;
	double currentCost;
	double currentSolutionEstimate;
	std::vector<state> neighbors;
	environment *env;
	std::unordered_map<double, int> counts;
	
	dataStructure queue;
	//	priorityQueue queue.forwardQueue, queue.backwardQueue;
	//priorityQueue2 queue.forwardQueue, queue.backwardQueue;
	
	state goal, start;
	
	Heuristic<state> *forwardHeuristic;
	Heuristic<state> *backwardHeuristic;
	
	//keep track of whether we expand a node or put it back to open
	bool expand;
	
	double currentPr;
	
};

template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::GetPath(environment *env, const state& from, const state& to,
																			 Heuristic<state> *forward, Heuristic<state> *backward, std::vector<state> &thePath)
{
	if (InitializeSearch(env, from, to, forward, backward, thePath) == false)
		return;
	
	while (!ExpandAPair(thePath))
	{ }
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
bool NBSFF<state, action, environment, dataStructure, priorityQueue>::InitializeSearch(environment *env, const state& from, const state& to,
																					  Heuristic<state> *forward, Heuristic<state> *backward,
																					  std::vector<state> &thePath)
{
	this->env = env;
	forwardHeuristic = forward;
	backwardHeuristic = backward;
	currentSolutionEstimate = 0;
	currentCost = DBL_MAX;
	queue.Reset();
//	queue.forwardQueue.Reset();
//	queue.backwardQueue.Reset();
	ResetNodeCount();
	thePath.resize(0);
	start = from;
	goal = to;
	if (start == goal)
		return false;
	
	queue.forwardQueue.AddOpenNode(start, env->GetStateHash(start), 0, forwardHeuristic->HCost(start, goal),backwardHeuristic->HCost(start,start));
	queue.backwardQueue.AddOpenNode(goal, env->GetStateHash(goal), 0, backwardHeuristic->HCost(goal, start),forwardHeuristic->HCost(goal,goal));
	
	return true;
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
bool NBSFF<state, action, environment, dataStructure, priorityQueue>::ExpandAPair(std::vector<state> &thePath)
{
	uint64_t nForward, nBackward;
	bool result = queue.GetNextPair(nForward, nBackward);
	// if failed, see if we have optimal path (but return)
	if (result == false)
	{
		if (currentCost == DBL_MAX)
		{
			thePath.resize(0);
			return true;
		}
		ExtractFromMiddle(thePath);
		return true;
	}
	//else if (queue.forwardQueue.Lookup(nForward).data == queue.backwardQueue.Lookup(nBackward).data) // if success, see if nodes are the same (return path)
	//{
	//	if (queue.TerminateOnG())
	//		printf("NBSFF: Lower Bound on C* from g+g (gsum)\n");
	//	ExtractFromMiddle(thePath);
	//	return true;
	//}
	else if (!fless(queue.GetLowerBound(), currentCost))
	{
		ExtractFromMiddle(thePath);
		return true;
	}
	
	counts[queue.GetLowerBound()]+=2;
	Expand(nForward, queue.forwardQueue, queue.backwardQueue, forwardHeuristic, backwardHeuristic, goal);
	Expand(nBackward, queue.backwardQueue, queue.forwardQueue, backwardHeuristic, forwardHeuristic,start);
	return false;
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::ExtractFromMiddle(std::vector<state> &thePath)
{
	std::vector<state> pFor, pBack;
//	std::cout << "Extracting from " << middleNode << "\n";
	ExtractPathToGoal(middleNode, pBack);
//	std::cout << "And from: Extracting from " << middleNode << "\n";
	ExtractPathToStart(middleNode, pFor);
	reverse(pFor.begin(), pFor.end());
	thePath = pFor;
	thePath.insert( thePath.end(), pBack.begin()+1, pBack.end() );
}


template <class state, class action, class environment, class dataStructure, class priorityQueue>
bool NBSFF<state, action, environment, dataStructure, priorityQueue>::DoSingleSearchStep(std::vector<state> &thePath)
{
	return ExpandAPair(thePath);
}


template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::Expand(uint64_t nextID,
																			priorityQueue &current,
																			priorityQueue &opposite,
																			Heuristic<state> *fheuristic, 
																			Heuristic<state> *bheuristic,
																			const state &target)
{
	//cout << "before close\n";
	current.Close(nextID);
	//cout << "after close\n";
	//this can happen when we expand a single node instead of a pair
	//if (fgreatereq(current.Lookup(nextID).g + current.Lookup(nextID).h, currentCost))
	//	return;
	
	nodesExpanded++;
	env->GetSuccessors(current.Lookup(nextID).data, neighbors);
	for (auto &succ : neighbors)
	{
		nodesTouched++;
		uint64_t childID;
		auto loc = current.Lookup(env->GetStateHash(succ), childID);

		// screening
		double edgeCost = env->GCost(current.Lookup(nextID).data, succ);
		if (fgreatereq(current.Lookup(nextID).g+edgeCost, currentCost))
			continue;

		switch (loc)
		{
			case kFFClosed: // ignore
				break;
			case kFFOpen: // update cost if needed
			{
				if (fless(current.Lookup(nextID).g+edgeCost, current.Lookup(childID).g))
				{
					double oldGCost = current.Lookup(childID).g;
					current.Lookup(childID).parentID = nextID;

					current.ChangeKey(childID, current.Lookup(nextID).g + edgeCost);
					//cout << "changekey called\n";
				//	cout << "id: " << childID << "\n";
					//cout << "children old g" <<oldGCost <<"new g" << current.Lookup(nextID).g + edgeCost <<"\n";
					
					// TODO: check if we improved the current solution?
					uint64_t reverseLoc;
					auto loc = opposite.Lookup(env->GetStateHash(succ), reverseLoc);
					if (loc == kFFOpen)
					{
						if (fless(current.Lookup(nextID).g+edgeCost + opposite.Lookup(reverseLoc).g, currentCost))
						{
							if (currentCost == DBL_MAX)
							{
								printf("NBSFF: first solution %llu\n", nodesExpanded);
								std::cout << "Through " << succ << " (better)\n";
							}
							// TODO: store current solution
//							printf("NBSFF Potential updated solution found, cost: %1.2f + %1.2f = %1.2f (%llu nodes)\n",
//								   current.Lookup(nextID).g+edgeCost,
//								   opposite.Lookup(reverseLoc).g,
//								   current.Lookup(nextID).g+edgeCost+opposite.Lookup(reverseLoc).g,
//								nodesExpanded);
							currentCost = current.Lookup(nextID).g+edgeCost + opposite.Lookup(reverseLoc).g;
							
							middleNode = succ;
						}
					}

					else if (loc == kFFClosed)
					{
						current.Remove(childID);
					}

				}
			}
				break;
			case kFFUnseen:
			{
				uint64_t reverseLoc;
				auto loc = opposite.Lookup(env->GetStateHash(succ), reverseLoc);

				if (loc == kFFClosed)// then
				{
					break;			//do nothing. do not put this node to open
				}
				else//loc == kUnseen or kOpen
				{
					//double edgeCost = env->GCost(current.Lookup(nextID).data, succ);
					//if(fless(current.Lookup(nextID).g + edgeCost + heuristic->HCost(succ, target),currentPr))
					//	current.AddOpenNode(succ,
					//		env->GetStateHash(succ),
					//		current.Lookup(nextID).g + edgeCost,
					//		heuristic->HCost(succ, target),
					//		nextID,0);
					//else
					double newNodeF = current.Lookup(nextID).g + edgeCost + fheuristic->HCost(succ, target);
					if (fless(newNodeF, currentCost))
					{
/*
//commented out for suboptimal test
						if (fless(newNodeF, queue.GetLowerBound()))
							current.AddOpenNode(succ,
												env->GetStateHash(succ),
												current.Lookup(nextID).g + edgeCost,
												heuristic->HCost(succ, target),
												nextID, kOpenReady);
						else
*/

						state source = goal;
						if (target == goal)
							source = start;
							
						current.AddOpenNode(succ,
												env->GetStateHash(succ),
												current.Lookup(nextID).g + edgeCost,
												fheuristic->HCost(succ, target),
												bheuristic->HCost(succ, source),
												nextID);

						if (loc == kFFOpen)
						{
							//double edgeCost = env->GCost(current.Lookup(nextID).data, succ);
							if (fless(current.Lookup(nextID).g + edgeCost + opposite.Lookup(reverseLoc).g, currentCost))
							{
								if (currentCost == DBL_MAX)
								{
//									if (&current == &queue.forwardQueue)
//										std::cout << "Searching forward\n";
//									else
//										std::cout << "Searching backward\n";
									printf("NBSFF: first solution %llu ", nodesExpanded);
//									
//									std::cout << "Through " << succ << " (first) \n";
//									
//									uint64_t theID;
//									auto loc = queue.forwardQueue.Lookup(env->GetStateHash(succ), theID);
//									std::cout << "Forward:\n";
//									switch (loc)
//									{
//										case kOpenReady: std::cout << "Initially in open ready\n"; break;
//										case kOpenWaiting: std::cout << "Initially in open waiting\n"; break;
//										case kClosed: std::cout << "Initially in closed\n"; break;
//										case kUnseen: std::cout << "Initially in UNSEEN\n"; break;
//									}
//									loc = queue.backwardQueue.Lookup(env->GetStateHash(succ), theID);
//									std::cout << "Backward:\n";
//									switch (loc)
//									{
//										case kOpenReady: std::cout << "Initially in open ready\n"; break;
//										case kOpenWaiting: std::cout << "Initially in open waiting\n"; break;
//										case kClosed: std::cout << "Initially in closed\n"; break;
//										case kUnseen: std::cout << "Initially in UNSEEN\n"; break;
//									}
									
								}
								// TODO: store current solution
//								printf("NBSFF Potential solution found, cost: %1.2f + %1.2f = %1.2f (%llu nodes)\n",
//									   current.Lookup(nextID).g + edgeCost,
//									   opposite.Lookup(reverseLoc).g,
//									   current.Lookup(nextID).g + edgeCost + opposite.Lookup(reverseLoc).g,
//									   nodesExpanded);
								currentCost = current.Lookup(nextID).g + edgeCost + opposite.Lookup(reverseLoc).g;
								
								middleNode = succ;
//								std::cout << "One more time, solution passes through " << middleNode << " (first) \n";
								
							}
							
						}
						
					}
					else {
						//std::cout << "***Not adding " << succ << " to open because cost is worse than current of " << currentCost << "\n";
					}
				}
				
			}
				break;
		}
	}
}


template <class state, class action, class environment, class dataStructure, class priorityQueue>
uint64_t NBSFF<state, action, environment, dataStructure, priorityQueue>::GetDoubleExpansions() const
{
	uint64_t doubles = 0;
	for (unsigned int x = 0; x < queue.forwardQueue.size(); x++)
	{
		uint64_t key;
		const auto &data = queue.forwardQueue.Lookat(x);
		if (data.where == kFFClosed)
		{
			auto loc = queue.backwardQueue.Lookup(env->GetStateHash(data.data), key);
			if (loc == kFFClosed)
				doubles++;
		}

	}
	return doubles;
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::OpenGLDrawPlot() const
{
	GLint matrixMode;

	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);

	glEnable(GL_BLEND); // for text fading
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ditto

													   // set orthograhic 1:1  pixel transform in local view coords
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(0, 1, 0);

	// left, right, bottom, top, near, far
	double dRight, dLeft, dTop, dBottom;
	dRight = 100;
	dLeft = -100;
	dTop = 100;
	dBottom=-100;
	double tpW = (dRight - dLeft)*.05;
	double tpH = (dTop - dBottom)*.05;
	glOrtho(dLeft - tpW, dRight + tpW, dBottom - tpH, dTop + tpH, -1, 1);
	//printf("Drawing axis (%1.2f, %1.2f, %1.2f, %1.2f)\n", dLeft, dTop, dRight, dBottom);

	glColor3f(1, 1, 1); // draw axis
	glBegin(GL_LINES);
	glVertex2d(dLeft - tpW, 0); glVertex2d(dRight + tpW, 0);
	glVertex2d(0, dBottom - tpH); glVertex2d(0, dTop + tpH);
	glEnd();

	GLdouble xx, yy, zz, rad;
	xx = yy = zz = 0;
	rad = 1;
	GLfloat r, g, b, t;
	//GetColor(r, g, b, t);
	r = 1; g = 0; b = 0; t = 1;
	glColor4f(r, g, b, t);
	
	for (unsigned int x = 0; x < queue.forwardQueue.size(); x++)
	{
		const auto &data = queue.forwardQueue.Lookat(x);
		if (data.where == kFFOpen)
		{
			r = 1; g = 0; b = 0;
			xx = data.g-data.revH;
			yy = data.g+data.h;
			DrawSphere(xx, yy, zz, rad);
		}
	}
	for (unsigned int x = 0; x < queue.backwardQueue.size(); x++)
	{
		const auto &data = queue.backwardQueue.Lookat(x);
		if (data.where == kFFOpen)
		{
			r = 1; g = 0; b = 0;
			xx = 0 - (data.g + data.h);
			yy = 0-(data.g - data.revH);
			DrawSphere(xx, yy, zz, rad);
		}
	}

	//DrawSphere(xx, yy, zz, rad);


	glLineWidth(3.0);


	glPopMatrix(); // GL_MODELVIEW
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(matrixMode);

	glEnable(GL_DEPTH_TEST);
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::OpenGLDraw() const
{
	OpenGLDraw(queue.forwardQueue);
	OpenGLDraw(queue.backwardQueue);
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::OpenGLDraw(const priorityQueue &queue) const
{
	double transparency = 0.9;
	if (queue.size() == 0)
		return;
	uint64_t top = -1;
	//	double minf = 1e9, maxf = 0;

	for (unsigned int x = 0; x < queue.size(); x++)
	{
		const auto &data = queue.Lookat(x);
		if (data.where == kFFOpen)
		{
			env->SetColor(0.0, 1.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}

		else if (data.where == kFFClosed)
		{
			env->SetColor(1.0, 0.0, 0.0, transparency);
			env->OpenGLDraw(data.data);
		}
	}
}

/*
template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::Draw(Graphics::Display &d) const
{
	Draw(d, queue.forwardQueue);
	Draw(d, queue.backwardQueue);
}


template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::Draw(Graphics::Display &d, const priorityQueue &queue) const
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
			env->Draw(d, data.data);
		}
		if (data.where == kOpenWaiting)
		{
			env->SetColor(0.0, 0.5, 0.5, transparency);
			env->Draw(d, data.data);
		}
		else if (data.where == kOpenReady)
		{
			env->SetColor(0.0, 1.0, 0.0, transparency);
			env->Draw(d, data.data);
		}
		else if (data.where == kClosed)
		{
			env->SetColor(1.0, 0.0, 0.0, transparency);
			env->Draw(d, data.data);
		}
	}
}

template <class state, class action, class environment, class dataStructure, class priorityQueue>
void NBSFF<state, action, environment, dataStructure, priorityQueue>::DrawBipartiteGraph(Graphics::Display &d) const
{
	double val = queue.GetLowerBound();
	//currentCost
	assert(!"Implementaion incomplete");
	//	Draw(d, queue.forwardQueue);
//	Draw(d, queue.backwardQueue);
}
//void DrawBipartiteGraph(Graphics::Display &d);

*/

#endif /* NBSFF_h */
