#!/usr/bin/python
# -*- coding: utf-8 -*-
# czyta komunikaty z magistrali CAN
# zapisuje do tablicy biblioteki pandas
# wyœwietla odpowiednio posortowane
#
# kogoœ repozytory bliblioteki komunikacji dla rekuperatora Zehnder 
# https://github.com/marco-hoyer/zcan/blob/9ac9ec891ac3d852b140a39da95212c98c116f57/src/main/python/zcan/mapping.py#L6

import os
import can
import pandas as pd
import numpy as np


#os.system('sudo ip link set can0 type can bitrate 100000')
#os.system('sudo ifconfig can0 up')


def ScrCursor(y, x):
    print("\033[%d;%dH" % (y, x))
    
can0 = can.interface.Bus(channel = 'can0', bustype = 'socketcan_ctypes')# socketcan_native
df = pd.DataFrame(columns = ['count',  'ext_id', 'id', 'dlc', 'data', 'nazwa', 'value'])

#msg = can.Message(arbitration_id=0x123, data=[0, 1, 2, 3, 4, 5, 6, 7], extended_id=False)

os.system('cls||clear')




i = 0

while 1:
    msg = can0.recv(10.0)
    if not msg is None:
        id = msg.arbitration_id
        if not(id in df.index):
            df.loc[id]=[0, -1, '', 0,[],'---',0]
        df.loc[id]['id']= hex(id).rstrip("L")
        df.loc[id]['dlc']= msg.dlc
        df.loc[id]['count'] += 1
        df.loc[id]['data'] = msg.data
        df.loc[id]['ext_id'] = msg.is_extended_id   


        if id == 0x448041: 
            df.loc[id]['nazwa'] = '/wywiew_temp  [C]'
            v = float(msg.data[0]-msg.data[1])/10
            df.loc[id]['value'] = v
        if id == 0x00488041: 
            df.loc[id]['nazwa'] = '/wywiew_wilg [%]'
            v = msg.data[0]
            df.loc[id]['value'] = v
            
        if id == 0x44c041: 
            df.loc[id]['nazwa'] = '/wyrzut_temp [C]'
            v = float(msg.data[0]-msg.data[1])/10            
            df.loc[id]['value'] = v
        if id == 0x0048C041: 
            df.loc[id]['nazwa'] = '/wyrzut_wilg [%]'
            v = msg.data[0]
            df.loc[id]['value'] = v
            
        if id == 0x00370041: 
            df.loc[id]['nazwa'] = '/czerpnia_temp [C]'
            v = float(msg.data[0]-msg.data[1])/10
            df.loc[id]['value'] = v
        if id == 0x00490041: 
            df.loc[id]['nazwa'] = '/czerpnia_wilg [%]'
            v = msg.data[0]
            df.loc[id]['value'] = v

        if id == 0x00458041: 
            df.loc[id]['nazwa'] = '/nawiew_temp ok [C]'
            v = float(msg.data[0]-msg.data[1])/10
            df.loc[id]['value'] = v
        if id == 0x00498041: 
            df.loc[id]['nazwa'] = '/nawiew_wilg ok [%]'
            v = msg.data[0]
            df.loc[id]['value'] = v

        if id == 0x1DC041: 
            df.loc[id]['nazwa'] = '/wydajnosc_wyw [m3]'
            v = msg.data[0]+msg.data[1]*256
            df.loc[id]['value'] = v
        if id == 0x1E0041: 
            df.loc[id]['nazwa'] = '/wydajnosc_naw [m3]'
            v = msg.data[0]+msg.data[1]*256
            df.loc[id]['value'] = v


        if id == 0x00354041: 
            df.loc[id]['nazwa'] = '?? Wywiew tem ok [C]'
            df.loc[id]['value'] = float(msg.data[0]-msg.data[1])/10
 
        if msg.dlc == 2:
            df.loc[id]['value'] = float(msg.data[0]-msg.data[1])/10
      
        if id == 0x454041: 
            df.loc[id]['nazwa'] = '???Wlot przed [C]'
            df.loc[id]['value'] = float(msg.data[0]-msg.data[1])/10
       
        if id == 0x001E8041: 
            df.loc[id]['nazwa'] = '??? Wlot szyb. [rpm]'
            df.loc[id]['value'] = msg.data[0]+msg.data[1]*256
        if id == 0x001E4041: 
            df.loc[id]['nazwa'] = '??? Wylot szyb. [rpm]'
            df.loc[id]['value'] = msg.data[0]+msg.data[1]*256
        if id == 0x00200041: 
            df.loc[id]['nazwa'] = '??? Moc skonsum.[W]'
            df.loc[id]['value'] = msg.data[0]
        if id == 0x001D4041:
            df.loc[id]['nazwa'] = '??? Moc wen.wyl.[%]'
            df.loc[id]['value'] = msg.data[0]
        if id == 0x0001D8041:
            df.loc[id]['nazwa'] = '??? Moc wen.wlo.[%]'
            df.loc[id]['value'] = msg.data[0]
            
            
        if id == 0x00144041: 
            df.loc[id]['nazwa'] = '??? Tryb pozostalo [s]'
            df.loc[id]['value'] = msg.data[0]+msg.data[1]*256


        if id == 0x450041: 
            df.loc[id]['nazwa'] = '??? Tem zew.2 [C]'
            df.loc[id]['value'] = float(msg.data[0]-msg.data[1])/10
        if id == 0x248041: 
            df.loc[id]['nazwa'] = '??? Wyrzut tem ok [C]'
            df.loc[id]['value'] = float(msg.data[0]-msg.data[1])/10
        

        
#        dl = []
#        for d in msg.data
#            dl.append(d)
        df.loc[id]['data'] = msg.data
        
        
    ScrCursor(0,0)
    print(df.sort_values(by='count', ascending=False)[:45])
#    for r in df.items():
#        print(r)
    i += 1
    if i % 100 == 0:
#        df.to_csv('can-pand.cvs', index=True);
        print("Zapis")
    else:
        print(str(i)+"       ")
        
   

#os.system('sudo ifconfig can0 down')

'''
#ad_ds = pd.interval_range(start=1, end=2, periods=1)
#ad_df = pd.DataFrame([[1, 0] , [9, 0]], columns = ['dlc', 'count'])
#ad_df = pd.Series([])
#print(ad_df)

i = 4
if not(i in ad_df.index):
    ad_df.loc[i]=[0,0]

ad_df.loc[i]['count'] += 1

ad_df.sort_values(by='dlc')

print(ad_df.tail())
'''