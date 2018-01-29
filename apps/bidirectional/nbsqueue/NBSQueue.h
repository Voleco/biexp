//
//  NBSQueue.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 2/10/17.
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef NBSQueue_h
#define NBSQueue_h

#include "BDOpenClosed.h"

#define WEIGHTED
#define WEIGHT 2

//low g -> low f
template <class state>
struct NBSCompareOpenReady {
	NBSCompareOpenReady(): weight(
#ifdef WEIGHTED 
		WEIGHT
#else
		1
#endif
		){
		//std::cout << "ready weight" << weight << "\n";
	};
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
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
struct NBSCompareOpenWaiting {
	NBSCompareOpenWaiting() : weight(
#ifdef WEIGHTED 
		WEIGHT
#else
		1
#endif
		) {
		//std::cout << "waiting weight" << weight << "\n";
	};
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
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
class NBSQueue {
public:
	NBSQueue():weight(
#ifdef WEIGHTED 
		WEIGHT
#else
		1
#endif
		) {}
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
/*
		// move items with f<CLowerBound to ready
		//while (forwardQueue.OpenWaitingSize() != 0 && fless(forwardQueue.PeekAt(kOpenWaiting).g+forwardQueue.PeekAt(kOpenWaiting).h, CLowerBound))
		//{
		//	forwardQueue.PutToReady();
		//}
		//while (backwardQueue.OpenWaitingSize() != 0 && fless(backwardQueue.PeekAt(kOpenWaiting).g+backwardQueue.PeekAt(kOpenWaiting).h, CLowerBound))
		//{
		//	backwardQueue.PutToReady();
		//}
*/

		if (forwardQueue.OpenSize() == 0)
			return false;
		if (backwardQueue.OpenSize() == 0)
			return false;

