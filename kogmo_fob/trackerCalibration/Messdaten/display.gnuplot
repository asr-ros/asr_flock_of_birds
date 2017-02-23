#!/bin/bash

echo "
#set dgrid3d 50,50,1
#set xlabel 'rotation x (degree)'
#set ylabel 'rotation y (degree)'
#set zlabel 'similarity'
#set hidden3d
#set noztics
set terminal post
#splot '$1' using (\$2):(\$3):(\$4) with linespoints
plot '$1' using (\$2):(\$3) notitle
" | gnuplot > $2
