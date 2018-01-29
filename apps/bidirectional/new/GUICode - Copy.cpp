//
//  GUICode.cpp
//  hog2 glut
//
//  Created by Nathan Sturtevant on 7/29/15.
//  Copyright (c) 2015 University of Denver. All rights reserved.
//

#include <vector>
#include <fstream>
#include <string>
#include "GUICode.h"
#include "ScenarioLoader.h"
#include "Map2DEnvironment.h"
#include "MapGenerators.h"
#include "MapOverlay.h"
#include "TemplateAStar.h"
#include "MM.h"
#include "SVGUtil.h"
#include "NBSQueueGF.h"
#include "NBS.h"
#include "NBSFF.h"
#include "NBSGreedy.h"
#include "NBSLazy.h"
#include "NBSSwitch.h"
#include "BSStar.h"

Map *map = 0;
MapEnvironment *me = 0;
MapOverlay *mo;
xyLoc start, goal;
xyLoc chosenPoint;
xyLoc meetingPoint;
double forwardGLimit = DBL_MAX;

std::vector<int> counts;

bool mouseTracking = false;
bool mouseTracked = false;
bool drawSearch = false;
bool choosingMeetingPoint = false;
bool paused = false;
void SetupMapOverlay();

int gStepsPerFrame = 2;

TemplateAStar<xyLoc, tDirection, MapEnvironment> forward;
TemplateAStar<xyLoc, tDirection, MapEnvironment> backward;

TemplateAStar<xyLoc, tDirection, MapEnvironment> preProcess;

ZeroHeuristic<xyLoc> *z = new ZeroHeuristic<xyLoc>;


MM<xyLoc, tDirection, MapEnvironment> mm;
NBSFF<xyLoc, tDirection, MapEnvironment> nbs;
NBSGreedy<xyLoc, tDirection, MapEnvironment> nbsgreedy;
NBSLazy<xyLoc, tDirection, MapEnvironment> nbslazy;
TemplateAStar<xyLoc, tDirection, MapEnvironment> compare;
MM<xyLoc, tDirection, MapEnvironment> mm0;
TemplateAStar<xyLoc, tDirection, MapEnvironment> compare0;
BSStar<xyLoc, tDirection, MapEnvironment> bs;

bool forwardSearchRunning = false;
bool backwardSearchRunning = false;
bool mmSearchRunning = false;
bool compareSearchRunning = false;
bool mm0SearchRunning = false;
bool bsSearchRunning = false;
bool compare0SearchRunning = false;
bool searchRan = false;
std::vector<xyLoc> path;
std::vector<xyLoc> goalPath;
std::vector<xyLoc> forwardPath, backwardPath;
bool recording = false;
bool hidePre = false;
bool hideChosen = false;

bool selected = false;

std::fstream svgFile;
bool saveSVG = false;

enum bibfs {
	XX = 0,
	NN = 1,
	NF = 2,
	NR = 3,
	FN = 4,
	FF = 5,
	FR = 6,
	RN = 7,
	RF = 8,
	RR = 9
};

const char *bibfs_desc[10] = {
	"", "NN", "NF", "NR", "FN", "FF", "FR", "RN", "RF", "RR"
};



void InstallHandlers()
{
	InstallWindowHandler(MyWindowHandler);
	InstallMouseClickHandler(MyClickHandler);
	InstallKeyboardHandler(MyKeyboardHandler, "Save SVG", "Export graphics to SVG File", kNoModifier, 's');
	InstallKeyboardHandler(MyKeyboardHandler, "Record", "Start/stop recording movie", kNoModifier, 'r');
	InstallKeyboardHandler(MyKeyboardHandler, "Draw", "Toggle drawing search", kNoModifier, 'd');
	InstallKeyboardHandler(MyKeyboardHandler, "Choose", "Toggle choosing meeting point", kNoModifier, 'c');
	InstallKeyboardHandler(MyKeyboardHandler, "Hide", "Hide open", kNoModifier, 'h');
	InstallKeyboardHandler(MyKeyboardHandler, "Hide Chosen", "Hide chosen", kNoModifier, 'g');
	InstallKeyboardHandler(MyKeyboardHandler, "Pause", "Toggle pause", kNoModifier, 'p');
	InstallKeyboardHandler(MyKeyboardHandler, "Step", "Single algorithm step", kNoModifier, 'o');
	InstallKeyboardHandler(MyKeyboardHandler, "Single Viewport", "Set to use a single viewport", kNoModifier, '1');
	InstallKeyboardHandler(MyKeyboardHandler, "Two Viewports", "Set to use two viewports", kNoModifier, '2');
	InstallKeyboardHandler(MyKeyboardHandler, "Three Viewports", "Set to use three viewports", kNoModifier, '3');
	InstallKeyboardHandler(MyKeyboardHandler, "Four Viewports", "Set to use four viewports", kNoModifier, '4');
	InstallKeyboardHandler(MyKeyboardHandler, "Slower", "Slow down visualization", kNoModifier, '[');
	InstallKeyboardHandler(MyKeyboardHandler, "Faster", "Speed up visualization", kNoModifier, ']');
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);
		mouseTracking = false;
		delete map;
		delete me;
		map = 0;
		me = 0;
	}
	else if (eType == kWindowCreated)
	{
		printf("Window %ld created\n", windowID);
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		SetNumPorts(windowID, 1);
		
		delete map;
		delete me;
		//		map = new Map("/Users/nathanst/hog2/maps/dao/brc000d.map");
//		map = new Map("/Users/nathanst/hog2/maps/dao/brc000d.map");
//		map = new Map("/Users/nathanst/hog2/maps/dao/brc203d.map");
		//map = new Map("/Users/nathanst/hog2/maps/dao/lak308d.map");
		//map = new Map("/Users/nathanst/hog2/maps/da2/ht_chantry.map");
		//map = new Map("/Users/nathanst/hog2/maps/da2/w_woundedcoast.map");
		
		//map = new Map("/Users/nathanst/hog2/maps/random/random512-35-6.map");
		//map = new Map("/Users/nathanst/hog2/maps/da2/lt_backalley_g.map");
		//map = new Map("/Users/nathanst/hog2/maps/bgmaps/AR0011SR.map");
		map = new Map("/home/jingwei/Desktop/Shared/nbs_ad/hog2/maps/bgmaps/AR0012SR.map");
		//map = new Map("/Users/nathanst/hog2/maps/rooms/8room_000.map");
		//map = new Map("/Users/nathanst/hog2/maps/mazes/maze512-16-0.map");
		//map = new Map("/Users/nathanst/hog2/maps/mazes/maze512-1-0.map");
		//map = new Map("/Users/nathanst/hog2/maps/dao/orz107d.map");
		//{
		//	map = new Map(128,128);
		//	MakeMaze(map, 1);
		//}
		
		map->SetTileSet(kWinter);
		me = new MapEnvironment(map);
		me->SetDiagonalCost(1.5);
	}
	
}

