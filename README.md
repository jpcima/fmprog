# fmprog
Automatic FM patch programmer with Genetic Algorithm

This generates a FM instrument for OPN2/OPNA to match a reference sound file.

# How to build

You will need:

- `build-essential`
- `git`
- `cmake`
- `libaubio-dev`
- `qtbase5-dev`
- `qtmultimedia5-dev`

## Download sources

`git clone --recursive https://github.com/jpcima/fmprog.git`

## Compile the program

```
mkdir fmprog/build
cd fmprog/build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

The program `FMProg` will be found inside the `build` folder.

# How to use

1. Load a sound file.

Click `Load` and select a reference sound file.
Ideally this file is a clean recording of a playing note, without silence at the beginning nor at the end.
You can try `examples/Marimba.wav`.

The program guesses the pitch and displays it in the `Pitch` box. You can change this value.

2. Run the algorithm

You can choose the targeted FM chip in the `FM clock` box.

Then, click `Start`. As it computes, the `Gen` number will increase.
It can be suspended at any moment by clicking `Pause`,

3. Save the result

You may hear the current result by clicking `Play`.
When it is satisfying enough, record the instrument by clicking `Save`.

# License information

The source code of this program is licensed under the Boost Software License 1.0.

This mention excludes the following third-party items, which are provided under a different license, as indicated in their file headers.

- `sources/chips/`
- `sources/file-formats/`
- `sources/synth/`

# References

- YEE-KING, Matthew et ROTH, Martin. Synthbot: an Unsupervised Software synthesizer Programmer. In : *ICMC.* 2008.
