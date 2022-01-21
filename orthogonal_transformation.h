#pragma once

#ifndef READ_WRITE_BMP_H
#define READ_WRITE_BMP_H

#include<cmath>
#include"read_write_bmp.h"
#include"complex_matrix_operators.h" // 正交变换中可能出现复数

//离散傅里叶变换（零填充）
bmp_file DFT_zero_padding(const bmp_file& origin_bmp)
{
	bmp_file result_bmp;

	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:	// 8位256色调色板图
	{
		//*提取非四字节对齐像素到一零填充的扩展图像
		int new_biWidth = origin_bmp.bmp_info_header.biWidth * 2;
		int new_biHeight = origin_bmp.bmp_info_header.biHeight * 2;
		int new_num_pixels = new_biHeight * new_biWidth;
		int new_biWidth_actual = ((((origin_bmp.bmp_info_header.biBitCount * new_biWidth) + 31) & ~31) >> 3);
		ComplexMatrix origin_bmp_extended(new_biHeight, new_biWidth); // 空间域的中间结果
		if (new_num_pixels > 1)	// 防止访问越界
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					//**由于行号存储倒序，即先底行后顶行，故图像和矩阵的x方向相反，同时y方向相同；故需行读取反向，使得图像不翻转且位于矩阵左上角
					//**前面的系数是为了使DFT频谱中心点位于P*Q大小的频率矩形的中心
					origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = double((i + j) % 2 == 0 ? 1 : (-1)) * origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];
				}

		//*计算上述所得已填充和改变DFT中心的图的DFT频谱；二维DFT可拆分为两次一维DFT
		//**计算每行的一维DFT
			//*** 待优化：零填充时第一遍后半部分行没必要算，因为都是0
		ComplexMatrix DFT_each_row(new_biHeight, new_biWidth);
		Complex temp_complex_1(.0, .0), temp_complex_2(.0, .0);
		for (int x = 0; x < new_biHeight; x++)			// 外层，目标点的纵坐标
			for (int v = 0; v < new_biWidth; v++)		// 外层，目标点的横坐标
				for (int y = 0; y < new_biWidth; y++)	// 内层，对一行内元素遍历求和
				{
					temp_complex_1.re = cos(-2.0 * pi * v * y / new_biWidth);
					temp_complex_1.im = sin(-2.0 * pi * v * y / new_biWidth);
					temp_complex_2 = origin_bmp_extended.c[x][y] * temp_complex_1;
					DFT_each_row.c[x][v].re += temp_complex_2.re;
					DFT_each_row.c[x][v].im += temp_complex_2.im;
				}
		//**计算列的一维DFT
		ComplexMatrix DFT_result(new_biHeight, new_biWidth);
		for (int u = 0; u < new_biHeight; u++)			// 外层，目标点的纵坐标
			for (int v = 0; v < new_biWidth; v++)		// 外层，目标点的横坐标
				for (int x = 0; x < new_biHeight; x++)	// 内层，对一列内元素遍历求和
				{
					temp_complex_1.re = cos(-2.0 * pi * u * x / new_biHeight);
					temp_complex_1.im = sin(-2.0 * pi * u * x / new_biHeight);
					temp_complex_2 = DFT_each_row.c[x][v] * temp_complex_1;
					DFT_result.c[u][v].re += temp_complex_2.re;
					DFT_result.c[u][v].im += temp_complex_2.im;
				}
		//**计算频谱（模值）
		//***DFT结果频谱所有项，及其求和
		ComplexMatrix DFT_frequency_spectrum(new_biHeight, new_biWidth);
		double temp_modulus = .0, temp_max = .0;	// 记录下全部幅度中的最大值，以便标定（假设幅度最小值是0）
		for (int u = 0; u < new_biHeight; u++)
			for (int v = 0; v < new_biWidth; v++)
			{
				temp_modulus = sqrt(DFT_result.c[u][v].re * DFT_result.c[u][v].re + DFT_result.c[u][v].im * DFT_result.c[u][v].im);

				temp_modulus = log(1 + abs(temp_modulus));	// 转换为对数尺度以更好地可视化

				DFT_frequency_spectrum.c[u][v].re = temp_modulus;
				if (temp_max < temp_modulus)temp_max = temp_modulus;
			}
		//***标定到[0,255]
		uint8_t* result_frequency_spectrum = new uint8_t[new_biWidth_actual * new_biHeight];
		if (new_biWidth_actual * new_biHeight > 1) //防止访问越界
			for (int u = 0; u < new_biHeight; u++)
				for (int v = 0; v < new_biWidth; v++)
				{
					// 由于读入时的反序，写入时也需要反序
					result_frequency_spectrum[u * new_biWidth_actual + v] = uint8_t(round(DFT_frequency_spectrum.c[new_biHeight - 1 - u][v].re / temp_max * 255));
				}


		//*结果bmp：尺寸变为2倍
		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp.bmp_info_header.biHeight = new_biHeight;
		result_bmp.bmp_info_header.biWidth = new_biWidth;
		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp.bmp_info_header.biSizeImage = ((((new_biWidth * result_bmp.bmp_info_header.biBitCount) + 31) & ~31) >> 3) * new_biHeight;
		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp.bmp_pixel8 = result_frequency_spectrum;	// 储存正交变换频谱（模值，标定到[0,255]间的整数）
		result_bmp.biWidth_actual = new_biWidth_actual;
	}
	break;
	default:
	break;
	}
	return result_bmp;
}

