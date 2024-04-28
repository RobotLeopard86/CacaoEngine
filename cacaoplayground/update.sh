#!/bin/sh

ninja -C ../cacaoglalglfw -t clean
ninja -C ../cacaoglalglfw
ninja -C ../cacao -t clean
ninja -C ../cacao
(cd ..; python scripts/update_playground.py)
ninja -C playground -t clean
ninja -C playground