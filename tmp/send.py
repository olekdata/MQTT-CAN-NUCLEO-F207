#!/usr/bin/python
# -*- coding: utf-8 -*-
import can
#import time
import sys

#os.system('sudo ifconfig can0 down')
#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')

can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native

print ("Help: send.py id(+200) data[1] data[2] ...")

d=[]
i = 0;

for a in sys.argv:
   if i > 1:
     d.append(int(a))
   i+=1
#   print i
#print d

#while True:
print ("send can0 msg")
#msg = can.Message(arbitration_id=0x0100+int(sys.argv[1]), data=[int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])], extended_id=False)
#msg = can.Message(arbitration_id=0x0100+int(sys.argv[1]), data=d, extended_id=False)
msg = can.Message(arbitration_id=int(sys.argv[1]), data=d, extended_id=False)
can0.send(msg)
print msg
print (sys.argv)
print ("\n")
#time.sleep(3)


#os.system('sudo ifconfig can0 down')

