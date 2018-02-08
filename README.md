# oscillosope-vector-display
Experimental vector display on oscilloscope using soundcard audio output

__Building Library__

Install ALSA development package

sudo apt-get install asound2-dev

git clone https://github.com/tedyapo/oscillosope-vector-display.git

cd oscillosope-vector-display

cd src

make

__Building Example__

cd ../examples

make

__Running Example__

connect L and R audio outputs to X- and Y- inputs of oscilloscope

./bouncing_balls

![example image](/doc/images/DS1054Z.jpg)
