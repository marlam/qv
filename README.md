# qv (quick view): a viewer for 2D data (images, renderings, sensor data, ...)

## Description

qv shows images and image-like data and provides analysis tools such as
statistics, histogram, range selection, and color maps.

It is useful for all kinds of 2D data from sensors, simulation, rendering, or
high dynamic range imaging.

Start it from the command line and give it a list of files or directories to
open, then use keyboard and mouse to analyze the data. Each file can contain
multiple images, called frames in qv.

## Key bindings

### General

- F1: toggle help

- Q: quit

- F11: toggle fullscreen

- ESC: end fullscreen or quit

### Manage files

- o: open file(s)

- w: close current file

- r: reload the current file

- <,>: go 1 file backward/forward

### Switch frames and channels

- ←,→: 1 frame backward/forward

- ↑,↓: 10 frames backward/forward

- ⇞,⇟: 100 frames backward/forward

- c: color channel

- 0-9: channel with this index

### View, zoom, move

- l: toggle linear interpolation when magnifying

- g: toggle grid when magnifying

- -,+,=: zoom out/in/reset

- mouse,␣: move/reset

### Adjust displayed interval (e.g. for HDR)

- (,): shift left/right

- {,}: decrease/increase lower bound

- [,]: decrease/increase upper bound

- \: reset value range

### Analysis tools

- i: toggle info overlay

- s: toggle statistics overlay

- h: toggle histogram overlay

- m: toggle color map overlay

### Choose color map

- F4: disable color map

- F5: use sequential color map (press again to cycle)

- F6: use diverging color map (press again to cycle)

- F7: use qualitative color map

- F8: use color map from clipboard in CSV format (see [gencolormap](https://marlam.de/gencolormap))
