/**
 * @author Alan Perez-Rathke 
 *
 * @date March 30, 2010 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#ifndef PkLocalSiblingGroupReconstructor_h
#define PkLocalSiblingGroupReconstructor_h

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkLociArray.h"
#include "PkRunnable.h"

/**
* The purpose of this class is to read an incomplete group from a command buffer and
* populate the group until the highest ordered individual is added to it.  If adding
* any single individual changes the the allele map in any way, the new group is 
* copied into the command buffer for further processing. Once the last individual is
* added, a new task can be generated to check to see if it is a subset
*
* Things to think about:
* 1.) The underlying command buffer should have two implementations:
*     - a single buffer that may become a bottleneck for all threads
*     - a 'work stealing' implementation which decentralizes the buffer
*     - both implementations can then be compared to each other for performance
* 2.) The subset culling phase may have to come immediately after a set is generated
*     (after adding highest ordered individual) to reduce peak memory usage.  This is
*     theoretically okay as another task can just steal the work of processing a group
*     already in the queue.
* 3.) The problem of work stealing: Termination.  The simplest way is to poll all
*     threads for work, if all threads have no work to give, then terminate.  However,
*     work is dynamically generated - so it's possible that work may exist at a later
*     time.  There is also overhead of having to lock the command buffer each time
*     you want to pop a new group.  Another implementation is for each thread
*     to be seeded with it's own command buffer containing a unique singleton
*     group and just run until termination.
*/
class PkLocalSiblingGroupReconstructor : public PkRunnable
{
public:

	// Default constructor so that we can work with containers easier
	PkLocalSiblingGroupReconstructor();

	// Constructor which initializes lociIndices and the command buffer to work with
	PkLocalSiblingGroupReconstructor(
		  const PkLociArray( PkInt )& lociIndices
		, class PkSiblingGroupCommandBufferInterface& commandBuffer
		);

	// Initializes lociIndices and the command buffer to work with
	void init(
		  const PkLociArray( PkInt )& lociIndices
		, class PkSiblingGroupCommandBufferInterface& commandBuffer
		);

	// Reads commands from the command buffer until no work is available.  A command
	// should just be an incomplete sibling group. Can generate new commands by 
	// pushing groups that have different allele mappings.  The popped commands
	// (groups) are completed by adding higher order individuals and then subsets are culled
	virtual PkBool doWork( const PkUInt threadId );

	// @return - const collection of bitsets to output for this job
	inline const PkBitSetArray& getOutputEncodedSiblingSets() const
	{
		return m_EncodedSiblingSets;
	}

private:

	// @return - collection of bitsets to output for this job
	inline PkBitSetArray& getOutputEncodedSiblingSets()
	{
		return m_EncodedSiblingSets;
	}

	// @return - const collection of loci indices assigned to this job
	inline const PkLociArray( PkInt )& getLociIndices() const 
	{ 
		return m_LociIndices; 
	}

	// @return - reference to our internal command buffer interface 
	inline class PkSiblingGroupCommandBufferInterface& getCommandBuffer()
	{
		PkAssert( NULL != m_pCommandBuffer );
		return *m_pCommandBuffer;
	}

	// A command buffer passed in externally, but deallocated internally
	class PkSiblingGroupCommandBufferInterface* m_pCommandBuffer;

	// The collection of loci indices which we locally reconstruct sibling relationships from
	PkLociArray( PkInt ) m_LociIndices;

	// The sibling groups that are finished by this constructor encoded as bitsets
	PkBitSetArray m_EncodedSiblingSets;
};

#endif // PkLocalSiblingGroupReconstructor_h
