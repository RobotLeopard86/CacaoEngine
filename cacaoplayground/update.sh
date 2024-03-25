#!/bin/sh

ninja -C ../cacaoglglfw -t clean
ninja -C ../cacaoglglfw
ninja -C ../cacao -t clean
ninja -C ../cacao
(cd ..; python scripts/update_playground.py)
ninja -C playground -t clean
ninja -C playground
cp ../libs/generated/libassimp.so ./libassimp.so.5
cp ../libs/generated/libglfw.so ./libglfw.so.3