#!/usr/bin/python
# -*- coding: utf-8 -*-
import can
#import time
import sys
import argparse


#os.system('sudo ifconfig can0 down')
#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')


parser = argparse.ArgumentParser(description="Sterowanie roletami")
parser.add_argument("i", type=int, help="id rolety")
parser.add_argument("d", type=int, help="kierunek; 00 do stm32; 01 do rpi")
parser.add_argument("r", type=int, help="rekord; 1 rolety; 2-5 sensory")
parser.add_argument("v", type=int, help="wartosc; 0/1 off/on")
parser.add_argument('-s', '--set', action='store_true', help='ustaw')		
parser.add_argument('-i', '--info', action='store_true', help='do tylu')		
parser.add_argument('-c', '--config', action='store_true', help='zatrzymaj')		


args = parser.parse_args()


can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native

if args.set:
    msg = can.Message(arbitration_id=0x0100+args.i, data=[args.d, args.r, args.v], extended_id=False)
    can0.send(msg)
    msg = can0.recv(5.0)
    print ("send: ")
    print (sys.argv)
    print ("receive: ")
    print msg



#os.system('sudo ifconfig can0 down')

