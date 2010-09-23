from Screens.Screen import Screen
from Screens.MessageBox import MessageBox
from Plugins.Plugin import PluginDescriptor
from Components.MenuList import MenuList
from Tools.Directories import fileExists
from Components.ServiceList import ServiceList
from Components.ActionMap import ActionMap
from Components.config import config
from os import system
from Components.Label import Label
from Components.AVSwitch import AVSwitch
from time import sleep
from Components.Console import Console
from enigma import eTimer
from Components.HTMLComponent import HTMLComponent
from Components.GUIComponent import GUIComponent
from enigma import eListboxPythonStringContent, eListbox, gFont, eServiceCenter, eDVBResourceManager
from enigma import eServiceReference
from enigma import eMemtest
from enigma import eSctest
from enigma import eDVBDB
from Components.NimManager import nimmanager
from enigma import eDVBCI_UI,eDVBCIInterfaces

class TestResultList(HTMLComponent, GUIComponent):
	def __init__(self, source):
		GUIComponent.__init__(self)
		self.l = eListboxPythonStringContent()
		self.list = source
		self.l.setList(self.list)
		self.l.setFont( gFont("Regular", 23))
		self.l.setItemHeight(25)

	GUI_WIDGET = eListbox

	def postWidgetCreate(self, instance):
		self.instance.setSelectionEnable(0)
		self.instance.setContent(self.l)

	def updateList(self,list):
		self.l.setList(list)

class FactoryTest(Screen):
	skin = """
		<screen position="120,125" size="440,400" title="Test Menu" >
			<widget name="testlist" position="10,0" size="340,350" />
			<widget name="resultlist" position="370,0" size="60,350" />
			<widget name="testdate" position="20,350" size="150,25" font="Regular;22" />
			<widget name="testversion" position="20,375" size="150,25" font="Regular;22" />
			<widget name="mactext" position="180,350" size="230,25" font="Regular;22" />			
		</screen>"""
	def __init__(self, session):

		self["actions"] = ActionMap(["OkCancelActions","WizardActions"],
		{
			"ok": self.TestAction,
			"cancel": self.keyCancel,
			"agingstart": self.Agingmode,
		}, -2)

		Screen.__init__(self, session)
		TESTPROGRAM_DATE = "2009-12-09"
		TESTPROGRAM_VERSION = "Version 00.01"
		self["testdate"]=Label((TESTPROGRAM_DATE))
		self["testversion"]=Label(("Loading version..."))
		self["mactext"]=Label(("Loading mac address..."))
		nimConfig = nimmanager.getNimConfig(0)
		nimConfig.configMode.slot_id=0
		nimConfig.configMode.value= "simple"
		nimConfig.diseqcMode.value="diseqc_a_b"
		nimConfig.diseqcA.value="160"
		nimConfig.diseqcB.value="100"
		nimConfig = nimmanager.getNimConfig(1)
		nimConfig.configMode.slot_id=1		
		nimConfig.configMode.value= "simple"
		nimConfig.diseqcMode.value="diseqc_a_b"
		nimConfig.diseqcA.value="130"
		nimConfig.diseqcB.value="192"
		nimmanager.sec.update()		
		
		system("cp /usr/lib/enigma2/python/Plugins/SystemPlugins/FactoryTest/testdb /etc/enigma2/lamedb")
		db = eDVBDB.getInstance()
		db.reloadServicelist()

		tlist = []
		tlist.append((" 0. sata & extend hdd test",0))
		tlist.append((" 1. Front test",1))
		tlist.append((" 2. Smartcard test",2))
		tlist.append((" 3. T1 H 22K x 4:3 CVBS",3))
		tlist.append((" 4. T1 V 22k o 16:9 RGB",4))
		tlist.append((" 5. T2 H 22k x 4:3 YC",5))
		tlist.append((" 6. T2 V 22k o 16:9 CVBS",6))
		tlist.append((" 7. VCR Scart loop",7))
		tlist.append((" 8. rs232 test",8))
		tlist.append((" 9. usb test",9))
		tlist.append(("10. ethernet & mac test",10))
#		tlist.append(("11. DRAM test",11))
#		tlist.append(("12. Flash test",12))
#		tlist.append(("13. DRAM+Flash test",13))
		tlist.append(("11. factory default",11))
		tlist.append(("12. shutdown",12))
		self["testlist"] = MenuList(tlist)
		self.rlist = []
