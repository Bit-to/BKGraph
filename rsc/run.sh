#!/bin/bash

$(dirname $(readlink -f "$0"))/BKGraph $@ & disown
