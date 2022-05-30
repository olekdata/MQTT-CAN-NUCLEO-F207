#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import can

can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native


#os.system('sudo ifconfig can0 down')
#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')


#msg = can.Message(arbitration_id=0x123, data=[0, 1, 2, 3, 4, 5, 6, 7], extended_id=False)

print "odbiór wszystkiego z can0"
print "Ctrl^Z - przerwij"


while 1:
  msg = can0.recv(10000)
  print msg
		
  
#  if msg is None:
#    print('Timeout occurred, no message.')
#    break

#os.system('sudo ifconfig can0 down')