#		for x in range(15):
		for x in range(12):
			self.rlist.append((".."))
		self["resultlist"] = TestResultList(self.rlist)
		self.NetworkState = 0
		self.first = 0

		self.avswitch = AVSwitch()
		self.memTest = eMemtest()
		self.scTest= eSctest()
		
		self.feid=0

		self.servicelist = ServiceList()
		self.oldref = session.nav.getCurrentlyPlayingServiceReference()
		print "oldref",self.oldref
		session.nav.stopService() # try to disable foreground service
		
		self.tunemsgtimer = eTimer()
		self.tunemsgtimer.callback.append(self.tunemsg)

		self.camstep = 1
		self.camtimer = eTimer()
		self.camtimer.callback.append(self.cam_state)
		self.getmacaddr()
		self.getversion()
		
		self.tunerlock = 0
		self.tuningtimer = eTimer()
		self.tuningtimer.callback.append(self.updateStatus)

	def updateStatus(self):
		index = self["testlist"].getCurrent()[1]
		if index ==2 or index==3:
			tunno = 1
			result = eSctest.getInstance().getFrontendstatus(0)
		else:
			tunno = 2
			result = eSctest.getInstance().getFrontendstatus(1)		
		if index == 2 or index==4:
			hv = "Hor"
		else:
			hv = "Ver"
			
		print "eSctest.getInstance().getFrontendstatus - %d"%result
		if result == 0:
			self.tunerlock = 0
			self.tunemsgtimer.stop()
			self.session.nav.stopService()
			self.session.open( MessageBox, _("Tune%d %s Locking Fail..."%(tunno,hv)), MessageBox.TYPE_ERROR)	
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
		else :
			self.tunerlock = 1

	def getversion(self):
		try:
			fd = open("/proc/stb/info/version","r")
			version = fd.read()
			self["testversion"].setText(("Version %s"%version))
		except:
			self["testversion"].setText(("Version no load"))
			

	def readmac(self, result, retval,extra_args=None):
		(statecallback) = extra_args
		if self.macConsole is not None:
			if retval == 0:
				self.macConsole = None
				content =result.split()
				self["mactext"].setText(("MAC : "+content[10]))
 	
	def getmacaddr(self):
		try:
			cmd = "ip -o addr"
			self.macConsole = Console()	
			self.macConsole.ePopen(cmd, self.readmac)	
#			self["stattext"].setText((macaddr))
		except:
			return
		
	def TestAction(self):
		print "line - ",self["testlist"].getCurrent()[1]
		index = self["testlist"].getCurrent()[1]
		result = 0
		if index==0:
			self.Test0()
		elif index==1:
			self.Test1()
		elif index>2 and index<7:
			self.TestTune(index)
		elif index==7:
			self.Test6()
		elif index==8:
			self.Test7()
		elif index==9:
			self.Test8()
		elif index==10:
			self.Test9()
		elif index == 2:
			self.Test10()
#		elif index == 11:
#			self.Test11()
#		elif index ==12:
#			self.Test12()
#		elif index==13:
#			self.Test13()
		elif index==10:
			self.Test14()
		elif index==11:
			self.Test15()


	def Test0(self):
		result = 0
		checktab=0
		try:
			mtab = open('/etc/mtab','r')
			while(1):
				disk = mtab.readline().split(' ')
				if len(disk) < 2:
					break
				if disk[1].startswith('/media/hdd'):
					checktab+=1
				elif disk[1].startswith('/media/sdb1'):
					checktab+=10
				if checktab==11:
					break
		except:
			checktab = 0

		if checktab==0:
			self.session.open( MessageBox, _("Sata & extend hdd test error"), MessageBox.TYPE_ERROR)
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
			return
		elif checktab < 11:
			self.session.open( MessageBox, _("one hdd test error"), MessageBox.TYPE_ERROR)
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
			return

		try:
			if fileExists("/media/sdb1"):
				dummy=open("/media/sdb1/dummy03","w")
				dummy.write("complete")
				dummy.close()
				dummy=open("/media/sdb1/dummy03","r")
				if dummy.readline()=="complete":
					print "complete"
				else:
					result = 1
				dummy.close()
				system("rm /media/sdb1/dummy03")
			else:
				result = 1
		except:
			result = 1
		try:
			if fileExists("/media/hdd"):
				dummy=open("/media/hdd/dummy03","w")
				dummy.write("complete")
				dummy.close()
				dummy=open("/media/hdd/dummy03","r")
				if dummy.readline()=="complete":
					print "complete"
				else:
					result += 1
				dummy.close()
				system("rm /media/hdd/dummy03")
			else:
				result += 1
		except:
			result += 1
			
		if result ==0:
			self.session.open( MessageBox, _("Sata & extend hdd test pass"), MessageBox.TYPE_INFO)
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
		elif result == 1:
			self.session.open( MessageBox, _("one hdd test error"), MessageBox.TYPE_ERROR)
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
		else:
			self.session.open( MessageBox, _("Sata & extend hdd test error"), MessageBox.TYPE_ERROR)
			self.rlist[self["testlist"].getCurrent()[1]]="fail"

	def Test1(self):
		self.session.openWithCallback(self.displayresult ,FrontTest)

	def displayresult(self):
		global fronttest
		if fronttest == 1:
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
		else:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"

	INTERNAL_PID_STATUS_NOOP = 0
	INTERNAL_PID_STATUS_WAITING = 1
	INTERNAL_PID_STATUS_SUCCESSFUL = 2
	INTERNAL_PID_STATUS_FAILED = 3

	def TestTune(self,index):	
		if self.oldref is None:
			eref = eServiceReference("1:0:19:1324:3EF:1:C00000:0:0:0")
			serviceHandler = eServiceCenter.getInstance()
			servicelist = serviceHandler.list(eref)
			if not servicelist is None:
				ref = servicelist.getNext()
			else:
				ref = self.getCurrentSelection()
				print "servicelist none"
		else:
			ref = self.oldref
		self.session.nav.stopService() # try to disable foreground service
		if index==3:
			ref.setData(0,1)
			ref.setData(1,0x6D3)
			ref.setData(2,0x3)
			ref.setData(3,0xA4)
			ref.setData(4,0xA00000)
			self.session.nav.playService(ref)
			self.avswitch.setColorFormat(0)
			self.avswitch.setAspectRatio(0)
		elif index==4:
			ref.setData(0,0x19)
			ref.setData(1,0x83)
			ref.setData(2,0x6)
			ref.setData(3,0x85)
			ref.setData(4,0x640000)
			self.session.nav.playService(ref)
			self.avswitch.setColorFormat(1)
			self.avswitch.setAspectRatio(6)			
		elif index==5:
