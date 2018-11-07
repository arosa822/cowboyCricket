import os
from shutil import copyfile
import shutil

#global variables - specific for user
#Global variables
WIFISSID = '<Your Wifi SSID>'
PASSWORD = '<Your Wifi Password>'
SOURCE_NAME = 'Bin'
SOURCE_LABEL = 'ESP'
TOKEN = '<your token>'

def createFolder(directory):
    try:
        if not os.path.exists(directory):
            os.makedirs(directory)
    except OSError:
        print ('Error: Creating directory. ' +  directory)

def userInput():
    #Get the data we need
    client = input("Client code? ")
    fileNumber = int(input("How many source files do you want to create? "))
    
    #parent directory of source folder
    folderLoc = ('./%s' % client)
    return client, fileNumber , folderLoc

def main(): 
    #ask for input - how many source files? 

    client, fileNumber, folderLoc = userInput()
    #create parent folder and duplicate
    createFolder(folderLoc + '/' + client)

    for i in range (fileNumber):
        srcFolder = folderLoc + '/' + client + '/' + 'src%d' % (i)
        srcFile = 'src%d' % (i) +'.ino'
        configFile = srcFolder + '/config.ino'
        createFolder(srcFolder)

        #copy src code from main
        src = './source/source.ino'
        dst = srcFolder + '/' + srcFile
        shutil.copy2(src,dst)

        # create config file and input variables
        f = open(srcFolder + '/config.h', "w+")
        f.write('#define TOKEN  "%s"\r\n' % TOKEN)
        f.write('#define WIFISSID  "%s"\r\n' % WIFISSID)
        f.write('#define PASSWORD  "%s"\r\n' % PASSWORD)
        f.write('#define SOURCE_NAME  "%s"\r\n' % (SOURCE_NAME + str(i)))
        f.write('#define SOURCE_LABEL  "%s"\r\n' % (SOURCE_LABEL + str(i)))
        f.close()


if __name__ == '__main__':
        main()
