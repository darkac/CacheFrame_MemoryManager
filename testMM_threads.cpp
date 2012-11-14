// Last modified: 2012-10-18 17:32:55
 
/**
 * @file: testMM.cpp
 * @author: tongjiancong(lingfenghx@gmail.com)
 * @date:   2012-08-28 18:23:34
 * @brief: 
 **/
 
/* Talk is cheap, show me the code. */
/* Good luck and have fun. */
 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sys/time.h>

#include "hash.h"
#include "function.h"
#include "ListHandler.h"
#include "MemoryDict.h"
#include "MemoryManager.h"
#include "CacheFrame.h"

using namespace std;

#define THREAD_NUM 1000
#define TERM_NUM 10000000

FILE *pIndex;
MemoryDict dict;
hashnode_t *hashTable[MAX_HASH];
CCacheFrame *CF;
pthread_mutex_t ot_mutex = PTHREAD_MUTEX_INITIALIZER;

void LoadDict()
{
	string strDictDir = "/home/fan/Index/ori/";
	string strDictName = "GOV2.Rand";
	bool bRet = dict.Load(strDictDir, strDictName);

	if(!bRet) {
		cout << "Error in reading the dict" << endl;
		return ;
	}
	string file =strDictDir + "/" + strDictName + ".index";
	pIndex = fopen(file.c_str(), "rb");
	assert(pIndex != NULL);
}

void *run(void *arg)
{
	unsigned int range = 1024;
	int tno = *(int *)arg;
	unsigned int docid1, docid2, val1, val2;
	
	for (int i = 0; i < TERM_NUM / THREAD_NUM; ++i)
	{
		int termid =  TERM_NUM / THREAD_NUM * tno + i;
		ListHandler *LH = new ListHandler(termid, dict.m_vecDict[termid]);
		if (LH->GetLength() >= range)
		{
			docid1 = 1023;
			val1 = LH->GetItem(docid1);
			docid2 = 1024;
			val2 = LH->GetItem(docid2);
			pthread_mutex_lock(&ot_mutex);
			cout << termid << "\t" << docid1 << "\t-\t" 
				<< val1 << "\t" << LH->GetLength() << endl;
			cout << termid << "\t" << docid2 << "\t-\t" 
				<< val2 << "\t" << LH->GetLength() << endl;
			pthread_mutex_unlock(&ot_mutex);
		}
		delete LH;
	}

	return (void *)0;
}

int main()
{
	init_hash();

	LoadDict();

	CF = new CCacheFrame(MEMORYSIZE);

	pthread_t tid[THREAD_NUM];
	int arg[THREAD_NUM];
	for (int i = 0; i < THREAD_NUM; ++i)
	{
		arg[i] = i;
		pthread_create(&tid[i], NULL, run, &arg[i]);
	}
	for (int i = 0; i < THREAD_NUM; ++i)
	{
		pthread_join(tid[i], NULL);
	}

	int docid = 1024;
	int listLength, offset;
	int *space = (int *)malloc(1024);
	for (int termid = 0; termid < TERM_NUM; ++termid)
	{
		listLength = dict.m_vecDict[termid].m_nFreq;
		offset = dict.m_vecDict[termid].m_nOffset;
		space = (int *)realloc(space, listLength * sizeof(int));
		fseek(pIndex, offset, SEEK_SET);
		fread(space, sizeof(int), listLength, pIndex);

		if (listLength >= docid)
		{
			docid = 1023;
			cerr << termid << "\t" << docid << "\t-\t" 
				<< space[docid - 1] << "\t" << listLength << endl;
			docid = 1024;
			cerr << termid << "\t" << docid << "\t-\t" 
				<< space[docid - 1] << "\t" << listLength << endl;
		}
	}
	freeResource(space);
	
	delete CF;
	free_hash();

	return 0;
}