#			self.camstep = 1
#			self.camtimer.start(100,True)
			ref.setData(0,1)
			ref.setData(1,0x6D3)
			ref.setData(2,0x3)
			ref.setData(3,0xA4)
			ref.setData(4,0x820000)
			self.session.nav.playService(ref)
			self.avswitch.setColorFormat(2)			
			self.avswitch.setAspectRatio(0)			
		elif index==6:
			self.camstep = 1
			self.camtimer.start(100,True)
			ref.setData(0,0x19)
			ref.setData(1,0x83)
			ref.setData(2,0x6)
			ref.setData(3,0x85)
			ref.setData(4,0xC00000)
			self.session.nav.playService(ref)
			self.avswitch.setColorFormat(0)			
			self.avswitch.setAspectRatio(6)
		self.tuningtimer.start(2000,True)
		self.tunemsgtimer.start(3000, True)

	def cam_state(self):
		if self.camstep == 1:
			slot = 0
			state = eDVBCI_UI.getInstance().getState(slot)
			print '-1-stat',state
			if state > 0:
				self.camstep=2
				self.camtimer.start(100,True)
			else:
				self.session.nav.stopService()
				self.session.open( MessageBox, _("NO_CAM1_NOT_INSERTED"), MessageBox.TYPE_ERROR)
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
				self.tunemsgtimer.stop()
#				self.rlist[index]="fail"
#				self["resultlist"].updateList(self.rlist)
		elif self.camstep == 2:
			slot = 0
			appname = eDVBCI_UI.getInstance().getAppName(slot)
			print 'appname',appname
			if appname is None:
				self.session.nav.stopService()
				self.session.open( MessageBox, _("NO_GET_APPNAME"), MessageBox.TYPE_ERROR)
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
				self.tunemsgtimer.stop()				
			else:
				self.camstep=3
				self.camtimer.start(100,True)		
		elif self.camstep==3:
			slot = 1
			state = eDVBCI_UI.getInstance().getState(slot)
			print '-2-stat',state
			if state > 0:
				self.camstep=4
				self.camtimer.start(100,True)
			else:
				self.session.nav.stopService()
				self.session.open( MessageBox, _("NO_CAM2_NOT_INSERTED"), MessageBox.TYPE_ERROR)
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
				self.tunemsgtimer.stop()				
#				self.rlist[index]="fail"
#				self["resultlist"].updateList(self.rlist)
		elif self.camstep == 4:
			slot = 1
			appname = eDVBCI_UI.getInstance().getAppName(slot)
			print 'appname',appname
			if appname is None:
				self.session.nav.stopService()
				self.session.open( MessageBox, _("NO_GET_APPNAME"), MessageBox.TYPE_ERROR)
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
				self.tunemsgtimer.stop()				
			else:
				self.setSource()
				self.camstep = 5
				self.session.open( MessageBox, _("CAM OK!"), MessageBox.TYPE_INFO,2)

	def setSource(self):
		filename = ("/proc/stb/tsmux/ci0_input")
		fd = open(filename,'w')
		fd.write('B')
		fd.close()
#		filename = ("/proc/stb/tsmux/ci1_input")
#		fd = open(filename,'w')
#		fd.write('CI0')
#		fd.close()
		fd=open("/proc/stb/tsmux/input1","w")
		fd.write("CI0")
		fd.close()
		print "CI loop test!!!!!!!!!!!!!!"
			
	def resetSource(self):
		fd=open("/proc/stb/tsmux/input1","w")
		fd.write("B")
		fd.close()
		print "CI loop test end!!!!!!!!!!!!!!"
		
	def tunemsg(self):
		self.tuningtimer.stop()
		self.session.openWithCallback(self.tuneback, MessageBox, _("%s ok?" %(self["testlist"].getCurrent()[0])), MessageBox.TYPE_YESNO)

	def tuneback(self,yesno):
		self.session.nav.stopService() # try to disable foreground service
		if yesno:
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
			if self.tunerlock == 0:
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
			elif self["testlist"].getCurrent()[1] == 5 and self.camstep < 5:
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
		else:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
		self.resetSource()
		self["resultlist"].updateList(self.rlist)
				
	def Test6(self):
		self.avswitch.setInput("SCART")
		sleep(2)
		self.session.openWithCallback(self.check6, MessageBox, _("Scart loop ok?"), MessageBox.TYPE_YESNO)

	def check6(self,yesno):
		if yesno:
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
		else:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
		self.avswitch.setInput("ENCODER")

	def check7(self):
		global rstest
		if rstest == 1:
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
		else:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"

	def Test7(self):
		self.session.openWithCallback(self.check7,RS232Test)

	def Agingmode(self):
		self.session.openWithCallback(self.checkaging,AgingTest)

	def checkaging(self):
		global Agingresult
		if(Agingresult ==1):
			self["testlist"].moveToIndex(11)
			self.Test14()
			self["testlist"].moveToIndex(12)
