This is the final project for Advanced Computer Graphics made by Jason Kraft and Uyen Uong

The source code can be found in the ./src directory.
Presentation work can be found in the ./presentation directory.
The final report PDF can be found in the ./report directory.
The thumbnail image is in the root directory.

USAGE:
  Build project inside ./build directory.
  ./render -input ../src/<mesh_model.obj> [-printing_size <width> <height> <length>] [-offset_increment <increment>] [-beam_width <width>]
  Note:
    -printing_size is used to set the working volume of the printer. Recommend sizes for each mesh are as follows:
      bunny_1k.obj:           0.2 0.2 0.2
      cube.obj:               0.5 0.5 0.5
      dragon_simplified.obj:  5.0 5.0 5.0
    -offset_increment is used to set the amount the offset will increment for each cut direction.
      We recommend using an increment of about 1/5th of the smallest printing dimension.
    -beam_width is used to set the beam width of the beam search. We recommend a beam width of 4 in most cases.
