#!/bin/bash

gunzip -c jd-ecg-20131123-1.dat.gz > a.a
octave ecg-50hz-filter.m a.a t.t > /dev/null
gnuplot ecg_chart_filtered.gnuplot 
rm a.a t.t

