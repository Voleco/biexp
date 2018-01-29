#include <ext/hash_set>
#include <map>
#include <vector>
#include <math.h>
#include "MyHash.h"
#include "Map2DEnvironment.h"
#include "MultiAgentEnvironment.h"


#ifndef MINIMAX_H
#define MINIMAX_H


/*!
	two players minimax implementation
*/
template<class state, class action, class environment>
class Minimax {

	public:

	// type definitions
	typedef typename MultiAgentEnvironment<state,action>::MAState CRState;

	// SearchCache for the path from the root to the current node
	struct CRStateHash {
		size_t operator()( CRState *s ) const {
			return CRHash<state>( *s );
		}
	};
	struct CRStateEqual {
		bool operator()( const CRState *c1, const CRState *c2 ) const {
			return ( *c1 == *c2 );
		}
	};
	typedef __gnu_cxx::hash_set<CRState*, CRStateHash, CRStateEqual> SearchCache;

	// transposition tables
	class TPEntry {
		public:
		TPEntry( CRState &_pos, double _value = 0. ): pos(_pos), value(_value) {};
		CRState pos; // the node that has to be stored
		CRState next_pos; // next position in the optimal solution path
		bool no_next_pos;
		double value;
		double alpha;
		double beta;
	};
	struct TPEntryHash {
		size_t operator()( const TPEntry &e ) const {
			return CRHash<state>( e.pos );
		}
	};
	struct TPEntryEqual {
		bool operator()( const TPEntry &e1, const TPEntry &e2 ) const {
			return( e1.pos == e2.pos );
		}
	};

	typedef __gnu_cxx::hash_set<TPEntry, TPEntryHash, TPEntryEqual> TranspositionTable;
	typedef std::map<double, TranspositionTable> GTTables;
	typedef std::pair<double, TranspositionTable> GTTables_Pair;



	// constructor
	Minimax( environment *_env, bool _canPause ):
		max_depth_reached(0), env(_env), canPause(_canPause)
		{};

	// functions
//	double iterative_minimax( CRState pos, std::vector<CRState> &path, bool minFirst = true );

	double minimax( CRState pos, std::vector<CRState> &path, bool minFirst = true, int max_depth = 100 );

	// gets updated after each call to minimax (or minimax_update)
	int max_depth_reached;

	unsigned int nodesExpanded, nodesTouched;

	protected:

	double minimax_help( CRState pos, bool minFirst, int depth, double alpha, double beta );

	// evaluation function for the leafs
	double EvalFunc( CRState &pos, bool minsTurn = true );
	// consistent heuristic function that provides a lower bound on the solution
	double MinHCost( CRState &pos, bool minsTurn = true );
	double MinGCost( CRState &pos1, CRState &pos2 );
	bool GoalTest(const  CRState &pos );
	double TerminalCost( CRState &pos );


	// variables
	environment *env;
	bool canPause;
	// caching the path from the root to the current node (during the computation)
//	SearchCache min_scache, max_scache;

	// transposition table
	GTTables min_gttables, max_gttables;
};






/*------------------------------------------------------------------------------
| MiniMax Implementation
------------------------------------------------------------------------------*/

// can be found in Minimax.cpp
template<>
double Minimax<xyLoc,tDirection,MapEnvironment>::MinHCost( CRState &pos, bool minsTurn );


/*
template<class state, class action, class environment>
double Minimax<state,action,environment>::iterative_minimax( CRState pos, std::vector<CRState> &path, bool minFirst ) {

	double result = 0.;
	int bound = 0;
	while( true ) {
		result = minimax_update( pos, path, minFirst, bound );
		if( result < (double)bound ) break;
		bound++;
	}
	return result;
}
*/