void StepAlgorithms()
{
	for (int x = 0; x < gStepsPerFrame; x++)
	{
		if (forwardSearchRunning)
		{
			forwardSearchRunning = !forward.DoSingleSearchStep(forwardPath);
			if (!forwardSearchRunning)
			{
				printf("forward finished\n");
			}
		}
	}

	for (int x = 0; x < gStepsPerFrame; x++)
	{
		if (backwardSearchRunning)
		{
			backwardSearchRunning = !backward.DoSingleSearchStep(backwardPath);
			if (!backwardSearchRunning)
			{
				printf("backward finished\n");
			}
		}
	}

	for (int x = 0; x < gStepsPerFrame/2; x++)
	{
		if (mmSearchRunning)
		{
			mmSearchRunning = !nbs.DoSingleSearchStep(path);
			if (!mmSearchRunning)
				printf("NBS*: %llu nodes expanded\n", nbs.GetNodesExpanded());
		}
	}
	for (int x = 0; x < gStepsPerFrame; x++)
	{
		if (mm0SearchRunning)
		{
			mm0SearchRunning = !mm0.DoSingleSearchStep(path);
			if (!mm0SearchRunning)
				printf("MM0: %llu nodes expanded\n", mm0.GetNodesExpanded());
		}
	}
	for (int x = 0; x < gStepsPerFrame; x++)
	{
		if (bsSearchRunning)
		{
			bsSearchRunning = !bs.DoSingleSearchStep(path);
			if (!bsSearchRunning)
				printf("BS*: %llu nodes expanded\n", bs.GetNodesExpanded());
		}
	}
	for (int x = 0; x < gStepsPerFrame; x++)
	{
		if (compareSearchRunning)
		{
			compareSearchRunning = !compare.DoSingleSearchStep(path);
			if (!compareSearchRunning)
			{
				printf("A*: %llu nodes expanded const %1.1f\n", compare.GetNodesExpanded(), me->GetPathLength(path));
			}
		}
	}
	for (int x = 0; x < gStepsPerFrame; x++)
	{
		if (compare0SearchRunning)
		{
			compare0SearchRunning = !compare0.DoSingleSearchStep(path);
			if (!compare0SearchRunning)
			{
				printf("BFS: %llu nodes expanded const %1.1f\n", compare0.GetNodesExpanded(), me->GetPathLength(path));
			}
		}
	}
}

void MyKeyboardHandler(unsigned long windowID, tKeyboardModifier, char key)
{
	switch (key)
	{
		case 's':
		{
			svgFile.open("/Users/nathanst/Desktop/test.svg", std::fstream::out | std::fstream::trunc);
			saveSVG = true;
			break;
		}
		case 'r':
		{
			recording = !recording;
			break;
		}
		case 'p':
		{
			paused = !paused;
			break;
		}
		case 'o':
		{
			StepAlgorithms();
			break;
		}
		case 'd':
		{
			drawSearch = !drawSearch;
			break;
		}
		//choose a meeting point
		case 'c':
		{
			choosingMeetingPoint = !choosingMeetingPoint;
			break;
		}
		case 'h':
		{
			hidePre = !hidePre;
			break;
		}
		case 'g':
		{
			hideChosen = !hideChosen;
			break;
		}
		case '1':
		{
			SetNumPorts(windowID, 1);
			break;
		}
		case '2':
		{
			SetNumPorts(windowID, 2);
			break;
		}
		case '3':
		{
			SetNumPorts(windowID, 3);
			break;
		}
		case '4':
		{
			SetNumPorts(windowID, 4);
			break;
		}
		case '[':
		{
			gStepsPerFrame /= 2;
			break;
		}
		case ']':
		{
			gStepsPerFrame *= 2;
			break;
		}
	}
}


void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	if (saveSVG && viewport == 0)
	{
		svgFile << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width = \""+std::to_string(10*map->GetMapWidth()+10)+"\" height = \""+std::to_string(10*map->GetMapHeight()+10+100)+"\">";
		recColor black = {0, 0, 0};
		recColor white = {1, 1, 1};
		//svgFile << SVGDrawRect(0, 0, map->GetMapWidth(), map->GetMapHeight()+10+1, white);
		if (mo)
		{
			svgFile << mo->SVGDraw();

			for (int x = 1; x < 10; x++)
			{
				recColor r = mo->GetValueColor(x);
				glColor3f(r.r, r.g, r.b);
				char num[16];
				sprintf(num, "%d", counts[x]);
				
				svgFile << SVGDrawRect(x*map->GetMapWidth()/11+0, map->GetMapHeight()+2, 5, 5, r);
				svgFile << SVGDrawText(x*map->GetMapWidth()/11+6, map->GetMapHeight()+4, bibfs_desc[x], black, 3);
				svgFile << SVGDrawText(x*map->GetMapWidth()/11+6, map->GetMapHeight()+7, num, black, 3);
			}

			svgFile << SVGDrawText(start.x+1, start.y+2, "start", black, 5);
			svgFile << SVGDrawText(goal.x+1, goal.y+2, "goal", black, 5);
		}
		else {
			svgFile << me->SVGDraw();
		}
		
		svgFile << "</svg>";


		saveSVG = false;
		svgFile.close();
	}

	map->OpenGLDraw();
	if (mo)
	{
		mo->OpenGLDraw();
		for (int x = 1; x < 10; x++)
		{
			recColor r = mo->GetValueColor(x);
			glColor3f(r.r, r.g, r.b);
			DrawBox(-1+0.2*x-1.0/40.0, -1-1.0/40.0, 0, 1.0/40.0);
			glColor3f(1.0, 1.0, 1.0);
			DrawText(-1+0.2*x+1.0/40.0, -1-1.0/40.0, -0.01, 1.0/10.0, bibfs_desc[x]);
			char num[16];
			sprintf(num, "%d", counts[x]);
			DrawText(-1+0.2*x+1.0/40.0, -1+1.0/40.0, -0.01, 1.0/10.0, num);
		}
	}
	if (mouseTracking)
	{
		me->SetColor(1.0, 0, 0);
		glLineWidth(3.0);
		me->GLDrawLine(start, goal);
		glLineWidth(1.0);
		me->SetColor(1.0, 1.0, 1.0);
		me->GLDrawLine(start, goal);
		//me->GLLabelState(start, "Start", 5.0);
		//me->GLLabelState(goal, "Goal", 5.0);
		me->HighlightState(start, 5);
		me->HighlightState(goal, 5);
	}
	if (mouseTracked && 0)
	{
		me->SetColor(1.0, 1.0, 1.0);
		glLineWidth(1.0);
		me->GLLabelState(start, "start", map->GetMapHeight()/8.0);
		me->GLLabelState(goal, "goal", map->GetMapHeight()/8.0);
		glLineWidth(1.0);
	}

	if (drawSearch)
	{
		if (!paused)
		{
			StepAlgorithms();
		}
		if (searchRan)
		{
			if (viewport == 0)
			{
				//compare.OpenGLDraw();
				forward.OpenGLDraw();
				backward.OpenGLDraw();
				me->HighlightState(start, 5);
				me->HighlightState(goal, 5);
				if (selected)
				{
					if (!hideChosen)
						me->HighlightState(chosenPoint, 5);
					me->HighlightState(meetingPoint, 1);
				}
			}

			else if (viewport == 1)
				nbs.OpenGLDraw();
			else if (viewport == 2)
				bs.OpenGLDraw();
			else if (viewport == 3)
				mm.OpenGLDraw();
		}
	}
	else
	{
		if(!hidePre)
			preProcess.OpenGLDraw();
		for (int k = 0; k < goalPath.size();k++)
			me->HighlightState(goalPath[k], 1);
		//me->DrawPath(goalPath);
		me->HighlightState(start, 5);
		me->HighlightState(goal, 5);
		if (selected)
		{
			if(!hideChosen)
				me->HighlightState(chosenPoint, 5);
			me->HighlightState(meetingPoint, 1);
		}


	}

	if (recording && viewport == GetNumPorts(windowID)-1)
	{
		static int cnt = 0;
		char fname[255];
		sprintf(fname, "/home/jingwei/Desktop/Shared/Movies/tmp1/BI-%d%d%d%d", (cnt/1000)%10, (cnt/100)%10, (cnt/10)%10, cnt%10);
		SaveScreenshot(windowID, fname);
		printf("Saved %s\n", fname);
		cnt++;
	}
	//if (goalPath.size() > 0)
	//{
	//	goal = goalPath.back();
	//	goalPath.pop_back();
	//	SetupMapOverlay();
	//}
}

