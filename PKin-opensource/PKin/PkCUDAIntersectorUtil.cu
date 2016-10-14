/**
 * @author Marco Maggioni 
 * @author Alan Perez-Rathke 
 *
 * @date April 7, 2011 
 *
 * Department of Computer Science
 * University of Illinois at Chicago 
 */

#include "PkBuild.h"
#include "PkTypes.h"
#include "PkAssert.h"
#include "PkMiscUtil.h"
#include "PkGlobalData.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cutil.h>

// Switch which controls whether to use GPU to convert to CPU memory formats
#define Pk_ENABLE_CUDA_MAP_GPU_TO_CPU_INTERSECTION 1

// The number of CUDA threads in block x and y dimensions
#define Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X 16
#define Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y 16
#define Pk_CUDA_NUM_TOTAL_INTERSECTION_THREADS_IN_BLOCK 256
#if (Pk_CUDA_NUM_TOTAL_INTERSECTION_THREADS_IN_BLOCK != (Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y))
	#error Number of total intersection threads must be the product of number of threads in x and y
#endif

// CUDA assert
#if Pk_ENABLE_ASSERT
	#define PkCUDAAssert( X ) \
		if ( !(X) ) { printf( "CUDA Thread %d:%d failed assert at %s:%d!", blockIdx.x, threadIdx.x, __FILE__, __LINE__ ); return; }
#else
	#define PkCUDAAssert( X ) // do nothing
#endif

// The underlying block primitive for representing a bitset
typedef PkPooledRawBitSetArray::block_type PkCUDABlockType;
// A basic byte type
typedef PkPooledRawBitSetArray::byte_type  PkCUDAByteType;
// A size type for memory allocations, etc.
typedef PkPooledRawBitSetArray::size_type  PkCUDASizeType;
// A size for counting bits
typedef unsigned short                     PkCUDABitCountType;

// The number of bits that a CUDA block type can store
#define Pk_CUDA_BITS_PER_BLOCK PkPooledRawBitSetArray::bits_per_block

namespace Pk
{

///**********************************************************

// @return - CPU pointer to data from GPU, client is responsbile for freeing allocated memory
template < typename tData >
tData* CUDACopyFromGPUToCPU
(
  const tData* gpu_data
, const size_t num_data_values
)
{
	// Copy bit counts back to CPU
	tData * cpu_data = (tData*) malloc( sizeof( tData ) * num_data_values );
	PkAssert( NULL != cpu_data );
	cudaMemcpy(
	  cpu_data
	, gpu_data
	, sizeof( tData ) * num_data_values
	, cudaMemcpyDeviceToHost
	);

	return cpu_data;
}

// @return - CPU pointer to data from GPU, release CPU memory, client is responsbile for freeing CPU allocated memory
template < typename tData >
tData* CUDACopyFromGPUToCPUAndReleaseGPU
(
  const tData*& gpu_data
, const size_t num_data_values
)
{
	tData* cpu_data = CUDACopyFromGPUToCPU<tData>( gpu_data, num_data_values );
	PkAssert( NULL != cpu_data );
	cudaFree( (void*)gpu_data );
	gpu_data = (const tData*)NULL;
	return cpu_data;
}

///**********************************************************

// GPU kernel for calculating the intersection
__global__ void CUDASwizzledSiblingIntersectionKernel(
  PkCUDABlockType * gpu_sib_sets_matrix_a
, PkCUDABlockType * gpu_sib_sets_matrix_b
, PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
)
{
	// Get position in the overall computation matrix
	const unsigned int locus_a_offset = blockIdx.y * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y + threadIdx.y;
	const unsigned int locus_b_offset = blockIdx.x * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X + threadIdx.x;

	if ( ( locus_a_offset < num_sib_sets_at_locus_a ) && ( locus_b_offset < num_sib_sets_at_locus_b ) )
	{
		// Compute the intersection
		for ( int itr_block=0; itr_block<num_bit_blocks_per_sib_set; ++itr_block )
		{
			gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * itr_block) + (num_sib_sets_at_locus_b * locus_a_offset) + locus_b_offset ] = 
				  gpu_sib_sets_matrix_a[ ( num_sib_sets_at_locus_a * itr_block ) + locus_a_offset ] 
				& gpu_sib_sets_matrix_b[ ( num_sib_sets_at_locus_b * itr_block ) + locus_b_offset ];
		}
	}
}

///**********************************************************

// GPU kernel for counting the number of bits in each intersection bit set
__global__ void CUDASwizzledCountBitsKernel(
  PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
)
{
	// Store byte to bit count mapping
	// @todo - move to shared memory!
	const PkCUDAByteType num_bits_in_byte_table[] = Pk_INIT_NUM_BITS_IN_BYTE_TABLE;

	// Get position in the overall computation matrix
	const unsigned int locus_a_offset = blockIdx.y * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y + threadIdx.y;
	const unsigned int locus_b_offset = blockIdx.x * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X + threadIdx.x;

	// Make sure we are in bounds
	if ( ( locus_a_offset < num_sib_sets_at_locus_a ) && ( locus_b_offset < num_sib_sets_at_locus_b ) )
	{
		// Initialize bit count to 0
		const size_t idx_bit_set = (num_sib_sets_at_locus_b * locus_a_offset) + locus_b_offset;
		gpu_sib_sets_matrix_intersection_bit_counts[ idx_bit_set ] = 0;

		// Count number of bits
		for ( int itr_block=0; itr_block<num_bit_blocks_per_sib_set; ++itr_block )
		{
			// Cast block to a byte buffer
			const PkCUDAByteType* byte_buffer = 
				(PkCUDAByteType*)(&(gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * itr_block) + idx_bit_set ]));
			
			// Iterate over bytes and count bits
			for ( int itr_byte=0; itr_byte<sizeof(PkCUDABlockType); ++itr_byte )
			{
				gpu_sib_sets_matrix_intersection_bit_counts[ idx_bit_set ] += num_bits_in_byte_table[ byte_buffer[ itr_byte ] ];
			}
		}
	}
}

