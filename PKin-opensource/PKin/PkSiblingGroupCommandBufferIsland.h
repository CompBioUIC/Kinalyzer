/**
 * @author Alan Perez-Rathke 
 *
 * @date March 31, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkSiblingGroupCommandBufferIsland_h
#define PkSiblingGroupCommandBufferIsland_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkSiblingGroupCommandBufferInterface.h"
#include "PkSiblingGroup.h"
#include "PkSiblingGroupUnorderedSet.h"

// A simple multi-threaded work queue, can act as a single, centralized command buffer
class PkSiblingGroupCommandBufferIsland : public PkSiblingGroupCommandBufferInterface
{
public:

	// Obtains a command from the command buffer
	// @return true if command exists, false otherwise
	virtual PkBool getNextCommand( PkSiblingGroup& outCommand )
	{	
		// Determine if commands are available
		const PkBool bCommandExists = !this->getInternalCommandBuffer().empty();

		// Pop command if queue is not empty		
		if ( bCommandExists )
		{
			// LIFO command queue
			outCommand = *(this->getInternalCommandBuffer().back());
			this->getInternalCommandBuffer().pop_back();
		}

		// Return whether or not a command exists
		return bCommandExists;
	}

	// Pushes a command onto the command buffer
	virtual void pushCommand( PkSiblingGroup& inCommand )
	{
		// Cache the hash for this sibling group
		PkSiblingGroupHasher::computeAndCacheHashFor( inCommand );
		
		// Register the command to allocate internal hash and check if it exists already
		PkBool bAlreadyExists = false;
		const PkSiblingGroup& groupToPush = this->registerCommand( inCommand, bAlreadyExists );

		// Only push the command for further expansion if it's not already in the hash
		if ( !bAlreadyExists )
		{
			// Add sibling group to command queue
			this->getInternalCommandBuffer().push_back( &groupToPush );
		}
	}

	// Meant to be called when a command has finished processing
	// In this case, serves the purpose of merging any added individuals
	virtual void finalizeCommand( const PkSiblingGroup& inCommand )
	{
		PkBool bAlreadyExists = false;
		this->registerCommand( inCommand, bAlreadyExists );
		// Note: we're implementing work stealing in terms of an island command buffer
		// and therefore we can steal a command which doesn't exist in our internal
		// hash - this is okay.  TODO: figure out a clean way to enable this assert
		// for a true island cluster.
		// PkAssert( bAlreadyExists );
	}

	// Per-job initialization can be overloaded here
	virtual void localInit( const PkLociArray( PkInt )& lociIndices )
	{
		// Store one past the last inidividual index assigned to this command buffer
		const PkUInt onePastLastIndex = this->getIdxStartIndividual() + this->getNumIndividualsToAssign();

		// Cache global population size and reference to population
		const PkArray(PkIndividual)& population = PkGlobalData::getPopulation(); 
		const PkUInt populationSize = population.size();
		PkAssert( populationSize > 0 );

		// Assert on bounds checks
		PkAssert( this->getIdxStartIndividual() >= 0 );
		PkAssert( onePastLastIndex <= populationSize );

		// Cache reference to internal command buffer and size it to fit at least population size number of groups
		PkArray( const PkSiblingGroup* )& internalCommandBuffer = this->getInternalCommandBuffer();
		PkAssert( internalCommandBuffer.size() == 0 && "Expecting empty command buffer!" );
	
		// Optimization: pre-size sibling groups command buffer based on (n choose 2) initial possible sets
		internalCommandBuffer.reserve( this->getNumIndividualsToAssign() * ( this->getNumIndividualsToAssign() - 1 ) / 2 );
	
		// Seed the command buffer with singleton sets
		for ( PkUInt i=this->getIdxStartIndividual(); i<onePastLastIndex; ++i )
		{
			// Initialize a singleton group
			PkSiblingGroup singletonGroup(
				  population[ i ]
				, i
				, lociIndices
				, populationSize
				);

			// Push the command, this will merge any equivalent groups
			this->pushCommand( singletonGroup );
		}
	}

	// Sets range of population to create commands for during local initialization
	inline void setLocalInitPopulationRange(
			  const PkUInt idxStartIndividual
			, const PkUInt numIndividualsToAssign
			)
	{
		m_idxStartIndividual = idxStartIndividual;
		m_NumIndividualsToAssign = numIndividualsToAssign;
	}

private:

	// If command exists, will merge individuals of parameter and hashed command
	// If command doesn't exist, will allocate a hash entry for it
	// @return - const reference to command in hash table
	const PkSiblingGroup& registerCommand( const PkSiblingGroup& inCommand, PkBool& bOutAlreadyExists )
	{
		// The sibling group to output, stored within hash table
		const PkSiblingGroup* pOutGroup = NULL;

		// Avoid pushing equivalent groups
		PkSiblingGroupUnorderedSet::iterator itr = m_InternalSiblingGroupHash.find( inCommand );
		bOutAlreadyExists = itr != m_InternalSiblingGroupHash.end();
		
		// If group already exists, merge the individuals
		if ( bOutAlreadyExists )
		{
			pOutGroup = &(*itr);
			// HACK, we're going to change the value of data stored within a set!!
			// This is okay because assigned individuals are not used for hashing
			// or equality testing.
			const_cast< PkSiblingGroup* >( pOutGroup )->forceAssignIndividualsFrom( inCommand );
		}
		// Else, group wasn't found, insert into hash table
		else
		{
			// Insert sibling group command into hash table
			const std::pair< PkSiblingGroupUnorderedSet::iterator, bool > result = m_InternalSiblingGroupHash.insert( inCommand );
			PkAssert( result.first != m_InternalSiblingGroupHash.end() );
			PkAssert( result.second );
			pOutGroup = &(*result.first);
		}

		// Return reference to group within hash table
		PkAssert( pOutGroup );
		return (*pOutGroup);
	}

	// @return reference to internal command buffer
	inline PkArray( const PkSiblingGroup* )& getInternalCommandBuffer()
	{
		return m_InternalCommandBuffer;
	}

	// @return index of the individual to start generating commands for during local initialization
	inline PkUInt getIdxStartIndividual() const
	{
		return m_idxStartIndividual;
	}

	// @return total number of indivudals to generate commands for during local initialization
	inline PkUInt getNumIndividualsToAssign() const
	{
		return m_NumIndividualsToAssign;
	}

	// Our internal command buffer, readable and writable only through push/pop interface
	PkArray( const PkSiblingGroup* ) m_InternalCommandBuffer;

	// An internal hash to control duplicate commands from being pushed
	PkSiblingGroupUnorderedSet m_InternalSiblingGroupHash;

	// The index of the individual to start generating commands for during local initialization
	PkUInt m_idxStartIndividual;
	
	// The total number of indivudals to generate commands for during local initialization
	PkUInt m_NumIndividualsToAssign;
};

#endif // PkSiblingGroupCommandBufferIsland_h