bool MyClickHandler(unsigned long windowID, int, int, point3d loc, tButtonType button, tMouseEventType mType)
{
	//	return false;
	static point3d startLoc;
	if (mType == kMouseDown)
	{
		switch (button)
		{
			case kRightButton: printf("Right button\n"); break;
			case kLeftButton: printf("Left button\n"); break;
			case kMiddleButton: printf("Middle button\n"); break;
		}
	}
	if (button != kLeftButton)
		return false;
	if (choosingMeetingPoint == false)
	{
		switch (mType)
		{
		case kMouseDown:
		{
			if (mo) 
			{
				delete mo;
				mo = 0;
			}


			int x, y;
			map->GetPointFromCoordinate(loc, x, y);
			start.x = x; start.y = y;
			goal = start;
			mouseTracking = true;
			mouseTracked = true;

			return true;
		}
		case kMouseDrag:
		{
			int x, y;
			map->GetPointFromCoordinate(loc, x, y);
			goal.x = x; goal.y = y;
			mouseTracking = true;
			mouseTracked = true;
			return true;
		}
		case kMouseUp:
			if (mouseTracking)
			{
				int x, y;
				map->GetPointFromCoordinate(loc, x, y);
				goal.x = x; goal.y = y;

				preProcess.SetHeuristic(me);
				preProcess.GetPath(me, start, goal, goalPath);

				//100, 59) to (176, 112
				//				start.x = 100;
				//				start.y = 59;
				//				goal.x = 176;
				//				goal.y = 112;
				//
				//				//102, 170) to (82, 11 //  100, 192) to (106, 60
				//101, 143) to (93, 155
				//				start.x = 101;
				//				start.y = 143;
				//				goal.x = 93;
				//				goal.y = 155;

				//				start.x = 75;
				//				start.y = 33;
				//				goal.x = 93;//78;
				//				goal.y = 116;//132-10;

				//				forward.GetPath(me,
				//								{static_cast<uint16_t>(goal.x-17),
				//									static_cast<uint16_t>(goal.y+22)}, goal, goalPath);
				//				recording = true;


				mouseTracking = false;
				//SetupMapOverlay();
				SetNumPorts(windowID, 1);
				compare.SetHeuristic(me);
				compare.InitializeSearch(me, start, goal, path);

				compare0.SetHeuristic(z);
				compare0.InitializeSearch(me, start, goal, path);
				nbs.InitializeSearch(me, start, goal, me, me, path);
				nbsgreedy.InitializeSearch(me, start, goal, me, me, path);
				nbslazy.InitializeSearch(me, start, goal, me, me, path);
				bs.InitializeSearch(me, start, goal, me, me, path);
				//mm.InitializeSearch(me, start, goal, z, z, path);
				mm.InitializeSearch(me, start, goal, me, me, path);
				mm0.InitializeSearch(me, start, goal, z, z, path);

				mmSearchRunning = true;
				compareSearchRunning = true;
				mm0SearchRunning = true;
				bsSearchRunning = true;
				compare0SearchRunning = true;


				meetingPoint = goal;
				forward.SetHeuristic(me);
				forward.InitializeSearch(me, start,goal, forwardPath);
				forward.SetStopGCost(forwardGLimit);
				//printf("for g limit: %f\n",forwardGLimit);
				backward.SetHeuristic(me);
				backward.InitializeSearch(me, goal,start, backwardPath);
				double backwardGLimit = std::max(0.0, me->GetPathLength(goalPath)- forwardGLimit);
				backward.SetStopGCost(backwardGLimit);
				//printf("back g limit: %f\n",backwardGLimit);
				forwardSearchRunning = true;
				backwardSearchRunning = true;

				selected = false;
				searchRan = true;
				return true;
			}
		}
	}
	else
	{
		printf("else...\n");
		switch (mType)
		{
		case kMouseDown:
		{
			return true;
		}
		case kMouseDrag:
		{
			return true;
		}
		case kMouseUp:
		{
			int x, y;
			map->GetPointFromCoordinate(loc, x, y);
			chosenPoint.x = x; chosenPoint.y = y;
			printf("ok1.1...\n");
			std::vector<xyLoc> tmpPath;
			backward.SetHeuristic(me);
			backward.SetStopGCost(DBL_MAX);
			backward.SetStopFCost(DBL_MAX);
			backward.GetPath(me, start, chosenPoint, tmpPath);
			printf("ok1.2...\n");
			forwardGLimit = me->GetPathLength(tmpPath);
			printf("ok2...\n");
			if (forwardGLimit >= me->GetPathLength(goalPath))
			{
				meetingPoint = goal;
			}
			else
			{
				std::vector<xyLoc> copyPath;
				copyPath = goalPath;
				while (copyPath.size() > 0)
				{
					copyPath.pop_back();
					if (forwardGLimit >= me->GetPathLength(copyPath))
						break;
				}
				meetingPoint = copyPath[copyPath.size() - 1];
			}
			printf("ok3...\n");
			SetNumPorts(windowID, 1);
			printf("ok4...\n");
			forward.SetHeuristic(me);
			forward.InitializeSearch(me, start, goal, forwardPath);
			forward.SetStopGCost(forwardGLimit);
			forward.SetStopFCost(me->GetPathLength(goalPath));

			printf("for g limit: %f, f limit: %f\n", forwardGLimit, me->GetPathLength(goalPath));

			backward.SetHeuristic(me);
			backward.InitializeSearch(me, goal, start, backwardPath);
			double backwardGLimit = std::max(0.0, me->GetPathLength(goalPath)- forwardGLimit);
			backward.SetStopGCost(backwardGLimit);
			backward.SetStopFCost(me->GetPathLength(goalPath));

			printf("back g limit: %f, f limit: %f\n", backwardGLimit, me->GetPathLength(goalPath));


			forwardSearchRunning = true;
			backwardSearchRunning = true;

			compare.SetHeuristic(me);
			compare.InitializeSearch(me, start, goal, path);

			compare0.SetHeuristic(z);
			compare0.InitializeSearch(me, start, goal, path);
			nbs.InitializeSearch(me, start, goal, me, me, path);
			nbsgreedy.InitializeSearch(me, start, goal, me, me, path);
			nbslazy.InitializeSearch(me, start, goal, me, me, path);
			bs.InitializeSearch(me, start, goal, me, me, path);
			mm.InitializeSearch(me, start, goal, me, me, path);
			mm0.InitializeSearch(me, start, goal, z, z, path);

			mmSearchRunning = true;
			compareSearchRunning = true;
			mm0SearchRunning = true;
			bsSearchRunning = true;
			compare0SearchRunning = true;

			selected = true;

			searchRan = true;

			return true;


		}
		}
	}
	return false;
}