//离散余弦变换（零填充）
bmp_file DCT_zero_padding(const bmp_file& origin_bmp)
{
	bmp_file result_bmp;

	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:	// 8位256色调色板图
	{
		//*提取非四字节对齐像素到一零填充的扩展图像
		int new_biWidth = origin_bmp.bmp_info_header.biWidth * 2;
		int new_biHeight = origin_bmp.bmp_info_header.biHeight * 2;
		int new_num_pixels = new_biHeight * new_biWidth;
		int new_biWidth_actual = ((((origin_bmp.bmp_info_header.biBitCount * new_biWidth) + 31) & ~31) >> 3);
		ComplexMatrix origin_bmp_extended(new_biHeight, new_biWidth); // 空间域的中间结果
		if (new_num_pixels > 1)	// 防止访问越界
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					//**由于行号存储倒序，即先底行后顶行，故图像和矩阵的x方向相反，同时y方向相同；故需行读取反向，使得图像不翻转且位于矩阵左上角
					origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = double(origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j]);
				}

		//*计算上述所得已填充的图的DCT频谱；二维DCT可拆分为两次一维DCT
		//**计算每行的一维DCT
		ComplexMatrix DCT_each_row(new_biHeight, new_biWidth);
		double temp_double = .0;
		for (int x = 0; x < new_biHeight; x++)			// 外层，目标点的纵坐标
		{
			for (int v = 0; v < new_biWidth; v++)		// 外层，目标点的横坐标
			{
				for (int y = 0; y < new_biWidth; y++)	// 内层，对一行内元素遍历求和
				{
					temp_double = cos((2.0 * y + 1) * pi * v / (2.0 * new_biWidth));
					DCT_each_row.c[x][v].re += origin_bmp_extended.c[x][y].re * temp_double;
				}
				DCT_each_row.c[x][v].re *= sqrt(2.0 / new_biWidth);
			}
			// v = 0 时即每行第一个点系数特殊
			DCT_each_row.c[x][0].re *= (1 / sqrt(2.0));
		}
		//**计算列的一维DCT
		ComplexMatrix DCT_result(new_biHeight, new_biWidth);
		for (int v = 0; v < new_biWidth; v++)			// 外层，目标点的横坐标
		{
			for (int u = 0; u < new_biHeight; u++)		// 外层，目标点的纵坐标
			{
				for (int x = 0; x < new_biHeight; x++)	// 内层，对一列内元素遍历求和
				{
					temp_double = cos((2.0 * x + 1) * pi * u / (2.0 * new_biHeight));
					DCT_result.c[u][v].re += DCT_each_row.c[x][v].re * temp_double;
				}
				DCT_result.c[u][v].re *= sqrt(2.0 / new_biHeight);
			}
			// u = 0 时即每列第一个点系数特殊
			DCT_result.c[0][v].re *= (1 / sqrt(2.0));
		}

		//**计算频谱（模值）
		//***DCT结果频谱所有项，及其求和
		ComplexMatrix DCT_frequency_spectrum(new_biHeight, new_biWidth);
		double temp_modulus = .0, temp_max = .0;	// 记录下全部幅度中的最大值，以便标定（假设幅度最小值是0）
		for (int u = 0; u < new_biHeight; u++)
			for (int v = 0; v < new_biWidth; v++)
			{
				temp_modulus = DCT_result.c[u][v].re;

				temp_modulus = log(1 + abs(temp_modulus));	// 转换为对数尺度以更好地可视化

				DCT_frequency_spectrum.c[u][v].re = temp_modulus;
				if (temp_max < temp_modulus)temp_max = temp_modulus;
			}
		//***标定到[0,255]
		uint8_t* result_frequency_spectrum = new uint8_t[new_biWidth_actual * new_biHeight];
		if (new_biWidth_actual * new_biHeight > 1) //防止访问越界
			for (int u = 0; u < new_biHeight; u++)
				for (int v = 0; v < new_biWidth; v++)
				{
					// 由于读入时的反序，写入时也需要反序
					result_frequency_spectrum[u * new_biWidth_actual + v] = uint8_t(round(DCT_frequency_spectrum.c[new_biHeight - 1 - u][v].re / temp_max * 255));
				}

		//*结果bmp：尺寸变为2倍
		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp.bmp_info_header.biHeight = new_biHeight;
		result_bmp.bmp_info_header.biWidth = new_biWidth;
		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp.bmp_info_header.biSizeImage = ((((new_biWidth * result_bmp.bmp_info_header.biBitCount) + 31) & ~31) >> 3) * new_biHeight;
		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp.bmp_pixel8 = result_frequency_spectrum;	// 储存正交变换频谱（模值，标定到[0,255]间的整数）
		result_bmp.biWidth_actual = new_biWidth_actual;
	}
	break;
	default:
		break;
	}
	return result_bmp;
}