// @return - Pointer to GPU array containing bit counts, client is responsible for freeing and for transferring to CPU
PkCUDABitCountType * CUDACountIntersectionBits(
  const PkCUDABlockType* gpu_sib_sets_matrix_intersection
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
, const dim3 dimComputingGrid
, const dim3 dimComputingBlock
)
{
	// Determine number of values necessary for storing bit counts and hash values
	const size_t num_data_values = num_sib_sets_at_locus_a * num_sib_sets_at_locus_b;
	
	// Allocate bit counts array
	PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts = NULL;
	cudaMalloc((void**)&gpu_sib_sets_matrix_intersection_bit_counts, num_data_values*sizeof(PkCUDABitCountType) );		
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_bit_counts );

	// Wait for intersections to finish before executing bit counts kernel
	PkVerify( cudaSuccess == cudaThreadSynchronize() );	

	// Count the number of bits in each sibling set
	CUDASwizzledCountBitsKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_matrix_intersection_bit_counts
		, gpu_sib_sets_matrix_intersection
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		);

	return gpu_sib_sets_matrix_intersection_bit_counts;
}

///**********************************************************

// From boost::hash

template <class T>
__device__ std::size_t CUDAHashValueUnsigned(T val)
{
     const int size_t_bits = std::numeric_limits<std::size_t>::digits;
     // ceiling(std::numeric_limits<T>::digits / size_t_bits) - 1
     const int length = (std::numeric_limits<T>::digits - 1)
         / size_t_bits;

     std::size_t seed = 0;

     // Hopefully, this loop can be unrolled.
     for(unsigned int i = length * size_t_bits; i > 0; i -= size_t_bits)
     {
         seed ^= (std::size_t) (val >> i) + (seed<<6) + (seed>>2);
     }
     seed ^= (std::size_t) val + (seed<<6) + (seed>>2);

     return seed;
}

template <class T>
__device__ void CUDAHashCombineUnsigned(std::size_t& seed, T const& v)
{
    seed ^= CUDAHashValueUnsigned(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <class It>
__device__ void CUDAHashRangeUnsigned(std::size_t& seed, It first, It last)
{
	for(; first != last; ++first)
    {
		CUDAHashCombineUnsigned(seed, *first);
	}
}

// GPU kernel for mapping each intersection bit set to a hash value
__global__ void CUDASwizzledHashBlocksKernel(
  std::size_t * gpu_sib_sets_matrix_intersection_hashes
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
)
{
	// Get position in the overall computation matrix
	const unsigned int locus_a_offset = blockIdx.y * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y + threadIdx.y;
	const unsigned int locus_b_offset = blockIdx.x * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X + threadIdx.x;

	// Make sure we are in bounds
	if ( ( locus_a_offset < num_sib_sets_at_locus_a ) && ( locus_b_offset < num_sib_sets_at_locus_b ) )
	{
		// Initialize bit count to 0
		const size_t idx_bit_set = (num_sib_sets_at_locus_b * locus_a_offset) + locus_b_offset;

		// Hashing routine below is based on Boost's hash library

		// Initialize our hashed bit set seed to 0
		std::size_t hashed_bit_set = 0;

		// Hash combine each block
		for ( int itr_block=0; itr_block<num_bit_blocks_per_sib_set; ++itr_block )
		{
			// Get block value to hash
			const PkCUDABlockType block =
				gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * itr_block) + idx_bit_set ];
			
			CUDAHashCombineUnsigned( hashed_bit_set, block );
		}

		// Hash combine with the bit count
		CUDAHashCombineUnsigned( hashed_bit_set, gpu_sib_sets_matrix_intersection_bit_counts[ idx_bit_set ] );

		// Store resulting bit set hash
		gpu_sib_sets_matrix_intersection_hashes[ idx_bit_set ] = hashed_bit_set;
	}
}

// @return - Pointer to GPU array containing bit set hashes, client is responsible for freeing and for transferring to CPU
std::size_t * CUDAComputeIntersectionHashes(
  const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
, const dim3 dimComputingGrid
, const dim3 dimComputingBlock
)
{
	// Determine number of values necessary for storing bit counts and hash values
	const size_t num_data_values = num_sib_sets_at_locus_a * num_sib_sets_at_locus_b;
	
	// Allocate bit counts array
	std::size_t * gpu_sib_sets_matrix_intersection_hashes = NULL;
	cudaMalloc((void**)&gpu_sib_sets_matrix_intersection_hashes, num_data_values*sizeof(std::size_t) );		
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_hashes );

	// Count the number of bits in each sibling set
	CUDASwizzledHashBlocksKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_matrix_intersection_hashes
		, gpu_sib_sets_matrix_intersection
		, gpu_sib_sets_matrix_intersection_bit_counts
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		);

	return gpu_sib_sets_matrix_intersection_hashes;
}

///**********************************************************