bibfs GetLocationClassification(xyLoc l, double optimal)
{
	double startDist, goalDist;
	forward.GetClosedListGCost(l, startDist);
	backward.GetClosedListGCost(l, goalDist);

	// Only Show NEAR in each direction
//	if (startDist < optimal/2)
//		return NF;
//	if (goalDist < optimal/2)
//		return FN;
//	return RR;

	// Only show N*, F*
//	if (startDist < optimal/2)
//		return NR;
//	if (startDist < optimal)
//		return NF;
//	if (startDist == optimal/2)
//		return NN;
//	return RR;

	if (0) // Draw MM & A* heuristic regions that don't overlap
	{
		bool MM = false;
		bool ASTAR = false;
		if (startDist <= optimal && me->HCost(l, goal)+startDist <= optimal)
			ASTAR = true;
		if ((startDist <= optimal/2 && me->HCost(l, goal)+startDist <= optimal) ||
			(goalDist <= optimal/2 && me->HCost(l, start)+goalDist <= optimal))
			MM = true;
		if (ASTAR && MM)
			return XX;
//			return NF;
		if (MM)
			return RN;
		if (ASTAR)
			return FN;
		else return XX;
	}
	
	if (0) // Draw MM heuristic regions
	{
		if (startDist <= optimal/2 && goalDist <= optimal/2)
		{
			if (me->HCost(l, goal)+startDist > optimal || me->HCost(l, start)+goalDist > optimal)
				return NN;
			return RN;
			//return XX;
			return NN;
		}
		else if (startDist <= optimal/2 && goalDist <= optimal)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NN;
			return RN;
			return XX;
			return NF;
		}
		else if (startDist <= optimal/2)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NN;
			return RN;
			return XX;
			return NR;
		}
		else if (startDist <= optimal && goalDist <= optimal/2)
		{
			if (me->HCost(l, start)+goalDist > optimal)
				return NN;
			return RN;
			return XX;
			return FN;
		}
		else if (startDist <= optimal && goalDist <= optimal)
		{
			return XX;
			return FF;
		}
		else if (startDist <= optimal)
		{
			return XX;
			return FR;
		}
		else if (goalDist <= optimal/2)
		{
			if (me->HCost(l, start)+goalDist > optimal)
				return NN;
			return RN;
		}
		else if (goalDist <= optimal)
		{
			return XX;
			return RF;
		}
		else {
			return XX;
			return RR;
		}
	}
	
	if (0) // Draw A* heuristic regions
	{
		if (startDist <= optimal/2 && goalDist <= optimal/2)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NN;
			return RN;
			//return XX;
			return NN;
		}
		else if (startDist <= optimal/2 && goalDist <= optimal)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NR;
			return FN;
			return XX;
			return NF;
		}
		else if (startDist <= optimal/2)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NR;
			return FN;
			return XX;
			return NR;
		}
		else if (startDist <= optimal && goalDist <= optimal/2)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NR;
			return FN;
			return XX;
			return FN;
		}
		else if (startDist <= optimal && goalDist <= optimal)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NR;
			return FN;
			return FN;
			return XX;
			return FF;
		}
		else if (startDist <= optimal)
		{
			if (me->HCost(l, goal)+startDist > optimal)
				return NR;
			return FN;
			return XX;
			return FR;
		}
		else if (goalDist <= optimal/2)
		{
			return XX;
			return RN;
		}
		else if (goalDist <= optimal)
		{
			return XX;
			return RF;
		}
		else {
			return XX;
			return RR;
		}
	}
	
	if (0) // DRAW just BFS regions
	{
		if (startDist <= optimal/2 && goalDist <= optimal/2)
		{
			return FN;
			return NN;
		}
		else if (startDist <= optimal/2 && goalDist <= optimal)
		{
			return FN;
			return NF;
		}
		else if (startDist <= optimal/2)
		{
			return FN;
			return NR;
		}
		else if (startDist <= optimal && goalDist <= optimal/2)
		{
			return FN;
			return FN;
		}
		else if (startDist <= optimal && goalDist <= optimal)
		{
			return FN;
			return FF;
		}
		else if (startDist <= optimal)
		{
			return FN;
			return FR;
		}
		else if (goalDist <= optimal/2)
		{
			return XX;
			return RN;
		}
		else if (goalDist <= optimal)
		{
			return XX;
			return RF;
		}
		else {
			return XX;
			return RR;
		}
	}

	if (1)
	{
		// DRAW all regions
		if (startDist <= optimal/2 && goalDist <= optimal/2)
		{
			return NN;
		}
		else if (startDist <= optimal/2 && goalDist <= optimal)
		{
			return NF;
		}
		else if (startDist <= optimal/2)
		{
			return NR;
		}
		else if (startDist <= optimal && goalDist <= optimal/2)
		{
			return FN;
		}
		else if (startDist <= optimal && goalDist <= optimal)
		{
			return FF;
		}
		else if (startDist <= optimal)
		{
			return FR;
		}
		else if (goalDist <= optimal/2)
		{
			return RN;
		}
		else if (goalDist <= optimal)
		{
			return RF;
		}
		else {
			return RR;
		}
	}
}

void SetupMapOverlay()
{
	if (start.x >= map->GetMapWidth() || start.x < 0 || start.y >= map->GetMapHeight() || start.y < 0)
	{
		std::cout << "Invalid path: " << start << " to " << goal << "\n";
		return;
	}
	std::cout << "Doing map overlay from " << start << " to " << goal << "\n";
	counts.resize(0);
	counts.resize(10);
	delete mo;
	mo = new MapOverlay(map);
	mo->SetColorMap(MapOverlay::customColorMap);
	
	mo->SetColor(XX, colors::black);

	mo->SetColor(NN, colors::cyan);
	mo->SetColor(NF, colors::lightblue);
	mo->SetColor(NR, colors::blue);
	mo->SetColor(FN, colors::lightgreen);
	mo->SetColor(RN, colors::green);

	mo->SetColor(FF, colors::cyan);
	mo->SetColor(FR, colors::darkblue);
	mo->SetColor(RF, colors::darkgreen);

	mo->SetColor(RR, colors::darkgray);
	
	//forward.SetStopAfterGoal(false);
	//backward.SetStopAfterGoal(false);
	std::vector<xyLoc> path;
	forward.GetPath(me, start, goal, path);
	backward.GetPath(me, goal, start, path);
	
	double optimal;
	forward.GetClosedListGCost(goal, optimal);
	for (int x = 0; x < map->GetMapWidth(); x++)
	{
		for (int y = 0; y < map->GetMapHeight(); y++)
		{
			if (map->GetTerrainType(x, y) == kGround)
			{
				xyLoc l(x, y);
				bibfs i = GetLocationClassification(l, optimal);
				counts[i]++;
				mo->SetOverlayValue(x, y, i);
			}
		}
	}
//	mo->SetD
	mo->SetTransparentValue(XX);
	mo->SetOverlayValue(start.x, start.y, 10);
	mo->SetOverlayValue(goal.x, goal.y, 10);
	for (int x = 0; x < counts.size(); x++)
	{
		switch (x)
		{
			case 1: printf("NN: %d\n", counts[x]); break;
			case 2: printf("NF: %d\n", counts[x]); break;
			case 3: printf("NR: %d\n", counts[x]); break;
			case 4: printf("FN: %d\n", counts[x]); break;
			case 5: printf("FF: %d\n", counts[x]); break;
			case 6: printf("FR: %d\n", counts[x]); break;
			case 7: printf("RN: %d\n", counts[x]); break;
			case 8: printf("RF: %d\n", counts[x]); break;
			case 9: printf("RR: %d\n", counts[x]); break;
			default: break;
		}
	}
}