//小波变换（哈尔小波）
bmp_file* DWT_Haar_1_and_2_scale(const bmp_file& origin_bmp, bool from_extern)
{
	bmp_file* result_bmp = new bmp_file[2];
	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:	// 8位256色调色板图
	{
		//*提取非四字节对齐像素到一零填充的扩展图像
		ComplexMatrix origin_bmp_extended(origin_bmp.bmp_info_header.biHeight, origin_bmp.bmp_info_header.biWidth); // 空间域的中间结果
		if (origin_bmp.bmp_info_header.biHeight * origin_bmp.bmp_info_header.biWidth > 1)	// 防止访问越界
		{
			if(from_extern)	//从外界调用小波变换
				for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
					for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
					{
						//**由于行号存储倒序，即先底行后顶行，故图像和矩阵的x方向相反，同时y方向相同；故需行读取反向，使得图像不翻转且位于矩阵左上角
						origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = double(origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j]);
					}
			else // 小波变换调用下一尺度小波变换
				for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
					for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
					{
						//**由于行号存储倒序，即先底行后顶行，故图像和矩阵的x方向相反，同时y方向相同；故需行读取反向，使得图像不翻转且位于矩阵左上角
						origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = origin_bmp.bmp_pixel8_double[i * origin_bmp.biWidth_actual + j];
					}
		}

		//*计算上述所得已填充的图的DWT；二维DWT可拆分为两次一维DWT
		//*1尺度哈尔FWT的系数是1/2；用一次来计算二尺度FWT时是1/sqrt(2)
		//**计算每行的一维DWT（沿水平方向计算，分垂直整体和垂直细节，即沿垂直方向的细节）
		ComplexMatrix DWT_each_row(origin_bmp.bmp_info_header.biHeight, origin_bmp.bmp_info_header.biWidth);
		for (int x = 0; x < origin_bmp.bmp_info_header.biHeight; x++)			// 目标点的纵坐标
		{
			for (int y = 0; y < origin_bmp.bmp_info_header.biWidth / 2; y++)	// 对一行内元素遍历
			{
				//***低频分量
				DWT_each_row.c[x][y].re = (origin_bmp_extended.c[x][2 * y].re + origin_bmp_extended.c[x][2 * y + 1].re) / (2.0);
				//***高频分量（垂直细节）
				DWT_each_row.c[x][y + origin_bmp.bmp_info_header.biWidth / 2].re = (origin_bmp_extended.c[x][2 * y].re - origin_bmp_extended.c[x][2 * y + 1].re) / (2.0);
			}
		}
		//**计算每列的一维DWT（沿垂直方向计算，分水平整体和水平细节，即沿水平方向的细节）
		ComplexMatrix DWT_result(origin_bmp.bmp_info_header.biHeight, origin_bmp.bmp_info_header.biWidth);
		for (int y = 0; y < origin_bmp.bmp_info_header.biWidth; y++)			// 目标点的横坐标
		{
			for (int x = 0; x < origin_bmp.bmp_info_header.biHeight / 2; x++)	// 对一列内元素遍历
			{
				//***低频分量
				DWT_result.c[x][y].re = (DWT_each_row.c[2 * x][y].re + DWT_each_row.c[2 * x + 1][y].re) / (2.0);
				//***高频分量（水平细节）
				DWT_result.c[x + origin_bmp.bmp_info_header.biHeight / 2][y].re = (DWT_each_row.c[2 * x][y].re - DWT_each_row.c[2 * x + 1][y].re) / (2.0);
			}
		}

		//*结果图
		uint8_t* result_frequency_spectrum = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
		//**左上图cA（尺度空间）
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //防止访问越界
			for (int u = 0; u < origin_bmp.bmp_info_header.biHeight / 2; u++)
				for (int v = 0; v < origin_bmp.bmp_info_header.biWidth / 2; v++)
				{
					// 由于读入时的反序，写入时也需要反序
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight - 1 - u) * origin_bmp.biWidth_actual + v] = CLAMP_0_255(DWT_result.c[u][v].re);
				}
		//**三幅差分图（小波空间）分别标定到[0,255]
		double temp_double = .0, temp_max = .0, temp_min = .0, temp_diff = .0;
		int temp_row_index = 0;
		//***左下图cV（垂直细节）：计算完后在 右上 ，最后存放到左下去
		for (int u = 0; u < origin_bmp.bmp_info_header.biHeight / 2; u++)
		{
			temp_row_index = u * origin_bmp.bmp_info_header.biWidth;
			for (int v = origin_bmp.bmp_info_header.biWidth / 2; v < origin_bmp.bmp_info_header.biWidth; v++)
			{
				temp_double = DWT_result.c[u][v].re;
				if (temp_max < temp_double)temp_max = temp_double;
				if (temp_min > temp_double)temp_min = temp_double;
			}
		}
		temp_diff = temp_max - temp_min;
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //防止访问越界
			for (int u = 0; u < origin_bmp.bmp_info_header.biHeight / 2; u++)
				for (int v = origin_bmp.bmp_info_header.biWidth / 2; v < origin_bmp.bmp_info_header.biWidth; v++)
				{
					// 由于读入时的反序，写入时也需要反序
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight / 2 - 1 - u) * origin_bmp.biWidth_actual + v - origin_bmp.bmp_info_header.biWidth / 2] 
						= uint8_t(round(abs(DWT_result.c[u][v].re - temp_min) / temp_diff * 255));
				}
		//***cH（水平方向细节）：计算完后在 左下 ，最后存放到右上去
		temp_double = .0; temp_max = .0; temp_min = .0; temp_diff = .0;
		for (int u = origin_bmp.bmp_info_header.biHeight / 2; u < origin_bmp.bmp_info_header.biHeight; u++)
		{
			temp_row_index = u * origin_bmp.bmp_info_header.biWidth;
			for (int v = 0; v < origin_bmp.bmp_info_header.biWidth / 2; v++)
			{
				temp_double = DWT_result.c[u][v].re;
				if (temp_max < temp_double)temp_max = temp_double;
				if (temp_min > temp_double)temp_min = temp_double;
			}
		}
		temp_diff = temp_max - temp_min;
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //防止访问越界
			for (int u = origin_bmp.bmp_info_header.biHeight / 2; u < origin_bmp.bmp_info_header.biHeight; u++)
				for (int v = 0; v < origin_bmp.bmp_info_header.biWidth / 2; v++)
				{
					// 由于读入时的反序，写入时也需要反序
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight + origin_bmp.bmp_info_header.biHeight / 2 - 1 - u) * origin_bmp.biWidth_actual + v + origin_bmp.bmp_info_header.biWidth / 2]
						= uint8_t(round(abs(DWT_result.c[u][v].re - temp_min) / temp_diff * 255));
				}
		//***右下图cD（对角细节）
		temp_double = .0; temp_max = .0; temp_min = .0; temp_diff = .0;
		for (int u = origin_bmp.bmp_info_header.biHeight / 2; u < origin_bmp.bmp_info_header.biHeight; u++)
		{
			temp_row_index = u * origin_bmp.bmp_info_header.biWidth;
			for (int v = origin_bmp.bmp_info_header.biWidth / 2; v < origin_bmp.bmp_info_header.biWidth; v++)
			{
				temp_double = DWT_result.c[u][v].re;
				if (temp_max < temp_double)temp_max = temp_double;
				if (temp_min > temp_double)temp_min = temp_double;
			}
		}
		temp_diff = temp_max - temp_min;
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //防止访问越界
			for (int u = origin_bmp.bmp_info_header.biHeight / 2; u < origin_bmp.bmp_info_header.biHeight; u++)
				for (int v = origin_bmp.bmp_info_header.biWidth / 2; v < origin_bmp.bmp_info_header.biWidth; v++)
				{
					// 由于读入时的反序，写入时也需要反序
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight - 1 - u) * origin_bmp.biWidth_actual + v] = uint8_t(round(abs(DWT_result.c[u][v].re - temp_min) / temp_diff * 255));
				}

		//*结果bmp：尺寸变为2倍
		result_bmp[0].bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp[0].bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp[0].bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp[0].bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp[0].bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp[0].bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight;
		result_bmp[0].bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth;
		result_bmp[0].bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp[0].bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp[0].bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp[0].bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp[0].bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp[0].bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp[0].bmp_info_header.biSizeImage = origin_bmp.bmp_info_header.biSizeImage;
		result_bmp[0].bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp[0].bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp[0].bmp_pixel8 = result_frequency_spectrum;	// 储存正交变换（模值，标定到[0,255]间的整数）
		result_bmp[0].biWidth_actual = origin_bmp.biWidth_actual;

		//*2尺度变换
		if (from_extern)
		{
			//**从1尺度变换结果提取原图
			bmp_file origin_for_2_scale;
			origin_for_2_scale.bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight / 2;
			origin_for_2_scale.bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth / 2;
			origin_for_2_scale.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
			origin_for_2_scale.biWidth_actual = origin_for_2_scale.bmp_info_header.biWidth;
			origin_for_2_scale.bmp_pixel8_double = new double[origin_for_2_scale.bmp_info_header.biHeight * origin_for_2_scale.bmp_info_header.biWidth];
			if (origin_for_2_scale.bmp_info_header.biHeight * origin_for_2_scale.bmp_info_header.biWidth > 1) //防止访问越界
				for (int u = 0; u < origin_for_2_scale.bmp_info_header.biHeight; u++)
					for (int v = 0; v < origin_for_2_scale.bmp_info_header.biWidth; v++)
					{
						// 由于读入时的反序，写入时也需要反序
						origin_for_2_scale.bmp_pixel8_double[(origin_for_2_scale.bmp_info_header.biHeight - 1 - u) * origin_for_2_scale.biWidth_actual + v]
							= DWT_result.c[u][v].re;
					}
			//**2尺度变换
			bmp_file* ptr_for_2_scale = DWT_Haar_1_and_2_scale(origin_for_2_scale, false);

			//*拼接形成2尺度的总变换图
			//**提取2尺度变换结果
			uint8_t* result_2_scale = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
			if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //防止访问越界
			{
				for (int u = origin_for_2_scale.bmp_info_header.biHeight; u < origin_bmp.bmp_info_header.biHeight; u++)
				{
					for (int v = 0; v < origin_for_2_scale.bmp_info_header.biWidth; v++)	//2尺度变换结果，在左上角；其余部分全照搬1尺度结果
					{
						result_2_scale[u * origin_bmp.biWidth_actual + v]
							= ptr_for_2_scale[0].bmp_pixel8[(u - origin_for_2_scale.bmp_info_header.biHeight) * origin_for_2_scale.biWidth_actual + v];
					}
					for (int v = origin_for_2_scale.bmp_info_header.biWidth; v < origin_bmp.bmp_info_header.biWidth; v++)
					{
						result_2_scale[u * origin_bmp.biWidth_actual + v] = result_bmp[0].bmp_pixel8[u * origin_bmp.biWidth_actual + v];
					}
				}
				for (int u = 0; u < origin_for_2_scale.bmp_info_header.biHeight; u++)
				{
					for (int v = 0; v < origin_bmp.bmp_info_header.biWidth; v++)
					{
						result_2_scale[u * origin_bmp.biWidth_actual + v] = result_bmp[0].bmp_pixel8[u * origin_bmp.biWidth_actual + v];
					}
				}
			}

			//**结果bmp
			result_bmp[1].bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
			result_bmp[1].bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
			result_bmp[1].bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
			result_bmp[1].bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
			result_bmp[1].bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

			result_bmp[1].bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight;
			result_bmp[1].bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth;
			result_bmp[1].bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
			result_bmp[1].bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
			result_bmp[1].bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
			result_bmp[1].bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
			result_bmp[1].bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
			result_bmp[1].bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
			result_bmp[1].bmp_info_header.biSizeImage = origin_bmp.bmp_info_header.biSizeImage;
			result_bmp[1].bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
			result_bmp[1].bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

			result_bmp[1].bmp_pixel8 = result_2_scale;	// 储存正交变换（模值，标定到[0,255]间的整数）
			result_bmp[1].biWidth_actual = origin_bmp.biWidth_actual;
		}
	}
	break;
	default:
	break;
	}

	return result_bmp;
}