template<class state,class action,class environment>
double Minimax<state,action,environment>::minimax( CRState pos, std::vector<CRState> &path, bool minFirst, int max_depth ) {

	double result;

	nodesTouched = 0;
	nodesExpanded = 0;

	// during the run of minimax_help, the minimal depth is stored in max_depth_reached
	// and there we substract it from max_depth later to get the actual depth we reached
	max_depth_reached = max_depth;

	// just to make sure
	/*
	min_scache.clear();
	max_scache.clear();
	*/

	// just to make sure
	min_gttables.clear();
	max_gttables.clear();

	result = minimax_help( pos, minFirst, max_depth, -DBL_MAX, DBL_MAX );

	// the temporary search cache should be empty
	/*
	assert( min_scache.empty() );
	assert( max_scache.empty() );
	*/

	// extract the path from the cache
	path.clear(); path.push_back( pos );
	TPEntry mytpentry( pos );
	typename GTTables::iterator gttit;
	typename TranspositionTable::iterator tit;
	GTTables* current_gttables = minFirst?&min_gttables:&max_gttables;
	GTTables* next_gttables    = minFirst?&max_gttables:&min_gttables;
	GTTables* temp;
	int depth = max_depth;
	// this loop terminates because we only computed down to a certain depth
	while( true ) {
		gttit = current_gttables->find( (double)depth );
		if( gttit != current_gttables->end() ) {
			tit = gttit->second.find( mytpentry );
			if( tit != gttit->second.end() ) {
				if( !tit->no_next_pos ) {
					path.push_back( tit->next_pos );
					mytpentry.pos = tit->next_pos;
					depth--;
					temp = current_gttables;
					current_gttables = next_gttables;
					next_gttables = temp;
				} else
					break;
			} else
				break;
		} else
			break;
	}
	

	// cleanup
	min_gttables.clear();
	max_gttables.clear();

	max_depth_reached = max_depth - max_depth_reached;

	return result;
}