		if (forwardQueue.OpenReadySize() == 0)
			forwardQueue.PutToReady();
		if (backwardQueue.OpenReadySize() == 0)
			backwardQueue.PutToReady();
		double fBound = DBL_MAX;
		double gBound = DBL_MAX;
		while (true)
		{
			fBound = DBL_MAX;
			if (forwardQueue.OpenWaitingSize() > 0)
				fBound = std::min(fBound, forwardQueue.PeekAt(kOpenWaiting).g + weight*forwardQueue.PeekAt(kOpenWaiting).h);
			if (backwardQueue.OpenWaitingSize() > 0)
				fBound = std::min(fBound, backwardQueue.PeekAt(kOpenWaiting).g + weight*backwardQueue.PeekAt(kOpenWaiting).h);

			gBound = (forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon) ;

			//if (forwardQueue.OpenReadySize() > 1 || backwardQueue.OpenReadySize() > 1)
			//{
			//	std::cout << "ready g" << forwardQueue.PeekAt(kOpenReady).g << " , " << backwardQueue.PeekAt(kOpenReady).g << "\n";
			//	if (fBound != DBL_MAX)
			//	{
			//		std::cout << "waitingF g h" << forwardQueue.PeekAt(kOpenWaiting).g << " , " << forwardQueue.PeekAt(kOpenWaiting).h << "\n";
			//		std::cout << "waitingB g h" << backwardQueue.PeekAt(kOpenWaiting).g << " , " << backwardQueue.PeekAt(kOpenWaiting).h << "\n";
			//	}
			//}

			if(!fgreater(gBound,fBound))
			{
				nextForward = forwardQueue.Peek(kOpenReady);
				nextBackward = backwardQueue.Peek(kOpenReady);
				
				CLowerBound = gBound;
				CLowerBound = std::max(CLowerBound, forwardQueue.PeekAt(kOpenReady).g + weight*forwardQueue.PeekAt(kOpenReady).h);
				CLowerBound = std::max(CLowerBound, backwardQueue.PeekAt(kOpenReady).g + weight*backwardQueue.PeekAt(kOpenReady).h);

				return true;
			}

			//std::cout << "ready g" << forwardQueue.PeekAt(kOpenReady).g << " , " << backwardQueue.PeekAt(kOpenReady).g << "\n";
			//if (fBound != DBL_MAX)
			//{
			//	std::cout << "waitingF g h" << forwardQueue.PeekAt(kOpenWaiting).g << " , " << forwardQueue.PeekAt(kOpenWaiting).h << "\n";
			//	std::cout << "waitingB g h" << backwardQueue.PeekAt(kOpenWaiting).g << " , " << backwardQueue.PeekAt(kOpenWaiting).h << "\n";
			//}

			if (forwardQueue.OpenWaitingSize() == 0)
				backwardQueue.PutToReady();
			else if (backwardQueue.OpenWaitingSize() == 0)
				forwardQueue.PutToReady();
			else
			{
				if (fless(forwardQueue.PeekAt(kOpenWaiting).g + weight*forwardQueue.PeekAt(kOpenWaiting).h, backwardQueue.PeekAt(kOpenWaiting).g + weight*backwardQueue.PeekAt(kOpenWaiting).h))
				{
					forwardQueue.PutToReady();
				}
				else
					backwardQueue.PutToReady();
			}

		}
		/*
		while (true)
		{
			if (forwardQueue.OpenSize() == 0)
				return false;
			if (backwardQueue.OpenSize() == 0)
				return false;
			if ((forwardQueue.OpenReadySize() != 0) && (backwardQueue.OpenReadySize() != 0) &&
				(!fgreater((forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon)/weight, CLowerBound)))
			{
				nextForward = forwardQueue.Peek(kOpenReady);
				nextBackward = backwardQueue.Peek(kOpenReady);
				return true;
			}
			bool changed = false;

//			if (forwardQueue.OpenWaitingSize() != 0 && backwardQueue.OpenWaitingSize() != 0)
//			{
//				const auto i3 = forwardQueue.PeekAt(kOpenWaiting);
//				const auto i4 = backwardQueue.PeekAt(kOpenWaiting);
//				if ((!fgreater(i3.g+i3.h, CLowerBound)) && (!fgreater(i4.g+i4.h, CLowerBound)))
//				{
//					changed = true;
//					if (fgreater(i3.g, i4.g))
//						forwardQueue.PutToReady();
//					else
//						backwardQueue.PutToReady();
//				}
//				else if (!fgreater(i3.g+i3.h, CLowerBound))
//				{
//					changed = true;
//					forwardQueue.PutToReady();
//				}
//				else if (!fgreater(i4.g+i4.h, CLowerBound))
//				{
//					changed = true;
//					backwardQueue.PutToReady();
//				}
//			}
			if (backwardQueue.OpenWaitingSize() != 0)
			{
				const auto i4 = backwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i4.g+weight*i4.h, CLowerBound))
				{
					changed = true;
					backwardQueue.PutToReady();
				}
			}
			if (forwardQueue.OpenWaitingSize() != 0)
			{
				const auto i3 = forwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i3.g+weight*i3.h, CLowerBound))
				{
					changed = true;
					forwardQueue.PutToReady();
				}
			}
			if (!changed)
			{
				CLowerBound = DBL_MAX;
				if (forwardQueue.OpenWaitingSize() != 0)
				{
					const auto i5 = forwardQueue.PeekAt(kOpenWaiting);
					CLowerBound = std::min(CLowerBound, i5.g+weight*i5.h);
				}
				if (backwardQueue.OpenWaitingSize() != 0)
				{
					const auto i6 = backwardQueue.PeekAt(kOpenWaiting);
					CLowerBound = std::min(CLowerBound, i6.g+weight*i6.h);
				}
				if ((forwardQueue.OpenReadySize() != 0) && (backwardQueue.OpenReadySize() != 0))
					CLowerBound = std::min(CLowerBound, (forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon)/weight);
			}

		}
		*/
		return false;
	}
	void Reset()
	{
		CLowerBound = 0;
		forwardQueue.Reset();
		backwardQueue.Reset();
	}
	void SetWeight(double w) { weight = w; }
	double GetLowerBound() { return CLowerBound; }
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> forwardQueue;
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> backwardQueue;
private:
	double CLowerBound;
	double weight;
};

#endif /* NBSQueue_h */
