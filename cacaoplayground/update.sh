#!/bin/sh

ninja -C ../cacaoglglfw -t clean
ninja -C ../cacaoglglfw
ninja -C ../cacao -t clean
ninja -C ../cacao
(cd ..; python scripts/update_playground.py)
ninja -C playground -t clean
ninja -C playground