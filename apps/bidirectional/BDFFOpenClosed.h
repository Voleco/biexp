/*
 *  BDFFOpenClosed.h
 */

#ifndef BDFFOPENCLOSED_H
#define BDFFOPENCLOSED_H

#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <ext/hash_map>
#include <stdint.h>

struct openInfo {
	double g;        
	double h;         
	double revH;
};

static bool operator==(const openInfo& a, const openInfo &b)
{
	return (a.g == b.g && a.h == b.h && a.revH == b.revH);
}

static bool operator<(const openInfo& a, const openInfo &b)
{
	double f1 = a.g + a.h;
	double f2 = b.g + b.h;

	if (fequal(f1, f2))
	{
		return (!fgreater(a.g, b.g));
	}
	return (fgreater(f1, f2)); // low f over high
}

static std::ostream &operator<<(std::ostream &out, const openInfo &d)
{
	out << "[" << " g:" << +d.g << ", h:" << +d.h;
	out << ", revH:" << +d.revH << "]";
	return out;
}

struct openInfoHash
{
	std::size_t operator()(const openInfo & x) const
	{
		return std::hash<double>{}(x.g) | std::hash<double>{}(x.h) | std::hash<double>{}(x.revH);
	}
};

struct openBucket {
	std::unordered_set<uint64_t> states;
};

enum FFstateLocation {
	kFFOpen,
	kFFClosed,
	kFFUnseen
};



const uint64_t kTBDFFNoNode = 0xFFFFFFFFFFFFFFFFull;

template<typename state>
class BDFFOpenClosedData {
public:
	BDFFOpenClosedData() {}
	BDFFOpenClosedData(const state &theData, double gCost, double hCost, double reverseHCost, uint64_t parent, uint64_t openLoc, FFstateLocation location)
	:data(theData), g(gCost), h(hCost), revH(reverseHCost), parentID(parent), openLocation(openLoc), where(location) { reopened = false; }
	state data;
	double g;
	double h;
	double revH;
	uint64_t parentID;
	uint64_t openLocation;
	bool reopened;
	FFstateLocation where;
};

template<typename state, typename CmpKey0, typename CmpKey1, class dataStructure = BDFFOpenClosedData<state> >
class BDFFOpenClosed {
public:
	BDFFOpenClosed();
	~BDFFOpenClosed();
	void Reset();
	uint64_t AddOpenNode(const state &val, uint64_t hash, double g, double h,double revH, uint64_t parent=kTBDFFNoNode);
	uint64_t AddClosedNode(state &val, uint64_t hash, double g, double h,double revh, uint64_t parent=kTBDFFNoNode);
	void ChangeKey(uint64_t objKey, double ng, double nh = DBL_MIN, double nrevH = DBL_MIN);
	void Remove(uint64_t objKey);
	//void IncreaseKey(uint64_t objKey);
	FFstateLocation Lookup(uint64_t hashKey, uint64_t &objKey) const;
	inline dataStructure &Lookup(uint64_t objKey) { return elements[objKey]; }
	inline const dataStructure &Lookat(uint64_t objKey) const { return elements[objKey]; }

	//if exist min pair, return true, else(one queue is empty) return false;
	//if it returns true, left and right should be set to the pair that is supposed to be returned.
	//bool ExtractMinPair(uint64_t& left, uint64_t& right) ;
	

	void Close(uint64_t objKey);



//TODO
	
	//uint64_t GetOpenItem(unsigned int which, stateLocation where){	return priorityQueues[where][which];}

//TODO 	
	size_t OpenSize() const { return openSize; }

	size_t ClosedSize() const { return size()-OpenSize(); }
	size_t size() const { return elements.size(); }

	std::unordered_map<openInfo, std::unordered_set<uint64_t>,openInfoHash> open;

private:

	size_t openSize;

	// storing the element id; looking up with...hash?
	typedef __gnu_cxx::hash_map<uint64_t, uint64_t, AHash64> IndexTable;
	
	IndexTable table;
	//all the elements, open or closed
	std::vector<dataStructure> elements;
};





template<typename state, typename CmpKey0, typename CmpKey1, class dataStructure>
BDFFOpenClosed<state, CmpKey0, CmpKey1, dataStructure>::BDFFOpenClosed()
{
	openSize = 0;
}

template<typename state, typename CmpKey0, typename CmpKey1, class dataStructure>
BDFFOpenClosed<state, CmpKey0, CmpKey1, dataStructure>::~BDFFOpenClosed()
{
}

/**
 * Remove all objects from queue.
 */
template<typename state, typename CmpKey0, typename CmpKey1,   class dataStructure>
void BDFFOpenClosed<state, CmpKey0, CmpKey1,   dataStructure>::Reset()
{
	table.clear();
	elements.clear();
	open.clear();
	openSize = 0;
}

