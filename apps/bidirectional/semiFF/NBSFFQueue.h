//
//  NBSFFQueue.h
//  hog2 glut
//
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef NBSFFQueue_h
#define NBSFFQueue_h

#include "BDFFOpenClosed.h"



template <class state>
struct NBSFFCompareOpenReady {
	NBSFFCompareOpenReady() {	};
	bool operator()(const BDFFOpenClosedData<state> &i1, const BDFFOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + i1.h;
		double f2 = i2.g + i2.h;

		if (fequal(f1, f2))
		{
			return (!fgreater(i1.g, i2.g));
		}
		return (fgreater(f1, f2)); // low f over high
	}

};

template <class state>
struct NBSFFCompareOpenWaiting {
	NBSFFCompareOpenWaiting() {	};
	bool operator()(const BDFFOpenClosedData<state> &i1, const BDFFOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + i1.h;
		double f2 = i2.g + i2.h;
		
		if (fequal(f1, f2))
		{
			return (!fgreater(i1.g, i2.g));
		}
		return (fgreater(f1, f2)); // low f over high
	}

};

template <typename state, int epsilon = 0>
class NBSFFQueue {
public:
	NBSFFQueue(){}
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
		//cout << "forward open:\n";
		//for (auto i = forwardQueue.open.begin(); i != forwardQueue.open.end(); i++)
		//{
		//	cout << "info: "<<i->first<<" count " <<i->second.size()<<"\n";
		//}
		//cout << "backward open:\n";
		//for (auto i = backwardQueue.open.begin(); i != backwardQueue.open.end(); i++)
		//{
		//	cout << "info: " << i->first << " count " << i->second.size()<<"\n";
		//}

		if (forwardQueue.OpenSize() == 0)
			return false;
		if (backwardQueue.OpenSize() == 0)
			return false;

		double lb = DBL_MAX;
		double clb;
		openInfo fbest, bbest;
		for (auto i = forwardQueue.open.begin(); i != forwardQueue.open.end(); i++)
		{
			openInfo forInfo = i->first;
			for (auto j = backwardQueue.open.begin(); j != backwardQueue.open.end(); j++)
			{
				openInfo backInfo = j->first;
				//cout << "ok1 \n";
				clb = 0;
				clb = std::max(clb, forInfo.g + forInfo.h + backInfo.g - backInfo.revH);
				clb = std::max(clb, backInfo.g + backInfo.h + forInfo.g - forInfo.revH);
				if (fless(clb, lb))
				{
					fbest = forInfo;
					bbest = backInfo;
					lb = clb;
				}
				else if (fequal(clb, lb))
				{
					if (forInfo < fbest)
						fbest = forInfo;
					if (backInfo < bbest)
						bbest = backInfo;
				}
			}
		}
		//cout << "ok2 \n";
		//cout << (forwardQueue.open.find(fbest)->second).size() << "\n";
		nextForward = *((forwardQueue.open.find(fbest)->second).begin());
		nextBackward = *((backwardQueue.open.find(bbest)->second).begin());
		//cout << "ok3 \n";
		CLowerBound = lb;

		//cout << "f open size " << forwardQueue.OpenSize() << " b open size " << backwardQueue.OpenSize() << "\n";
		//cout << "pair: f Info " << fbest << " b Info " << bbest << "\n";
		//cout << "f id " << nextForward << " b id " << nextBackward << "\n";

		return true;

	}
	void Reset()
	{
		CLowerBound = 0;
		forwardQueue.Reset();
		backwardQueue.Reset();
	}
	double GetLowerBound() { return CLowerBound; }
	BDFFOpenClosed<state, NBSFFCompareOpenReady<state>, NBSFFCompareOpenWaiting<state>> forwardQueue;
	BDFFOpenClosed<state, NBSFFCompareOpenReady<state>, NBSFFCompareOpenWaiting<state>> backwardQueue;
private:
	double CLowerBound;
};

#endif /* NBSFFQueue_h */
