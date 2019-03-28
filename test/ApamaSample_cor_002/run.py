# Sample PySys testcase
# Copyright (c) 2015-2016, 2018 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors. 
# Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG 

from pysys.constants import *
from apama.basetest import ApamaBaseTest
from apama.correlator import CorrelatorHelper
from pysys.utils.filecopy import filecopy
class PySysTest(ApamaBaseTest):
	def execute(self):
		#self.assertTrue(os.path.exists(f'{PROJECT.EXAMPLES_DIR}/FileTransport/Release/libGoFileTransport.so'))
		for f in ['read.txt', 'config.yaml']:
			filecopy(f'{self.input}/{f}', f'{self.output}/{f}')
		
		correlator = CorrelatorHelper(self, name='correlator')
		correlator.start(logfile='correlator.log', inputLog='correlator.input.log', config=[f'{self.output}/config.yaml'], environ={'EXAMPLES_DIR':PROJECT.EXAMPLES_DIR}, arguments=['-DEXAMPLES_DIR=%s'%PROJECT.EXAMPLES_DIR])
		correlator.injectEPL(filenames=['ConnectivityPluginsControl.mon', 'ConnectivityPlugins.mon'], filedir=f'{PROJECT.APAMA_HOME}/monitors')
		correlator.injectEPL(filenames=['simple.mon'])

		self.waitForSignal('correlator.log', expr="Hello from transport", condition=">=10")
		self.waitForSignal('write.txt', expr="Hello from correlator", condition=">=10")

		
	def validate(self):
		self.assertOrderedGrep('correlator.log', exprList=[f'Hello from transport {i}'  for i in range(0, 10)])
		self.assertOrderedGrep('write.txt', exprList=[f'Hello from correlator {i}'  for i in range(0, 10)])
		self.assertOrderedGrep('correlator.log', exprList=['Got response from apamax.StringResponse\(0,"Hello to Go from Apama"\)'])