// GPU kernel for transposing swizzled bits
// Note: Assumes cudaMemset was used to initialize the transpose matrix to all 0's
__global__ void CUDASwizzledTransposeKernel(
  PkCUDABlockType * gpu_sib_sets_matrix_intersection_transpose
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
, const size_t num_bits_per_sib_set
)
{
	// Get position in the overall computation matrix
	const size_t locus_a_offset = blockIdx.y * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y + threadIdx.y;
	const size_t locus_b_offset = blockIdx.x * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X + threadIdx.x;

	// Make sure we are in bounds
	if ( ( locus_a_offset < num_sib_sets_at_locus_a ) && ( locus_b_offset < num_sib_sets_at_locus_b ) )
	{
		// Store bit set offset
		const size_t idx_bit_set = (num_sib_sets_at_locus_b * locus_a_offset) + locus_b_offset;

		// Early out if bit set is empty
		if ( 0 == gpu_sib_sets_matrix_intersection_bit_counts[ idx_bit_set ] )
		{
			return;
		}

		// Transpose each bit set
		for ( size_t itr_block=0; itr_block<num_bit_blocks_per_sib_set; ++itr_block )
		{
			// Transpose block
			const PkCUDABlockType block = 
				gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * itr_block) + idx_bit_set ];
			
			// Early out if block is empty
			// @todo: branching may not be warranted
			if ( 0 == block )
			{
				continue;
			}

			// itr_bit is relative to this bit offset:
			const size_t bit_offset = Pk_CUDA_BITS_PER_BLOCK * itr_block;

			// Transpose each bit
			for ( size_t itr_bit=0; (itr_bit < Pk_CUDA_BITS_PER_BLOCK) && ((bit_offset+itr_bit)<num_bits_per_sib_set); ++itr_bit )
			{
				if ( block & (1 << itr_bit) )
				{
					// Note: locus_a_offset is the "row", locus_b_offset is the "column", and itr_block is the "depth"
					// To transpose, we want each vertical plane to become a horizontal plane.
					// This can be accomplished by simply swapping row and depth
					// Note: (locus_a_offset/Pk_CUDA_BITS_PER_BLOCK) is the new block offset (i.e. - "itr_block")
					// -and- (locus_a_offset%Pk_CUDA_BITS_PER_BLOCK) is the bit offset within that block
					// -and- (bit_offset+itr_bit) is the new row (i.e. - what used to be "locus_a_offset")
					const size_t idx_trans_block = (num_bits_per_sib_set*num_sib_sets_at_locus_b * (locus_a_offset/Pk_CUDA_BITS_PER_BLOCK)) + (num_sib_sets_at_locus_b * (bit_offset+itr_bit)) + locus_b_offset;
					gpu_sib_sets_matrix_intersection_transpose[ idx_trans_block ] |= ( 1 << (locus_a_offset%Pk_CUDA_BITS_PER_BLOCK) );
				}
			} // end iteration over bits
		} // end iteration over blocks
	} // end bounds check
}

// @return - Pointer to GPU matrix containing transpose of intersections, client is responsible for freeing
PkCUDABlockType* CUDATransposeIntersections(
  const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
, const size_t num_bits_per_sib_set
, const size_t num_bit_blocks_for_locus_a
, const dim3 dimComputingGrid
, const dim3 dimComputingBlock
)
{
	// Allocate a matrix to store the transpose
	const size_t num_matrix_transpose_intersection_bytes = num_bits_per_sib_set * num_sib_sets_at_locus_b * num_bit_blocks_for_locus_a * sizeof( PkCUDABlockType );
	PkCUDABlockType * gpu_sib_sets_matrix_intersection_transpose = NULL;
	cudaMalloc((void**)&gpu_sib_sets_matrix_intersection_transpose, num_matrix_transpose_intersection_bytes );		
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_transpose );

	// Zero out the transpose with cudaMemset
	PkVerify( cudaSuccess == cudaMemset( (void*)gpu_sib_sets_matrix_intersection_transpose, 0, num_matrix_transpose_intersection_bytes ) );
	PkVerify( cudaSuccess == cudaThreadSynchronize() );	

	// Note: Assumes cudaMemset was used to initialize the transpose matrix to all 0's
	CUDASwizzledTransposeKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_matrix_intersection_transpose
		, gpu_sib_sets_matrix_intersection
		, gpu_sib_sets_matrix_intersection_bit_counts
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		, num_bits_per_sib_set
	);

	return gpu_sib_sets_matrix_intersection_transpose;
}

///**********************************************************