#			self["testlist"].instance.moveSelection(self["testlist"].instance.moveDown)
			
		

	def Test8(self):
		try:
			result = 0
			mtab = open('/etc/mtab','r')
			while(1):
				disk = mtab.readline().split(' ')
				if len(disk) < 2:
					break
				if disk[1].startswith('/media/hdd'):
					continue
				elif disk[1].startswith('/media/sdb1'):
					continue
				elif disk[1].startswith('/media/sd'):
					result=result +1

			if result < 0 :
				result = 0
			if result == 3:
				self.session.open( MessageBox, _("USB test pass %d devices"%result), MessageBox.TYPE_INFO)			
				self.rlist[self["testlist"].getCurrent()[1]]="pass"
			else:
				self.session.open( MessageBox, _("USB test error : Success-%d"%result+" Fail-%d"%(3-result)), MessageBox.TYPE_ERROR)
				self.rlist[self["testlist"].getCurrent()[1]]="fail"
		except:
			if result < 0 :
				result = 0
			if result == 3:
				self.session.open( MessageBox, _("USB test pass %d devices"%result), MessageBox.TYPE_INFO)			
				self.rlist[self["testlist"].getCurrent()[1]]="pass"
			else:
				self.session.open( MessageBox, _("USB test error : Success-%d"%result+" Fail-%d"%(3-result)), MessageBox.TYPE_ERROR)
				self.rlist[self["testlist"].getCurrent()[1]]="fail"

	def Test9(self):
		self.session.openWithCallback(self.macresult ,MacConfig)	

	def macresult(self):
		global ethtest
		if ethtest == 1:
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
		else:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
		self.getmacaddr()
	
	def MemTest(self, which):
		index = which
		result = 0
		if index==0:
			result = eMemtest.getInstance().dramtest()
		elif index==1:
			result = eMemtest.getInstance().flashtest()
			result = 0	#	temp
		else:
			result = eMemtest.getInstance().dramtest()
			result = eMemtest.getInstance().flashtest()
			result = 0	#	temp
			
		index = index+10
		
		if result == 0:
			print index,self.rlist[index]
			self.rlist[index]="pass"
		else:
			print index,self.rlist[index]
			self.rlist[index]="fail"
		self["resultlist"].updateList(self.rlist)
			
	def scciresult(self):
		global smartcardtest
		if smartcardtest == 1:
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
		else:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"

	def Test10(self):
		self.session.openWithCallback(self.scciresult ,SmartCartTest)	

	def Test11(self):
		self.MemTest(1)
		
	def Test12(self):
		self.MemTest(2)

	def Test13(self):
		self.MemTest(3)	


	def Test14(self):
		try:
			system("rm -R /etc/enigma2")
			system("cp -R /usr/share/enigma2/defaults /etc/enigma2")
			self.rlist[self["testlist"].getCurrent()[1]]="pass"
			self["resultlist"].updateList(self.rlist)
		except:
			self.rlist[self["testlist"].getCurrent()[1]]="fail"
			self["resultlist"].updateList(self.rlist)
			self.session.open( MessageBox, _("Factory reset fail"), MessageBox.TYPE_ERROR)

	def Test15(self):
		self.session.openWithCallback(self.shutdown ,MessageBox, _("Do you want to shut down?"), MessageBox.TYPE_YESNO)

	def shutdown(self, yesno):
		if yesno :
			from os import _exit
			system("/usr/bin/showiframe /boot/backdrop.mvi")
			_exit(1)
		else:
			return
		
	def keyCancel(self):
		print "exit"
		self.close()

ethtest = 0
class MacConfig(Screen):
	skin = """
		<screen position="120,250" size="440,100" title="Mac Config" >
			<eLabel text="Mac Address " position="10,15" size="160,40" font="Regular;30" />		
			<widget name="text" position="190,15" size="200,40" font="Regular;30" />
			<widget name="text1" position="390,15" size="40,40" font="Regular;30" />		
			<eLabel text=" " position="5,55" zPosition="-1" size="430,5" backgroundColor="#02e1e8e6" />		
			<widget name="stattext" position="30,75" size="300,25" font="Regular;20" />
		</screen>"""

	def __init__(self, session):
		self["actions"] = ActionMap(["DirectionActions","OkCancelActions"],
		{
			"ok": self.keyOk,
			"left": self.keyleft,
			"right": self.keyright,
			"cancel": self.keyCancel,
		}, -2)

		Screen.__init__(self, session)
	
		self.result = 0
		self.macfd = 0
		self.macaddr = "000000000000"
		self.NetworkState = 0
		self["text"]=Label((self.macaddr))
		self["text1"]= Label(("< >"))
		self["stattext"]= Label((""))
		self.displaymac()
		self.loadmacaddr()
		self.getmacaddr()
		self.pingtest()
		global ethtest
		ethtest = 0

	def loadmacaddr(self):
		try:
			result = 0
			self.macfd = 0
			mtab = open('/etc/mtab','r')
			while(1):
				disk = mtab.readline().split(' ')
				if len(disk) < 2:
					break
				if disk[1].startswith('/media/sd'):
					print 'try..',disk[1]
					if  fileExists(disk[1]+"/Vuplus_mac.txt"):
						self.macfd = open(disk[1]+"/Vuplus_mac.txt","r+")
						break
			if self.macfd == 0:
				self["text"].setText(("cannot read usb!!"))
				self["text1"].setText((" "))
				self["stattext"].setText((" Press Exit Key."))
				self.NetworkState=0
				return
			
			macaddr=self.macfd.readline().split(":")
			self.macaddr=macaddr[1]+macaddr[2]+macaddr[3]+macaddr[4]+macaddr[5]+macaddr[6]
			self.displaymac()
			self.NetworkState = 1
		except:
			self["text"].setText(("cannot read usb!!"))
			self["text1"].setText((" "))
			self["stattext"].setText((" Press Exit Key."))
			self.NetworkState=0
