#!/bin/bash
for i in 1 2 3 4 5 6 7 8 9 10;do

	iperf -c 224.0.67.67 -u -b 300M -t 30 

done