//
//  NBSFFQueue.h
//  hog2 glut
//
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef NBSFFQueue_h
#define NBSFFQueue_h

#include "BDFFOpenClosed.h"

//#define WEIGHTED
//#define WEIGHT 4

//low g -> low f
template <class state>
struct NBSFFCompareOpenReady {
	NBSFFCompareOpenReady(): weight(
#ifdef WEIGHTED 
		WEIGHT
#else
		1
#endif
		){
		//std::cout << "ready weight" << weight << "\n";
	};
	bool operator()(const BDFFOpenClosedData<state> &i1, const BDFFOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + weight*i1.h;
		double f2 = i2.g + weight*i2.h;
		
		if (fequal(i1.g, i2.g))
		{
			return (!fless(f1, f2));
		}
		return (fgreater(i1.g, i2.g)); // low g over high
	}
	void SetWeight(double w) { weight = w; }
	double weight;
};

template <class state>
struct NBSFFCompareOpenWaiting {
	NBSFFCompareOpenWaiting() : weight(
#ifdef WEIGHTED 
		WEIGHT
#else
		1
#endif
		) {
		//std::cout << "waiting weight" << weight << "\n";
	};
	bool operator()(const BDFFOpenClosedData<state> &i1, const BDFFOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + weight*i1.h;
		double f2 = i2.g + weight*i2.h;
		
		if (fequal(f1, f2))
		{
			return (!fgreater(i1.g, i2.g));
		}
		return (fgreater(f1, f2)); // low f over high
	}
	void SetWeight(double w) { weight = w; }
	double weight;
};

template <typename state, int epsilon = 0>
class NBSFFQueue {
public:
	NBSFFQueue():weight(
#ifdef WEIGHTED 
		WEIGHT
#else
		1
#endif
		) {}
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
		if (forwardQueue.OpenSize() == 0)
			return false;
		if (backwardQueue.OpenSize() == 0)
			return false;
		unsigned int floc, bloc;
		floc = bloc = 0;
		//inefficient implementation
		for (unsigned int i = 0; i < forwardQueue.OpenWaitingSize(); i++)
		{
			for (unsigned int j = 0; j < backwardQueue.OpenWaitingSize(); j++)
			{
				auto fitem = forwardQueue.Lookat(forwardQueue.GetOpenItem(i,kOpenWaiting));
				auto bitem = backwardQueue.Lookat(backwardQueue.GetOpenItem(j, kOpenWaiting));
				auto fbest = forwardQueue.Lookat(forwardQueue.GetOpenItem(floc, kOpenWaiting));
				auto bbest = backwardQueue.Lookat(backwardQueue.GetOpenItem(bloc, kOpenWaiting));
				//double lb = fbest.g + bbest.g;
				double lb = 0;
				lb = std::max(lb,fbest.g  + bbest.g + weight*(fbest.h - bbest.revH));
				lb = std::max(lb, bbest.g  + fbest.g + weight*(bbest.h - fbest.revH));
				//double clb = fitem.g + bitem.g;
				double clb = 0;
				clb = std::max(clb, fitem.g + bitem.g + weight*(fitem.h - bitem.revH));
				clb = std::max(clb, bitem.g + fitem.g + weight*(bitem.h - fitem.revH));
				if (fless(clb, lb))
				{
					floc = i;
					bloc = j;
				}
			}
		}
		uint64_t fidx, bidx;
		fidx = forwardQueue.GetOpenItem(floc, kOpenWaiting);
		bidx = backwardQueue.GetOpenItem(bloc, kOpenWaiting);
		double g1, h1, revH1, g2, h2, revH2;
		auto fbest = forwardQueue.Lookat(fidx);
		auto bbest = backwardQueue.Lookat(bidx);
		g1 = fbest.g;
		h1 = fbest.h;
		revH1 = fbest.revH;
		g2 = bbest.g;
		h2 = bbest.h;
		revH2 = bbest.revH;
		
		forwardQueue.Lookup(fidx).g = 0;
		forwardQueue.Lookup(fidx).h = 0;
		backwardQueue.Lookup(bidx).g = 0;
		backwardQueue.Lookup(bidx).h = 0;
		forwardQueue.KeyChanged(fidx);
		backwardQueue.KeyChanged(bidx);

		forwardQueue.PutToReady();
		backwardQueue.PutToReady();

		forwardQueue.Lookup(fidx).g = g1;
		forwardQueue.Lookup(fidx).h = h1;
		backwardQueue.Lookup(bidx).g = g2;
		backwardQueue.Lookup(bidx).h = h2;

		nextForward = forwardQueue.Peek(kOpenReady);
		nextBackward = backwardQueue.Peek(kOpenReady);

		//CLowerBound = g1 + g2;
		CLowerBound = 0;
		CLowerBound = std::max(CLowerBound, g1+g2 + weight*(h1 -revH2));
		CLowerBound = std::max(CLowerBound, g2+g1 + weight*(h2 -revH1));

		return true;

	}
	void Reset()
	{
		CLowerBound = 0;
		forwardQueue.Reset();
		backwardQueue.Reset();
	}
	void SetWeight(double w) { weight = w; }
	double GetLowerBound() { return CLowerBound; }
	BDFFOpenClosed<state, NBSFFCompareOpenReady<state>, NBSFFCompareOpenWaiting<state>> forwardQueue;
	BDFFOpenClosed<state, NBSFFCompareOpenReady<state>, NBSFFCompareOpenWaiting<state>> backwardQueue;
private:
	double CLowerBound;
	double weight;
};

#endif /* NBSFFQueue_h */