#			self.session.open( MessageBox, _("Mac address fail"), MessageBox.TYPE_ERROR)

	def readmac(self, result, retval,extra_args=None):
		(statecallback) = extra_args
		if self.macConsole is not None:
			if retval == 0:
				self.macConsole = None
				content =result.split()
				self["stattext"].setText(("now : "+content[10]))
 	
	def getmacaddr(self):
		if self.NetworkState==0:
			return
		try:
			cmd = "ip -o addr"
			self.macConsole = Console()	
			self.macConsole.ePopen(cmd, self.readmac)	
#			self["stattext"].setText((macaddr))
		except:
			return
			
	def keyleft(self):
		if self.NetworkState==0:
			return
		macaddress = long(self.macaddr,16)-1
		if macaddress < 0 :
			macaddress = 0xffffffffffff
		self.macaddr = "%012x"%macaddress
		self.displaymac()

	def keyright(self):
		if self.NetworkState==0:
			return
		macaddress = long(self.macaddr,16)+1
		if macaddress > 0xffffffffffff:
			macaddress = 0
		self.macaddr = "%012x"%macaddress
		self.displaymac()

	def displaymac(self):
		macaddr= self.macaddr
		self["text"].setText(("%02x:%02x:%02x:%02x:%02x:%02x"%(int(macaddr[0:2],16),int(macaddr[2:4],16),int(macaddr[4:6],16),int(macaddr[6:8],16),int(macaddr[8:10],16),int(macaddr[10:12],16))))

	def keyOk(self):
		if self.NetworkState==0:
			return
		try:
			system("/etc/init.d/networking stop")
			system("ifconfig eth0 down")
			macaddr = self.macaddr
			macaddrcmd="ifconfig eth0 hw ether %02x:%02x:%02x:%02x:%02x:%02x"%(int(macaddr[0:2],16),int(macaddr[2:4],16),int(macaddr[4:6],16),int(macaddr[6:8],16),int(macaddr[8:10],16),int(macaddr[10:12],16))
			system(macaddrcmd)
#make_mac_sector 00-99-99-99-00-00 > /tmp/mac.sector
#flash_eraseall /dev/mtd4
#nandwrite /dev/mtd4 /tmp/mac.sector -p			
			cmd = "make_mac_sector %02x-%02x-%02x-%02x-%02x-%02x > /tmp/mac.sector"%(int(macaddr[0:2],16),int(macaddr[2:4],16),int(macaddr[4:6],16),int(macaddr[6:8],16),int(macaddr[8:10],16),int(macaddr[10:12],16))
			system(cmd)
			system("flash_eraseall /dev/mtd4")
			system("nandwrite /dev/mtd4 /tmp/mac.sector -p")
			macaddress = long(macaddr,16)+1
			if macaddress > 0xffffffffffff:
				macaddress = 0
			macaddr = "%012x"%macaddress
			macwritetext = "MAC:%02x:%02x:%02x:%02x:%02x:%02x"%(int(macaddr[0:2],16),int(macaddr[2:4],16),int(macaddr[4:6],16),int(macaddr[6:8],16),int(macaddr[8:10],16),int(macaddr[10:12],16))
			self.macfd.seek(0)
			self.macfd.write(macwritetext)
			self.macaddr = macaddr
			system("ifconfig eth0 up")
			self.loadmacaddr()
			system("ifconfig eth0 192.168.0.10")
			system("/etc/init.d/networking start")
			self.close()
		except:
			self.session.open( MessageBox, _("Mac address fail"), MessageBox.TYPE_ERROR)
			global ethtest
			ethtest = 0
			self.close()
		
	def pingtest(self):
		self["stattext"].setText(("now : wait to finish ping test..."))
		system("/etc/init.d/networking stop")
		system("ifconfig eth0 192.168.0.10")
		system("/etc/init.d/networking start")
		cmd1 = "ping -c 1 192.168.0.100"
		self.PingConsole = Console()
		self.PingConsole.ePopen(cmd1, self.checkNetworkStateFinished,self.NetworkStatedataAvail)
		
	def checkNetworkStateFinished(self, result, retval,extra_args):
		(statecallback) = extra_args
		if self.PingConsole is not None:
			if retval == 0:
				self.PingConsole = None
				content = result.splitlines()
#				print 'content',content
				x = content[4].split()
#				print 'x',x
				if x[0]==x[3]:
					statecallback(1)
				else:
					statecallback(0)					
			else:
				statecallback(0)


	def NetworkStatedataAvail(self,data):
		global ethtest
		if data == 1:
			ethtest = 1
			print "success"
			self.session.open( MessageBox, _("Ping test pass"), MessageBox.TYPE_INFO)
		else:
			ethtest = 0
			print "fail"
			self.session.open( MessageBox, _("Ping test fail"), MessageBox.TYPE_ERROR)
		self.getmacaddr()
		

	def keyCancel(self):
		if self.macfd != 0:
			self.macfd.close()
		self.close()



sccitest = 0