// GPU kernel for transposing swizzled bits
__global__ void CUDASwizzledCountSubsetsKernel(
  unsigned int * gpu_sib_sets_matrix_intersection_subsets_counts 
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection_transpose
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
, const size_t num_bits_per_sib_set
, const size_t num_bit_blocks_for_locus_a
)
{
	// Store byte to bit count mapping
	// @todo - move to shared memory!
	const PkCUDAByteType num_bits_in_byte_table[] = Pk_INIT_NUM_BITS_IN_BYTE_TABLE;

	// Get position in the overall computation matrix
	const size_t locus_a_offset = blockIdx.y * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y + threadIdx.y;
	const size_t locus_b_offset = blockIdx.x * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X + threadIdx.x;

	if ( ( locus_a_offset < num_sib_sets_at_locus_a ) && ( locus_b_offset < num_sib_sets_at_locus_b ) )
	{
		// Determine which sibling set we're working with
		const size_t idx_bit_set = (num_sib_sets_at_locus_b * locus_a_offset) + locus_b_offset;

		// Initialize subset count to 0
		gpu_sib_sets_matrix_intersection_subsets_counts[ idx_bit_set ] = 0;

		// Initialize first bit offsets
		size_t first_block = num_bit_blocks_per_sib_set;
		size_t first_bit_in_first_block = Pk_CUDA_BITS_PER_BLOCK;
		size_t first_bit_in_transpose = Pk_CUDA_BITS_PER_BLOCK * num_bit_blocks_per_sib_set;

		// Determine first horizontal plane to start intersecting
		for ( first_block=0; first_block<num_bit_blocks_per_sib_set; ++first_block )
		{
			const PkCUDABlockType block =
				gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * first_block) + idx_bit_set ];
			
			// Skip blocks that are empty
			// @todo: branching may not be warranted
			if ( 0 == block )
			{
				continue;
			}

			// the bit iterator below is relative to this offset
			const size_t bit_offset = first_block * Pk_CUDA_BITS_PER_BLOCK;

			// test to see if this block contains the first true bit
			for ( 
				  first_bit_in_first_block=0
				; (first_bit_in_first_block<Pk_CUDA_BITS_PER_BLOCK) && (( bit_offset + first_bit_in_first_block ) < num_bits_per_sib_set)
				; ++first_bit_in_first_block
				)
			{
				if ( block & (1 << first_bit_in_first_block) )
				{
					first_bit_in_transpose = bit_offset + first_bit_in_first_block;
					break;
				}
			} // end iteration over bits

			// End iteration over blocks once we found the first bit
			if ( first_bit_in_first_block != Pk_CUDA_BITS_PER_BLOCK )
			{
				break;
			}
		} // end iteration over blocks to find first bit

		// Early out if first bit is invalid (i.e. - all bits are 0)
		// @todo: could also just use bit counts then would never get here
		if ( first_bit_in_transpose == num_bits_per_sib_set )
		{
			return;
		}

		// Iterate over every column
		for ( size_t itr_trans_locus_b=0; itr_trans_locus_b<num_sib_sets_at_locus_b; ++itr_trans_locus_b )
		{
			// Iterate over every block in this horizontal slize of the transpose

			for ( size_t itr_trans_locus_a_block=0; itr_trans_locus_a_block<num_bit_blocks_for_locus_a; ++itr_trans_locus_a_block )
			{
				// Initialize intersection result to first block
				PkCUDABlockType transposeBlockIntersectionResult =
					gpu_sib_sets_matrix_intersection_transpose[ (num_bits_per_sib_set*num_sib_sets_at_locus_b * itr_trans_locus_a_block) + (num_sib_sets_at_locus_b*first_bit_in_transpose) + itr_trans_locus_b ];

				// Intersect with remaining bits
				size_t next_bit_in_next_block = first_bit_in_first_block+1;
				for ( size_t next_block=first_block; next_block<num_bit_blocks_per_sib_set; ++next_block )
				{
					const PkCUDABlockType block =
						gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * next_block) + idx_bit_set ];
			
					// Skip zero blocks
					// @todo: branching may not be warranted
					if ( 0 == block )
					{
						next_bit_in_next_block = 0;
						continue;
					}

					// Find next true bit
					for ( ; next_bit_in_next_block<Pk_CUDA_BITS_PER_BLOCK; ++next_bit_in_next_block )
					{
						if ( block & (1 << next_bit_in_next_block) )
						{
							// determine row in transpose
							const size_t next_bit_in_transpose = (Pk_CUDA_BITS_PER_BLOCK*next_block) + next_bit_in_next_block;

							// update transpose intersection result
							transposeBlockIntersectionResult &=
								gpu_sib_sets_matrix_intersection_transpose[ (num_bits_per_sib_set*num_sib_sets_at_locus_b * itr_trans_locus_a_block) + (num_sib_sets_at_locus_b*next_bit_in_transpose) + itr_trans_locus_b ];
						}
					} // end iteration over bits (non-transpose)

					// Reset bit iterator for next block
					next_bit_in_next_block = 0;
				} // end iteration over the remaining vertical column (non-transpose)

				// Count the number of bits in this column
				// Cast block to a byte buffer
				const PkCUDAByteType* byte_buffer = (PkCUDAByteType*)(&transposeBlockIntersectionResult);
			
				// Iterate over bytes and count bits
				for ( size_t itr_byte=0; itr_byte<sizeof(PkCUDABlockType); ++itr_byte )
				{
					gpu_sib_sets_matrix_intersection_subsets_counts[ idx_bit_set ] += num_bits_in_byte_table[ byte_buffer[ itr_byte ] ];
				}
			} // end transpose iteration over locus a blocks (depth)
		} // end transpose iteration over locus b (the columns)

	} // end bounds check
}

// @return - Pointer to GPU subsets counts, client is responsible for freeing memory
unsigned int * CUDASwizzledCountSubsets(
  const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection_transpose
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_bit_blocks_per_sib_set
, const size_t num_bits_per_sib_set
, const size_t num_bit_blocks_for_locus_a
, const dim3 dimComputingGrid
, const dim3 dimComputingBlock
)
{
	// Determine number of values necessary for storing bit counts and hash values
	const size_t num_data_values = num_sib_sets_at_locus_a * num_sib_sets_at_locus_b;
	
	// Allocate bit counts array
	unsigned int * gpu_sib_sets_matrix_intersection_subsets_counts = NULL;
	cudaMalloc((void**)&gpu_sib_sets_matrix_intersection_subsets_counts, num_data_values*sizeof(unsigned int) );		
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_subsets_counts );

	// Make sure transpose is ready
	PkVerify( cudaSuccess == cudaThreadSynchronize() );	

	CUDASwizzledCountSubsetsKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_matrix_intersection_subsets_counts 
		, gpu_sib_sets_matrix_intersection
		, gpu_sib_sets_matrix_intersection_transpose
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		, num_bits_per_sib_set
		, num_bit_blocks_for_locus_a
		);

	return gpu_sib_sets_matrix_intersection_subsets_counts;
}

///**********************************************************

