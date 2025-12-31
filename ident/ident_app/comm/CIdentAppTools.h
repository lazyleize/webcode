#ifndef CGYPAYAPP_TOOLS_H_
#define CGYPAYAPP_TOOLS_H_

#include "tools/commdef.h"

namespace CIdentAppTools
{
		void DelMapF(CStr2Map& dataMap,CStr2Map& outMap);
		string GetTransConfigNotEmpty(const string & tid,const string & attrName);
		string GetFormatDateNow();
};
#endif
