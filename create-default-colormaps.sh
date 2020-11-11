#!/bin/bash
gencolormap -f ppm -n 512 --type=brewer-sequential --hue=0   | tad convert -i FORMAT=ppm - colormap-sequential-0.png
gencolormap -f ppm -n 512 --type=brewer-sequential --hue=120 | tad convert -i FORMAT=ppm - colormap-sequential-1.png
gencolormap -f ppm -n 512 --type=brewer-sequential --hue=240 | tad convert -i FORMAT=ppm - colormap-sequential-2.png
gencolormap -f ppm -n 512 --type=cubehelix                   | tad convert -i FORMAT=ppm - colormap-sequential-3.png
gencolormap -f ppm -n 512 --type=brewer-diverging  --hue=0   | tad convert -i FORMAT=ppm - colormap-diverging-0.png
gencolormap -f ppm -n 512 --type=brewer-diverging  --hue=120 | tad convert -i FORMAT=ppm - colormap-diverging-1.png
gencolormap -f ppm -n 512 --type=brewer-diverging  --hue=240 | tad convert -i FORMAT=ppm - colormap-diverging-2.png
gencolormap -f ppm -n 512 --type=moreland                    | tad convert -i FORMAT=ppm - colormap-diverging-3.png
gencolormap -f ppm -n 512 --type=brewer-qualitative          | tad convert -i FORMAT=ppm - colormap-qualitative-0.png
gencolormap -f ppm -n 512 --type=brewer-qualitative -d 360   | tad convert -i FORMAT=ppm - colormap-qualitative-1.png