class ScCiTest(Screen):
	skin = """
		<screen position="120,225" size="440,200" title="CI Smartcard Test" >
			<widget name="testlist" position="10,0" size="340,120" />
			<widget name="resultlist" position="370,0" size="60,120" />
			<eLabel text=" " position="5,125" zPosition="-1" size="430,5" backgroundColor="#02e1e8e6" />		
			<widget name="text" position="10,140" size="420,50" font="Regular;25" />
		</screen>"""
	step=1
	def __init__(self, session):
		self["actions"] = ActionMap(["DirectionActions","OkCancelActions"],
		{
			"ok": self.keyOk,
			"up": self.up,
			"down":self.down,
			"cancel": self.keyCancel,
		}, -2)

		Screen.__init__(self, session)
		tlist = []
		tlist.append(("Smartcard 1 Test",0))
		tlist.append(("Smartcard 2 Test",1))
		tlist.append(("CI 1 Test",2))
		tlist.append(("CI 2 Test",3))
		self["testlist"] = MenuList(tlist)
		self.rlist = []
		for x in range(4):
			self.rlist.append((".."))
		self["resultlist"] = TestResultList(self.rlist)
		self.result = 0
		self.removecard = eTimer()
		self.removecard.callback.append(self.remove_card)
		self["text"]=Label(("Press OK Key"))
		self.camstate= eTimer()
		self.camstate.callback.append(self.cam_state)
		self.camtry = 5
		self.camstep = 0

	def keyCancel(self):
		global sccitest
		print "result ", self.result
		if self.result==15:
			sccitest=1
		self.resetSource()
		self.close()

	def up(self):
		self["text"].setText(_("Press OK Key"))
		self["testlist"].instance.moveSelection(self["testlist"].instance.moveUp)
		
	def down(self):
		self["text"].setText(_("Press OK Key"))
		self["testlist"].instance.moveSelection(self["testlist"].instance.moveDown)
		
	def keyOk(self):
		print "line - ",self["testlist"].getCurrent()[1]
		index = self["testlist"].getCurrent()[1]
		result = 0
		if index==0 or index==1:		
			self["text"].setText(_("Insert Card?"))
			self.ScTest(True)
		elif index ==2 or index==3:
			self["text"].setText(_("Insert Cam"))
			self.CamTest()

	def CamTest(self):
		self.camtry = 10
		self.camstep = 1
		self.camstate.start(1000,True)		

	def setSource(self, cislot):
		filename = ("/proc/stb/tsmux/ci%d_input"%cislot)
		fd = open(filename,'w')
		fd.write('A')
		fd.close()

	def setInputSource(self, cislot):
		fd=open("/proc/stb/tsmux/input0","w")
		if cislot==0:
			fd.write("CI0")
		else:
			fd.write("CI1")
		fd.close()
			
	def resetSource(self):
		fd=open("/proc/stb/tsmux/input0","w")
		fd.write("A")
		fd.close()
#		fd = open("/proc/stb/tsmux/ci0_input","w")
#		fd.write("CI0")
#		fd.close()
#		fd = open("/proc/stb/tsmux/ci1_input","w")
#		fd.write("CI1")
#		fd.close()

	def channelstart(self):
		ref = eServiceReference("1:0:19:1324:3EF:1:C00000:0:0:0")
		ref.setData(0,0x19)
		ref.setData(1,0x83)
		ref.setData(2,0x6)
		ref.setData(3,0x85)
		ref.setData(4,0x640000)
		self.session.nav.playService(ref)

	def channelstop(self):
		self.session.nav.stopService() # try to disable foreground service		
	
	def cam_state(self):
		index = self["testlist"].getCurrent()[1] 
		if (index-2)==0:
			slot = 1
		else:
			slot = 0
		print 'cam_state', self.camstep,self.camtry
		if self.camstep == 1:
			state = eDVBCI_UI.getInstance().getState(slot)
			print 'stat',state
			if state == 1:
				self.camstep=2
				self.camtry=10
				self["text"].setText(_("Getting Cam name...."))
				self.camstate.start(5000,True)
			else:
				self.camtry-=1
				if self.camtry>0:
					self.camstate.start(1000,True)
				else:
					self.session.open( MessageBox, _("NO_NOT_INSERTED"), MessageBox.TYPE_ERROR)
					self.rlist[index]="fail"
					self["resultlist"].updateList(self.rlist)

		elif self.camstep == 2:
			appname = eDVBCI_UI.getInstance().getAppName(slot)
			print 'appname',appname
			if appname is None:
				self.camtry-=1
				if self.camtry>0:
					self.camstate.start(1000,True)
				else:
					self.session.open( MessageBox, _("NO_GET_APPNAME"), MessageBox.TYPE_ERROR)
					self.rlist[index]="fail"
					self["resultlist"].updateList(self.rlist)
			else:
				self["text"].setText(_("Get Cam name : %s"%appname+". \n Remove Cam!"))
				self.channelstart()
				self.setInputSource(slot)
				self.setSource(slot)
				self.camstep=3
				self.camtry=30
				self.camstate.start(1000,True)		
		elif self.camstep==3:
			state = eDVBCI_UI.getInstance().getState(slot)
			print 'stat', state
			if state == 0:
				self.channelstop()
				self.result += (1<<index)
				print self.result
				self.rlist[index]="pass"
				self["text"].setText(_("Press OK Key"))
				self["resultlist"].updateList(self.rlist)				
				if index==2:
					self.down()
				elif index == 3:
					self.keyCancel()
			else:
				self.camtry-=1
				if self.camtry>0:
					self.camstate.start(1000,True)
				else:
					self.channelstop()
					self.session.open( MessageBox, _("NO_REMOVE_CAM"), MessageBox.TYPE_ERROR)
					self.rlist[index]="fail"
					self["resultlist"].updateList(self.rlist)

	def check_smart_card(self,which):
		index = which
		result  = 0
		if which==0:
			result = eSctest.getInstance().check_smart_card("/dev/sci0")
		elif which ==1:
			result = eSctest.getInstance().check_smart_card("/dev/sci1")
		else:
			result = -1

		print result			
		
		if result == 0:
			print 'pass'
		else:
			if result ==-1:
				self.session.open( MessageBox, _("1:NO_DEV_FOUND"), MessageBox.TYPE_ERROR)
			elif result == -2:
				self.session.open( MessageBox, _("1:SC_NOT_INSERTED"), MessageBox.TYPE_ERROR)
			elif result == -3:
				self.session.open( MessageBox, _("1:SC_NOT_VALID_ATR"), MessageBox.TYPE_ERROR)
			elif result == -5:
				self.session.open( MessageBox, _("1:SC_READ_TIMEOUT"), MessageBox.TYPE_ERROR)
			self.rlist[which]="fail"
			self["resultlist"].updateList(self.rlist)
		return result
		
	def remove_card(self):
		index = self["testlist"].getCurrent()[1]
		if index==0:
			result = eSctest.getInstance().eject_smart_card("/dev/sci0")	
		elif index==1:
			result = eSctest.getInstance().eject_smart_card("/dev/sci1")	
		print 'remove result' ,result
		if result == 0:
			self.rlist[index]="pass"
			self.result += (1<<index)
		else:
			if result ==-1:
				self.session.open( MessageBox, _("2:NO_DEV_FOUND"), MessageBox.TYPE_ERROR)
			elif result == -2:
				self.session.open( MessageBox, _("2:SC_NOT_INSERTED"), MessageBox.TYPE_ERROR)
			elif result == -3:
				self.session.open( MessageBox, _("2:SC_NOT_VALID_ATR"), MessageBox.TYPE_ERROR)
			elif result == -4:
				self.session.open( MessageBox, _("2:SC_NOT_REMOVED"), MessageBox.TYPE_ERROR)
			self.rlist[index]="fail"
		self["resultlist"].updateList(self.rlist)
		self["text"].setText(_("Press OK Key"))
		self.down()
		return result
	

	def ScTest(self, yesno):
		if yesno==False:
			return
		index = self["testlist"].getCurrent()[1]
		result = self.check_smart_card(index)
		if result==0:
			self.removecard.start(100,True)
			self["text"].setText(_("Read Ok. Remove Card!"))
		else:
			return

