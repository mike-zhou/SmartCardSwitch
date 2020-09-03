
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

