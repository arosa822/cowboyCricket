### Objective
Creat folder structure containing a copy of the main source code along with unique configuration files.  The configuration files contain definitions unique to the device.

#### File structure: 
* client
    * src(1)
       * src(1).ino
       * config.ino (unique to device) 
       
    * src(2)
       * src(2).ino
       * config.ino (uniqe to device) 

#### Definitions as global variables
* TOKEN
* WIFISSID
* PASSWORD
* SOURCE_NAME
* SOURCE_LABEL 

#### User prompts: 
* Client code? -> client 
* How many source files fo you want to create? -> number of source files to be created

Author: Alex Rosa / Date Created: 10/07/18