// GPU kernel for swizzling the bit set blocks into a transpose format for better memory coalescing
__global__ void CUDASwizzleMatrixBytesKernel(
  PkCUDABlockType * gpu_sib_sets_swizzled_matrix
, const PkCUDABlockType * gpu_sib_sets_matrix
, const size_t num_sib_sets
, const size_t num_bit_blocks_per_sib_set
)
{
	// Make sure we only have a linear block structure
	// PkCUDAAssert( 0 == blockIdx.y )
	// PkCUDAAssert( 0 == threadIdx.y )

	// Linearize this thread to a number from [0, Pk_CUDA_NUM_TOTAL_INTERSECTION_THREADS-1]
	const size_t bit_block_offset = Pk_CUDA_NUM_TOTAL_INTERSECTION_THREADS_IN_BLOCK * blockIdx.x + threadIdx.x;
		 
	// Do nothing if we're out of bounds
	if ( bit_block_offset >= num_sib_sets * num_bit_blocks_per_sib_set )
	{
		// Make sure we only allocated enough blocks to cover the buffer size
		// PkCUDAAssert( 0 < threadIdx.x )
		return;
	}

	// Compute transposed coordinates

	// For original coordinates:
	// row_orig = bit_block_offset / num_bit_blocks_per_sib_set
	// col_orig = bit_block_offset % num_bit_blocks_per_sib_set
	// width_orig = num_bit_blocks_per_sib_set
	// orig_block_offset = width_orig * row_orig + col_orig
	//
	// For transposed coordinates:
	// row_trans = col_orig
	// col_trans = row_orig
	// width_trans = num_sib_sets
	// trans_block_offset = width_trans * row_trans + col_trans = num_sib_sets * col_orig + row_orig
	// trans_block_offset = num_sib_sets * (bit_block_offset % num_bit_blocks_per_sib_set) + ( bit_block_offset / num_bit_blocks_per_sib_set )
	gpu_sib_sets_swizzled_matrix[ ( num_sib_sets * ( bit_block_offset % num_bit_blocks_per_sib_set ) ) +
		( bit_block_offset / num_bit_blocks_per_sib_set ) ] = gpu_sib_sets_matrix[ bit_block_offset ];
}

PkCUDABlockType* CUDASwizzleSiblingSets(
  const PkCUDABlockType* gpu_sib_sets_matrix
, const size_t num_sib_sets
, const size_t num_bit_blocks_per_sib_set
)
{
	const size_t num_total_bit_blocks = num_sib_sets * num_bit_blocks_per_sib_set;
	const unsigned int num_threads_in_block_x = Pk_CUDA_NUM_TOTAL_INTERSECTION_THREADS_IN_BLOCK;
	const unsigned int num_threads_in_block_y = 1;
	const unsigned int num_blocks_x = num_total_bit_blocks / num_threads_in_block_x + ( ( num_total_bit_blocks % num_threads_in_block_x ) > 0 );
	const unsigned int num_blocks_y = 1;
	const dim3 dimComputingGrid( num_blocks_x, num_blocks_y );
	const dim3 dimComputingBlock( num_threads_in_block_x, num_threads_in_block_y );

	// Allocate space for swizzled buffer on GPU
	PkCUDABlockType* gpu_sib_sets_swizzled_matrix = NULL;
	cudaMalloc( (void**)&gpu_sib_sets_swizzled_matrix, num_total_bit_blocks * sizeof( PkCUDABlockType ) );
	PkAssert( NULL != gpu_sib_sets_swizzled_matrix );

	// Execute GPU kernel
	CUDASwizzleMatrixBytesKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_swizzled_matrix
		, gpu_sib_sets_matrix
		, num_sib_sets
		, num_bit_blocks_per_sib_set
	);
	
	return gpu_sib_sets_swizzled_matrix;
}

///**********************************************************

// GPU kernel for converting GPU intersection format to CPU friendly format
__global__ void CUDAUnswizzleIntersectionMatrixKernel(
  PkCUDABlockType * gpu_sib_sets_matrix_intersection_for_cpu
, const PkCUDABlockType * gpu_sib_sets_matrix_intersection
, const size_t num_sib_sets_at_locus_a
, const size_t num_sib_sets_at_locus_b
, const size_t num_buffer_elements
)
{
	// Get position in the overall computation matrix
	unsigned int locus_a_offset = blockIdx.y * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y + threadIdx.y;
	unsigned int locus_b_offset = blockIdx.x * Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X + threadIdx.x;

	if ( ( locus_a_offset < num_sib_sets_at_locus_a ) && locus_b_offset < ( num_sib_sets_at_locus_b ) )
	{
		// Map to cpu formatted memory
		for ( int itr_buffer_element=0; itr_buffer_element<num_buffer_elements; ++itr_buffer_element )
		{
			gpu_sib_sets_matrix_intersection_for_cpu[ (num_sib_sets_at_locus_b*num_buffer_elements*locus_a_offset) + (num_buffer_elements*locus_b_offset) + itr_buffer_element ] =
				gpu_sib_sets_matrix_intersection[ (num_sib_sets_at_locus_a*num_sib_sets_at_locus_b * itr_buffer_element) + (num_sib_sets_at_locus_b * locus_a_offset) + locus_b_offset ];
		}
	}
}

void CUDAUnswizzleIntersectionMatrix(
	  PkPooledRawBitSetArray& outputSets
	, const PkPooledRawBitSetArray& partitionSets
	, const size_t num_matrix_intersection_bytes
	, const PkCUDABlockType* gpu_sib_sets_matrix_intersection
	, const dim3 dimComputingGrid
	, const dim3 dimComputingBlock
	)
{
	PkLogf( "\tBegin CUDA conversion from GPU memory format to CPU memory format.\n" );
	PkAssert( ( outputSets.size() * partitionSets.size() * outputSets.num_blocks() * sizeof( PkCUDABlockType ) ) 
		== num_matrix_intersection_bytes );
	PkAssert( outputSets.num_blocks() == partitionSets.num_blocks() );

	// Begin timer
	unsigned int timer;
	cutCreateTimer(&timer);
	cutStartTimer(timer);				

	// Allocate another GPU intersection matrix to convert to CPU format
	PkCUDABlockType* gpu_sib_sets_matrix_intersection_for_cpu = NULL;
	cudaMalloc((void**)&gpu_sib_sets_matrix_intersection_for_cpu, num_matrix_intersection_bytes );		
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_for_cpu );

	// Run GPU kernel to convert to CPU memory format
	CUDAUnswizzleIntersectionMatrixKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_matrix_intersection_for_cpu
		, gpu_sib_sets_matrix_intersection
		, outputSets.size()
		, partitionSets.size()
		, outputSets.num_blocks()
		);

	// Allocate CPU memory for intersection
	PkCUDABlockType* cpu_sib_sets_matrix_intersection = (PkCUDABlockType*) malloc( num_matrix_intersection_bytes );
	PkAssert( NULL != cpu_sib_sets_matrix_intersection );
	cudaMemcpy(
		  cpu_sib_sets_matrix_intersection
		, gpu_sib_sets_matrix_intersection_for_cpu
		, num_matrix_intersection_bytes
		, cudaMemcpyDeviceToHost
		);

	PkLogf( "\t\tCUDA conversion finished (%d ms) - now converting to PkBitSetArray.\n", (unsigned int)cutGetTimerValue(timer) );
	
	// Free temporary GPU resources
	cudaFree( gpu_sib_sets_matrix_intersection_for_cpu );

	// Pass ownership to output pooled array
	outputSets.reinit(
		  cpu_sib_sets_matrix_intersection
		, true /* b_owned */
		, outputSets.num_bits()
		, outputSets.size() * partitionSets.size()
		, 2 /* num_default_reserved_pool_elements */
		);

	// Assert that total number of bytes is consistent
	PkAssert( outputSets.num_total_bytes() == num_matrix_intersection_bytes );

	// Stop timer
	cutStopTimer(timer);
	PkLogf( "\tEnd CUDA conversion from GPU memory format to CPU memory format (%d ms).\n", (unsigned int)cutGetTimerValue(timer) );
	cutDeleteTimer(timer);
}