smartcardtest = 0

class SmartCartTest(Screen):
	skin = """
		<screen position="300,240" size="160,120" title="Front Test" >
			<widget name="text" position="10,10" size="140,100" font="Regular;22" />
		</screen>"""

	def __init__(self, session):
		self["actions"] = ActionMap(["DirectionActions", "OkCancelActions"],
		{
			"cancel": self.keyCancel,
			"ok" : self.keyCancel
		}, -2)

		Screen.__init__(self, session)
#		self["text"]=Label(("Press Key LEFT"))
		self["text"]=Label(("Testing Smartcard 1..."))
		self.step = 0
		self.smartcardtimer = eTimer()
		self.smartcardtimer.callback.append(self.check_smart_card)
		self.smartcardtimer.start(100,True)
		self.smartcard=0
		global smartcardtest
		smartcardtest = 0

	def check_smart_card(self):
		global smartcardtest
		index = self.smartcard
		result  = 0
		if index==0:
			result = eSctest.getInstance().check_smart_card("/dev/sci0")
		elif index ==1:
			result = eSctest.getInstance().check_smart_card("/dev/sci1")
		else:
			result = -1

		print result			
		
		if result == 0:
			print 'pass'
			if(index== 0):
				self.smartcard = 1
				self["text"].setText(_("Testing Smartcard 2..."))
				self.smartcardtimer.start(100,True)
				return
			elif (index==1):
				smartcardtest = 1
				self.session.open( MessageBox, _("Smart Card OK!!"), MessageBox.TYPE_INFO,2)
				self.step = 1
				self["text"].setText(_("Smart Card OK!!"))
				self.smartcardtimer.stop()
#			self.session.openWithCallback(self.check6, MessageBox, _("Scart loop ok?"), MessageBox.TYPE_INFO)
		else:
			if result ==-1:
				self.session.open( MessageBox, _("%d:NO_DEV_FOUND"%(index+1)), MessageBox.TYPE_ERROR)
			elif result == -2:
				self.session.open( MessageBox, _("%d:SC_NOT_INSERTED"%(index+1)), MessageBox.TYPE_ERROR)
			elif result == -3:
				self.session.open( MessageBox, _("%d:SC_NOT_VALID_ATR"%(index+1)), MessageBox.TYPE_ERROR)
			elif result == -5:
				self.session.open( MessageBox, _("%d:SC_READ_TIMEOUT"%(index+1)), MessageBox.TYPE_ERROR)
			if(index==0):
				self["text"].setText(_("Smart Card 1 Error!"))
			elif (index==1):
				self["text"].setText(_("Smart Card 2 Error!"))
			self.smartcardtimer.stop()

				
	def keyCancel(self):
		self.close()

	

fronttest = 0

