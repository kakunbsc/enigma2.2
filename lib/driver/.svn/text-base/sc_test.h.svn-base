#ifdef BUILD_VUPLUS /* ikseong  */
#ifndef __sc_test_h
#define __sc_test_h

#include <lib/base/object.h>
#include <lib/python/connections.h>

class eSctest
{
	static eSctest *instance;
protected:
	int VFD_fd;
	
public:
	eSctest();
	~eSctest();
	int check_smart_card(char *dev_name);
	int eject_smart_card(char *dev_name);
	int VFD_Open();
	void VFD_Close();
	int turnon_VFD();
	int turnoff_VFD();
	int getFrontendstatus(int fe);
	
	static eSctest *getInstance() { return instance; }
};
#endif
#endif

