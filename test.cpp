#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "hiredis/hiredis.h"
#include "hiredis/hircluster.h"
#include "hiredis/async.h"
#include "hiredis/adapters/libevent.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "libevent_core.lib")
#pragma comment(lib, "libevent_extras.lib")

struct event_base *base;

using namespace std;

#if _MSC_VER>=1900  
#include "stdio.h"   
_ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned);
#ifdef __cplusplus   
extern "C"
#endif   
FILE* __cdecl __iob_func(unsigned i) {
	return __acrt_iob_func(i);
}
#endif /* _MSC_VER>=1900 */  


void PrintTime()
{
	SYSTEMTIME sys;
	GetLocalTime( &sys );
	printf( "%d.%d.%d\n", sys.wMinute, sys.wSecond, sys.wMilliseconds);
}

ofstream fout;

redisClusterContext* connRedisCluster(const char* addr)
{
	redisClusterContext *cc = redisClusterConnect(addr, HIRCLUSTER_FLAG_NULL);
	if (cc != NULL && cc->err) {
		printf("Error: %s\n", cc->errstr);
	}
		
	return cc;
}

void queryRedis(redisClusterContext *cc, std::vector<std::string> res)
{
	redisReply * reply;
    
	reply = (redisReply *)redisClusterCommand(cc, "zrangebyscore 15_lvl %s %s", res.at(0).c_str(), res.at(1).c_str());
	
	if (reply == NULL)
	{
		redisClusterFree(cc);
		return;
	}

	for (int i = 0; i < reply->elements; i++)
	{
		fout << reply->element[i]->str;
		cout << "running......" << endl;
	}
		
	freeReplyObject(reply);

}


std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


int main(int argc, char *argv[])
{
	char szinput[256] = { 0 };//1.xyz
	char szoutput[256] = { 0 };
	char lvl[10] = { 0 };

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0)
		{
			i++;
			strcpy(szinput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-o") == 0)
		{
			i++;
			strcpy(szoutput, argv[i]);
			continue;
		}

		if (strcmp(argv[i], "-l") == 0)
		{
			i++;
			strcpy(lvl, argv[i]);
			continue;
		}
	}


	redisClusterContext *cc;
	cc = connRedisCluster("192.168.0.20:7006,192.168.0.27:7000,192.168.0.28:7001,192.168.0.29:7002,192.168.0.31:7004,192.168.0.32:7005");

	ifstream fin(szinput);  //"E:/pyProject/hiredis-vip-win/range.txt"
	fout.open(szoutput);

	if (!fin.is_open() || !fout.is_open())
	{
		cout << "open file failed.	" << endl
			<< "programing terminate." << endl;
		return(EXIT_FAILURE);
	}

	string s;
	int count = 0;
	while (getline(fin, s))
	{		
		std::vector<std::string> res = split(s, ",");
		redisClusterAppendCommand(cc, "zrangebyscore %s %s %s",lvl, res.at(0).c_str(), res.at(1).c_str());
		count++;
		//queryRedis(cc, res);
	}
	redisReply * reply;
	for (int i = 0; i < count; i++)
	{
		redisClusterGetReply(cc, (void **)&reply);
		for (int j = 0; j < reply->elements; j++)
		{
			fout << reply->element[j]->str;
		}
		freeReplyObject(reply);
	}
	redisClusterReset(cc);
	fin.close();
	fout.close();

	redisClusterFree(cc);
	
	return 0;
}