///**********************************************************

// Push an intersection buffer to GPU
PkCUDABlockType* CUDAPushSiblingSetsToGPU( const PkPooledRawBitSetArray& sibling_sets )
{
	PkLogf( "\tPushing sibling set ID %x to GPU.\n", &sibling_sets );
	PkAssert( sizeof( PkPooledRawBitSetArray::block_type ) == sizeof( PkCUDABlockType ) );

	// Allocate GPU sibling sets matrix
	PkCUDABlockType* gpu_sib_sets_matrix = NULL;
	cudaMalloc( (void**)&gpu_sib_sets_matrix, sibling_sets.num_total_bytes() );
	PkAssert( NULL != gpu_sib_sets_matrix );

	// Copy CPU sibling set chunks to GPU
	PkCUDASizeType byte_offset = 0;
	for ( int itr_chunk=0; itr_chunk < sibling_sets.get_chunks().size(); ++itr_chunk )
	{
		// Copy sibling sets to memory
		cudaMemcpy(
			  &(((PkCUDAByteType*)gpu_sib_sets_matrix)[ byte_offset ]) /* dst_ptr */
			, sibling_sets.get_chunks()[ itr_chunk ].first /* src_ptr */
			, sibling_sets.get_chunks()[ itr_chunk ].second /* num_bytes */
			, cudaMemcpyHostToDevice
			);
			byte_offset += sibling_sets.get_chunks()[ itr_chunk ].second;
	}
	// Assert we pushed every last byte!
	PkAssert( sibling_sets.num_total_bytes() == byte_offset );

	// Output GPU pointer
	return gpu_sib_sets_matrix;
}

///**********************************************************

