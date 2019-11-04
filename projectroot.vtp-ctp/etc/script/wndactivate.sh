#!/bin/bash

sleep 10

while [ 1 ]
do

sleep 2

WND=$(xdotool getactivewindow)
xdotool windowactivate $WND

done 
