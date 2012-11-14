// Last modified: 2012-10-19 20:11:27
 
/**
 * @file: CacheFrame.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-09-26 19:29:11
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hash.h"
#include "function.h"
#include "CacheFrame.h"

CCacheFrame::CCacheFrame(int _size)
{
	firstCache = (cachenode_t *)malloc(sizeof(cachenode_t));
	memset(firstCache, 0, sizeof(cachenode_t));
	lastCache = firstCache;

	if (pthread_mutex_init(&cache_mutex, NULL) != 0)
	{
		printf("cache_mutex init failed.\n");
		exit(-1);
	}

	cacheCapacity = (unsigned long long)_size * 1024 * 1024;
	blockUnUsed = cacheCapacity / (BLOCKSIZE * sizeof(int));
	MM = new MemoryManager(cacheCapacity, blockUnUsed);
}

CCacheFrame::~CCacheFrame()
{
	cachenode_t *tmp = firstCache->c_next;
	cachenode_t *todel;
	while ((todel = tmp) != NULL)
	{
		tmp = tmp->c_next;
		freeResource(todel);
	}

	lastCache = firstCache;
	freeResource(firstCache);

	pthread_mutex_destroy(&cache_mutex);

	delete MM;
}

cachenode_t *CCacheFrame::get_first() const
{
	return firstCache;
}

cachenode_t *CCacheFrame::get_last() const
{
	return lastCache;
}

unsigned int CCacheFrame::NumOfBlock(const unsigned int listLen) const
{
	return (listLen + BLOCKSIZE - 1) / BLOCKSIZE;
}

bool CCacheFrame::ExistInCache(const unsigned int termid) const
{
	return existInHash(termid);
}

bool CCacheFrame::IsSpaceEnough(const unsigned int listLen) const
{
	if (NumOfBlock(listLen) > blockUnUsed)
		return 0;
	else
		return 1;
}

void CCacheFrame::CacheListInsert(
		unsigned int termid, 
		unsigned int listLen, 
		unsigned long long file_offset)
{
	int slot = HashKey(termid);
	ht_wrlock(slot);
	hashnode_t *tmp = getHashNode(termid);
	if (tmp != NULL)
	{
		// nothing to do here
	}
	else
	{
		cachenode_t *newcnode = (cachenode_t *)malloc(sizeof(cachenode_t));
		memset(newcnode, 0, sizeof(cachenode_t));
		newcnode->m_termid = termid;
		newcnode->m_listlen = listLen;

		hashnode_t *newhnode = (hashnode_t *)malloc(sizeof(hashnode_t));
		memset(newhnode, 0, sizeof(hashnode_t));
		newhnode->m_key = termid;
		newhnode->m_listLength = listLen;
		newhnode->m_cache_node = newcnode;
		newhnode->h_next = hashTable[slot]->h_next;
		hashTable[slot]->h_next = newhnode;
	
		//cf_lock_mutex();

		/*
		if (IsSpaceEnough(listLen) == 0)
		{
			//CacheListEvict(listLen, slot);
			CacheListEvict(listLen);
		}
		*/

		if (firstCache == lastCache)
		{
			firstCache->c_next = newcnode;
			newcnode->c_prev = firstCache;
			newcnode->c_next = NULL;
			lastCache = newcnode;
		}
		else
		{
			firstCache->c_next->c_prev = newcnode;
			newcnode->c_next = firstCache->c_next;
			newcnode->c_prev = firstCache;
			firstCache->c_next = newcnode;
		}
		blockUnUsed -= NumOfBlock(listLen);
		//MM->InsertData(&(newcnode->m_start_node), termid, listLen, file_offset);
		//cf_unlock_mutex();

		MM->InsertData(&(newcnode->m_start_node), termid, listLen, file_offset);
		/// Should this statement inside the cf_lock?
	}
	ht_unlock(slot);
	// In fact, here will be a tiny time gap,
	// but i don't think it will cause any problem.
	// Since the termid has *just* been inserted into the CacheFrame,
	// it won't be evicted at this time gap.
	ht_rdlock(slot);

	return ;
}

void CCacheFrame::CacheListUpdate(cachenode_t *pcur)
{
	cf_lock_mutex();
	if (firstCache->c_next != pcur)
	{
		if (pcur != lastCache)
			pcur->c_next->c_prev = pcur->c_prev;
		else
			lastCache = pcur->c_prev;
		pcur->c_prev->c_next = pcur->c_next;

		pcur->c_next = firstCache->c_next;
		firstCache->c_next->c_prev = pcur;
		firstCache->c_next = pcur;
		pcur->c_prev = firstCache;
	}
	else
	{
		/* firstCache -> pcur -> ... ... ... -> NULL
		 *                 ^              ^
		 *                 |      or      |
		 *             lastCache      lastCache
		 */
		// nothing to do here
	}
	cf_unlock_mutex();
	return ;
}

void CCacheFrame::CacheListEvict(int cur_len)
//void CCacheFrame::CacheListEvict(int cur_len, int ori_slot)
{
	unsigned int termid;
	int listlen;
	cachenode_t *temp;
	blockunit_t *startblock;
	hashnode_t *hn, *cur_hnode;

	//cf_lock_mutex();
	while (IsSpaceEnough(cur_len) == 0)
	{
		termid = lastCache->m_termid;
		listlen = lastCache->m_listlen;
		startblock = lastCache->m_start_node;

		int slot = HashKey(termid);
		hn = hashTable[slot];

		ht_wrlock(slot);
		while ((cur_hnode = hn->h_next) != NULL)
		{
			if (cur_hnode->m_key == termid)
				break;
			hn = hn->h_next;
		}

		if (cur_hnode == NULL)
		{
			// In fact, this branch will never be reached
			ht_unlock(slot);
		}
		else
		{
			hn->h_next = cur_hnode->h_next;
			freeResource(cur_hnode);
			ht_unlock(slot);
			
			int nBlock = NumOfBlock(listlen);
			blockUnUsed += nBlock;
			MM->EvictData(startblock, nBlock);
		}

		lastCache->c_prev->c_next = NULL;
		temp = lastCache->c_prev;
		freeResource(lastCache);
		lastCache = temp;
	}
	//cf_unlock_mutex();

	return ;
}

unsigned int CCacheFrame::GetElementFromPMemory(unsigned long long element_index)
{
	return MM->GetPMemory(element_index);
}

unsigned long long CCacheFrame::GetBaseOfBlock(unsigned int block_index)
{
	return MM->GetOffsetArray(block_index);
}

void CCacheFrame::cf_lock_mutex()
{
	pthread_mutex_lock(&cache_mutex);
}

void CCacheFrame::cf_unlock_mutex()
{
	pthread_mutex_unlock(&cache_mutex);
}
