#!/bin/bash
gencolormap -n 512 --type=brewer-sequential --hue=0   > colormap-sequential-0.csv
gencolormap -n 512 --type=brewer-sequential --hue=120 > colormap-sequential-1.csv
gencolormap -n 512 --type=brewer-sequential --hue=240 > colormap-sequential-2.csv
gencolormap -n 512 --type=cubehelix                   > colormap-sequential-3.csv
gencolormap -n 512 --type=brewer-diverging  --hue=0   > colormap-diverging-0.csv
gencolormap -n 512 --type=brewer-diverging  --hue=120 > colormap-diverging-1.csv
gencolormap -n 512 --type=brewer-diverging  --hue=240 > colormap-diverging-2.csv
gencolormap -n 512 --type=moreland                    > colormap-diverging-3.csv
gencolormap -n 512 --type=brewer-qualitative          > colormap-qualitative-0.csv
