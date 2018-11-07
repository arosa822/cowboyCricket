##Contents

* srcCodeGen
    * This folder contains a python script used to duplicate firmware (C++) source files with unique header files (.h) specific to the device. The "Source Name" and "Source label" definitions will have unique identifiers. 

    * I am also including the key and several other definitions in these configuration files to keep sensitive information out of my git repository

* testDeploy
    * This Folder contains a python script used to simulate a data feed for multiple devices in two separate locations for testing work flow management once deployed at scale.  The data feed is sent to Ubidots, a 3rd party IOT platform. 
