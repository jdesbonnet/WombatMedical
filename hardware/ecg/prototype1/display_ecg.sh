#!/bin/bash
# Params: UARTDevice, UARTSpeed
rm t.t
./ecg_tool -b $2  $1 -n 1000 store > t.t
gnuplot display_ecg.gp