// alpha = minimum score that the maximum player is assured of
// beta  = maximum score that the minimum player is assured of
template<class state,class action,class environment>
double Minimax<state,action,environment>::minimax_help( CRState pos, bool minFirst, int depth, double alpha, double beta ) {

	/* verbose
	fprintf( stdout, "considered position (%u,%u) (%u,%u) at depth %d for player ", pos[0].x, pos[0].y, pos[1].x, pos[1].y, depth );
	fprintf( stdout, "%s ", (minFirst?"min":"max") );
	fprintf( stdout, "and alpha=%g beta=%g\n", alpha, beta );
	*/
	nodesTouched++;

	// store the minimal depth reached
	if( depth < max_depth_reached )
		max_depth_reached = depth;

	// if we reached a leaf node
	if( GoalTest( pos ) ) {
//		fprintf( stdout, "terminal => %f\n", TerminalCost( pos ) );
		return TerminalCost( pos );
	}

	// determine whether we yet encountered this position on the
	// path from the root to this node
	// technically we need this to make the game tree finite
	/*
	if( minFirst ) {
		if( min_scache.find( &pos ) != min_scache.end() ) {
//			fprintf( stdout, "doubling \\infty\n" );
			return DBL_MAX;
		}
	} else {
		if( max_scache.find( &pos ) != max_scache.end() ) {
//			fprintf( stdout, "doubling \\infty\n" );
			return DBL_MAX;
		}
	}
	*/

	// in case of a consistent heuristic we can also prune
	// because we are then guaranteed not to reach the terminals anymore
	double hcost = MinHCost( pos, minFirst );
	if( beta <= hcost ) {
//		fprintf( stdout, "h-prune \\infty\n" );
		return hcost;
	}

	// if we reached the bottom of the computation tree
	if( depth <= 0 ) {
//		fprintf( stdout, "depth prune %f\n", MinHCost( pos, minFirst ) );
		return hcost;
//		return EvalFunc( pos, minFirst );
	}

	// transposition table lookup
	TPEntry mytpentry( pos );
	typename GTTables::iterator gttit;
	std::pair<typename GTTables::iterator, bool> insert_return;
	typename TranspositionTable::iterator tit;
	bool old_transposition_available = false;
	bool old_value_upper_bound = true; // true, false = old value is lower bound

	GTTables *current_gttables = minFirst?&min_gttables:&max_gttables;
	gttit = current_gttables->find( (double)depth );
	if( gttit != current_gttables->end() ) {
		// if a transposition table for this gCost was found
		tit = gttit->second.find( mytpentry );
		// check if the position is in the hash table
		if( tit != gttit->second.end() ) {

			old_transposition_available = true;

			// check for usability of the hash table entry
			if( tit->value <= tit->alpha ) {
				old_value_upper_bound = true;
				if( tit->value < beta )
					beta = tit->value;
			}
			if( tit->beta <= tit->value ) {
				old_value_upper_bound = false;
				if( alpha < tit->value )
					alpha = tit->value;
			}
			if( beta <= alpha ) {
				return tit->value;
			}
			if( tit->alpha < tit->value && tit->value < tit->beta ) {
				return tit->value;
			}

		}
	} else {
		// else create a new transposition table for this gCost
		GTTables_Pair mypair;
		mypair.first = (double)depth;
		insert_return = current_gttables->insert( mypair );
		// sanity check: do we still correctly create the transposition tables?
		assert( insert_return.second );
		gttit = insert_return.first;
	}


	// push the current node on the path cache
	/*
	if( minFirst )
		min_scache.insert( &pos );
	else
		max_scache.insert( &pos );
	*/

	// variable declarations
	std::vector<state> next_mystates;
	CRState child, next_pos;
	bool no_next_pos = true;
	double child_value, pathcost;
	double result=minFirst?DBL_MAX:-DBL_MAX;
	double temp_alphabeta=minFirst?beta:alpha;

	nodesExpanded++;

	// generate the next moves/children
	int myid = minFirst?1:0;
	// compute recursively for all the children
	env->GetSuccessors( pos[myid], next_mystates );
	if( canPause ) next_mystates.push_back( pos[myid] );

	// for all possible next moves
	while( !next_mystates.empty() ) {

		child = pos;
		child[myid] = next_mystates.back();
		next_mystates.pop_back();

		pathcost = MinGCost( pos, child );

		if( minFirst ) {
		// Min Node
			child_value = pathcost +
				minimax_help( child, !minFirst, depth - 1, alpha - pathcost, temp_alphabeta - pathcost );
			// min player updates his score
			if( child_value < result ) {
				result = child_value;
				next_pos = child;
				no_next_pos = false;
			}
			if( child_value < temp_alphabeta )
				temp_alphabeta = child_value;

			// beta cutoff
			if( temp_alphabeta <= alpha ) break;

		} else {
		// Max Node
			child_value = pathcost +
				minimax_help( child, !minFirst, depth - 1, temp_alphabeta - pathcost, beta - pathcost );
			// max player updates his score
			if( child_value > result ) {
				result = child_value;
				next_pos = child;
				no_next_pos = false;
			}
			if( child_value > temp_alphabeta )
				temp_alphabeta = child_value;

			// alpha cutoff
			if( beta <= temp_alphabeta ) break;
		}

	}

	// cleanup
	// update the list of moves from the root to this nodes parent
	/*
	if( minFirst )
		min_scache.erase( &pos );
	else
		max_scache.erase( &pos );
	*/

	// combine the current value with the old hash entry
	if( old_transposition_available ) {
		if( old_value_upper_bound ) {
			// sanity check
			assert( result <= tit->value );
			if( result == tit->value )
				// in this case, result is the correct value, thus we increase the window slightly
				// to make the ttable lookup correct
				beta = result + 1.;
		} else {
			// sanity check
			assert( tit->value <= result );
			if( result == tit->value )
				alpha = result - 1.;
		}
		// the old entry is no longer needed
		gttit->second.erase( tit );
	}
	mytpentry.next_pos = next_pos;
	mytpentry.no_next_pos = no_next_pos;
	mytpentry.value = result;
	mytpentry.alpha = alpha;
	mytpentry.beta  = beta;
	gttit->second.insert( mytpentry );

//	fprintf( stdout, "%f\n", result ); //minFirst?beta:alpha );
	return result; //( (minFirst)?beta:alpha );
}


template<class state, class action, class environment>
double Minimax<state,action,environment>::EvalFunc( CRState &pos, bool minsTurn ) {
	return 0.;
//	return MinHCost( pos, minsTurn );
}

// \see Minimax.cpp for an implementation for state=xyLoc
template<class state, class action, class environment>
double Minimax<state,action,environment>::MinHCost( CRState &pos, bool minsTurn ) {
	if( GoalTest( pos ) ) return 0.;

	if( canPause )
		return ( 2. * env->HCost( pos[1], pos[0] ) - (minsTurn?MinGCost(pos,pos):0.) );
	else
		// distance from cop to the robber
		return env->HCost( pos[1], pos[0] );
}


template<class state, class action, class environment>
double Minimax<state,action,environment>::MinGCost( CRState&, CRState& ) {
	return 1.;
}

template<class state, class action, class environment>
bool Minimax<state,action,environment>::GoalTest( CRState &pos ) {
	return( pos[0] == pos[1] );
}

template<class state, class action, class environment>
double Minimax<state,action,environment>::TerminalCost( CRState& ) {
	return 0.;
}


#endif
