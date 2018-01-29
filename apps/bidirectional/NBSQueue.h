//
//  NBSQueue.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 2/10/17.
//  Copyright © 2017 University of Denver. All rights reserved.
//
//DPNBSQueue

#ifndef NBSQueue_h
#define NBSQueue_h

#include "BDOpenClosed.h"

#define WEIGHTED
#define WEIGHT 100

//low g -> low f
template <class state>
struct NBSCompareOpenReady {
	NBSCompareOpenReady(): weight(
#ifdef WEIGHTED 
		WEIGHT/100.0
#else
		1
#endif
		){
		//std::cout << "ready weight" << weight << "\n";
	};

#ifdef WEIGHTED 
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
	{
		//if (fequal(i1.g, i2.g))
		//{
		//	return (!fgreater(i1.h, i2.h));
		//}
		//return (fgreater(i1.g, i2.g)); // low g over high

		double f1 = i1.g + weight*i1.h;
		double f2 = i2.g + weight*i2.h;

		if (fequal(i1.g, i2.g))
		{
			return (!fless(f1, f2));
		}
		return (fgreater(i1.g, i2.g)); // low g over high
	}
#else
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
#endif
	void SetWeight(double w) { weight = w; }
	double weight;
};

template <class state>
struct NBSCompareOpenWaiting {
	NBSCompareOpenWaiting() : weight(
#ifdef WEIGHTED 
		WEIGHT / 100.0
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
		WEIGHT / 100.0
#else
		1
#endif
		) {}
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
		bool gotpair = false;
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

			//if(fless(gBound,fBound))
			if (!fgreater(gBound, fBound))
			{
				nextForward = forwardQueue.Peek(kOpenReady);
				nextBackward = backwardQueue.Peek(kOpenReady);
				gotpair = true;
				CLowerBound = gBound;
				CLowerBound = std::max(CLowerBound, forwardQueue.PeekAt(kOpenReady).g + weight*forwardQueue.PeekAt(kOpenReady).h);
				CLowerBound = std::max(CLowerBound, backwardQueue.PeekAt(kOpenReady).g + weight*backwardQueue.PeekAt(kOpenReady).h);

				return true;
				break;
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
		if (gotpair)
		{

			//v1

			unsigned int floc, bloc;
			floc = bloc = 0;
			for (unsigned int i = 0; i < forwardQueue.OpenReadySize(); i++)
			{

				auto fitem = forwardQueue.Lookat(forwardQueue.GetOpenItem(i, kOpenReady));

				auto fbest = forwardQueue.Lookat(forwardQueue.GetOpenItem(floc, kOpenReady));

				//low h
				if (fless(fitem.h, fbest.h))
					floc = i;
				else if (fequal(fitem.h, fbest.h) && fless(fitem.g, fbest.g))
					floc = i;


				//DPS
				
				//if (fitem.h == 0)
				//{
				//	if (fbest.h != 0)
				//		floc = i;
				//	else if (fbest.h == 0 && fless(fitem.g, fbest.g))
				//		floc = i;
				//}
				//else//fitem.h != 0
				//{
				//	if (fbest.h == 0)
				//		continue;
				//	else//fbest.h!=0
				//	{
				//		if (fless((CLowerBound - fitem.g) / fitem.h, (CLowerBound - fbest.g) / fbest.h))
				//			floc = i;
				//		else if (fequal((CLowerBound - fitem.g) / fitem.h, (CLowerBound - fbest.g) / fbest.h))
				//		{
				//			//if (fless(fitem.g, fbest.g))
				//			if (fgreater(fitem.g, fbest.g))
				//				floc = i;
				//		}
				//		
				//	}
				//}
				
			}
			for (unsigned int j = 0; j < backwardQueue.OpenReadySize(); j++)
			{

				auto bitem = backwardQueue.Lookat(backwardQueue.GetOpenItem(j, kOpenReady));

				auto bbest = backwardQueue.Lookat(backwardQueue.GetOpenItem(bloc, kOpenReady));

				//low g
				
				//if (fless(bitem.g, bbest.g))
				//	bloc = j;
				//else if (fequal(bitem.g, bbest.g) && fless(bitem.h, bbest.h))
				//	bloc = j;
				//

				//low h
				
				if (fless(bitem.h,bbest.h))
					bloc = j;
				else if (fequal(bitem.h, bbest.h) && fless(bitem.g, bbest.g))
					bloc = j;
				

				//DPS
				//if (bitem.h == 0)
				//{
				//	if (bbest.h != 0)
				//		bloc = j;
				//	else if (bbest.h == 0 && fless(bitem.g, bbest.g))
				//		bloc = j;
				//}
				//else//bitem.h != 0
				//{
				//	if (bbest.h == 0)
				//		continue;
				//	else//bbest.h!=0
				//	{
				//		if (fless((CLowerBound - bitem.g) / bitem.h, (CLowerBound - bbest.g) / bbest.h))
				//			bloc = j;
				//		else if (fequal((CLowerBound - bitem.g) / bitem.h, (CLowerBound - bbest.g) / bbest.h))
				//		{
				//			if (fgreater(bitem.g, bbest.g))
				//			//if (fless(bitem.g, bbest.g))
				//				bloc = j;
				//		}
				//	}
				//}
				//
			}



			uint64_t fidx, bidx;
			fidx = forwardQueue.GetOpenItem(floc, kOpenReady);
			bidx = backwardQueue.GetOpenItem(bloc, kOpenReady);


			//v2

			//uint64_t fidx, bidx;
			//fidx = forwardQueue.GetOpenItem(forwardQueue.OpenReadySize()-1, kOpenReady);
			//bidx = backwardQueue.GetOpenItem(backwardQueue.OpenReadySize() - 1, kOpenReady);

			double g1, h1, g2, h2;
			auto fbest = forwardQueue.Lookat(fidx);
			auto bbest = backwardQueue.Lookat(bidx);
			g1 = fbest.g;
			h1 = fbest.h;
			g2 = bbest.g;
			h2 = bbest.h;
		
			int dir =2;
			//if (fless(fbest.h, bbest.h))
			//	dir = 0;
			//else if (fequal(fbest.h, bbest.h))
			//{
			//	if (fless(fbest.g, bbest.g))
			//		dir = 0;
			//	if (fequal(fbest.g, bbest.g))
			//		dir = 2;
			//}


			nextForward = nextBackward = kTBDNoNode;
			if (dir !=1)
			{
				forwardQueue.Lookup(fidx).g = 0;
				forwardQueue.Lookup(fidx).h = 0;
				forwardQueue.KeyChanged(fidx);
				forwardQueue.Lookup(fidx).g = g1;
				forwardQueue.Lookup(fidx).h = h1;
				nextForward = forwardQueue.Peek(kOpenReady);
			}
			if(dir!=0)
			{
				backwardQueue.Lookup(bidx).g = 0;
				backwardQueue.Lookup(bidx).h = 0;
				backwardQueue.KeyChanged(bidx);
				backwardQueue.Lookup(bidx).g = g2;
				backwardQueue.Lookup(bidx).h = h2;
				nextBackward = backwardQueue.Peek(kOpenReady);
			}

			

			return true;
		}
		*/
		//original nbs
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