/**
 * Add object into open list.
 */
template<typename state, typename CmpKey0, typename CmpKey1,   class dataStructure>
uint64_t BDFFOpenClosed<state, CmpKey0, CmpKey1,   dataStructure>::AddOpenNode(const state &val, uint64_t hash, double g, double h,double revH, uint64_t parent)
{
	// should do lookup here...
	if (table.find(hash) != table.end())
	{
		//return -1; // TODO: find correct id and return
		assert(false);
	}

	//TODO openlocation
	elements.push_back(dataStructure(val, g, h,revH, parent, 0, kFFOpen));
	

	if (parent == kTBDFFNoNode)
		elements.back().parentID = elements.size()-1;
	table[hash] = elements.size()-1; // hashing to element list location

	openInfo info;
	info.g = g; info.h = h; info.revH = revH;

	open[info].insert(elements.size() - 1);
	//TODO recompute best pair

	openSize++;

	return elements.size()-1;
}

/**
 * Add object into closed list.
 */
template<typename state, typename CmpKey0, typename CmpKey1,   class dataStructure>
uint64_t BDFFOpenClosed<state, CmpKey0, CmpKey1,   dataStructure>::AddClosedNode(state &val, uint64_t hash, double g, double h,double revH, uint64_t parent)
{
	// should do lookup here...
	assert(table.find(hash) == table.end());
	elements.push_back(dataStructure(val, g, h,revH, parent, 0, kClosed));
	if (parent == kTBDFFNoNode)
		elements.back().parentID = elements.size()-1;
	table[hash] = elements.size()-1; // hashing to element list location
	return elements.size()-1;
}

/**
 * Indicate that the key for a particular object has changed.
 */



template<typename state, typename CmpKey0, typename CmpKey1, class dataStructure>
void BDFFOpenClosed<state, CmpKey0, CmpKey1, dataStructure>::ChangeKey(uint64_t objKey, double ng, double nh, double nrevH)
{
	openInfo oldInfo, newInfo;
	auto& item = elements[objKey];

	oldInfo.g = item.g;
	oldInfo.h = item.h;
	oldInfo.revH = item.revH;

	newInfo.g = ng;
	item.g = ng;
	if (nh == DBL_MIN)
		newInfo.h = item.h;
	else
	{
		newInfo.h = nh;
		item.h = nh;
	}

	if (nrevH == DBL_MIN)
		newInfo.revH = item.revH;
	else
	{
		newInfo.revH = nrevH;
		item.revH = nrevH;
	}

	open[oldInfo].erase(open[oldInfo].find(objKey));
	if (open[oldInfo].size() == 0)
		open.erase(open.find(oldInfo));


	open[newInfo].insert(objKey);

		//TODO recompute best pair

}

template<typename state, typename CmpKey0, typename CmpKey1, class dataStructure>
void BDFFOpenClosed<state, CmpKey0, CmpKey1, dataStructure>::Remove(uint64_t objKey)
{
	openInfo oldInfo;
	auto& item = elements[objKey];

	oldInfo.g = item.g;
	oldInfo.h = item.h;
	oldInfo.revH = item.revH;

	open[oldInfo].erase(open[oldInfo].find(objKey));
	if (open[oldInfo].size() == 0)
		open.erase(open.find(oldInfo));

	openSize--;
	//TODO recompute best pair
}


/**
 * Returns location of object as well as object key.
 */

template<typename state, typename CmpKey0, typename CmpKey1,   class dataStructure>
FFstateLocation BDFFOpenClosed<state, CmpKey0, CmpKey1,   dataStructure>::Lookup(uint64_t hashKey, uint64_t &objKey) const
{
	typename IndexTable::const_iterator it;
	it = table.find(hashKey);
	if (it != table.end())
	{
		objKey = (*it).second;
		return elements[objKey].where;
	}
	return kFFUnseen;
}




/**
 * Move the best item to the closed list and return key.
 */

template<typename state, typename CmpKey0, typename CmpKey1, class dataStructure>
void BDFFOpenClosed<state, CmpKey0, CmpKey1, dataStructure>::Close(uint64_t objKey)
{
	auto& item = elements[objKey];
	item.where = kFFClosed;

	//cout << "ok1\n";
	
	openInfo oldInfo;
	oldInfo.g = item.g;
	oldInfo.h = item.h;
	oldInfo.revH = item.revH;

	//if(open[oldInfo].find(objKey) == open[oldInfo].end())
//		cout << "unexpected\n";
	open[oldInfo].erase(open[oldInfo].find(objKey));
	//cout << "close erase1\n";
	if (open[oldInfo].size() == 0)
		open.erase(open.find(oldInfo));
	//cout << "close erase2\n";
	openSize--;
	//TODO recompute best pair
}





#endif
