#!/usr/bin/env octave -q
#
# Remove 50Hz mains noise from ECG. Assuming 500sps sample rate.
# This video was helpful in writing this script:
# https://www.youtube.com/watch?v=r7ypfE5TQK0
#
# Command line:
# octave  ecg-50hz-filter.m  ecg.dat ecg-filtered.dat > /dev/null
#
# Joe Desbonnet,
# jdesbonnet@gmail.com
# 5 Mar 2017

datain = argv(){1}
dataout = argv(){2}

ecg_data = dlmread(datain, " ")

# Extract column #1 (channel #1) as vector
ecg_ch1 = ecg_data( : , 1)

# Sample rate 500Hz.
sps=500

# Stop band frequencies are normalized to nyquist frequency. 
# ie f=1.0 means nyquist frequency.
nyquist_f = sps/2
f1 = (50-6)/nyquist_f
f2 = (50+6)/nyquist_f

stop50hz = fir1(128, [f1 f2], 'stop')

ecg_ch1_filtered = filter(stop50hz,1,ecg_ch1)

dlmwrite (dataout, ecg_ch1_filtered, " " )

