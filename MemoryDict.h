#ifndef _MEMORYDICT_H_
#define _MEMORYDICT_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class DictItem
{
public:
	long long unsigned m_nOffset;
	unsigned int m_nFreq;
	//string m_strTerm;
};


class MemoryDict
{
public:
	MemoryDict();
	bool Load(const string& strDir, const string& strName);
	bool Save(const string& strDir, const string& strName);
	bool AddTerm(const string& strTerm, unsigned int& nFreq);
	bool Clear();
	bool Close();
	vector <DictItem> m_vecDict;

protected:
	unsigned long long m_nOffset;
};

#endif
