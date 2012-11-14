// Last modified: 2012-10-18 17:39:55
 
/**
 * @file: MemoryManager.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-27 09:41:48
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */

#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>

#include "hash.h"
#include "function.h"
#include "MemoryManager.h"

//#define READV

MemoryManager::MemoryManager(unsigned long long _size, unsigned int _blockNum)
{
	memorySize = _size;
	numBlock = _blockNum;
	
	pMemory = (unsigned int *)malloc(memorySize);
	checkPointer((void *)pMemory, __LINE__);
	
	offsetArray = (unsigned long long *)malloc(numBlock * sizeof(unsigned long long));
	checkPointer((void *)offsetArray, __LINE__);

	for (unsigned int i = 0; i < numBlock; ++i)
	{
		offsetArray[i] = (unsigned long long)i * BLOCKSIZE;
	}
	
	headFree = (blockunit_t *)malloc(sizeof(blockunit_t));
	checkPointer((void *)headFree, __LINE__);
	tailFree = (blockunit_t *)malloc(sizeof(blockunit_t));
	checkPointer((void *)tailFree, __LINE__);
	
	unitArray = (blockunit_t **)malloc(numBlock * sizeof(blockunit_t *));
	checkPointer((void *)unitArray, __LINE__);
	
	for (unsigned int i = 0; i < numBlock; ++i)
	{
		unitArray[i] = (blockunit_t *)malloc(sizeof(blockunit_t));
		checkPointer((void *)unitArray[i], __LINE__);
		unitArray[i]->m_index_Global = i;
		unitArray[i]->m_index_List = 0;
		unitArray[i]->m_used_int = 0;
	}
	
	unitArray[0]->b_next = unitArray[1];
	unitArray[0]->b_prev = headFree;
	for (unsigned int i = 1; i < numBlock - 1; ++i)
	{
		unitArray[i]->b_next = unitArray[i + 1];
		unitArray[i]->b_prev = unitArray[i - 1];
	}
	unitArray[numBlock - 1]->b_next = NULL;
	unitArray[numBlock - 1]->b_prev = unitArray[numBlock - 2];

	headFree->b_next = unitArray[0];
	tailFree = unitArray[numBlock - 1];
	
	if (pthread_mutex_init(&m_mutex_memory, NULL) != 0)
	{
		printf("mutex_memory init failed.\n");
		exit(-1);
	}
}

MemoryManager::~MemoryManager()
{
	freeResource(pMemory);
	freeResource(offsetArray);
	
	for (unsigned int i = 0; i < numBlock; ++i)
	{
		freeResource(unitArray[i]);
	}
	freeResource(unitArray);
	
	freeResource(headFree);
	//freeResource(tailFree);
	
	pthread_mutex_destroy(&m_mutex_memory);
}

void MemoryManager::InsertData(
		blockunit_t **start_node,
		unsigned int termid, 
		unsigned int listLength,
		unsigned long long offset)
{
	assert(pIndex != NULL);
	assert(listLength > 0);
	assert(offset >= 0);

	mm_lock_mutex();
	*start_node = headFree->b_next;
	blockunit_t *temp = headFree->b_next;
	temp->b_prev = NULL;

	int fp = fileno(pIndex);
	lseek(fp, offset, SEEK_SET);
#ifdef READV
	int limit_readv = sysconf(_SC_IOV_MAX);
	int blockcnt = (listLength + BLOCKSIZE - 1) / BLOCKSIZE;
	int iovcnt = limit_readv, i, index_list = 0;
	struct iovec *iov = (struct iovec *)malloc(iovcnt * sizeof(struct iovec));
	memset(iov, 0, iovcnt * sizeof(struct iovec));
	while (blockcnt > limit_readv)
	{
		blockcnt -= limit_readv;
		for (i = 0; i < iovcnt; ++i)
		{
			temp->m_index_List = ++index_list;
			temp->m_used_int = BLOCKSIZE;
			iov[i].iov_base = pMemory + offsetArray[temp->m_index_Global];
			iov[i].iov_len = BLOCKSIZE * sizeof(int);
			temp = temp->b_next;
		}
		ssize_t bytes_read = readv(fp, iov, iovcnt);
		assert((unsigned int)bytes_read == iovcnt * BLOCKSIZE * sizeof(int));
	}
	//free(iov);

	iovcnt = blockcnt;
	iov = (struct iovec *)realloc(iov, iovcnt * sizeof(struct iovec));
	for (i = 0; i < iovcnt - 1; ++i)
	{
		temp->m_index_List = ++index_list;
		temp->m_used_int = BLOCKSIZE;
		iov[i].iov_base = pMemory + offsetArray[temp->m_index_Global];
		iov[i].iov_len = BLOCKSIZE * sizeof(int);
		temp = temp->b_next;
		listLength -= BLOCKSIZE;
	}
	temp->m_index_List = ++index_list;
	temp->m_used_int = listLength;
	iov[i].iov_base = pMemory + offsetArray[temp->m_index_Global];
	iov[i].iov_len = temp->m_used_int * sizeof(int);

	ssize_t bytes_read = readv(fp, iov, iovcnt);
	assert((unsigned int)bytes_read == 
			((iovcnt - 1) * BLOCKSIZE + temp->m_used_int) * sizeof(int));
	free(iov);
#else
	//lseek(fp, offset, SEEK_SET);
	unsigned int index_list = 1;
	while (listLength > BLOCKSIZE)
	{
		listLength -= BLOCKSIZE;
		temp->m_index_List = index_list++;
		temp->m_used_int = BLOCKSIZE;
		read(fp, pMemory + offsetArray[temp->m_index_Global], BLOCKSIZE * sizeof(int));
		temp = temp->b_next;
	}

	temp->m_index_List = index_list;
	temp->m_used_int = listLength;
	read(fp, pMemory + offsetArray[temp->m_index_Global], temp->m_used_int * sizeof(int));
#endif

	if (temp != tailFree)
	{
		temp->b_next->b_prev = headFree;
		headFree->b_next = temp->b_next;
	}
	else
	{
		tailFree = headFree;
		headFree->b_next = NULL;
	}

	mm_unlock_mutex();
}

void MemoryManager::EvictData(blockunit_t *bnode, int block_num)
{
		blockunit_t *startNode = bnode;
		blockunit_t *endNode = NULL;
		blockunit_t *temp = startNode;

		int num = block_num;
		while (num-- > 0)
		{
			endNode = temp;
			temp->m_used_int = 0;
			temp->m_index_List = 0;
			temp = temp->b_next;
		}

		mm_lock_mutex();

		tailFree->b_next = startNode;
		startNode->b_prev = tailFree;
		endNode->b_next = NULL;
		tailFree = endNode;

		mm_unlock_mutex();
}

unsigned int MemoryManager::GetPMemory(unsigned long long index)
{
	return pMemory[index];
}

unsigned long long MemoryManager::GetOffsetArray(unsigned int index)
{
	return offsetArray[index];
}

void MemoryManager::mm_lock_mutex()
{
	pthread_mutex_lock(&m_mutex_memory);
}

void MemoryManager::mm_unlock_mutex()
{
	pthread_mutex_unlock(&m_mutex_memory);
}
