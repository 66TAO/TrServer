#pragma once
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <stdint.h>
#include <assert.h>
#include <string.h>
using namespace std;

class Buffer
{
public:
	int read(int fd);
	std::string getmessage()
	{
		return message;
	}
private:
	char RX_buf[2048];
	string message;
};

