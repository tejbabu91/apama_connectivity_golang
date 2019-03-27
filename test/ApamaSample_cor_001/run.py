# Sample PySys testcase
# Copyright (c) 2015-2016, 2018 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors. 
# Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG 

from pysys.constants import *
from apama.basetest import ApamaBaseTest
from apama.correlator import CorrelatorHelper
from pysys.utils import filecopy
from pysys.utils import fileutils

import shutil, threading

TEST_SUBJECT_DIR = PROJECT.TEST_SUBJECT_DIR + '/examples/sample'
socketProcessMutex = threading.Lock()

class PySysTest(ApamaBaseTest):

	def execute(self):
		# copy the build(Release folder) and config yaml to output
		fileutils.mkdir(self.output+'/Release')
		with socketProcessMutex:
			self.copytree(TEST_SUBJECT_DIR+'/Release',self.output+'/Release')
		filecopy.filecopy(self.input+'/sample.yaml', self.output+'/sample.yaml')
		
		# create the correlator helper and start the correlator and an 
		# engine receive listening on the Echo Channel
		correlator = CorrelatorHelper(self, name='mycorrelator')
		correlator.start(logfile='mycorrelator.log', inputLog='mycorrelator.input.log',config=[self.output+'/sample.yaml'])
		correlator.injectEPL(['ConnectivityPluginsControl.mon', 'ConnectivityPlugins.mon'], filedir=PROJECT.APAMA_HOME+'/monitors')
		correlator.receive(filename='receive.evt', channels=['EchoChannel'])

		# inject the simple monitor into the correlator
		correlator.injectEPL(filenames=[self.input+'/DemoApp.mon'])
		#self.wait(3)
		
		# wait for receipt msg towards transport
		# we could use correlator.flush() here instead
		self.waitForSignal('mycorrelator.log', expr="\<connectivity.diag.goTransport\> \[transport\] Towards Host:",condition="==4")

		
	def validate(self):
		# look for the log statements in the correlator log file
		exprList = []
		exprList.append('\<connectivity.diag.goTransport\> \[transport\] Towards Host: \{\} \/ \{metadata:\{contentType:application/json,sag.type:apamax.golang.StringRequest,requestId:0,sag.channel:goRequest\},data:\{data:Hello to Go from Apama\}\}')
		exprList.append('\<connectivity.diag.goTransport\> \[transport\] Towards Host: \{\} \/ \{metadata:\{contentType:application/json,sag.type:apamax.golang.Request,requestId:1,sag.channel:goRequest\},data:\{data:Hello to Go from Apama\}\}')
		exprList.append('\<connectivity.diag.goTransport\> \[transport\] Towards Host: \{\} \/ \{metadata:\{contentType:application/json,sag.type:apamax.golang.Request,requestId:2,sag.channel:goRequest\},data:\{data:\{odd:\[one,three,five\],prime:\[two,three,five\],even:\[zero,two,four\]\}\}\}')
		exprList.append('\<connectivity.diag.goTransport\> \[transport\] Towards Host: \{\} \/ \{metadata:\{contentType:application/json,sag.type:apamax.golang.StringRequest,requestId:1,sag.channel:goRequest\},data:\{data:hello to transport 1\}\}')
		self.assertOrderedGrep('mycorrelator.log', exprList=exprList)

		exprList = []
		exprList.append('\<connectivity.goTransport.goTransport\> C\+\+ deliverMessageTowardsTransport: Message\<metadata=\{contentType:application/json\}, payload=\{"metadata":\{"contentType":"application/json","sag.type":"apamax.golang.StringRequest","requestId":0,"sag.channel":"goRequest"\},"data":\{"data":"Hello to Go from Apama"\}\}\>')
		exprList.append('\<connectivity.goTransport.goTransport\> C\+\+ deliverMessageTowardsTransport: Message\<metadata=\{contentType:application/json\}, payload=\{"metadata":\{"contentType":"application/json","sag.type":"apamax.golang.Request","requestId":1,"sag.channel":"goRequest"\},"data":\{"data":"Hello to Go from Apama"\}\}\>')
		exprList.append('\<connectivity.goTransport.goTransport\> C\+\+ deliverMessageTowardsTransport: Message\<metadata=\{contentType:application/json\}, payload=\{"metadata":\{"contentType":"application/json","sag.type":"apamax.golang.Request","requestId":2,"sag.channel":"goRequest"\},"data":\{"data":\{"odd":\["one","three","five"\],"prime":\["two","three","five" ...\>')
		exprList.append('\<connectivity.goTransport.goTransport\> C\+\+ deliverMessageTowardsTransport: Message\<metadata=\{contentType:application/json\}, payload=\{"metadata":\{"contentType":"application/json","sag.type":"apamax.golang.StringRequest","requestId":1,"sag.channel":"goRequest"\},"data":\{"data":"hello to transport 1"\}\}\>')
		self.assertOrderedGrep('mycorrelator.log', exprList=exprList)
	
		
	
	def copytree(self,src, dst, symlinks=False, ignore=None):
		for item in os.listdir(src):
		        s = os.path.join(src, item)
		        d = os.path.join(dst, item)
		        if os.path.isdir(s):
		            shutil.copytree(s, d, symlinks, ignore)
		        else:
		            shutil.copy2(s, d)
