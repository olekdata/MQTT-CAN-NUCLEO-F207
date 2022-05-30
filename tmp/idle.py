#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import can
import paho.mqtt.client as mqtt
import logging
logging.basicConfig(filename='receive.log', level=logging.DEBUG, format='%(asctime)s %(message)s')

topic = 'DataSoft/roleta/'

print("Start")

can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native


#os.system('sudo ifconfig can0 down')
#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')


def on_connect(mqttc, obj, flags, rc):
    print("rc: " + str(rc))


def on_message(mqttc, obj, msg):
#    print("receive -> "+ msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
    l = msg.topic.rfind('/set')
    if l>0:
        p = msg.topic.rfind('/', 0, l-1)
        id = int(msg.topic[p+1:l])
        if msg.payload == "ON":
            v = 1
        else:
            v = 0
        cm = can.Message(arbitration_id=0x0100+id, data=[0, 1, v], extended_id=False)
        can0.send(cm)
        print("MQTT => CAN "+str(cm))
    
	
	
def on_publish(mqttc, obj, mid):
    print("mid: " + str(mid))


def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))


def on_log(mqttc, obj, level, string):
    print("log: " + string)

mqttc = mqtt.Client(client_id = "mqtt_test_py")
	
logger = logging.getLogger(__name__)
mqttc.enable_logger(logger)	
mqttc.on_message = on_message
mqttc.on_connect = on_connect
#mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe
mqttc.connect("192.168.1.200", 1883, 60)
mqttc.subscribe(topic+"#", 0)
mqttc.loop_start()

while 1:
    msg = can0.recv(30.0)
    #print(msg)
    if not(msg is None) and not(msg.is_extended_id):
        if msg.data[0] > 0: # CAN=>MQTT; STM=>RPI = id
            id = msg.data[0]
            if msg.data[1] == 1: #roleta
                if msg.data[2] == 1:
                    s = "ON"
                else:
                    s = "OFF"
                t = topic+str(id)
                mqttc.publish(t, s)
                print "CAN => MQTT {} - {}".format(t , s)		
            if (msg.data[1] > 1) and (msg.data[1] < 8): # sensor
                n = msg.data[1] - 1
                if msg.data[2] == 1:
                    s = "ON"
                else:
                    s = "OFF"
                t = topic+str(id)+'/sensor'+str(n)
                mqttc.publish(t, s)                
                print "CAN => MQTT {} - {}".format(t , s)
            if (msg.data[1] == 9): # prad
                s = str(255*msg.data[2]+msg.data[3])
                t = topic+str(id)+'/prad'
                mqttc.publish(t, s)                
                print "CAN => MQTT {} - {}".format(t , s)
            if (msg.data[1] == 8): # temp
                s = str(255*msg.data[2]+msg.data[3])
                t = topic+str(id)+'/temp'
                mqttc.publish(t, s)                
                print "CAN => MQTT {} - {}".format(t , s)
		
  

#os.system('sudo ifconfig can0 down')
