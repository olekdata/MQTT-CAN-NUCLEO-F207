#!/usr/bin/python
# -*- coding: utf-8 -*-

# uruchamiany atomatycznie jako demon przez
# /etc/init.d/demon_can.sh
# log ./demon_can.log
# inicjowanie can0 w demon_can.sh


import logging
import logging.handlers
import argparse
import sys
import time  # this is only being used as part of the example

import os
import can
import paho.mqtt.client as mqtt


CAN_KIER_PRZY = 0
CAN_KIER_WYCH = 1 

CAN_FUN_ROLETA = 1
CAN_FUN_SWITCH1 = 2
CAN_FUN_SWITCH2 = 3
CAN_FUN_SEN_BIN_1 = 4
CAN_FUN_SEN_BIN_2 = 5
CAN_FUN_ROLETA_POZ = 6
CAN_FUN_SEN_PRAD = 21 

CAN_FUN_KONF_ID = 100   			
CAN_FUN_KONF_TYP = 101   			
CAN_FUN_KONF_ZAP_CZAS = 102 		
CAN_FUN_KONF_KALIBRACJA = 103

DATA_VAL_OFF = 0
DATA_VAL_ON = 1
DATA_VAL_OPEN = 0
DATA_VAL_CLOSE = 1
DATA_VAL_STOP = 255



# Deafults
LOG_FILENAME = "/home/pi/py/can/demon_can.log"
LOG_LEVEL = logging.INFO  # Could be e.g. "DEBUG" or "WARNING"

# Define and parse command line arguments
parser = argparse.ArgumentParser(description="My simple Python service")
parser.add_argument("-l", "--log", help="file to write log to (default '" + LOG_FILENAME + "')")

# If the log file is specified on the command line then override the default
args = parser.parse_args()
if args.log:
        LOG_FILENAME = args.log

# Configure logging to log to a file, making a new file at midnight and keeping the last 3 day's data
# Give the logger a unique name (good practice)
logger = logging.getLogger(__name__)
# Set the log level to LOG_LEVEL
logger.setLevel(LOG_LEVEL)
# Make a handler that writes to a file, making a new file at midnight and keeping 3 backups
#handler = logging.handlers.TimedRotatingFileHandler(LOG_FILENAME, when="midnight", backupCount=3)
handler = logging.handlers.TimedRotatingFileHandler(LOG_FILENAME, when="W0")
# Format each log message like this
formatter = logging.Formatter('%(asctime)s %(levelname)-8s %(message)s')
# Attach the formatter to the handler
handler.setFormatter(formatter)
# Attach the handler to the logger
logger.addHandler(handler)

# Make a class we can use to capture stdout and sterr in the log
class MyLogger(object):
        def __init__(self, logger, level):
                """Needs a logger and a logger level."""
                self.logger = logger
                self.level = level

        def write(self, message):
                # Only log if there is a message (not just a new line)
                if message.rstrip() != "":
                        self.logger.log(self.level, message.rstrip())

# Replace stdout with logging to file at INFO level
sys.stdout = MyLogger(logger, logging.INFO)
# Replace stderr with logging to file at ERROR level
sys.stderr = MyLogger(logger, logging.ERROR)

i = 0

topic_root = 'DataSoft/'
topic_can = 'DataSoft/can'
topic_roleta = '/roleta'
topic_switch = '/switch'
topic_roleta_pos = '/roleta_pos'

print("Start demon_can.py")

can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native


#os.system('sudo ifconfig can0 down')
#os.system('sudo ip link set can0 type can bitrate 50000')
#os.system('sudo ifconfig can0 up')

logger = logging.getLogger(__name__)

'''
i = 0

# Loop forever, doing something useful hopefully:
while True:
    logger.info("The counter is now " + str(i))
    print "This is a print"
    i += 1
    time.sleep(5)
#    if i == 3:
#        j = 1/0  # cause an exception to be thrown and the program to exit

'''

def on_connect(mqttc, obj, flags, rc):
    print("rc: " + str(rc))


topic_rekup = 'DataSoft/rekuperator'

