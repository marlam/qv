#!/bin/bash
gencolormap -f ppm -n 512 --type=brewer-sequential --hue=0   | tgd convert -i FORMAT=ppm - colormap-sequential-0.png
gencolormap -f ppm -n 512 --type=brewer-sequential --hue=120 | tgd convert -i FORMAT=ppm - colormap-sequential-1.png
gencolormap -f ppm -n 512 --type=brewer-sequential --hue=240 | tgd convert -i FORMAT=ppm - colormap-sequential-2.png
gencolormap -f ppm -n 512 --type=cubehelix                   | tgd convert -i FORMAT=ppm - colormap-sequential-3.png
gencolormap -f ppm -n 512 --type=brewer-diverging  --hue=0   | tgd convert -i FORMAT=ppm - colormap-diverging-0.png
gencolormap -f ppm -n 512 --type=brewer-diverging  --hue=120 | tgd convert -i FORMAT=ppm - colormap-diverging-1.png
gencolormap -f ppm -n 512 --type=brewer-diverging  --hue=240 | tgd convert -i FORMAT=ppm - colormap-diverging-2.png
gencolormap -f ppm -n 512 --type=moreland                    | tgd convert -i FORMAT=ppm - colormap-diverging-3.png
gencolormap -f ppm -n 512 --type=brewer-qualitative          | tgd convert -i FORMAT=ppm - colormap-qualitative-0.png
gencolormap -f ppm -n 512 --type=brewer-qualitative -d 360   | tgd convert -i FORMAT=ppm - colormap-qualitative-1.png
