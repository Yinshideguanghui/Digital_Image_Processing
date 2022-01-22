# Digital_Image_Processing

Some digital image processing tools

# File Description

(0)main.cpp

A simple test program including all the test items. Type in an integer to choose which to perform.

(1)read_write_bmp.h

Read and write 2-bit monocolor, 8-bit grey-scale and 24-bit color bmp files. Adapted to 4-byte alignment.

(2)image_interpolation.h

Using nearest, bilinear, and bicubic interpolation to rescale images.

(3)histogram_equalization.h

Histogram equalization for 8-bit grey-scale and 24-bit color bmps.

(4)filter__spatial_domain.h

Using Gaussian low-pass filter and median filter to deal with noise in spatial domain.

(5)orthogonal_transformation.h

Adpoting DFT, DCT and Haar DWT(up to two-scale) to 8-bit grey-scale color bmps.

Note: NOT adapted to 4-byte alignment.

(6)morphology.h

Adopting morphologic erosion to do boundary extraction on 2-bit monocolor bmps.

(7)image_segmentation.h

Using Otsu method and its imporved version using Laplacian edge information to perform global thresholding on 8-bit grey-scale bmps.

(8)Complex_xx .h/.cpp

Matrix tools used in some of the above methods.