void AnalyzeProblem(Map *m, Experiment e, double weight)
{
	WeightedHeuristic<xyLoc> wh(me, weight);
	compare.SetHeuristic(&wh);
	//forward.SetStopAfterGoal(false);
	//backward.SetStopAfterGoal(false);
	std::vector<xyLoc> path;
	start.x = e.GetStartX();
	start.y = e.GetStartY();
	goal.x = e.GetGoalX();
	goal.y = e.GetGoalY();
	

	forward.GetPath(me, start, goal, path);
	backward.GetPath(me, goal, start, path);

	double optimal;
	forward.GetClosedListGCost(goal, optimal);

	compare.GetPath(me, start, goal, path);
	assert(me->GetPathLength(path) == optimal);
	//printf("A* path length: %1.2f\t", me->GetPathLength(path));
	mm.GetPath(me, start, goal, &wh, &wh, path);
	//printf("MM path length: %1.2f\n", me->GetPathLength(path));

	printf("A*: %llu\tMM: %llu\t", compare.GetNodesExpanded(), mm.GetNodesExpanded());
	// full state space
	{
		counts.resize(0);
		counts.resize(10);
		for (int x = 0; x < m->GetMapWidth(); x++)
		{
			for (int y = 0; y < m->GetMapHeight(); y++)
			{
				if (m->GetTerrainType(x, y) == kGround)
				{
					counts[GetLocationClassification(xyLoc(x, y), optimal)]++;
				}
			}
		}
		for (int x = 0; x < counts.size(); x++)
		{
			switch (x)
			{
				case 1: printf("NN: %d\t", counts[x]); break;
				case 2: printf("NF: %d\t", counts[x]); break;
				case 3: printf("NR: %d\t", counts[x]); break;
				case 4: printf("FN: %d\t", counts[x]); break;
				case 5: printf("FF: %d\t", counts[x]); break;
				case 6: printf("FR: %d\t", counts[x]); break;
				case 7: printf("RN: %d\t", counts[x]); break;
				case 8: printf("RF: %d\t", counts[x]); break;
				case 9: printf("RR: %d\t", counts[x]); break;
				default: break;
			}
		}
	}
	// A*
	{
		counts.resize(0);
		counts.resize(10);
		for (int x = 0; x < compare.GetNumItems(); x++)
		{
			if (compare.GetItem(x).where == kClosedList)
				counts[GetLocationClassification(compare.GetItem(x).data, optimal)]++;
		}
		for (int x = 0; x < counts.size(); x++)
		{
			switch (x)
			{
				case 1: printf("NN: %d\t", counts[x]); break;
				case 2: printf("NF: %d\t", counts[x]); break;
				case 3: printf("NR: %d\t", counts[x]); break;
				case 4: printf("FN: %d\t", counts[x]); break;
				case 5: printf("FF: %d\t", counts[x]); break;
				case 6: printf("FR: %d\t", counts[x]); break;
				case 7: printf("RN: %d\t", counts[x]); break;
				case 8: printf("RF: %d\t", counts[x]); break;
				case 9: printf("RR: %d\t", counts[x]); break;
				default: break;
			}
		}
	}
	// MM
	{
		counts.resize(0);
		counts.resize(10);
		for (int x = 0; x < mm.GetNumForwardItems(); x++)
		{
			if (mm.GetForwardItem(x).where == kClosedList)
				counts[GetLocationClassification(mm.GetForwardItem(x).data, optimal)]++;
		}
		for (int x = 0; x < mm.GetNumBackwardItems(); x++)
		{
			if (mm.GetBackwardItem(x).where == kClosedList)
				counts[GetLocationClassification(mm.GetBackwardItem(x).data, optimal)]++;
		}
		for (int x = 0; x < counts.size(); x++)
		{
			switch (x)
			{
				case 1: printf("NN: %d\t", counts[x]); break;
				case 2: printf("NF: %d\t", counts[x]); break;
				case 3: printf("NR: %d\t", counts[x]); break;
				case 4: printf("FN: %d\t", counts[x]); break;
				case 5: printf("FF: %d\t", counts[x]); break;
				case 6: printf("FR: %d\t", counts[x]); break;
				case 7: printf("RN: %d\t", counts[x]); break;
				case 8: printf("RF: %d\t", counts[x]); break;
				case 9: printf("RR: %d\t", counts[x]); break;
				default: break;
			}
		}
	}
	printf("\n");
}

void AnalyzeMap(const char *map, const char *scenario, double weight)
{
	printf("Loading %s with scenario %s\n", map, scenario);
	ScenarioLoader s(scenario);
	Map *m = new Map(map);
	me = new MapEnvironment(m);
	me->SetDiagonalCost(1.5);
	for (int x = 0; x < s.GetNumExperiments(); x++)
	{
		AnalyzeProblem(m, s.GetNthExperiment(x), weight);
	}
}