class FrontTest(Screen):
	skin = """
		<screen position="300,240" size="160,180" title="Front Test" >
			<widget name="text" position="10,10" size="140,160" font="Regular;22" />
		</screen>"""

	def __init__(self, session):
		self["actions"] = ActionMap(["DirectionActions", "OkCancelActions"],
		{
			"ok": self.keyOk,
			"up":self.keyUp,
			"down":self.keyDown,			
			"cancel": self.keyCancel,
		}, -2)

		Screen.__init__(self, session)
		self["text"]=Label(("Wheel LEFT"))
		self.step = 1
		
		self.fronttimer= eTimer()
		self.fronttimer.callback.append(self.FrontAnimate)
		self.frontturnonoff = 0
		eSctest.getInstance().VFD_Open()

				
	def keyCancel(self):
		global fronttest
		if self.step==4:
			fronttest = 1
		else:
			fronttest = 0
		eSctest.getInstance().VFD_Close()
		self.close()

	def keyDown(self):
		if self.step==2:
			self.step = 3
			self["text"].setText(_("Press Front Wheel"))

	def keyUp(self):
		if self.step==1:
			self.step=2
			self["text"].setText(_("Wheel RIGHT"))
		else:
			print ""

	def keyOk(self):
		if self.step == 3:
			self.step =4
			self.fronttimer.start(1000,True)
			self["text"].setText(("Front Test OK!"))
		elif self.step==4:
			global fronttest
			self.fronttimer.stop()
			eSctest.getInstance().VFD_Close()
			fronttest = 1
			self.close()

	def FrontAnimate(self):
		if (self.frontturnonoff==0):
			eSctest.getInstance().turnon_VFD()
			self.frontturnonoff = 1
		else:
			self.frontturnonoff = 0
			eSctest.getInstance().turnoff_VFD()
		self.fronttimer.start(1000,True)
		
	

rstest = 0

import select

class RS232Test(Screen):
	skin = """
		<screen position="300,240" size="160,100" title="Front Test" >
			<widget name="text" position="10,10" size="140,80" font="Regular;22" />
		</screen>"""
	step=1
	def __init__(self, session):
		self["actions"] = ActionMap(["DirectionActions", "OkCancelActions"],
		{
			"cancel": self.keyCancel,
		}, -2)

		Screen.__init__(self, session)
		self["text"]=Label(("Press \"Enter\" Key"))
		self.timer = eTimer()
		self.timer.callback.append(self.checkrs232)
		self.timer.start(100, True)

	def checkrs232(self):
		global rstest
		try:
			rs=open('/dev/ttyS0','r')
			rd = [rs]
			r,w,e = select.select(rd, [], [], 10)
			if r:
				input = rs.read(1)
				if input == "\n":
#				if input == "m":
					rstest = 1
				else:
					rstest = 0 
			else:
				rstest = 0
		except:
			print 'error'
			rstest = 0
		self.close()

	def keyCancel(self):
		self.close()

Agingresult = 0

class AgingTest(Screen):
	skin = """
		<screen position="200,240" size="250,100" title="Aging Test" >
			<widget name="text1" position="10,10" size="230,40" font="Regular;22" />
			<widget name="text2" position="10,50" size="230,40" font="Regular;22" />
		</screen>"""
	step=1
	def __init__(self, session):
		self["actions"] = ActionMap(["WizardActions","GlobalActions"],
		{
			"agingend": self.keyEnd,
			"agingfinish": self.keyFinish,
			"volumeUp": self.nothing,
			"volumeDown": self.nothing,
			"volumeMute": self.nothing,		
		}, -2)

		Screen.__init__(self, session)
		self["text1"]=Label(("Exit - Press Pause Key"))
		self["text2"]=Label(("Reset - Press Stop Key"))
		self.servicelist = ServiceList()
		self.oldref = session.nav.getCurrentlyPlayingServiceReference()
		print "oldref",self.oldref
		session.nav.stopService() # try to disable foreground service
		self.chstart()

	def nothing(self):
		print "nothing"

	def chstart(self):
		if self.oldref is None:
			eref = eServiceReference("1:0:19:1324:3EF:1:C00000:0:0:0")
			serviceHandler = eServiceCenter.getInstance()
			servicelist = serviceHandler.list(eref)
			if not servicelist is None:
				ref = servicelist.getNext()
			else:
				ref = self.getCurrentSelection()
				print "servicelist none"
		else:
			ref = self.oldref
		self.session.nav.stopService() # try to disable foreground service
		ref.setData(0,0x19)
		ref.setData(1,0x83)
		ref.setData(2,0x6)
		ref.setData(3,0x85)
		ref.setData(4,0x640000)
		self.session.nav.playService(ref)

	def keyEnd(self):
		global Agingresult
		Agingresult = 0
		self.session.nav.stopService() # try to disable foreground service
		self.close()

	def keyFinish(self):
		global Agingresult
		Agingresult = 1
		self.session.nav.stopService() # try to disable foreground service
		self.close()
		

session = None

	
def cleanup():
	global Session
	Session = None
	global Servicelist
	Servicelist = None

def main(session, servicelist, **kwargs):
	global Session
	Session = session
	global Servicelist
	Servicelist = servicelist
	bouquets = Servicelist.getBouquetList()
	global bouquetSel
	bouquetSel = Session.openWithCallback(cleanup, FactoryTest)

#def Plugins(**kwargs):
#	return PluginDescriptor(name=_("Factory Test"), description="Test App for Factory", where = PluginDescriptor.WHERE_EXTENSIONSMENU, fnc=main)
