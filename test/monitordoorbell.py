# msm: source - https://www.hackster.io/Hoefnix/monitoring-the-doorbell-6a2000
#!/usr/bin/python.

#import RPi.GPIO as GPIO 
import time
import subprocess
import os
import testlib

#GPIO.setmode(GPIO.BCM)  
#GPIO.setup(24, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def erWordtGebeld():
        os.system("/usr/bin/fswebcam -c /home/marcela/Research/lib-sandboxing/cpython/test/cam.cfg")
        print('Doorbell rang at ' + time.strftime("%a om %H:%M:%S"))

#GPIO.add_event_detect(24, GPIO.FALLING, callback=erWordtGebeld, bouncetime=500)
print( "Deurbelmonitor is started at " + time.strftime("%A om %H:%M:%S") )
print( "Druk op Ctrl-C om te stoppen")

erWordtGebeld()
