// Last modified: 2012-10-17 21:06:35
 
/**
 * @file: CacheFrame.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-09-26 16:00:21
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _CACHEFRAME_H_
#define _CACHEFRAME_H_

#include <pthread.h>
#include "hash.h"
#include "MemoryManager.h"

typedef struct cachenode {
	int m_termid;
	int m_listlen;
	struct cachenode *c_prev;
	struct cachenode *c_next;
	blockunit_t *m_start_node;
} cachenode_t;

class CCacheFrame
{
private:
	cachenode_t *firstCache, *lastCache;
	pthread_mutex_t cache_mutex;

	unsigned long long cacheCapacity;
	unsigned int blockUnUsed;
	MemoryManager *MM;

public:
	CCacheFrame(int _size);
	~CCacheFrame();

	cachenode_t *get_first() const;
	cachenode_t *get_last() const;

	unsigned int NumOfBlock(const unsigned int listLen) const;
	bool ExistInCache(const unsigned int temrid) const;
	bool IsSpaceEnough(const unsigned int listLen) const;

	void CacheListInsert(
			unsigned int termid, 
			unsigned int listLen, 
			unsigned long long file_offset);
	void CacheListUpdate(cachenode_t *pcur);
	void CacheListEvict(int cur_len);

	unsigned int GetElementFromPMemory(unsigned long long element_index);
	unsigned long long GetBaseOfBlock(unsigned int block_index);

	void cf_lock_mutex();
	void cf_unlock_mutex();

};
extern CCacheFrame *CF;
 
#endif
