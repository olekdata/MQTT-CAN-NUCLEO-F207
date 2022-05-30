#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
File:  can-cfg.py

Title: konfikuracja modu³ów MCU-CAN

"""
import can
#import time
import sys
import argparse


CAN_FUN_KONF_ID = 100   			
CAN_FUN_KONF_TYP = 101   			
CAN_FUN_KONF_ZAP_CZAS = 102 		
CAN_FUN_KONF_KALIBRACJA = 103

#os.system('sudo ifconfig can0 down')
#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')


parser = argparse.ArgumentParser(description="can-cfg narzedzie")
parser.add_argument('-i', '--id', help="ID urzadzenia", type=int, required=True)
parser.add_argument('-n', '--nid', help="Ustaw nowe ID", type=int, required=False)
parser.add_argument('-t', '--typ', help="Ustaw typ urzadzenia 1-wylaczik; 2-roleta binarna; 4-roleta z pozycja", type=int, required=False)
parser.add_argument('-s', '--czas', help="Ustaw czas zasuwania/otwiernaia rolety 10=1s", type=int, required=False)
parser.add_argument('-k', '--kali', help="Kalibracja rolety", required=False)

args = parser.parse_args()

can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native
msg = None
if args.nid <> None:
    print args.nid
    msg = can.Message(arbitration_id=0x0100+args.id, data=[0, CAN_FUN_KONF_ID, args.nid], extended_id=False)
if args.typ  <> None:
    msg = can.Message(arbitration_id=0x0100+args.id, data=[0, CAN_FUN_KONF_TYP, args.typ], extended_id=False)
if args.czas <> None:
    msg = can.Message(arbitration_id=0x0100+args.id, data=[0, CAN_FUN_KONF_ZAP_CZAS, args.czas], extended_id=False)
if args.kali <> None:
    msg = can.Message(arbitration_id=0x0100+args.id, data=[0, CAN_FUN_KONF_KALIBRACJA, 0], extended_id=False)
    
if msg:
    print ("wyslano: ")
    print msg
    can0.send(msg)
    i = 0
    msgr = can0.recv(5.0)
    while msgr <> None and i < 10:
        if msgr.arbitration_id == msg.arbitration_id:
            print ("odebrano: ")
            print msgr
            break
        i += 1;
        msgr = can0.recv(5.0)



#os.system('sudo ifconfig can0 down')

