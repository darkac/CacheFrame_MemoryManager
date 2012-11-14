// Last modified: 2012-10-19 20:07:48
 
/**
 * @file: MemoryManager.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-27 09:38:55
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _MEMORYMANAGER_H_
#define _MEMORYMANAGER_H_

#include <cstdio>
#include <cstdlib>
#include <pthread.h>

#define MEMORYSIZE 1024        // in terms of Byte
#define BLOCKSIZE 2048          // in terms of int
#define MAX_TERM_OF_A_QUERY 16

typedef struct blockunit
{
	unsigned int m_index_Global; // index in Global
	unsigned int m_index_List;   // index in List
	unsigned int m_used_int;
	struct blockunit *b_prev;
	struct blockunit *b_next;
} blockunit_t;
//}__attribute__((align(16))) blockunit_t;

class MemoryManager
{
private:
	unsigned long long memorySize;
	unsigned int numBlock;
	
	unsigned int *pMemory;
	unsigned long long *offsetArray;
	
	blockunit_t **unitArray;
	blockunit_t *headFree, *tailFree;

	pthread_mutex_t m_mutex_memory;

public:
	MemoryManager(unsigned long long _size, unsigned int _blockNum);
	~MemoryManager();
	
	void InsertData(
			blockunit_t **start_node,
			unsigned int termid,
			unsigned int listLength,
			unsigned long long offset);
	void EvictData(blockunit_t *bnode, int block_num);

	unsigned int GetPMemory(unsigned long long index);
	unsigned long long GetOffsetArray(unsigned int index);

	void mm_lock_mutex();
	void mm_unlock_mutex();
};

#endif
