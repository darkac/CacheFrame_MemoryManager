// Last modified: 2012-10-17 21:09:52
 
/**
 * @file: hash.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-28 20:19:11
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _HASH_H_
#define _HASH_H_

#include <pthread.h>

#define MAX_HASH 450000

typedef struct hashnode
{
	unsigned int m_key;
	unsigned int m_listLength;
	struct cachenode *m_cache_node;
	struct hashnode *h_next;
	pthread_rwlock_t m_rwlock;
} hashnode_t;
extern hashnode_t *hashTable[MAX_HASH];

//int HashKey(const unsigned int key);
inline int HashKey(const unsigned int key)
{
	return key % MAX_HASH;
}


//int HashKey(const char *key);

void init_hash();

void free_hash();

int existInHash(unsigned int termid);

hashnode_t *getHashNode(unsigned int termid);

void ht_rdlock(unsigned int slot);
void ht_wrlock(unsigned int slot);
void ht_unlock(unsigned int slot);
 
#endif
