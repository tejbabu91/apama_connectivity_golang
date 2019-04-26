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
		self.waitForSignal('mycorrelator.log', expr="Towards Host:",condition="==8")

		
	def validate(self):
		# look for the log statements in the correlator log file
		self.assertLineCount(file='mycorrelator.log', expr='<connectivity\.diag\.goTransport> (.*) Towards Host:', condition='==4')
		self.assertLineCount(file='mycorrelator.log', expr='<connectivity\.diag\.goTransport2> (.*) Towards Host:', condition='==4')
		self.assertLineCount(file='mycorrelator.log', expr='<connectivity\.(.*)\.goTransport>.*Towards Transport:', condition='==4')
		self.assertLineCount(file='mycorrelator.log', expr='<connectivity\.(.*)\.goTransport2>.*Towards Transport:', condition='==4')

		self.assertLineCount(file='mycorrelator.log', expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(0,any\(string,\"Hello to Go from Apama\"\)\)', condition='==1')
		self.assertLineCount(file='mycorrelator.log', expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.StringResponse\(0,""\)', condition='==1')
		self.assertLineCount(file='mycorrelator.log', expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(1,any\(string,"Hello to Go from Apama"\)\)', condition='==1')
		self.assertLineCount(file='mycorrelator.log', expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(2,any\(dictionary<any,any>,\{any\(string,"even"\):any\(sequence<any>,\[any\(string,"zero"\),any\(string,"two"\),any\(string,"four"\)\]\),any\(string,"odd"\):any\(sequence<any>,\[any\(string,"one"\),any\(string,"three"\),any\(string,"five"\)\]\),any\(string,"prime"\):any\(sequence<any>,\[any\(string,"two"\),any\(string,"three"\),any\(string,"five"\)\]\)\}\)\)', condition='==1')
		self.assertLineCount(file='mycorrelator.log', expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(1,any\(string,"hello to transport 2"\)\)', condition='==1')  
	
		
	
	def copytree(self,src, dst, symlinks=False, ignore=None):
		for item in os.listdir(src):
		        s = os.path.join(src, item)
		        d = os.path.join(dst, item)
		        if os.path.isdir(s):
		            shutil.copytree(s, d, symlinks, ignore)
		        else:
		            shutil.copy2(s, d)
