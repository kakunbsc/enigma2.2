#include <lib/dvb/ftest.h>
#include <lib/base/eerror.h>

eDVBFactoryTests::eDVBFactoryTests()
{
	int locked = 0;
}
eDVBFactoryTests::~eDVBFactoryTests()
{
	;
}

int eDVBFactoryTests::test(int index)
{
	eDebug("eDVBFactoryTests.test!!!!! %d",index);
	return index+1;
}

