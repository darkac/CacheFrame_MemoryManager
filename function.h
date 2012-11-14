// Last modified: 2012-10-12 20:34:32
 
/**
 * @file: function.h
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-28 20:11:29
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <cstdio>
#include <pthread.h>

extern FILE *pIndex;

void checkPointer(void *p, int line);

void freeResource(void *pointer);

void closeResource(FILE *fHandler);

void testPoint(int line);

void rdlock_rwlock(pthread_rwlock_t *rwlock);
void wrlock_rwlock(pthread_rwlock_t *rwlock);
void unlock_rwlock(pthread_rwlock_t *rwlock);

#endif