void AnalyzeNBS(const char *map, const char *scenario, double weight)
{
	NBS<xyLoc, tDirection, MapEnvironment> nbs;
	NBSGreedy<xyLoc, tDirection, MapEnvironment> nbsgreedy;
	NBSLazy<xyLoc, tDirection, MapEnvironment> nbslazy;
	BSStar<xyLoc, tDirection, MapEnvironment> bs;
	//NBS<xyLoc, tDirection, MapEnvironment, NBSQueueGF<xyLoc>, BDOpenClosed<xyLoc, NBSGLowHigh<xyLoc>, NBSFLowHigh<xyLoc>>> nbs;
	
	MM<xyLoc, tDirection, MapEnvironment> mm;
	MM<xyLoc, tDirection, MapEnvironment> mm0;
	TemplateAStar<xyLoc, tDirection, MapEnvironment> astar;
	
	printf("Loading %s with scenario %s\n", map, scenario);
	ScenarioLoader s(scenario);
	Map *m = new Map(map);
	me = new MapEnvironment(m);
	me->SetDiagonalCost(1.5);
//	me->SetDiagonalCost(ROOT_TWO);
	Timer t;
	ZeroHeuristic<xyLoc> z;
	// 406 is bad!
	for (int x = s.GetNumExperiments()-1; x >= 0; x--) // 547 to 540
	{
		if (fequal(s.GetNthExperiment(x).GetDistance(), 0))
			continue;
		xyLoc start, goal;
		start.x = s.GetNthExperiment(x).GetStartX();
		start.y = s.GetNthExperiment(x).GetStartY();
		goal.x = s.GetNthExperiment(x).GetGoalX();
		goal.y = s.GetNthExperiment(x).GetGoalY();
		printf("Problem %d of %d from ", x, s.GetNumExperiments());
		std::cout << start << " to " << goal << "\n";
		std::vector<xyLoc> correctPath;
		std::vector<xyLoc> mmPath;
		std::vector<xyLoc> mm0Path;
		std::vector<xyLoc> nbsPath;
		std::vector<xyLoc> nbsgreedyPath;
		std::vector<xyLoc> nbslazyPath;
		std::vector<xyLoc> bsPath;
		astar.SetHeuristic(me);

		double t1, t2, t3, t4, t5,t6;
		t.StartTimer();
		astar.GetPath(me, start, goal, correctPath);
		t.EndTimer();
		t1 = t.GetElapsedTime();
		//printf("%d %1.1f A* nodes: %llu necessary %llu time: %1.2f \n", x, me->GetPathLength(correctPath), astar.GetNodesExpanded(), astar.GetNecessaryExpansions(), t.GetElapsedTime());
		t.StartTimer();
		//bs.GetPath(me, start, goal, me, me, bsPath);
		t.EndTimer();
		t2 = t.GetElapsedTime();
		//printf("%d %1.1f BS nodes: %llu necessary %llu time: %1.2f \n", x, me->GetPathLength(bsPath), bs.GetNodesExpanded(), bs.GetNecessaryExpansions(), t.GetElapsedTime());
		t.StartTimer();
		//mm.GetPath(me, start, goal, me, me, mmPath);
		t.EndTimer();
		t3 = t.GetElapsedTime();
		//printf("%d %1.1f MM nodes: %llu necessary %llu time: %1.2f \n", x, me->GetPathLength(mmPath), mm.GetNodesExpanded(), mm.GetNecessaryExpansions(), t.GetElapsedTime());
		t.StartTimer();
		nbs.GetPath(me, start, goal, me, me, nbsPath);
		t.EndTimer();
		t4 = t.GetElapsedTime();
		//printf("%d %1.1f NBS nodes: %llu necessary %llu time: %1.2f \n", x, me->GetPathLength(nbsPath), nbs.GetNodesExpanded(), nbs.GetNecessaryExpansions(), t.GetElapsedTime());
		t.StartTimer();
		//mm0.GetPath(me, start, goal, &z, &z, mm0Path);
		nbslazy.GetPath(me, start, goal, me, me, nbslazyPath);
		t.EndTimer();
		t5 = t.GetElapsedTime();
		t.StartTimer();
		nbsgreedy.GetPath(me, start, goal, me, me, nbsgreedyPath);
		t.EndTimer();
		t6 = t.GetElapsedTime();
		//printf("%d %1.1f MM0 nodes: %llu necessary %llu time: %1.2f \n", x, me->GetPathLength(mm0Path), mm0.GetNodesExpanded(), mm0.GetNecessaryExpansions(), t.GetElapsedTime());

		//printf("NBSNecessaryRatios: NBS/A* %1.2f NBS/BS %1.2f NBS/MM %1.2f NBS/MM0 %1.2f\n",
		//	   (double)nbs.GetNecessaryExpansions()/astar.GetNecessaryExpansions(),
		//	   (double)nbs.GetNecessaryExpansions()/bs.GetNecessaryExpansions(),
		//	   (double)nbs.GetNecessaryExpansions()/mm.GetNecessaryExpansions(),
		//	   (double)nbs.GetNecessaryExpansions()/mm0.GetNecessaryExpansions()
		//	   );
		//printf("SelfNecessaryRatios: A* %1.2f BS %1.2f MM %1.2f NBS %1.2f MM0 %1.2f\n",
		//	   (double)astar.GetNodesExpanded()/astar.GetNecessaryExpansions(),
		//	   (double)bs.GetNodesExpanded()/bs.GetNecessaryExpansions(),
		//	   (double)mm.GetNodesExpanded()/mm.GetNecessaryExpansions(),
		//	   (double)nbs.GetNodesExpanded()/nbs.GetNecessaryExpansions(),
		//	   (double)mm0.GetNodesExpanded()/mm0.GetNecessaryExpansions()
		//	   );

		std::cout << "nodes:\t"
			<< astar.GetNodesExpanded() << "\t"
			<< bs.GetNodesExpanded() << "\t"
			<< mm.GetNodesExpanded() << "\t"
			<< mm.GetNecessaryExpansions() << "\t"
			<< nbs.GetNodesExpanded() << "\t"
			//<< mm0.GetNodesExpanded() << "\t"
			<<nbslazy.GetNodesExpanded()<<"\t"
			<< nbsgreedy.GetNodesExpanded() << "\n";
		std::cout << "time:\t"
			<< t1 << "\t"
			<< t2 << "\t"
			<< t3 << "\t"
			<< t4 << "\t"
			<< t5 << "\t"
			<< t6 << "\n";
//		std::cout << "\tMM:\t" << mm.GetNodesExpanded() << "\n";
//		printf("NBS* total\t%llu\tnecessary\t%llu\tdoubles\t%llu\t", nbs.GetNodesExpanded(), nbs.GetNecessaryExpansions(), nbs.GetDoubleExpansions());
//		printf("A* total\t%llu\tnecessary\t%llu\tratio\t%1.3f\n", astar.GetNodesExpanded(), astar.GetNecessaryExpansions(),
//			   (double)nbs.GetNecessaryExpansions()/astar.GetNecessaryExpansions());
		//if (!fequal)
		//if (!fequal(me->GetPathLength(nbsgreedyPath), me->GetPathLength(correctPath)))
		if (!fequal(nbslazy.GetSolutionCost(), me->GetPathLength(correctPath)))
		{
			std::cout << "error solution cost:\t expected cost\n";
			//std::cout << me->GetPathLength(nbsgreedyPath) << "\t" << me->GetPathLength(correctPath) << "\n";
			std::cout << nbslazy.GetSolutionCost() << "\t" << me->GetPathLength(correctPath) << "\n";

			//double d;
			//for (auto x : correctPath)
			//{
			//	astar.GetClosedListGCost(x, d);
			//	auto t = nbs.GetNodeForwardLocation(x);
			//	auto u = nbs.GetNodeBackwardLocation(x);
			//	std::cout << x << " is on " << t << " and " << u << "\n";
			//	std::cout << "True g: " << d;
			//	if (t != kUnseen)
			//		std::cout << " forward g: " << nbs.GetNodeForwardG(x);
			//	if (u != kUnseen)
			//		std::cout << " backward g: " << nbs.GetNodeBackwardG(x);
			//	std::cout << "\n";
			//}
			exit(0);
		}
		
	}
	printf("Exiting with no errors\n");
	exit(0);
}

