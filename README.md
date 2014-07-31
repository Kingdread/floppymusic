floppymusic
===========

floppymusic is a small program that will play MIDI files via floppy disk
drives. Don't believe me? See http://kingdread.de/~daniel/swars.mp4 for a small
example with just one drive or search youtube to find other examples of people
playing around with floppy drives.

Requirements
------------

- one or more floppy disk drives
- Raspberry Pi
- one or more MIDI files

Installation
------------

- adjust Makefile to your needs. The standard Makefile should be fine if you're
  using g++.
- run `make`
- it will produce a single executable `floppymusic`in the current directory

Usage
-----

To use floppymusic you have to configure it first. The only information it
needs is which pins it should use. You can specify this in `drives.cfg`. It
should look like

```
drive d1 s1
drive d2 s2
```

Where d1 is the pin connected to the direction input of the first drive, s1 is
the pin connected to the stepper input of the first drive, ...

When you've configured the drives, just run floppymusic and give it a MIDI file
as argument:

```
./floppymusic StarWars.mid
```

Run `floppymusic -h` to get an overview of available command line options.

License
-------

    floppymusic
    Copyright (C) 2014 Daniel Schadt

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

Closing remarks
---------------

I've written this for personal use and I do not guarantee that it will work for
you, neither do I take any responsibility for any damage inflicted to your
devices. Use it at your own risk and preferably if you know what you're doing.

The internet has many tutorials about how to connect floppy drives, I suggest
that you read/watch one of these as I do only provide this software and no
further support for your project.

Have fun!
