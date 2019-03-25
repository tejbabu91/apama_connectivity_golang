SAMPLE

   Sample HTTP Client transport using the C++ Plugin Development Kit


DESCRIPTION

   A Connectivity transport which can send HTTP GET requests to websites and
   send back the contents of the page in a response. Packaged as a sample to
   demonstrate creating transports using the Connectivity Plugins C++ API.
   
COPYRIGHT NOTICE

   $Copyright (c) 2016-2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$ 
   Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG 

FILES

   README.txt                            This file
   HTTPClient.cpp                        Source for the HTTPClient transport
   Makefile                              GNU Make compatible makefile
   make.bat                              Windows build script
   HTTPClientSample.{sln,vcxproj}        Microsoft Visual Studio solution/project
   DemoApp.mon                           An EPL program to use this transport
   httpclient-transport.yaml             Configuration file to load the transport
   httpclient.yaml                       End-to-end sample configuration
   httpclient.properties                 End-to-end sample configuration
   httpclient-feed.evt                   Sample input events
   sample_output.txt                     Sample output

BUILDING THE SAMPLES

   It is recommended that you copy this sample folder to an area of your 
   APAMA_WORK directory rather than running it directly from the installation 
   directory. For Windows users with UAC enabled this step is required to 
   avoid access denied errors when writing to the sample directory.

   Note also that on UNIX, GNU make (gmake) is required in order to build the 
   samples using the supplied Makefile.

   ** To build the samples on UNIX (Linux) **
   
   Source the apama_env file located in the installation bin directory to set 
   the environment variables required for building sample. Run gmake in the 
   current directory:  

      $ gmake

   If the C++ compiler is not in the default location assumed by the Makefile,
   this can be overridden by setting the CXX variable on the gmake command line.
   For example:

      $ gmake CXX=/usr/bin/gcc-4.8.5/g++

   would select a GNU C++ compiler installed under /usr/bin/gcc-4.8.5

   A successful build will produce one output file in the Release subdirectory:

      libHTTPClientSample.so


   ** To build the samples on Microsoft Windows **
   
   Prepare the Apama environment by selecting "Apama Command Prompt" from the 
   Start Menu, or by running the APAMA_HOME\bin\apama_env.bat script. 
   
   Then ensure that the Visual Studio environment is set correctly for the 
   Apama architecture you are building against; this typically means running:
   
   > "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
   
   Once the environment is configured, the sample can be built using:
   
   > make.bat release
   
   Alternatively the solution file can be opened in the Visual Studio IDE with:

   > make.bat ide
      
   A successful build will leave one DLL files in the Release subdirectory:

      HTTPClientSample.dll

   
RUNNING THE SAMPLES

   Running of the samples requires access to the Correlator and Apama command 
   line tools.
   
   To ensure that the environment is configured correctly for Apama, all the 
   commands below should be executed from an Apama Command Prompt, or from a 
   shell or command prompt where the bin\apama_env script has been run (or on 
   Unix, sourced). 

   This sample also makes use of the mapper sample and classifier codec plugin.
   See the README.txt instructions in each of those directories to build those
   samples before continuing. You will refer to the config files from those
   directories later.

   1. Start the Apama Correlator in the Apama Command Prompt by running:

      > correlator -f correlator.log --config ../mapper/mapper-codec.yaml --config classifier-codec.yaml --config httpclient-transport.yaml --config httpclient.yaml --config httpclient.properties
      
     The Apama Command Prompt can be started using the Windows Start Menu 
     shortcut.
      
   2. Inject the connectivity plugins support EPL from the monitors/ directory
   of your Apama installation:

      on Unix:
      > engine_inject "$APAMA_HOME/monitors/ConnectivityPluginsControl.mon" "$APAMA_HOME/monitors/ConnectivityPlugins.mon"
      
      on Windows:
      > engine_inject "%APAMA_HOME%\monitors\ConnectivityPluginsControl.mon" "%APAMA_HOME%\monitors\ConnectivityPlugins.mon"

   3. Inject the demo application from DemoApp.mon

      > engine_inject DemoApp.mon

   4. Send in the sample events from httpclient-feed.evt

      > engine_send httpclient-feed.evt

SAMPLE OUTPUT

   The sample event in httpclient-feed.evt requests the current location of
   the ISS from a public feed. You will need to be running on a machine with
   an internet connection for this to work. If it succeeds the correlator
   should produce a JSON document with the ISS's location on its standard
   output which looks something like the output in sample_output.txt.

   You can send in events with any http URL and the correlator will print the
   content of that page. Any errors will be reported in the log file
   correlator.log.
