//
//  CanonicalDijkstra.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 1/31/16.
//  Copyright © 2016 University of Denver. All rights reserved.
//

#ifndef CanonicalDijkstra_h
#define CanonicalDijkstra_h


#include "Map2DEnvironment.h"
#include "IndexOpenClosed.h"
#include <stdio.h>
#include "JPS.h"

class CanonicalDijkstra : public GenericSearchAlgorithm<xyLoc, tDirection, MapEnvironment>
{
public:
	CanonicalDijkstra();
	void GetPath(MapEnvironment *env, const xyLoc &from, const xyLoc &to, std::vector<xyLoc> &path);
	void GetPath(MapEnvironment *env, const xyLoc &from, const xyLoc &to, std::vector<tDirection> &path);
	
	bool InitializeSearch(MapEnvironment *env, const xyLoc& from, const xyLoc& to, std::vector<xyLoc> &thePath);
	bool DoSingleSearchStep(std::vector<xyLoc> &thePath);
	
	const char *GetName() { return "CanonicalDijkstra"; }
	uint64_t GetNodesExpanded() const;
	uint64_t GetNodesTouched() const;
	uint64_t GetNumOpenItems() const { return openClosedList.OpenSize(); }
	double GetClosedGCost(xyLoc s);
	void LogFinalStats(StatCollection *stats);
	void SetJumpLimit(int limit) { jumpLimit = limit; }
	void OpenGLDraw() const;
	void Draw() const;
	void OpenGLDraw(const MapEnvironment *env) const;
	std::string SVGDraw() const;
private:
	void GetJPSSuccessors(const xyLocParent &s, uint64_t pid, double pg);
	void GetJPSSuccessors(int x, int y, uint8_t parent, double cost, uint64_t pid, double pg, bool first = false);
	bool Passable(int x, int y);
	void ExtractPathToStartFromID(uint64_t node, std::vector<xyLoc> &thePath);
	//	AStarOpenClosed<xyLocParent, AStarCompare<xyLocParent> > openClosedList;
	IndexOpenClosed<xyLocParent> openClosedList;
	std::vector<jpsSuccessor> successors;
	MapEnvironment *env;
	xyLoc to;
	uint64_t nodesExpanded, nodesTouched;
	int jumpLimit;
};

#endif /* CanonicalDijkstra_h */