def Rekuperator(mqttc, msg):
    id = msg.arbitration_id

    if id == 0x448041: 
        v = float(msg.data[0]-msg.data[1])/10
        s = str(v)
        t = topic_rekup+'/wywiew_temp'
        mqttc.publish(t, s) 
        return True
    if id == 0x00488041: 
        v = msg.data[0]
        s = str(v)
        t = topic_rekup+'/wywiew_wilg'
        mqttc.publish(t, s)  
        return True
        
    if id == 0x44c041: 
        v = float(msg.data[0]-msg.data[1])/10            
        s = str(v)
        t = topic_rekup+'/wyrzut_temp'
        mqttc.publish(t, s)  
        return True
    if id == 0x0048C041: 
        v = msg.data[0]
        s = str(v)
        t = topic_rekup+'/wyrzut_wilg'
        mqttc.publish(t, s)  
        return True
        
    if id == 0x00370041: 
        v = float(msg.data[0]-msg.data[1])/10
        s = str(v)
        t = topic_rekup+'/czerpnia_temp'
        mqttc.publish(t, s)  
        return True
    if id == 0x00490041: 
        v = msg.data[0]
        s = str(v)
        t = topic_rekup+'/czerpnia_wilg'
        mqttc.publish(t, s)  
        return True

    if id == 0x00458041: 
        v = float(msg.data[0]-msg.data[1])/10
        s = str(v)
        t = topic_rekup+'/nawiew_temp'
        mqttc.publish(t, s)  
        return True
    if id == 0x00498041: 
        v = msg.data[0]
        s = str(v)
        t = topic_rekup+'/nawiew_wilg'
        mqttc.publish(t, s)  
        return True

    if id == 0x001DC041: 
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/wydajnosc_wyw'
        mqttc.publish(t, s)  
        return True
    if id == 0x001E0041: 
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/wydajnosc_naw'
        mqttc.publish(t, s)  
        return True
    if id == 0x001E4041: 
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/wywiew_wen'
        mqttc.publish(t, s)  
        return True
    if id == 0x001E8041: 
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/nawiew_wen'
        mqttc.publish(t, s)  
        return True
    if id == 0x00200041:
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/energia_curent'
        mqttc.publish(t, s)  
        return True
    if id == 0x00204041:
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/energia_total_year'
        mqttc.publish(t, s)  
        return True
    if id == 0x00208041:  
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/energia_total'
        mqttc.publish(t, s)  
        return True
    if id == 0x00358041:
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/energia_odzyskana_year'
        mqttc.publish(t, s)  
        return True
    if id == 0x0035C041:
        v = msg.data[0]+msg.data[1]*256
        s = str(v)
        t = topic_rekup+'/energia_odzyskana_total'
        mqttc.publish(t, s)  
        return True
    return False


# kogo� repozytory bliblioteki komunikacji dla rekuperatora Zehnder 
# https://github.com/marco-hoyer/zcan/blob/9ac9ec891ac3d852b140a39da95212c98c116f57/src/main/python/zcan/mapping.py#L6
# https://github.com/michaelarnauts/comfoconnect
#os.system('sudo ifconfig can0 down')


def IDFind(topic):
    i = topic.rfind(topic_can)+len(topic_can)
    n = ''
    while i>-1 and topic[i].isnumeric():
        n=n+topic[i]
        i += 1
    if len(n)>0:
        return int(n)
    else:
        return 0
    


def on_message(mqttc, obj, msg):
#    print("receive -> "+ msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
    if msg.topic.find('rekuperator') == -1:
        print "=> MQTT: {} - {}".format(msg.topic, msg.payload)
        
    if (msg.topic.find(topic_can) > -1) and (msg.topic.find('/set') > -1):   
        id = IDFind(msg.topic)
        if id < 1:
            return
        if msg.topic.find(topic_roleta) > -1:   # roleta
            if msg.payload == "STOP":
                v = 255
            elif msg.payload == "ON":
                v = 1
            else :
                v = 0
            cm = can.Message(arbitration_id=0x0100+id, data=[0, CAN_FUN_ROLETA, v], extended_id=False)  # 0 -> do STM32 1 -> roleta
            can0.send(cm)
            print("CANr => "+str(cm))
        if msg.topic.find(topic_switch) > -1: # w��cznik
            uint8_t = sw = 9
            v = 9;
            if msg.payload == "ON":
                sw = CAN_FUN_SWITCH1
                v = 1
            if msg.payload == "OFF":
                sw = CAN_FUN_SWITCH1
                v = 0
            if msg.payload == "ON2":
                sw = CAN_FUN_SWITCH2
                v = 1
            if msg.payload == "OFF2":
                sw = CAN_FUN_SWITCH2
                v = 0
            cm = can.Message(arbitration_id=0x0100+id, data=[0, sw, v], extended_id=False)  # 0 -> do STM32 2 -> switch
            can0.send(cm)
            print("CANs => "+str(cm))
        if msg.topic.find(topic_roleta_pos) > -1: #
            v = int(msg.payload)
            cm = can.Message(arbitration_id=0x0100+id, data=[0, CAN_FUN_ROLETA_POZ, v], extended_id=False)  
            can0.send(cm)
            print("CANs => "+str(cm))
    
    
