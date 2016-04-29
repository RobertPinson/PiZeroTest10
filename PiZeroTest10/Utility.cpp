#include "utility.h"
#include <string>
#include <stdio.h>
#include <string.h>

using namespace std;
	
namespace utility
{
	string hexstring(const uint8_t *pbtData, const size_t szBytes)
	{
		size_t  szPos;
		char hexstr[201];
 
		for (szPos = 0; szPos < szBytes; szPos++) {
			sprintf(hexstr + szPos * 3, "%02x", pbtData[szPos]);
			if (szPos + 1 != szBytes)
				hexstr[strlen(hexstr)] = '-';
		}
		
		return string(hexstr);
	}
}