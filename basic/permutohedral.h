/*
    Copyright (c) 2011, Philipp Krähenbühl
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the Stanford University nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Philipp Krähenbühl ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Philipp Krähenbühl BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _PERMUTOHEDRAL_H_
#define _PERMUTOHEDRAL_H_

#include "EagleeyeMacro.h"

#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <math.h>

#ifdef __SSE__
// SSE Permutoheral lattice
# define SSE_PERMUTOHEDRAL
#endif

#if defined(SSE_PERMUTOHEDRAL)
# include <emmintrin.h>
# include <xmmintrin.h>
# ifdef __SSE4_1__
#  include <smmintrin.h>
# endif
#endif

namespace eagleeye
{
/************************************************/
/***                Hash Table                ***/
/************************************************/

class EAGLEEYE_API HashTable{
	// Don't copy!
	HashTable( const HashTable & o ): key_size_ ( o.key_size_ ), filled_(0), capacity_(o.capacity_) {
		table_ = new int[ capacity_ ];
		keys_ = new short[ (capacity_/2+10)*key_size_ ];
		memset( table_, -1, capacity_*sizeof(int) );
	}
protected:
	size_t key_size_, filled_, capacity_;
	short * keys_;
	int * table_;
	void grow(){
		// Swap out the old memory
		short * old_keys = keys_;
		int * old_table = table_;
		int old_capacity = capacity_;
		capacity_ *= 2;
		// Allocate the new memory
		keys_ = new short[ (old_capacity+10)*key_size_ ];
		table_ = new int[ capacity_ ];
		memset( table_, -1, capacity_*sizeof(int) );
		memcpy( keys_, old_keys, filled_*key_size_*sizeof(short) );

		// Reinsert each element
		for( int i=0; i<old_capacity; i++ )
			if (old_table[i] >= 0){
				int e = old_table[i];
				size_t h = hash( old_keys+(getKey(e)-keys_) ) % capacity_;
				for(; table_[h] >= 0; h = h<capacity_-1 ? h+1 : 0);
				table_[h] = e;
			}

			delete [] old_keys;
			delete [] old_table;
	}
	size_t hash( const short * k ) {
		size_t r = 0;
		for( size_t i=0; i<key_size_; i++ ){
			r += k[i];
			r *= 1664525;
		}
		return r;
	}
public:
	explicit HashTable( int key_size, int n_elements ) : key_size_ ( key_size ), filled_(0), capacity_(2*n_elements) {
		table_ = new int[ capacity_ ];
		keys_ = new short[ (capacity_/2+10)*key_size_ ];
		memset( table_, -1, capacity_*sizeof(int) );
	}
	~HashTable() {
		delete [] keys_;
		delete [] table_;
	}
	int size() const {
		return filled_;
	}
	void reset() {
		filled_ = 0;
		memset( table_, -1, capacity_*sizeof(int) );
	}
	int find( const short * k, bool create = false ){
		if (2*filled_ >= capacity_) grow();
		// Get the hash value
		size_t h = hash( k ) % capacity_;
		// Find the element with he right key, using linear probing
		while(1){
			int e = table_[h];
			if (e==-1){
				if (create){
					// Insert a new key and return the new id
					for( size_t i=0; i<key_size_; i++ )
						keys_[ filled_*key_size_+i ] = k[i];
					return table_[h] = filled_++;
				}
				else
					return -1;
			}
			// Check if the current key is The One
			bool good = true;
			for( size_t i=0; i<key_size_ && good; i++ )
				if (keys_[ e*key_size_+i ] != k[i])
					good = false;
			if (good)
				return e;
			// Continue searching
			h++;
			if (h==capacity_) h = 0;
		}
	}
	const short * getKey( int i ) const{
		return keys_+i*key_size_;
	}

};

/************************************************/
/***          Permutohedral Lattice           ***/
/************************************************/

class EAGLEEYE_API Permutohedral
{
protected:
	int * offset_;
	float * barycentric_;

	struct Neighbors{
		int n1, n2;
		Neighbors( int n1=0, int n2=0 ):n1(n1),n2(n2){
		}
	};
	Neighbors * blur_neighbors_;
	// Number of elements, size of sparse discretized space, dimension of features
	int N_, M_, d_;
public:
	Permutohedral() :offset_( NULL ),barycentric_( NULL ),blur_neighbors_( NULL ),N_ ( 0 ),M_ ( 0 ),d_ ( 0 ) {
	}
	Permutohedral ( const Permutohedral& o ):offset_( NULL ),barycentric_( NULL ),blur_neighbors_( NULL ),N_ ( o.N_ ),M_ ( o.M_ ),d_ ( o.d_ )
	{
		if (o.barycentric_){
			barycentric_ = new float[ (d_+1)*N_ ];
			memcpy( barycentric_, o.barycentric_, (d_+1)*N_*sizeof(float) );
		}
		if (o.offset_){
			offset_ = new int[ (d_+1)*N_ ];
			memcpy( offset_, o.offset_, (d_+1)*N_*sizeof(int) );
		}
		if (o.blur_neighbors_){
			blur_neighbors_ = new Neighbors[ (d_+1)*N_ ];
			memcpy( blur_neighbors_, o.blur_neighbors_, (d_+1)*N_*sizeof(Neighbors) );
		}
	}
	Permutohedral& operator= ( const Permutohedral& o )
	{
		if (&o == this) return *this;
		if (barycentric_)    delete[] barycentric_;
		if (offset_)         delete[] offset_;
		if (blur_neighbors_) delete[] blur_neighbors_;
		offset_ = NULL; barycentric_ = NULL; blur_neighbors_ = NULL;
		N_ = o.N_; M_ = o.M_; d_ = o.d_;
		if (o.barycentric_){
			barycentric_ = new float[ (d_+1)*N_ ];
			memcpy( barycentric_, o.barycentric_, (d_+1)*N_*sizeof(float) );
		}
		if (o.offset_){
			offset_ = new int[ (d_+1)*N_ ];
			memcpy( offset_, o.offset_, (d_+1)*N_*sizeof(int) );
		}
		if (o.blur_neighbors_){
			blur_neighbors_ = new Neighbors[ (d_+1)*N_ ];
			memcpy( blur_neighbors_, o.blur_neighbors_, (d_+1)*N_*sizeof(Neighbors) );
		}
		return *this;
	}
	~Permutohedral(){
		if (barycentric_)    delete[] barycentric_;
		if (offset_)         delete[] offset_;
		if (blur_neighbors_) delete[] blur_neighbors_;
	}
	void init ( const float* feature, int feature_size, int N );

	#ifdef SSE_PERMUTOHEDRAL
	void compute ( __m128* out, const __m128* in, int value_size, int in_offset=0, int out_offset=0, int in_size = -1, int out_size = -1 ) const;
	void compute ( float* out, const float* in, int value_size, int in_offset=0, int out_offset=0, int in_size = -1, int out_size = -1 ) const;
	#else
	void compute ( float* out, const float* in, int value_size, int in_offset=0, int out_offset=0, int in_size = -1, int out_size = -1 ) const;
	#endif
};
}
#endif

