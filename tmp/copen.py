#!/usr/bin/python
# -*- coding: utf-8 -*-
# sudo ifconfig can0 txqueuelen 1000
import canopen
import time

network = canopen.Network()

network.connect(channel='can0', bustype='socketcan')

node = network.add_node(6, '/home/pi/py/can/sample.eds')

local_node = canopen.LocalNode(1, '/home/pi/py/can/sample.eds')
network.add_node(local_node)

for node_id in network:
    print(network[node_id])
    
    
    
# This will attempt to read an SDO from nodes 1 - 127
network.scanner.search()
# We may need to wait a short while here to allow all nodes to respond
time.sleep(1)
for node_id in network.scanner.nodes:
    print("Found node %d!" % node_id)
    
network.disconnect()    