// Intersects each set of outputSets with each set of partitionSets and append to outputSets
void CUDAIntersectSiblingSets(
  PkPooledRawBitSetArray& outputSets
, const PkPooledRawBitSetArray& partitionSets
)
{
	PkLogf( "Begin CUDA intersection.\n" );

	// Assert buffers have matching bit set specifications
	PkAssert( outputSets.num_bits() == partitionSets.num_bits() );
	PkAssert( outputSets.num_blocks() == partitionSets.num_blocks() );

	// Begin timer
	unsigned int timer;
	cutCreateTimer(&timer);
	cutStartTimer(timer);				

	// Early out if we don't have any sets to intersect
	if ( outputSets.size() <= 0 || partitionSets.size() <= 0 )
	{
		outputSets.clear();
		return;
	}

	// Store original locus sizes and block counts
	const size_t num_sib_sets_at_locus_a = outputSets.size();
	const size_t num_sib_sets_at_locus_b = partitionSets.size();
	const size_t num_bit_blocks_per_sib_set = outputSets.num_blocks();

	// Push first buffer to GPU
	PkCUDABlockType* gpu_sib_sets_matrix_a = CUDAPushSiblingSetsToGPU( outputSets );
	PkAssert( NULL != gpu_sib_sets_matrix_a );

	// Swizzle bits
	PkCUDABlockType* gpu_sib_sets_swizzled_matrix_a = CUDASwizzleSiblingSets(
		  gpu_sib_sets_matrix_a
		, num_sib_sets_at_locus_a
		, num_bit_blocks_per_sib_set
		);
	PkAssert( NULL != gpu_sib_sets_swizzled_matrix_a );

	// Push second buffer to GPU
	PkCUDABlockType* gpu_sib_sets_matrix_b = CUDAPushSiblingSetsToGPU( partitionSets );
	PkAssert( NULL != gpu_sib_sets_matrix_b );

	// Swizzle bits
	PkCUDABlockType* gpu_sib_sets_swizzled_matrix_b = CUDASwizzleSiblingSets(
		  gpu_sib_sets_matrix_b
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		);
	PkAssert( NULL != gpu_sib_sets_swizzled_matrix_b );

	// Prepare GPU intersection threads
	const unsigned int num_threads_in_block_x = Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_X;
	const unsigned int num_threads_in_block_y = Pk_CUDA_NUM_INTERSECTION_THREADS_IN_BLOCK_Y;
	const unsigned int num_blocks_x = num_sib_sets_at_locus_b / num_threads_in_block_x + ( ( num_sib_sets_at_locus_b % num_threads_in_block_x ) > 0 );
	const unsigned int num_blocks_y = num_sib_sets_at_locus_a / num_threads_in_block_y + ( ( num_sib_sets_at_locus_a % num_threads_in_block_y ) > 0 );
	const dim3 dimComputingGrid( num_blocks_x, num_blocks_y );
	const dim3 dimComputingBlock( num_threads_in_block_x, num_threads_in_block_y );

	// Free original (non-swizzled) sibling set matrices (these are blocking operations on the GPU)
	cudaFree( gpu_sib_sets_matrix_b );
	cudaFree( gpu_sib_sets_matrix_a );

	// Allocate GPU memory for intersection
	PkCUDABlockType* gpu_sib_sets_matrix_intersection = NULL;
	const size_t num_matrix_intersection_bytes = num_sib_sets_at_locus_a * num_sib_sets_at_locus_b * num_bit_blocks_per_sib_set * sizeof( PkCUDABlockType );
	cudaMalloc((void**)&gpu_sib_sets_matrix_intersection, num_matrix_intersection_bytes );		
	PkAssert( NULL != gpu_sib_sets_matrix_intersection );

	// Execute GPU intersection kernel
	CUDASwizzledSiblingIntersectionKernel<<<dimComputingGrid,dimComputingBlock>>>(
		  gpu_sib_sets_swizzled_matrix_a
		, gpu_sib_sets_swizzled_matrix_b
		, gpu_sib_sets_matrix_intersection
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		);

	// Deallocate swizzled buffers
	cudaThreadSynchronize(); // @todo: not sure if this is necessary as cudaFree may be blocking
	cudaFree( gpu_sib_sets_swizzled_matrix_b );
	cudaFree( gpu_sib_sets_swizzled_matrix_a );

	// Copy intersections back to CPU
	CUDAUnswizzleIntersectionMatrix(
		  outputSets
		, partitionSets
		, num_matrix_intersection_bytes
		, gpu_sib_sets_matrix_intersection
		, dimComputingGrid
		, dimComputingBlock
		);

// BEGIN INTEGRATION
	// Count bits for each bit set
	const PkCUDABitCountType * gpu_sib_sets_matrix_intersection_bit_counts = CUDACountIntersectionBits(
		  gpu_sib_sets_matrix_intersection
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		, dimComputingGrid
		, dimComputingBlock
		);
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_bit_counts );
	
	// Compute hash for each bit set
	const std::size_t * gpu_sib_sets_matrix_intersection_hashes = CUDAComputeIntersectionHashes(
		  gpu_sib_sets_matrix_intersection
		, gpu_sib_sets_matrix_intersection_bit_counts
		, num_sib_sets_at_locus_a
		, num_sib_sets_at_locus_b
		, num_bit_blocks_per_sib_set
		, dimComputingGrid
		, dimComputingBlock
		);
	PkAssert( NULL != gpu_sib_sets_matrix_intersection_hashes );
	
	// Copy hashes back to CPU, release from GPU
	const size_t num_data_values = num_sib_sets_at_locus_a * num_sib_sets_at_locus_b;

	const std::size_t * cpu_bit_hashes = CUDACopyFromGPUToCPUAndReleaseGPU<std::size_t>( gpu_sib_sets_matrix_intersection_hashes, num_data_values );
	PkAssert( NULL != cpu_bit_hashes );

	// Copy bit counts back to CPU, release from GPU
	const PkCUDABitCountType * cpu_bit_counts = CUDACopyFromGPUToCPUAndReleaseGPU<PkCUDABitCountType>( gpu_sib_sets_matrix_intersection_bit_counts, num_data_values );
	PkAssert( NULL != cpu_bit_counts );

	// Deallocate CPU data
	free( (void*)cpu_bit_hashes ); cpu_bit_hashes = NULL;
	free( (void*)cpu_bit_counts ); cpu_bit_counts = NULL;
// END INTEGRATION

	// Deallocate GPU intersection (blocks GPU)
	cudaFree( gpu_sib_sets_matrix_intersection );

	// Stop timer
	cutStopTimer(timer);
	PkLogf( "End CUDA intersection (%d ms).\n", (unsigned int)cutGetTimerValue(timer) );
	cutDeleteTimer(timer);
}

} // end of Pk namespace

// BEGIN ORIGINAL SAMPLE CODE:

#if 0

#define SIBGROUP_SIZE 1024

// GPU kernel for calculating the intersection
__global__ void SimpleSiblingIntersection(unsigned int * gpu_sib_group_a,unsigned int * gpu_sib_group_b,unsigned int * gpu_sib_group_intersection){

	//Get position in the overall computation matrix
	unsigned int row=blockIdx.y*16+threadIdx.y;
	unsigned int column=blockIdx.x*16+threadIdx.x;

	//Compute the intersection
	for(int k=0; k<4; ++k)
		gpu_sib_group_intersection[SIBGROUP_SIZE*SIBGROUP_SIZE*k+SIBGROUP_SIZE*row+column]=gpu_sib_group_a[SIBGROUP_SIZE*k+row] & gpu_sib_group_b[SIBGROUP_SIZE*k+column];
}

// GPU kernel with shared memory for calculating the intersection
__global__ void SiblingIntersection(unsigned int * gpu_sib_group_a,unsigned int * gpu_sib_group_b,unsigned int * gpu_sib_group_intersection){
	
	//Shared memory used as cache
	__shared__ unsigned int sib_group_shared_a[16][4];	
	__shared__ unsigned int sib_group_shared_b[16][4];	

	//First row loads the sibgroup in shared cache 
	if (threadIdx.y==0){
		for (int k=0; k<4; ++k)
			sib_group_shared_a[threadIdx.x][k]=gpu_sib_group_a[SIBGROUP_SIZE*k+blockIdx.y*16+threadIdx.x];
		for (int k=0; k<4; ++k)
			sib_group_shared_b[threadIdx.x][k]=gpu_sib_group_b[SIBGROUP_SIZE*k+blockIdx.x*16+threadIdx.x];
	}
	__syncthreads();
	
	//Get position in the overall computation matrix
	unsigned int row=blockIdx.y*16+threadIdx.y;
	unsigned int column=blockIdx.x*16+threadIdx.x;

	//Compute the intersection on sibgroup in shared memory
	for(int k=0; k<4; ++k)
		gpu_sib_group_intersection[SIBGROUP_SIZE*SIBGROUP_SIZE*k+SIBGROUP_SIZE*row+column]
			=sib_group_shared_a[threadIdx.y][k] & sib_group_shared_b[threadIdx.x][k];
}

