#!/bin/bash
rm -f ./render_obj
gcc -lm -o render_obj render_obj.c
./render_obj
rm -f ./render_obj
