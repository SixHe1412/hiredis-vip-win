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
	GetLocalTime(&sys);
	printf("%d.%d.%d\n", sys.wMinute, sys.wSecond, sys.wMilliseconds);
}


redisClusterContext* connRedisCluster(const char* addr)
{
	redisClusterContext *cc = redisClusterConnect(addr, HIRCLUSTER_FLAG_NULL);
	if (cc != NULL && cc->err) {
		printf("Error: %s\n", cc->errstr);
		// handle error
	}

	return cc;
}

std::vector<std::string> split(std::string str, std::string pattern)
{

	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();
	//int size = strlen(str);

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

	FILE *fin = NULL;  //"E:/pyProject/hiredis-vip-win/range.txt"
	if (strlen(szinput) != 0)
	{
		fin = fopen(szinput, "r");
	}
	else
	{
		fin = stdin;
	}

	std::ostream* out_s;
	ofstream fout;
	if (strlen(szoutput) != 0)
	{
		fout.open(szoutput);
		out_s = &fout;
	}
	else
	{
		out_s = &cout;
	}


	char line[50] = { 0 };
	string s;
	int count = 0;
	while (1)
	{
		if (fgets(line, 1024, fin) == NULL) break;
		s = line;
		s = s.substr(0, s.length() - 1);
		std::vector<std::string> res = split(s, ",");
		redisClusterAppendCommand(cc, "zrangebyscore %s %s %s", lvl, res.at(0).c_str(), res.at(1).c_str());
		count++;
		//queryRedis(cc, res);
	}
	redisReply * reply;
	for (int i = 0; i < count; i++)
	{
		redisClusterGetReply(cc, (void **)&reply);
		for (int j = 0; j < reply->elements; j++)
		{
			(*out_s) << reply->element[j]->str;
		}
		freeReplyObject(reply);
	}
	redisClusterReset(cc);
	//fin.close();
	//fout.close();

	redisClusterFree(cc);


	return 0;
}