int CudaTestMain (int argc, char **argv) {
	
	//Allocate memory for locus_a and locus_b n=m=128 sibgroup=128bits
	unsigned int sibgroup_size=SIBGROUP_SIZE*4*sizeof(unsigned int);
	unsigned int * sib_group_a = (unsigned int *) malloc(sibgroup_size);
	unsigned int * sib_group_b = (unsigned int *) malloc(sibgroup_size);
	
	//Create random sibgroups
	unsigned int iseed = (unsigned int)time(NULL);
	srand (iseed);
	for (int i=0; i<SIBGROUP_SIZE*4; ++i){
		sib_group_a[i]=rand();
		sib_group_b[i]=rand();		
	}
	
	//Allocate memory for the intersection n*m=128*128 sibgroup=128bits
	unsigned int sibgroup_size_intersection=SIBGROUP_SIZE*SIBGROUP_SIZE*4*sizeof(unsigned int);
	unsigned int * sib_group_intersection = (unsigned int *) malloc(sibgroup_size_intersection);
	unsigned int * sib_group_intersection_copy = (unsigned int *) malloc(sibgroup_size_intersection);
	
	//Compute the intersection as 32bit binary operation
	unsigned int baseline_timer;
	cutCreateTimer(&baseline_timer);
	cutStartTimer(baseline_timer);			
	for (int i=0; i<SIBGROUP_SIZE; ++i)
		for (int j=0; j<SIBGROUP_SIZE; ++j)
			for (int k=0; k<4; ++k) 
				sib_group_intersection[SIBGROUP_SIZE*SIBGROUP_SIZE*k+SIBGROUP_SIZE*i+j]=sib_group_a[SIBGROUP_SIZE*k+i] & sib_group_b[SIBGROUP_SIZE*k+j];
	cutStopTimer(baseline_timer);	
	printf("CPU intersection took %d ms\n", (unsigned int)cutGetTimerValue(baseline_timer));	
	
	//Allocate GPU memory for locus_a and locus_b
	unsigned int * gpu_sib_group_a;
	unsigned int * gpu_sib_group_b;
	cudaMalloc((void**)&gpu_sib_group_a,sibgroup_size);
	cudaMalloc((void**)&gpu_sib_group_b,sibgroup_size);
	
	//Allocate GPU memory for intersection
	unsigned int * gpu_sib_group_intersection;	
	cudaMalloc((void**)&gpu_sib_group_intersection,sibgroup_size_intersection);		
	
	//Copy sibling groups to memory
	unsigned int timer;
	cutCreateTimer(&timer);
	cutStartTimer(timer);				
	cudaMemcpy(gpu_sib_group_a,sib_group_a,sibgroup_size,cudaMemcpyHostToDevice);	
	cudaMemcpy(gpu_sib_group_b,sib_group_b,sibgroup_size,cudaMemcpyHostToDevice);	

	//Prepare GPU threads (16x16) and block (8x8)
	dim3 dimComputingGrid(SIBGROUP_SIZE/16,SIBGROUP_SIZE/16);
	dim3 dimComputingBlock(16,16);		
		
	//Execute GPU kernel
	SimpleSiblingIntersection<<<dimComputingGrid,dimComputingBlock>>>(gpu_sib_group_a,gpu_sib_group_b,gpu_sib_group_intersection);
	
	//Copy back the intersection
	cudaMemcpy(sib_group_intersection_copy,gpu_sib_group_intersection,sibgroup_size_intersection,cudaMemcpyDeviceToHost);			
	cutStopTimer(timer);
	printf("GPU intersection took  %d ms\n",(unsigned int)cutGetTimerValue(timer));
	printf("Speed-up %.2fx\n",cutGetTimerValue(baseline_timer)/cutGetTimerValue(timer));	
	cutDeleteTimer(timer);
	
	//Execute GPU kernel
	cutCreateTimer(&timer);
	cutStartTimer(timer);	
	cudaMemcpy(gpu_sib_group_a,sib_group_a,sibgroup_size,cudaMemcpyHostToDevice);	
	cudaMemcpy(gpu_sib_group_b,sib_group_b,sibgroup_size,cudaMemcpyHostToDevice);	
	SiblingIntersection<<<dimComputingGrid,dimComputingBlock>>>(gpu_sib_group_a,gpu_sib_group_b,gpu_sib_group_intersection);
	
	//Copy back the intersection
	cudaMemcpy(sib_group_intersection_copy,gpu_sib_group_intersection,sibgroup_size_intersection,cudaMemcpyDeviceToHost);			
	cutStopTimer(timer);
	printf("GPU intersection took  %d ms\n",(unsigned int)cutGetTimerValue(timer));
	printf("Speed-up %.2fx\n",cutGetTimerValue(baseline_timer)/cutGetTimerValue(timer));	
	
	//Compare gpu and cpu result for correctness
	for (int i=0; i<SIBGROUP_SIZE*SIBGROUP_SIZE*4; ++i)
		if (sib_group_intersection[i]!=sib_group_intersection_copy[i]) {
			printf("Gpu and cpu produce different results.\n");
			printf("%d != %d (%d)\n",sib_group_intersection[i],sib_group_intersection_copy[i],i);
			break;
		}
	
	//Deallocate GPU data structures
	cudaFree(gpu_sib_group_a);
	cudaFree(gpu_sib_group_b);
	cudaFree(gpu_sib_group_intersection);
	
	//Deallocate data structures
	free(sib_group_a);
	free(sib_group_b);
		
    return 0;
}

#endif // 0

// END ORIGINAL SAMPLE CODE