void Predictor(const char *map, const char *scenario, double weight)
{
	int MAX_STEP = 50;
	double SIGNIFICANT = 10.0;
	double DELTA = 1.0;
	double errorF, errorB;
	NBS<xyLoc, tDirection, MapEnvironment> nbs;

	TemplateAStar<xyLoc, tDirection, MapEnvironment> forwardAStar;
	TemplateAStar<xyLoc, tDirection, MapEnvironment> backwardAStar;

	printf("Loading %s with scenario %s\n", map, scenario);
	ScenarioLoader s(scenario);
	Map *m = new Map(map);
	me = new MapEnvironment(m);
	me->SetDiagonalCost(1.5);

	Timer t;
	ZeroHeuristic<xyLoc> z;

	for (int x = s.GetNumExperiments() - 1; x >= 0; x--) // 547 to 540
	{
		if (fequal(s.GetNthExperiment(x).GetDistance(), 0))
			continue;
		xyLoc start, goal;
		start.x = s.GetNthExperiment(x).GetStartX();
		start.y = s.GetNthExperiment(x).GetStartY();
		goal.x = s.GetNthExperiment(x).GetGoalX();
		goal.y = s.GetNthExperiment(x).GetGoalY();
		printf("Problem %d of %d from ", x, s.GetNumExperiments());
		std::cout << start << " to " << goal << "\n";
		std::vector<xyLoc> forwardAStarPath;
		std::vector<xyLoc> backwardAStarPath;

		std::vector<xyLoc> nbsPath;

		forwardAStar.SetHeuristic(me);
		backwardAStar.SetHeuristic(me);

		double t1, t2, t3, t4, t5, t6;
		
		
		forwardAStar.InitializeSearch(me, start, goal, forwardAStarPath);
		backwardAStar.InitializeSearch(me, goal, start, backwardAStarPath);


		t.StartTimer();

		bool solved = false;
		for (int i = 0; i < MAX_STEP; i++)
		{
			if (forwardAStar.DoSingleSearchStep(forwardAStarPath) || backwardAStar.DoSingleSearchStep(backwardAStarPath))
			{
				solved = true;
				break;
			}
		}
		if (solved)
			continue;

		errorF = 0;
		errorB = 0;

		for (int i = 0; i < forwardAStar.GetNumItems(); i++)
		{
			auto item = forwardAStar.GetItem(i);
			if (item.where != kClosedList)
				continue;
			auto st = item.data;
			double reverseH = me->HCost(st, start);

			errorF += item.g - reverseH;
		}

		for (int i = 0; i < backwardAStar.GetNumItems(); i++)
		{
			auto item = backwardAStar.GetItem(i);
			if (item.where != kClosedList)
				continue;
			auto st = item.data;
			double reverseH = me->HCost(st, goal);

			errorB += item.g - reverseH;
		}

		int openSizeF = forwardAStar.GetNumOpenItems();
		int openSizeB = backwardAStar.GetNumOpenItems();
		std::cout << "errors:\t"
			<< errorF << "\t"
			<< errorB << "\n";

		int nodeExp = 100;
		
		if (errorF > SIGNIFICANT && errorB > SIGNIFICANT)
		{
			nbs.GetPath(me, start, goal, me, me, nbsPath);
			nodeExp += nbs.GetNodesExpanded();
		}
		else if (errorF - errorB < DELTA && errorF - errorB > 0 - DELTA)//approximately equal
		{
			if (openSizeF<openSizeB)
			{
				forwardAStar.GetPath(me, start, goal, forwardAStarPath);
				nodeExp += forwardAStar.GetNodesExpanded();
			}
			else
			{
				backwardAStar.GetPath(me, goal, start, backwardAStarPath);
				nodeExp += backwardAStar.GetNodesExpanded();
			}
		}
		else if (errorF - errorB > DELTA)
		{
			forwardAStar.GetPath(me, start, goal, forwardAStarPath);
			nodeExp += forwardAStar.GetNodesExpanded();
		}
		else
		{
			backwardAStar.GetPath(me, goal, start, backwardAStarPath);
			nodeExp += backwardAStar.GetNodesExpanded();
		}
			

		t.EndTimer();
		t1 = t.GetElapsedTime();
	

		t.StartTimer();
		forwardAStar.GetPath(me, start, goal, forwardAStarPath);
		t.EndTimer();
		t2 = t.GetElapsedTime();
		
		t.StartTimer();
		backwardAStar.GetPath(me, goal, start, backwardAStarPath);
		t.EndTimer();
		t3 = t.GetElapsedTime();
		
		t.StartTimer();
		nbs.GetPath(me, start, goal, me, me, nbsPath);
		t.EndTimer();
		t4 = t.GetElapsedTime();
	

		std::cout << "nodes:\t"
			<< nodeExp << "\t"
			<< forwardAStar.GetNodesExpanded() << "\t"
			<< backwardAStar.GetNodesExpanded() << "\t"
			<< nbs.GetNodesExpanded() << "\n";
		//std::cout << "time:\t"
		//	<< t1 << "\t"
		//	<< t2 << "\t"
		//	<< t3 << "\t"
		//	<< t4 << "\n";

	}
	printf("Exiting with no errors\n");
	exit(0);
}

