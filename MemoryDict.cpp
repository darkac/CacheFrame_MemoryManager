#include "MemoryDict.h"

MemoryDict::MemoryDict()
{
	Clear();
}


bool MemoryDict::Load(const string& strDir, const string& strName)
{
	string strFile = strDir + strName + ".dict";
	ifstream stream(strFile.c_str(), ios::binary);
	printf("--> <%s>\n", strFile.c_str());
	if(!stream.is_open()) {
		cout << "Dict not found" << endl;
		return false;
	}
	unsigned int nTerm = 0;
	stream.read((char*) &nTerm, sizeof(nTerm));
	m_vecDict.resize(nTerm);

	for(unsigned int iTerm = 0; iTerm < nTerm; iTerm++)
	{
		unsigned long long nOffset;
		unsigned int nFreq;
		unsigned int nTermLength;

		stream.read((char*) &nOffset, sizeof(nOffset));
		stream.read((char*) &nFreq, sizeof(nFreq));
		stream.read((char*) &nTermLength, sizeof(nTermLength));
		
		m_vecDict[iTerm].m_nFreq = nFreq;
		m_vecDict[iTerm].m_nOffset = nOffset;
		
		char szBuf[4096];
		stream.read(szBuf, nTermLength);
		//m_vecDict[iTerm].m_strTerm.assign(szBuf, nTermLength);
	}
	stream.close();

	return true;
}

bool MemoryDict::Close()
{
	Clear();
	return true;
}

bool MemoryDict::Save(const string& strDir, const string& strName)
{
	string strFile = strDir + "/" + strName + ".dict";
	ofstream stream(strFile.c_str(), ios::binary);
	unsigned int nTerm = (unsigned int) m_vecDict.size();
	stream.write((char*) &nTerm, sizeof(nTerm));

	for(unsigned int iTerm = 0; iTerm < nTerm; iTerm++)
	{
		unsigned long long nOffset = m_vecDict[iTerm].m_nOffset;
		unsigned int nFreq = m_vecDict[iTerm].m_nFreq;
		//unsigned int nTermLength = (unsigned int) m_vecDict[iTerm].m_strTerm.length();

		stream.write((char*) &nOffset, sizeof(nOffset));
		stream.write((char*) &nFreq, sizeof(nFreq));
		//stream.write((char*) &nTermLength, sizeof(nTermLength));
		//stream.write(m_vecDict[iTerm].m_strTerm.c_str(), nTermLength);
	}
	stream.close();

	return true;
}

bool MemoryDict::Clear()
{
	m_nOffset = 0;
	m_vecDict.clear();
	
	return true;
}

bool MemoryDict::AddTerm(const string& strTerm, unsigned int& nFreq)
{
	DictItem item;
	item.m_nFreq = nFreq;
	item.m_nOffset = m_nOffset;
	//item.m_strTerm = strTerm;
	m_nOffset += nFreq * sizeof(int);

	m_vecDict.push_back(item);

	return true;
}
