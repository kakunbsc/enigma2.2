#include <lib/dvb/dvb.h>
#include <lib/dvb/frontendparms.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <lib/driver/sc_test.h>
#include <errno.h>
#include <lib/base/eerror.h>
#include <lib/base/estring.h>

#include <stdint.h>
#include <sys/time.h>
#include <poll.h>


#define SC_READWAIT_TIMEOUT	5
#define SC_REMOVE_TIMEOUT	10


#define SMART_CARD0	"/dev/sci0"			/* upper smart card */
#define SMART_CARD1	"/dev/sci1"			/* lower smart card */

#define SC_SUCCESS			0
#define NO_DEV_FOUND		-1
#define SC_NOT_INSERTED	-2
#define SC_NOT_VALID_ATR	-3
#define SC_NOT_REMOVED		-4
#define SC_READ_TIMEOUT	-5

eSctest *eSctest::instance;

eSctest::eSctest()
{
	instance = this;

}

eSctest::~eSctest()
{
	instance=NULL;

}


int eSctest::check_smart_card(char *dev_name)
{
	int fd;
	struct pollfd pollfd;
	unsigned char buf[64];
	int cnt = 0;
	int modem_status;
	int count = SC_READWAIT_TIMEOUT;
	
	fd = ::open(dev_name, O_RDWR);
	
	if(fd < 0){
		printf("sci0 open error\n");
		return NO_DEV_FOUND;
	}
	else
		printf("sci0 is opened fd : %d\n", fd);

	::tcflush(fd, TCIFLUSH);

	::ioctl(fd, TIOCMGET, &modem_status);

	if( modem_status & TIOCM_CAR)
		printf("card is now inserted\n");
	else
	{
		printf("card is NOT inserted\n");
		close(fd);
		return SC_NOT_INSERTED;
	}

	/* now smart card is inserted, let's do reset */

	modem_status |= TIOCM_RTS;
	::ioctl(fd, TIOCMSET, &modem_status);

	modem_status &= ~TIOCM_RTS;
	::ioctl(fd, TIOCMSET, &modem_status);


	/* now we can get the ATR */

	pollfd.fd = fd;
	pollfd.events = POLLIN|POLLOUT|POLLERR|POLLPRI;


	while(poll(&pollfd, 1, 1000)>=0 && count--){

		printf("pollfd.revents : 0x%x %d\n", pollfd.revents,count);
		if(pollfd.revents & POLLIN){
			printf(">>read \n");
			cnt = ::read(fd, buf, 64);
			printf("<<read cnt:%d\n", cnt);			
			if(cnt) 
			{
				int i;
				for( i = 0 ; i < cnt ; i ++)
					printf("read : 0x%x\n", buf[i]);
				break;
			}
			else
				printf("no data\n");
		}
	}
	
	::close(fd);		
	if(count<=0 ) return SC_READ_TIMEOUT;
	return SC_SUCCESS;
}
int eSctest::eject_smart_card(char *dev_name)
{
	int fd;
	struct pollfd pollfd;
	unsigned char buf[64];
	int cnt = 0;
	int modem_status;
	int count = SC_READWAIT_TIMEOUT;
	
	fd = ::open(dev_name, O_RDWR);
	
	if(fd < 0){
		printf("sci0 open error\n");
		return NO_DEV_FOUND;
	}
	else
		printf("sci0 is opened fd : %d\n", fd);

	::tcflush(fd, TCIFLUSH);

	::ioctl(fd, TIOCMGET, &modem_status);

	if( modem_status & TIOCM_CAR)
		printf("card is now inserted\n");
	else
	{
		printf("card is NOT inserted\n");
		close(fd);
		return SC_NOT_INSERTED;
	}
	/* now we can get the ATR */

	pollfd.fd = fd;
	pollfd.events = POLLIN|POLLOUT|POLLERR|POLLPRI;

	/* let's wait until card is removed for count secs.*/
	count = SC_REMOVE_TIMEOUT;	
	do{
		::ioctl(fd, TIOCMGET, &modem_status);
		printf("modem_status : 0x%x %d\n", modem_status,count);
		sleep(1);	

	}
	while((modem_status&TIOCM_CAR) && count--);

	if(count<=0 ) return SC_NOT_REMOVED;
	
	::close(fd);		
	return SC_SUCCESS;
}


int eSctest::VFD_Open()
{
	VFD_fd = open("/dev/dbox/lcd0", O_RDWR);
	return VFD_fd;
}

int eSctest::turnon_VFD()
{ 
	ioctl(VFD_fd, 0xa0a0a0a0, 0);
}

int eSctest::turnoff_VFD()
{
	ioctl(VFD_fd, 0x01010101, 0);	
}

void eSctest::VFD_Close()
{
	close(VFD_fd);
}

extern int frontend0_fd;
extern int frontend1_fd;

int eSctest::getFrontendstatus(int fe)
{
	fe_status_t status;

	int m_fd;
	int res;

	if (fe == 0)
		m_fd = frontend0_fd;
	else if (fe==1)
		m_fd = frontend1_fd;
	else 
		return -1;

	if (m_fd < 0)
	{
		eDebug("%d open error \n",fe);
		return -1;
	}
	else
		eDebug("%d open ok!!!! \n",m_fd);
	
	if ( ioctl(m_fd, FE_READ_STATUS, &status) < 0)
		eDebug("%d read error \n",fe);		

	if (status&FE_HAS_LOCK)
		return 1;
	else
		return 0;

}

