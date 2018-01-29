//
//  NBSQueueLazy.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 2/10/17.
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef NBSQueueLazy_h
#define NBSQueueLazy_h

#include"NBSQueue.h"
#include "BDOpenClosed.h"

//enum lbSource {
//	kForwardF = 0,
//	kBackwardF = 1,
//	kGG
//};

template <typename state, int epsilon = 1>
class NBSQueueLazy {
public:
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
		// move items with f<CLowerBound to ready
		while (forwardQueue.OpenWaitingSize() != 0 && fless(forwardQueue.PeekAt(kOpenWaiting).g+forwardQueue.PeekAt(kOpenWaiting).h, CLowerBound))
		{
			forwardF = std::max(forwardF, forwardQueue.PeekAt(kOpenWaiting).g + forwardQueue.PeekAt(kOpenWaiting).h);
			forwardQueue.PutToReady();	
		}
		while (backwardQueue.OpenWaitingSize() != 0 && fless(backwardQueue.PeekAt(kOpenWaiting).g+backwardQueue.PeekAt(kOpenWaiting).h, CLowerBound))
		{
			backwardF = std::max(backwardF, backwardQueue.PeekAt(kOpenWaiting).g + backwardQueue.PeekAt(kOpenWaiting).h);
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
					backwardF = std::max(backwardF, backwardQueue.PeekAt(kOpenWaiting).g + backwardQueue.PeekAt(kOpenWaiting).h);
					backwardQueue.PutToReady();
				}
			}
			if (forwardQueue.OpenWaitingSize() != 0)
			{
				const auto i3 = forwardQueue.PeekAt(kOpenWaiting);
				if (!fgreater(i3.g+i3.h, CLowerBound))
				{
					changed = true;
					forwardF = std::max(forwardF, forwardQueue.PeekAt(kOpenWaiting).g + forwardQueue.PeekAt(kOpenWaiting).h);
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
					forwardF = std::max(forwardF, forwardQueue.PeekAt(kOpenWaiting).g + forwardQueue.PeekAt(kOpenWaiting).h);
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
					backwardF = std::max(backwardF, backwardQueue.PeekAt(kOpenWaiting).g + backwardQueue.PeekAt(kOpenWaiting).h);
					backwardQueue.PutToReady();
				}
				else
					break;
			}
			sumG = forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon;
//3
			//if (fequal(CLowerBound, sumG))
			//{
			//	if (balance < 0)
			//	{
			//		nextForward = forwardQueue.Peek(kOpenReady);
			//		nextBackward = kTBDNoNode;
			//		balance++;
			//	}
			//	else
			//	{
			//		nextForward = kTBDNoNode;
			//		nextBackward = backwardQueue.Peek(kOpenReady);
			//		balance--;
			//	}
			//}
			//else if (fequal(CLowerBound, forwardF))
			//{
			//	nextForward = forwardQueue.Peek(kOpenReady);
			//	nextBackward = kTBDNoNode;
			//	balance++;
			//}
			//else (fequal(CLowerBound, backwardF))
			//{
			//	nextForward = kTBDNoNode;
			//	nextBackward = backwardQueue.Peek(kOpenReady);
			//	balance--;
			//}
			double forwardGMax, backwardGMax;
			auto item1 = forwardQueue.Lookat(forwardQueue.GetOpenItem(forwardQueue.OpenReadySize() - 1, kOpenReady));
			auto item2 = backwardQueue.Lookat(backwardQueue.GetOpenItem(backwardQueue.OpenReadySize() - 1, kOpenReady));
			forwardGMax = item1.g;
			backwardGMax = item2.g;
//1
			//if (balance < 0)
			//{
			//	SetForward(nextForward, nextBackward);
			//}
			//else
			//{
			//	SetBackward(nextForward, nextBackward);
			//}
//2
			//if (fequal(CLowerBound, sumG))
			//{
			//	if (balance < 0)
			//	{
			//		SetForward(nextForward,nextBackward);
			//	}
			//	else
			//	{
			//		SetBackward(nextForward, nextBackward);
			//	}
			//}
			//else if (fequal(CLowerBound, forwardF))
			//{
			//	//if (!fless(forwardGMax + backwardQueue.PeekAt(kOpenReady).g + epsilon, CLowerBound))
			//	//if(fgreater(2*forwardQueue.PeekAt(kOpenReady).g,CLowerBound)
			//	//	&& (forwardQueue.OpenReadySize()>balance))
			//	if(forwardQueue.OpenReadySize()>balance)
			//		SetBackward(nextForward, nextBackward);
			//	else
			//		SetForward(nextForward, nextBackward);
			//}
			//else //(fequal(CLowerBound, backwardF))
			//{
			//	//if (!fless(backwardGMax + forwardQueue.PeekAt(kOpenReady).g + epsilon, CLowerBound))
			//	//if (fgreater(2 * backwardQueue.PeekAt(kOpenReady).g, CLowerBound)
			//	//	&& (backwardQueue.OpenReadySize()>0-balance))
			//	if (backwardQueue.OpenReadySize()>std::abs(balance))
			//		SetForward(nextForward, nextBackward);
			//	else
			//		SetBackward(nextForward, nextBackward);
			//}

			if (balance < 0)
			{
				SetForward(nextForward, nextBackward);
			}
			else if(balance>0)
			{
				SetBackward(nextForward, nextBackward);
			}
			else
			{
				int thecase = 0;
				if (fequal(CLowerBound, sumG))
				{
					SetForward(nextForward, nextBackward);
					thecase = 0;
				}
				else if (fequal(CLowerBound, forwardF))
				{
					balance = 0 - forwardQueue.OpenReadySize() / 2;
					SetForward(nextForward, nextBackward);
					thecase = 1;
				}
				else
				{
					balance = backwardQueue.OpenReadySize() / 2;
					SetBackward(nextForward, nextBackward);
					thecase = 2;
				}
				//std::cout << "lb: " << CLowerBound << "\n";
				//std::cout << "balance: " << balance << "\n";
				//std::cout << "the case: " << thecase << "\n";
			}
			return true;
		}
		//std::cout << "balance: "<<balance<< "\n";
		return false;
	}
	void Reset()
	{
		CLowerBound = 0;
		forwardQueue.Reset();
		backwardQueue.Reset();
		forwardF = 0;
		backwardF = 0;
		sumG = 0;
		balance = 0;
	}
	double GetLowerBound() { return CLowerBound; }
	void SetForward(uint64_t &nextForward, uint64_t &nextBackward) {
		nextForward = forwardQueue.Peek(kOpenReady);
		nextBackward = kTBDNoNode;
		balance++;
	}
	void SetBackward(uint64_t &nextForward, uint64_t &nextBackward) {
		nextForward = kTBDNoNode;
		nextBackward = backwardQueue.Peek(kOpenReady);
		balance--;
	}
	int GetBalance() { return balance; }
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> forwardQueue;
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> backwardQueue;
private:
	double CLowerBound;
	//lbSource lbsrc;
	double forwardF;
	double backwardF;
	double sumG;
	int balance;
	int dir;
	double lastG;
};

#endif /* NBSQueueLazy_h */
