#!/usr/bin/python
# -*- coding: utf-8 -*-
# czyta komunikaty z magistrali CAN
# zapisuje do tablicy biblioteki pandas
# wyœwietla odpowiednio posortowane
#
# kogoœ repozytory bliblioteki komunikacji dla rekuperatora Zehnder 
# https://github.com/marco-hoyer/zcan/blob/9ac9ec891ac3d852b140a39da95212c98c116f57/src/main/python/zcan/mapping.py#L6
# https://github.com/michaelarnauts/comfoconnect


import os
import can
import paho.mqtt.client as mqtt


#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')


    
can0_ = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native

#msg = can.Message(arbitration_id=0x123, data=[0, 1, 2, 3, 4, 5, 6, 7], extended_id=False)



mqttc_ = mqtt.Client(client_id = "mqtt_test_py")
#mqttc.enable_logger(logger)	
#mqttc.on_message = on_message
#mqttc.on_connect = on_connect
#mqttc.on_publish = on_publish
#mqttc.on_subscribe = on_subscribe
mqttc_.connect("192.168.1.200", 1883, 60)
#mqttc.subscribe(topic_root+"#", 0)
mqttc_.loop_start()

topic = 'DataSoft/rekuperator'

i = 0

print("Pracuje")

while 1:
    msg = can0_.recv(10.0)
    if not msg is None:
        id = msg.arbitration_id

        if id == 0x448041: 
            v = float(msg.data[0]-msg.data[1])/10
            s = str(v)
            t = topic+'/wywiew_temp'
            mqttc_.publish(t, s)  
        if id == 0x00488041: 
            v = msg.data[0]
            s = str(v)
            t = topic+'/wywiew_wilg'
            mqttc_.publish(t, s)  
            
        if id == 0x44c041: 
            v = float(msg.data[0]-msg.data[1])/10            
            s = str(v)
            t = topic+'/wyrzut_temp'
            mqttc_.publish(t, s)  
        if id == 0x0048C041: 
            v = msg.data[0]
            s = str(v)
            t = topic+'/wyrzut_wilg'
            mqttc_.publish(t, s)  
            
        if id == 0x00370041: 
            v = float(msg.data[0]-msg.data[1])/10
            s = str(v)
            t = topic+'/czerpnia_temp'
            mqttc_.publish(t, s)  
        if id == 0x00490041: 
            v = msg.data[0]
            s = str(v)
            t = topic+'/czerpnia_wilg'
            mqttc_.publish(t, s)  

        if id == 0x00458041: 
            v = float(msg.data[0]-msg.data[1])/10
            s = str(v)
            t = topic+'/nawiew_temp'
            mqttc_.publish(t, s)  
        if id == 0x00498041: 
            v = msg.data[0]
            s = str(v)
            t = topic+'/nawiew_wilg'
            mqttc_.publish(t, s)  

        if id == 0x001DC041: 
            v = msg.data[0]+msg.data[1]*256
            s = str(v)
            t = topic+'/wydajnosc_wyw'
            mqttc_.publish(t, s)  
        if id == 0x001E0041: 
            v = msg.data[0]+msg.data[1]*256
            s = str(v)
            t = topic+'/wydajnosc_naw'
            mqttc_.publish(t, s)  
            
   

#os.system('sudo ifconfig can0 down')

