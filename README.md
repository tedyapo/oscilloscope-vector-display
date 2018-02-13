# oscilloscope-vector-display
Experimental vector display on oscilloscope using soundcard audio output

See [hackaday.io project page](https://hackaday.io/project/47192-oscilloscope-vector-game-display) for more details

__Building Library__

\# Install ALSA development package

sudo apt-get install asound2-dev

git clone https://github.com/tedyapo/oscilloscope-vector-display.git

cd oscilloscope-vector-display

cd src

make

__Building Examples__

\# Install SDL2 development package (plus dependencies) for astro example

sudo apt-get install libsdl2-dev

cd ../examples

make

__Running Example__

\# connect L and R audio outputs to X- and Y- inputs of oscilloscope

./bouncing_balls

\# Use -D <PCM device name> to select a different audio adapter
  
\# for example:

./wrencher -D hw:CARD=Device,DEV=0

\# aplay -L will list available devices

![example image](/doc/images/DS1054Z.jpg)
