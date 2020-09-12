
#################################################################################
# 3 programs are involved in the simulator server:
#     simulator: receive http request from simulatorServer and control GPIO
#     ImageCapture: capture image frames and send them to simulatorServer
#     simulatorServer: interact with browsers
#################################################################################

# steps to run simulator server

#install nodejs
sudo apt-get update
sudo apt-get install -y nodejs
sudo apt-get install npm
#install formidable
cd /home/pi/Developments/invenco/simulatorServer
npm install formidable

#create ramdisks
sudo mkdir /media/ramdisk
sudo mkdir /media/frameDisk
sudo mount -t tmpfs -o size=128m tmpfs /media/ramdisk
sudo mount -t tmpfs -o size=32m tmpfs /media/frameDisk

#start applications
cd /home/pi/Developments/invenco/simulator/Debug
sudo ./simulator
cd /home/pi/Developments/invenco/simulatorServer
sudo node app.js
#checking host_server_ip in /home/pi/Developments/invenco/ImageCapture/ImageCapture.properties
#to make sure ImageCapture send frames to local simulatorServer
cd /home/pi/Developments/invenco/ImageCapture/Release
./ImageCapture
####################################################################################

####################################################################################
# start simulator server after Raspberry powers up
####################################################################################
1. creates 2 directories at the first time.
	sudo mkdir /media/ramdisk
	sudo mkdir /media/frameDisk
	
2. create script file /home/pi/start_up_simulator_server with mode 777, and this file contains the following content:
	#!/bin/bash
	sudo mount -t tmpfs -o size=128m tmpfs /media/ramdisk
	sudo mount -t tmpfs -o size=32m tmpfs /media/frameDisk
	cd /home/pi/Developments/invenco/simulator/Debug
	sudo ./simulator &
	cd /home/pi/Developments/invenco/RS232ETH/Debug
	sudo ./RS232ETH &
	cd /home/pi/Developments/invenco/simulatorServer
	sudo node app.js &
	cd /home/pi/Developments/invenco/ImageCapture/Release
	sudo ./ImageCapture &

3. add the following line to /etc/rc.local before "exit 0"
	/home/pi/start_up_simulator_server

