#!/bin/sh

prefix=$1

ffmpeg -framerate 30 -f image2 -i "$prefix%05d.png" -c:v copy "$prefix.mp4"
