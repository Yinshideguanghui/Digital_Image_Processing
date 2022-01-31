# Digital_Image_Processing

Some digital image processing tools

# File Description

(0)main.cpp

A simple test program including all the test items. Type in an integer to choose which to perform.

简单的测试主程序。要选择执行哪个测试，请输入一个整数。

(1)read_write_bmp.h

Read and write 2-bit monocolor, 8-bit grey-scale and 24-bit color bmp files. Adapted to 4-byte alignment.

读入和写入2比特单色/8比特灰度/24比特彩色bmp文件。已适应四字节对齐。

(2)image_interpolation.h

Using nearest, bilinear, and bicubic interpolation to rescale images.

使用最邻近插值、双线性插值、双三次插值缩放图片。

(3)histogram_equalization.h

Histogram equalization for 8-bit grey-scale and 24-bit color bmps.

对8比特灰度bmp和24比特彩色bmp的直方图均衡化。

(4)filter__spatial_domain.h

Using Gaussian low-pass filter and median filter to deal with noise in spatial domain.

用高斯低通滤波器和中值滤波器在空间域处理噪声。

(5)orthogonal_transformation.h

Adpoting DFT, DCT and Haar DWT(up to two-scale) to 8-bit grey-scale bmps.

对8比特灰度bmp执行离散傅里叶变换、离散余弦变换和(最多二尺度的)哈尔小波变换。

Note: NOT adapted to 4-byte alignment.

注意：没有适应四字节对齐。

(6)morphology.h

Adopting morphologic erosion to do boundary extraction on 2-bit monocolor bmps.

对2比特单色图执行形态学腐蚀操作，并且通过形态学腐蚀实现边界提取。

(7)image_segmentation.h

Using Otsu method and its imporved version using Laplacian edge information to perform global thresholding on 8-bit grey-scale bmps.

使用Otsu方法及其结合图像拉普拉斯提取边缘信息的改进版本，对8比特灰度图执行全局阈值处理。

(8)complex_xx .h/.cpp

Matrix tools used in some of the above methods.

上述部分方法需要用到的自编矩阵运算工具库。
