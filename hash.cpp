// Last modified: 2012-10-17 21:10:18
 
/**
 * @file: hash.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-28 20:21:45
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hash.h"
#include "function.h"

/*
inline int HashKey(const unsigned int key)
{
	return key % MAX_HASH;
}
*/

/*
int HashKey(const char *str)
{
	unsigned long h = 0, g;
	while(*str)
	{
		h = (h << 4) + *str++;
		if((g = h & 0xf0000000))
			h ^= g >> 24;
		h &= ~g;
	}
	return (int)(h % MAX_HASH);
}
*/

void init_hash()
{
	for (unsigned int i = 0; i < MAX_HASH; ++i)
	{
		hashTable[i] = (hashnode_t *)malloc(sizeof(hashnode_t));
		memset(hashTable[i], 0, sizeof(hashnode_t));
		if (pthread_rwlock_init(&hashTable[i]->m_rwlock, NULL) != 0)
		{
			printf("%u-th rwlock_hash init failed.\n", i);
			exit(-1);
		}
	}
}

void free_hash()
{
	for (unsigned int i = 0; i < MAX_HASH; ++i)
	{
		hashnode_t *temp = hashTable[i]->h_next;
		hashnode_t *todel;
		while (temp != NULL)
		{
			todel = temp;
			temp = temp->h_next;
			freeResource(todel);
		}
		pthread_rwlock_destroy(&hashTable[i]->m_rwlock);
		freeResource(hashTable[i]);
	}
}

hashnode_t *getHashNode(unsigned int termid)
{
	int slot = HashKey(termid);
	hashnode_t *p = hashTable[slot];
	hashnode_t *q;
	while ((q = p->h_next) != NULL)
	{
		if (q->m_key == termid)
			break;
		p = p->h_next;
	}
	return q;
}

int existInHash(unsigned int termid)
{
	hashnode_t *tmp = getHashNode(termid);
	return (tmp != NULL) ? 1 : 0;
}

void ht_rdlock(unsigned int slot)
{
	pthread_rwlock_rdlock(&hashTable[slot]->m_rwlock);
}

void ht_wrlock(unsigned int slot)
{
	pthread_rwlock_wrlock(&hashTable[slot]->m_rwlock);
}

void ht_unlock(unsigned int slot)
{
	pthread_rwlock_unlock(&hashTable[slot]->m_rwlock);
}
