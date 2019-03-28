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

		# inject the simple monitor into the correlator
		correlator.injectEPL(filenames=[self.input+'/DemoApp.mon'])
				
		# wait for receipt msg towards host
		# we could use correlator.flush() here instead
		self.waitForSignal('mycorrelator.log', expr='Got response from apamax.golang.Response',condition="==5")

		
	def validate(self):
		# look for the log statements in the correlator log file
		self.assertGrep('mycorrelator.log', expr=' (ERROR|FATAL) ', contains=False)
		self.assertGrep('mycorrelator.log',expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(0,any\(string,"Hello to Go from Apama"\)\)')
		self.assertGrep('mycorrelator.log',expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(1,any\(dictionary<any,any>,\{any\(string,"even"\):any\(sequence<any>,\[any\(integer,0\),any\(integer,2\),any\(integer,4\),any\(integer,6\),any\(integer,8\)\]\),any\(string,"odd"\):any\(sequence<any>,\[any\(integer,1\),any\(integer,3\),any\(integer,5\),any\(integer,7\),any\(integer,9\)\]\),any\(string,"prime"\):any\(sequence<any>,\[any\(integer,2\),any\(integer,3\),any\(integer,5\),any\(integer,7\)\]\)\}\)\)')
		self.assertGrep('mycorrelator.log',expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(1,any\(string,"hello to transport 1"\)\)')
		self.assertGrep('mycorrelator.log',expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(2,any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"aa"\):any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"aa-inner1"\):any\(sequence<any>,\[any\(integer,1\),any\(integer,2\)\]\)\}\),any\(dictionary<any,any>,\{any\(string,"aa-inner2"\):any\(sequence<any>,\[any\(integer,3\),any\(integer,4\)\]\)\}\)\]\)\}\),any\(dictionary<any,any>,\{any\(string,"bb"\):any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"bb-inner"\):any\(sequence<any>,\[any\(integer,2\),any\(integer,3\)\]\)\}\)\]\)\}\),any\(dictionary<any,any>,\{any\(string,"cc"\):any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"cc-inner1"\):any\(sequence<any>,\[any\(integer,5\),any\(integer,4\)\]\)\}\),any\(dictionary<any,any>,\{any\(string,"cc-inner2"\):any\(sequence<any>,\[any\(integer,6\),any\(integer,4\)\]\)\}\),any\(dictionary<any,any>,\{any\(string,"cc-inner3"\):any\(sequence<any>,\[any\(integer,7\),any\(integer,4\)\]\)\}\)\]\)\}\)\]\)\)')
		self.assertGrep('mycorrelator.log',expr='apamax.golang.GoTransportSample \[1\] Got response from apamax.golang.Response\(3,any\(dictionary<any,any>,\{any\(string,"asd ijidjf. asdsd jkk."\):any\(dictionary<any,any>,\{any\(string,"cccc"\):any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"cccc ccccc"\):any\(sequence<any>,\[any\(float,12.1\),any\(float,13.33\),any\(float,123.111\)\]\)\}\)\]\)\}\),any\(string,"qq asdf"\):any\(dictionary<any,any>,\{any\(string,"aaaa"\):any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"aaaa aaaa"\):any\(sequence<any>,\[any\(float,12.1\),any\(float,13.33\),any\(float,123.111\)\]\)\}\)\]\)\}\),any\(string,"www world"\):any\(dictionary<any,any>,\{any\(string,"bbbb"\):any\(sequence<any>,\[any\(dictionary<any,any>,\{any\(string,"bbbb bbbb"\):any\(sequence<any>,\[any\(float,12.1\),any\(float,13.33\),any\(float,123.111\)\]\)\}\)\]\)\}\)\}\)\)')
		
	
	def copytree(self,src, dst, symlinks=False, ignore=None):
		for item in os.listdir(src):
		        s = os.path.join(src, item)
		        d = os.path.join(dst, item)
		        if os.path.isdir(s):
		            shutil.copytree(s, d, symlinks, ignore)
		        else:
		            shutil.copy2(s, d)
