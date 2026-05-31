#!/bin/bash

cc -g -S -I ../../internal -o "$2.S" "$1"