# Sample PySys testcase
# Copyright (c) 2015-2016, 2018 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors. 
# Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG 

from pysys.constants import *
from apama.basetest import ApamaBaseTest
from apama.correlator import CorrelatorHelper

class PySysTest(ApamaBaseTest):

	def execute(self):
		# create the correlator helper and start the correlator and an 
		# engine receive listening on the Echo Channel
		correlator = CorrelatorHelper(self, name='mycorrelator')
		correlator.start(logfile='mycorrelator.log', inputLog='mycorrelator.input.log')
		correlator.receive(filename='receive.evt', channels=['EchoChannel'])

		# inject the simple monitor into the correlator
		correlator.injectEPL(filenames=['simple.mon'])
		
		# create the send process and write events to its stdin
		correlator.sendEventStrings(
			'SimpleEvent("This is the first simple event")',
			'SimpleEvent("This is the second simple event")',
			channel='TestChannel',
		)
		
		# writes to stdin are asynchronous, wait for receipt in the receive file
		# we could use correlator.flush() here instead
		self.waitForSignal('receive.evt', expr="This is the", condition="==2")

		
	def validate(self):
		# look for the log statements in the correlator log file
		exprList = []
		exprList.append('Received simple event with message - This is the first simple event')
		exprList.append('Received simple event with message - This is the second simple event')
		self.assertOrderedGrep('mycorrelator.log', exprList=exprList)
	
		# look for the events in the receiver output
		exprList = []
		exprList.append('This is the first simple event')
		exprList.append('This is the second simple event')
		self.assertOrderedGrep('receive.evt', exprList=exprList)
	