//离散余弦变换（零填充）（通过计算对称扩展图的DFT来计算DCT，因效率太低而废弃）
/*bmp_file DCT_zero_padding(const bmp_file& origin_bmp, bool flag)
//{
//	bmp_file result_bmp;
//
//	switch (origin_bmp.bmp_info_header.biBitCount)
//	{
//	case 8:	// 8位256色调色板图
//	{
//		//*提取非四字节对齐像素到一零填充的扩展图像
//		int new_biWidth = origin_bmp.bmp_info_header.biWidth * 2;
//		int new_biWidth_extended = new_biWidth * 2;
//		int new_biHeight = origin_bmp.bmp_info_header.biHeight * 2;
//		int new_biHeight_extended = new_biHeight * 2;
//		int new_num_pixels = new_biHeight * new_biWidth;
//		int new_biWidth_actual = ((((origin_bmp.bmp_info_header.biBitCount * new_biWidth) + 31) & ~31) >> 3);
//		double temp_double = .0;
//		//*通过计算对称扩展图的DFT来计算DCT（《数字图像处理》第四版中译本P345）；二维DCT可拆分为两次一维DCT
//		//**系数向量s：只有第一个需要*1/sqrt(2.0)，直接乘即可
//		//**系数向量h
//		ComplexMatrix h_row(1, new_biWidth);		//用于行的DCT
//		for (int u = 0; u < new_biWidth; u++)
//		{
//			h_row.c[0][u].re = cos(-pi * u / 2 / new_biWidth);
//			h_row.c[0][u].im = sin(-pi * u / 2 / new_biWidth);
//			//h_row.c[0][u].re = cos(-pi * u / new_biWidth);
//			//h_row.c[0][u].im = sin(-pi * u / new_biWidth);
//		}
//		ComplexMatrix h_column(new_biHeight, 1);	//用于列的DCT
//		for (int u = 0; u < new_biHeight; u++)
//		{
//			h_column.c[u][0].re = cos(-pi * u / 2 / new_biHeight);
//			h_column.c[u][0].im = sin(-pi * u / 2 / new_biHeight);
//			//h_column.c[u][0].re = cos(-pi * u / new_biHeight);
//			//h_column.c[u][0].im = sin(-pi * u / new_biHeight);
//		}
//		//**第一次对行DCT
//		//***提取像素，同时列对称扩展（每行变两倍长）
//		ComplexMatrix origin_bmp_extended(new_biHeight, new_biWidth_extended); // 准备做第一次一维DCT
//		if (new_num_pixels > 1)	// 防止访问越界
//			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
//				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
//				{
//					temp_double = double(origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j]);
//					origin_bmp_extended.c[i][new_biWidth_extended - 1 - j].re = temp_double;
//					origin_bmp_extended.c[i][j].re = temp_double;
//				}
//		//***计算每行的一维DFT
//		ComplexMatrix DFT_each_row(new_biHeight, new_biWidth_extended);
//		Complex temp_complex_1(.0, .0), temp_complex_2(.0, .0);
//		for (int x = 0; x < new_biHeight; x++)
//			for (int v = 0; v < new_biWidth_extended; v++)
//				for (int y = 0; y < new_biWidth_extended; y++)
//				{
//					temp_complex_1.re = cos(-2.0 * pi * v * y / new_biWidth_extended);
//					temp_complex_1.im = sin(-2.0 * pi * v * y / new_biWidth_extended);
//					temp_complex_2 = origin_bmp_extended.c[x][y] * temp_complex_1;
//					DFT_each_row.c[x][v].re += temp_complex_2.re;
//					DFT_each_row.c[x][v].im += temp_complex_2.im;
//				}
//		//***哈达玛积（只提取结果的前半部分列为DCT结果，故后半部分列不用乘）
//		for (int i = 0; i < new_biHeight; i++)
//		{
//			DFT_each_row.c[i][0].re *= (1 / sqrt(2.0));
//			DFT_each_row.c[i][0].im *= (1 / sqrt(2.0));
//			for (int j = 0; j < new_biWidth; j++)
//			{
//				DFT_each_row.c[i][j] = DFT_each_row.c[i][j] * h_row.c[0][j];
//			}
//		}
//		//***提取结果的前半部分列为DCT结果，同时行对称扩展（每列变两倍长）
//		ComplexMatrix DCT_each_row_result(new_biHeight_extended, new_biWidth); // 准备做第二次一维DCT
//		for (int i = 0; i < new_biHeight; i++)
//			for (int j = 0; j < new_biWidth; j++)
//			{
//				temp_double = DFT_each_row.c[i][j].re;	// 只取实部
//				DCT_each_row_result.c[new_biHeight_extended - 1 - i][j].re = temp_double;
//				DCT_each_row_result.c[i][j].re = temp_double;
//			}
//		//***计算每列的一维DFT
//		ComplexMatrix DFT_each_column(new_biHeight_extended, new_biWidth);
//		for (int u = 0; u < new_biHeight_extended; u++)
//			for (int v = 0; v < new_biWidth; v++)
//				for (int x = 0; x < new_biHeight_extended; x++)
//				{
//					temp_complex_1.re = cos(-2.0 * pi * u * x / new_biHeight_extended);
//					temp_complex_1.im = sin(-2.0 * pi * u * x / new_biHeight_extended);
//					temp_complex_2 = DCT_each_row_result.c[x][v] * temp_complex_1;
//					DFT_each_column.c[u][v].re += temp_complex_2.re;
//					DFT_each_column.c[u][v].im += temp_complex_2.im;
//				}
//		//***哈达玛积（只提取结果的前半部分行为DCT结果，故后半部分行不用乘）
//		for (int j = 0; j < new_biWidth; j++)
//		{
//			DFT_each_column.c[0][j].re *= (1 / sqrt(2.0));
//			DFT_each_column.c[0][j].im *= (1 / sqrt(2.0));
//			for (int i = 0; i < new_biHeight; i++)
//			{
//				DFT_each_column.c[i][j] = DFT_each_column.c[i][j] * h_column.c[i][0];
//			}
//		}
//
//		//**提取结果的前半部分行为DCT结果，并计算频谱（模值）
//		//***DCT结果频谱所有项，及其求和
//		ComplexMatrix DCT_frequency_spectrum(new_biHeight, new_biWidth);
//		double temp_modulus = .0, temp_max = .0;	// 记录下全部幅度中的最大值，以便标定（假设幅度最小值是0）
//		for (int u = 0; u < new_biHeight; u++)
//			for (int v = 0; v < new_biWidth; v++)
//			{
//				temp_modulus = DFT_each_column.c[u][v].re;	// DCT只取实部
//
//				temp_modulus = log(1 + abs(temp_modulus));	// 转换为对数尺度以更好地可视化
//
//				DCT_frequency_spectrum.c[u][v].re = temp_modulus;
//				if (temp_max < temp_modulus)temp_max = temp_modulus;
//			}
//		//***标定到[0,255]
//		uint8_t* result_frequency_spectrum = new uint8_t[new_biWidth_actual * new_biHeight];
//		if (new_biWidth_actual * new_biHeight > 1) //防止访问越界
//			for (int u = 0; u < new_biHeight; u++)
//				for (int v = 0; v < new_biWidth; v++)
//				{
//					//****注意：上面得到的DCT结果图是行号翻转的
//					result_frequency_spectrum[u * new_biWidth_actual + v] = uint8_t(round(DCT_frequency_spectrum.c[new_biHeight - 1 - u][v].re / temp_max * 255));
//				}
//
//		//*结果bmp：尺寸变为2倍
//		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
//		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
//		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
//		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
//		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;
//
//		result_bmp.bmp_info_header.biHeight = new_biHeight;
//		result_bmp.bmp_info_header.biWidth = new_biWidth;
//		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
//		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
//		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
//		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
//		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
//		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
//		result_bmp.bmp_info_header.biSizeImage = ((((new_biWidth * result_bmp.bmp_info_header.biBitCount) + 31) & ~31) >> 3) * new_biHeight;
//		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
//		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;
//
//		result_bmp.bmp_pixel8 = result_frequency_spectrum;	// 储存正交变换频谱（模值，标定到[0,255]间的整数）
//		result_bmp.biWidth_actual = new_biWidth_actual;
//	}
//	break;
//	default:
//		break;
//	}
//	return result_bmp;
//}*/

#endif