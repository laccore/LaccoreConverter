LaccoreConverter
================

Utility that converts raw GeoTek core images to LacCore's standard image scale and orientation, and adds a ruler.



=========================
Building LaccoreConverter
=========================

LaccoreConverter is basically a frontend to a sequence of ImageMagick API calls. The included Visual Studio 2008 project must
be added to the VS solution provided by the developers of ImageMagick in order to build and run properly.

First, let's get a working build of ImageMagick:

1. Download the ImageMagick Windows source distribution, currently at the following URL:

http://www.imagemagick.org/script/advanced-windows-installation.php

2. Unzip. In Visual Studio, open the VisualMagick\configure workspace. Set build config to Release. Build and execute.

3. When prompted, choose Static Multi-threaded Runtimes (VisualStaticMT). You will also have the option to edit
magick_config.h, but no changes are necessary. Complete the configuration process.

4. In Visual Studio, open VisualMagick\VisualStaticMT.sln. Make "All" the active project, choose the "Release" configuration,
and build. You should be able to run the ImageMagick utilities found VisualMagick\bin from the command line.


Now we can get LaccoreConverter up and running:

5. Copy the LaccoreConverter directory into the VisualMagick\Magick++ directory.

6. In Visual Studio, right-click on the Solution (VisualStaticMT) in Solution Explorer, choose Add > Existing Project...,
and select VisualMagick\Magick++\LaccoreConverter\LaccoreConverter.vcproj

7. Right-click the LaccoreConverter project icon in Solution Explorer, and choose Project Dependencies...

8. Check all of the CORE_foo items and click OK.

9. Build LaccoreConverter (Release) and run.  Hopefully it works!
