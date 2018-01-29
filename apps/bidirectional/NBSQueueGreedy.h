//
//  NBSQueueGreedy.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 2/10/17.
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef NBSQueueGreedy_h
#define NBSQueueGreedy_h

#include"NBSQueue.h"
#include "BDOpenClosed.h"

enum lbSource {
	kForwardF = 0,
	kBackwardF = 1,
	kGG
};

template <typename state, int epsilon = 1>
class NBSQueueGreedy {
public:
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
		// move items with f<CLowerBound to ready
		while (forwardQueue.OpenWaitingSize() != 0 && fless(forwardQueue.PeekAt(kOpenWaiting).g+forwardQueue.PeekAt(kOpenWaiting).h, CLowerBound))
		{
			forwardQueue.PutToReady();
		}
		while (backwardQueue.OpenWaitingSize() != 0 && fless(backwardQueue.PeekAt(kOpenWaiting).g+backwardQueue.PeekAt(kOpenWaiting).h, CLowerBound))
		{
			backwardQueue.PutToReady();
		}
		bool lbStable = false;
		while (true)
		{
			if (forwardQueue.OpenSize() == 0)
				return false;
			if (backwardQueue.OpenSize() == 0)
				return false;
			if ((forwardQueue.OpenReadySize() != 0) && (backwardQueue.OpenReadySize() != 0) &&
				(!fgreater(forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon, CLowerBound)))
			{
				lbStable = true;
				break;
			}
			bool changed = false;


			if (backwardQueue.OpenWaitingSize() != 0)
			{
				const auto i4 = backwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i4.g+i4.h, CLowerBound))
				{
					changed = true;
					backwardQueue.PutToReady();
				}
			}
			if (forwardQueue.OpenWaitingSize() != 0)
			{
				const auto i3 = forwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i3.g+i3.h, CLowerBound))
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
					//if (fless(i5.g + i5.h, CLowerBound))
					//{
					//	CLowerBound = i5.g + i5.h;
					//	lbsrc = kForwardF;
					//}
					CLowerBound = std::min(CLowerBound, i5.g+i5.h);
				}
				if (backwardQueue.OpenWaitingSize() != 0)
				{
					const auto i6 = backwardQueue.PeekAt(kOpenWaiting);
					//if (fless(i6.g + i6.h, CLowerBound))
					//{
					//	CLowerBound = i6.g + i6.h;
					//	lbsrc = kBackwardF;
					//}
					CLowerBound = std::min(CLowerBound, i6.g+i6.h);
				}
				if ((forwardQueue.OpenReadySize() != 0) && (backwardQueue.OpenReadySize() != 0))
				{
					CLowerBound = std::min(CLowerBound, forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon);

				}
			}

		}
		if (lbStable == true)
		{
			while (forwardQueue.OpenWaitingSize() != 0)
			{
				const auto i = forwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i.g + i.h, CLowerBound))
				{
					forwardQueue.PutToReady();
				}
				else
					break;
			}
			while(backwardQueue.OpenWaitingSize() != 0)
			{
				const auto i = backwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i.g + i.h, CLowerBound))
				{
					backwardQueue.PutToReady();
				}
				else
					break;
			}
			if (forwardQueue.OpenReadySize() > backwardQueue.OpenReadySize())
			{
				nextForward = kTBDNoNode;
				nextBackward = backwardQueue.Peek(kOpenReady);
			}
			else
			{
				nextForward = forwardQueue.Peek(kOpenReady);
				nextBackward = kTBDNoNode;
			}
			return true;
		}
		return false;
	}
	void Reset()
	{
		CLowerBound = 0;
		forwardQueue.Reset();
		backwardQueue.Reset();
	}
	double GetLowerBound() { return CLowerBound; }
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> forwardQueue;
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> backwardQueue;
private:
	double CLowerBound;
	lbSource lbsrc;
};

#endif /* NBSQueueGreedy_h */