void AnalyzeBD(const char *scenario, int max_step)
{
	double errorF, errorB;
	NBS<xyLoc, tDirection, MapEnvironment> nbs;

	TemplateAStar<xyLoc, tDirection, MapEnvironment> forwardAStar;
	TemplateAStar<xyLoc, tDirection, MapEnvironment> backwardAStar;

	printf("Loading scenario %s\n", scenario);
	ScenarioLoader s(scenario);

	std::string base = "/home/jingwei/Desktop/Shared/nbs_ad/hog2/";
	Timer t;
	ZeroHeuristic<xyLoc> z;

	std::string mapName = "";
	for (int x = s.GetNumExperiments() - 1; x >= 0; x--)
	{
		if (fequal(s.GetNthExperiment(x).GetDistance(), 0))
			continue;
		std::string currentMapName(s.GetNthExperiment(x).GetMapName());
		if (mapName.compare(currentMapName)!=0)
		{
			mapName = currentMapName;
			std::string fullPath = base + mapName;
			Map *m = new Map(fullPath.c_str());
			me = new MapEnvironment(m);
			me->SetDiagonalCost(1.5);
		}
		xyLoc start, goal;
		start.x = s.GetNthExperiment(x).GetStartX();
		start.y = s.GetNthExperiment(x).GetStartY();
		goal.x = s.GetNthExperiment(x).GetGoalX();
		goal.y = s.GetNthExperiment(x).GetGoalY();
		printf("Problem %d of %d from ", x, s.GetNumExperiments());
		std::cout << start << " to " << goal << " "<<mapName <<"\n";
		std::vector<xyLoc> forwardAStarPath;
		std::vector<xyLoc> backwardAStarPath;

		std::vector<xyLoc> nbsPath;

		forwardAStar.SetHeuristic(me);
		backwardAStar.SetHeuristic(me);

		double t1, t2, t3, t4, t5, t6;


		forwardAStar.InitializeSearch(me, start, goal, forwardAStarPath);
		backwardAStar.InitializeSearch(me, goal, start, backwardAStarPath);


		t.StartTimer();

		bool solved = false;

		if (max_step < 50) 
		{
			double forwardCurrentF, backwardCurrentF;
			uint64_t id1 = forwardAStar.openClosedList.Peek();
			forwardCurrentF = forwardAStar.openClosedList.Lookup(id1).g + forwardAStar.openClosedList.Lookup(id1).h;
			uint64_t id2 = backwardAStar.openClosedList.Peek();
			backwardCurrentF = backwardAStar.openClosedList.Lookup(id2).g + backwardAStar.openClosedList.Lookup(id2).h;

			int layer = 0;
			while (layer < max_step)
			{
				if (forwardAStar.DoSingleSearchStep(forwardAStarPath))
				{
					solved = true;
					break;
				}
				if (forwardAStar.openClosedList.OpenSize()>0)
				{
					uint64_t nodeid = forwardAStar.openClosedList.Peek();
					if (!fequal(forwardAStar.openClosedList.Lookup(nodeid).g + forwardAStar.openClosedList.Lookup(nodeid).h, forwardCurrentF))
					{
						layer++;
						forwardCurrentF = forwardAStar.openClosedList.Lookup(nodeid).g + forwardAStar.openClosedList.Lookup(nodeid).h;
					}
				}
			}
			layer = 0;
			while (layer < max_step)
			{
				if (backwardAStar.DoSingleSearchStep(backwardAStarPath))
				{
					solved = true;
					break;
				}
				if (backwardAStar.openClosedList.OpenSize()>0)
				{
					uint64_t nodeid = backwardAStar.openClosedList.Peek();
					if (!fequal(backwardAStar.openClosedList.Lookup(nodeid).g + backwardAStar.openClosedList.Lookup(nodeid).h, backwardCurrentF))
					{
						layer++;
						backwardCurrentF = backwardAStar.openClosedList.Lookup(nodeid).g + backwardAStar.openClosedList.Lookup(nodeid).h;
					}
				}
			}
		}
		else
		{
			for (int i = 0; i < max_step; i++)
			{
				if (forwardAStar.DoSingleSearchStep(forwardAStarPath) || backwardAStar.DoSingleSearchStep(backwardAStarPath))
				{
					solved = true;
					break;
				}
			}
		}
		

		if (solved)
			continue;

		errorF = 0;
		errorB = 0;

		for (int i = 0; i < forwardAStar.GetNumItems(); i++)
		{
			auto item = forwardAStar.GetItem(i);
			if (item.where != kClosedList)
				continue;
			auto st = item.data;
			double reverseH = me->HCost(st, start);

			errorF += item.g - reverseH;
		}

		for (int i = 0; i < backwardAStar.GetNumItems(); i++)
		{
			auto item = backwardAStar.GetItem(i);
			if (item.where != kClosedList)
				continue;
			auto st = item.data;
			double reverseH = me->HCost(st, goal);

			errorB += item.g - reverseH;
		}

		int openSizeF = forwardAStar.GetNumOpenItems();
		int openSizeB = backwardAStar.GetNumOpenItems();
		std::cout << "errors:\t"
			<< errorF << "\t"
			<< errorB << "\n";

		std::cout << "opensize:\t"
			<< openSizeF << "\t"
			<< openSizeB << "\n";


		t.EndTimer();
		t1 = t.GetElapsedTime();


		t.StartTimer();
		forwardAStar.GetPath(me, start, goal, forwardAStarPath);
		t.EndTimer();
		t2 = t.GetElapsedTime();

		t.StartTimer();
		backwardAStar.GetPath(me, goal, start, backwardAStarPath);
		t.EndTimer();
		t3 = t.GetElapsedTime();

		t.StartTimer();
		nbs.GetPath(me, start, goal, me, me, nbsPath);
		t.EndTimer();
		t4 = t.GetElapsedTime();


		std::cout << "nodes:\t"
			<< forwardAStar.GetNodesExpanded() << "\t"
			<< backwardAStar.GetNodesExpanded() << "\t"
			<< nbs.GetNodesExpanded() << "\n";

		//std::cout << "time:\t"
		//	<< t1 << "\t"
		//	<< t2 << "\t"
		//	<< t3 << "\t"
		//	<< t4 << "\n";

	}
	printf("Exiting with no errors\n");
	exit(0);
}


void AnalyzeWeighted(const char *scenario)
{
	NBSFF<xyLoc, tDirection, MapEnvironment> nbs;
	TemplateAStar<xyLoc, tDirection, MapEnvironment> forwardAStar;
	//forwardAStar.SetWeight(3);
	TemplateAStar<xyLoc, tDirection, MapEnvironment> backwardAStar;
	//backwardAStar.SetWeight(3);

	printf("Loading scenario %s\n", scenario);
	ScenarioLoader s(scenario);

	std::string base = "/home/jingwei/Desktop/Shared/nbs_ad/hog2/";
	Timer t;
	ZeroHeuristic<xyLoc> z;

	std::string mapName = "";
	for (int x = s.GetNumExperiments() - 1; x >= 0; x--)
	{
		if (fequal(s.GetNthExperiment(x).GetDistance(), 0))
			continue;
		std::string currentMapName(s.GetNthExperiment(x).GetMapName());
		if (mapName.compare(currentMapName) != 0)
		{
			mapName = currentMapName;
			std::string fullPath = base + mapName;
			Map *m = new Map(fullPath.c_str());
			me = new MapEnvironment(m);
			me->SetDiagonalCost(1.5);
		}
		xyLoc start, goal;
		start.x = s.GetNthExperiment(x).GetStartX();
		start.y = s.GetNthExperiment(x).GetStartY();
		goal.x = s.GetNthExperiment(x).GetGoalX();
		goal.y = s.GetNthExperiment(x).GetGoalY();
		printf("Problem %d of %d from ", x, s.GetNumExperiments());
		std::cout << start << " to " << goal << " " << mapName << "\n";
		std::vector<xyLoc> forwardAStarPath;
		std::vector<xyLoc> backwardAStarPath;

		std::vector<xyLoc> nbsPath;

		forwardAStar.SetHeuristic(me);
		backwardAStar.SetHeuristic(me);

		double t1, t2, t3, t4, t5, t6;


		forwardAStar.InitializeSearch(me, start, goal, forwardAStarPath);
		backwardAStar.InitializeSearch(me, goal, start, backwardAStarPath);


		t.StartTimer();



		t.EndTimer();
		t1 = t.GetElapsedTime();


		t.StartTimer();
		forwardAStar.GetPath(me, start, goal, forwardAStarPath);
		t.EndTimer();
		t2 = t.GetElapsedTime();

		t.StartTimer();
		backwardAStar.GetPath(me, goal, start, backwardAStarPath);
		t.EndTimer();
		t3 = t.GetElapsedTime();

		t.StartTimer();
		nbs.GetPath(me, start, goal, me, me, nbsPath);
		t.EndTimer();
		t4 = t.GetElapsedTime();


		std::cout << "nodes:\t"
			<< forwardAStar.GetNodesExpanded() << "\t"
			<< backwardAStar.GetNodesExpanded() << "\t"
			<< nbs.GetNodesExpanded() << "\n";

		//std::cout << "time:\t"
		//	<< t1 << "\t"
		//	<< t2 << "\t"
		//	<< t3 << "\t"
		//	<< t4 << "\n";

	}
	printf("Exiting with no errors\n");
	exit(0);
}


#include "RubiksCube.h"
void tmp()
{
	printf("---NBS*---\n");
	Timer t;
	RubiksCube cube;
	RubiksState start, goal;
	std::vector<RubiksState> thePath;
	thePath.clear();
	ZeroHeuristic<RubiksState> z;
	t.StartTimer();
	NBS<RubiksState, RubiksAction, RubiksCube> nbs;
	nbs.GetPath(&cube, start, goal, &z, &z, thePath);
	
	t.EndTimer();
	printf("%llu nodes expanded\n", nbs.GetNodesExpanded());
	printf("%llu neccesary nodes expanded\n", nbs.GetNecessaryExpansions());
	printf("Solution path length %1.0f\n", cube.GetPathLength(thePath));
	printf("%1.2f elapsed\n", t.GetElapsedTime());
}