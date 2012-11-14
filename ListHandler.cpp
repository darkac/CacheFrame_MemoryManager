// Last modified: 2012-10-19 20:13:01
 
/**
 * @file: ListHandler.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-29 00:55:58
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/time.h>

#include "hash.h"
#include "function.h"
#include "CacheFrame.h"
#include "ListHandler.h"
#include "MemoryManager.h"

ListHandler::ListHandler(unsigned _termid, DictItem _item)
{
	termid = _termid;
	listLength = _item.m_nFreq;
	offset = _item.m_nOffset;
	lastAccess = NULL;

	int slot = HashKey(termid);
	ht_rdlock(slot); // wiil be unlock in the ~ListHandler()
	hashnode_t *tmp = getHashNode(termid);
	if (tmp != NULL)
	{
		cachenode_t *pcur = tmp->m_cache_node;
		CF->CacheListUpdate(pcur);
	}
	else
	{
		ht_unlock(slot);
		CF->cf_lock_mutex();
		if (CF->IsSpaceEnough(listLength) == 0)
		{
			CF->CacheListEvict(listLength);
		}
		CF->CacheListInsert(termid, listLength, offset);
		CF->cf_unlock_mutex();
	}
}

ListHandler::~ListHandler()
{
	lastAccess = NULL;
	
	int slot = HashKey(termid);
	ht_unlock(slot);
}

blockunit_t *ListHandler::GetStartNode()
{
	if (NULL == lastAccess)
	{
		int slot = HashKey(termid);
		hashnode_t *p = hashTable[slot], *q;
		while ((q = p->h_next) != NULL)
		{
			if (q->m_key == termid)
				break;
			p = p->h_next;
		}
		assert(q != NULL);
		return q->m_cache_node->m_start_node;
	}
	else
	{
		return lastAccess;
	}
}

unsigned int ListHandler::GetItem(unsigned int itemID)
{
	assert(itemID >= 0);
	
	lastAccess = GetStartNode();

	unsigned int lastIndex = lastAccess->m_index_List;
	
	if (itemID > (lastIndex - 1) * BLOCKSIZE)
	{
		unsigned int hops =  ((itemID + BLOCKSIZE - 1) / BLOCKSIZE) - lastIndex;
		while (hops-- > 0)
		{
			lastAccess = lastAccess->b_next;
		}
	}
	else
	{
		unsigned int hops = lastIndex - ((itemID + BLOCKSIZE - 1) / BLOCKSIZE);
		while (hops-- > 0)
		{
			lastAccess = lastAccess->b_prev;
		}
	}

	unsigned int blockid = lastAccess->m_index_Global;
	unsigned int id_in_block = (itemID % BLOCKSIZE == 0) ? BLOCKSIZE : (itemID % BLOCKSIZE);
	return CF->GetElementFromPMemory(CF->GetBaseOfBlock(blockid) + id_in_block - 1);
}

unsigned long long ListHandler::GetOffset()
{
	return offset;
}

unsigned long long ListHandler::GetLength()
{
	return listLength;
}
