#!/bin/bash

(
    SOURCEPK3FILE="./*.pk3"
    BFPQ3DIR="$HOME/Quake3/bfp" # change the path to save/overwrite the pk3 file

    if ls $SOURCEPK3FILE 1> /dev/null 2>&1; then
        echo "Moving the file..."
        mv $SOURCEPK3FILE $BFPQ3DIR
        sleep 3
        echo "COMPILED DATETIME:"
        date +'%d/%m/%Y %H:%M:%S'
    else
        echo "ERROR! CHECK THE LOG"
    fi
)