def on_publish(mqttc, obj, mid):
    print("mid: " + str(mid))

def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

def on_log(mqttc, obj, level, string):
    print("log: " + string)

mqttc = mqtt.Client(client_id = "mqtt_test_py")
mqttc.enable_logger(logger)	
mqttc.on_message = on_message
#mqttc.on_connect = on_connect
#mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe
#mqttc.connect("localhost", 1883, 60)
mqttc.connect("192.168.1.2", 1883, 60)
mqttc.subscribe(topic_root+"#", 0)
print("subscribe:"+topic_root+"#")
 
mqttc.loop_start()

while 1:
    msg = can0.recv(30.0)
    if Rekuperator(mqttc, msg):
        continue
    #print(msg)
    if not(msg is None) and not(msg.is_extended_id):
        print ("=> Can: " , msg)
        #print(msg)
        if msg.data[0] > 0: # CAN=>MQTT; STM=>RPI = id
            id = msg.data[0]
            if msg.data[1] == CAN_FUN_ROLETA: #roleta
                if msg.data[2] == 255:
                    s = "STOP"
                elif msg.data[2] == 1:
                    s = "ON"
                else:
                    s = "OFF"
                t = topic_can + str(id) + topic_roleta
                mqttc.publish(t, s)
                print "MQTT => {} - {}".format(t , s)		
            if (msg.data[1] == CAN_FUN_SWITCH1): # wlaczik 1
                n = msg.data[1] - 1
                if msg.data[2] == 1:
                    s = "ON"
                else:
                    s = "OFF"
                t = topic_can + str(id) + topic_switch
                mqttc.publish(t, s)                
                print "MQTT => {} - {}".format(t , s)
            if (msg.data[1] == CAN_FUN_SWITCH2): # wlaczik 2
                n = msg.data[1] - 1
                if msg.data[2] == 1:
                    s = "ON2"
                else:
                    s = "OFF2"
                t = topic_can + str(id) + topic_switch
                mqttc.publish(t, s)                
                print "MQTT => {} - {}".format(t , s)
            if (msg.data[1] == CAN_FUN_SEN_BIN_1): # sensor binarne
                if msg.data[2] == 1:
                    s = "ON"
                else:
                    s = "OFF"
                t = topic_can + str(id) + '/sensor1'
                mqttc.publish(t, s)                
                print "MQTT => {} - {}".format(t , s)
            if (msg.data[1] == CAN_FUN_SEN_BIN_2): # sensor binarne
                if msg.data[2] == 1:
                    s = "ON"
                else:
                    s = "OFF"
                t = topic_can + str(id) + '/sensor2'
                mqttc.publish(t, s)                
                print "MQTT => {} - {}".format(t , s)
            if (msg.data[1] == CAN_FUN_ROLETA_POZ): 
                s = str(msg.data[2])
                t = topic_can + str(id) + topic_roleta_pos
                mqttc.publish(t, s)                
                print "MQTT => {} - {}".format(t , s)
#            if (msg.data[1] == CAN_FUN_SEN_PRAD): # prad
#                s = str(255*msg.data[2]+msg.data[3])
#                t = topic_can + str(id) + '/prad'
#                mqttc.publish(t, s)                
#                print "MQTT => {} - {}".format(t , s)
            if (msg.data[1] == 8): # temp
                s = str(255*msg.data[2]+msg.data[3])
                t = topic_can + str(id) + topic_roleta+'temp'
                mqttc.publish(t, s)                
                print "MQTT => {} - {}".format(t , s)

  

#os.system('sudo ifconfig can0 down')
