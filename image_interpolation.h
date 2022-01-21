#pragma once

#include<iostream>

#include"complex_matrix_operators.h"
#include"read_write_bmp.h"

//最邻近插值
bmp_file image_nearest_neighbor_interpolation(bmp_file origin_bmp, int new_biWidth, int new_biHeight)
{
	//四字节对齐问题
	int new_biWidth_actual = new_biWidth;
	//*8位256色位图
	if (origin_bmp.bmp_info_header.biBitCount == 8)
	{
		new_biWidth_actual = (8 * new_biWidth + 31) / 32 * 4;
	}

	//像素坐标归一化（用较大一方）并建立可查的表（表从0开始）
	//*横坐标（列）
	double* origin_x = new double[origin_bmp.bmp_info_header.biWidth];
	double* new_x = new double[new_biWidth];
	if (origin_bmp.biWidth_actual > new_biWidth) //原宽度更大
	{
		double increment = (double(origin_bmp.bmp_info_header.biWidth) - 1) / (double(new_biWidth) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biWidth; i++)
			origin_x[i] = i;
		for (int i = 0; i < new_biWidth; i++)
			new_x[i] = increment * i;
	}
	else
	{
		double increment = (double(new_biWidth) - 1) / (double(origin_bmp.bmp_info_header.biWidth) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biWidth; i++)
			origin_x[i] = increment * i;
		for (int i = 0; i < new_biWidth; i++)
			new_x[i] = i;
	}
	//*纵坐标（行）
	double* origin_y = new double[origin_bmp.bmp_info_header.biHeight];
	double* new_y = new double[new_biHeight];
	if (origin_bmp.bmp_info_header.biHeight > new_biHeight) //原高度更大
	{
		double increment = (double(origin_bmp.bmp_info_header.biHeight) - 1) / (double(new_biHeight) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			origin_y[i] = i;
		for (int i = 0; i < new_biHeight; i++)
			new_y[i] = increment * i;
	}
	else
	{
		double increment = (double(new_biHeight) - 1) / (double(origin_bmp.bmp_info_header.biHeight) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			origin_y[i] = increment * i;
		for (int i = 0; i < new_biHeight; i++)
			new_y[i] = i;
	}

	//处理新图每一个像素
	uint8_t* new_pixel = new uint8_t[new_biWidth_actual * new_biHeight];
	for (int i = 0; i < new_biHeight; i++)
		for (int j = 0; j < new_biWidth; j++)
		{
			//新像素点坐标(new_x[j],new_y[i])
			//找到最邻近的4个像素在原图中坐标；存储先下后上时，(k1,k2)是左下角
			//*横向搜索直到横坐标差分值正负号反转
			int k1 = 0;
			if (origin_bmp.bmp_info_header.biWidth >= 2)				//原图像宽大于或等于2时
				while (((origin_x[k1] - new_x[j]) * (origin_x[k1 + 1] - new_x[j])) > 0)
					k1++;
			//*纵向搜索直到纵坐标差分值正负号反转
			int k2 = 0;
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//原图像高大于或等于2时
				while (((origin_y[k2] - new_y[i]) * (origin_y[k2 + 1] - new_y[i])) > 0)
					k2++;
			//寻找4个点中最近的一个
			if (origin_bmp.bmp_info_header.biWidth >= 2)				//原图像宽大于或等于2时
				k1 = abs(origin_x[k1] - new_x[j]) <= abs(origin_x[k1 + 1] - new_x[j]) ? (k1) : (k1 + 1);
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//原图像高大于或等于2时
				k2 = abs(origin_y[k2] - new_y[i]) <= abs(origin_y[k2 + 1] - new_y[i]) ? (k2) : (k2 + 1);
			//取得最临近点像素的值，赋给新像素
			//*8位256色位图
			if (origin_bmp.bmp_info_header.biBitCount == 8)
			{
				new_pixel[i * new_biWidth_actual + j] = origin_bmp.bmp_pixel8[k2 * origin_bmp.biWidth_actual + k1];
			}
		}
	//结果图
	bmp_file result_bmp;

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
	result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter
		/ origin_bmp.bmp_info_header.biWidth * new_biWidth;
	result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter
		/ origin_bmp.bmp_info_header.biHeight * new_biHeight;

	result_bmp.bmp_pixel8 = new_pixel;
	result_bmp.biWidth_actual = new_biWidth_actual;

	delete[] origin_x, origin_y, new_x, new_y;
	return result_bmp;
}


//双线性插值
bmp_file image_bilinear_interpolation(bmp_file origin_bmp, int new_biWidth, int new_biHeight)
{
	//四字节对齐问题
	int new_biWidth_actual = new_biWidth;
	//*8位256色位图
	if (origin_bmp.bmp_info_header.biBitCount == 8)
	{
		new_biWidth_actual = (8 * new_biWidth + 31) / 32 * 4;
	}

	//像素坐标归一化（用较大一方）并建立可查的表（表从0开始）
	//*横坐标（列）
	double* origin_x = new double[origin_bmp.bmp_info_header.biWidth];
	double* new_x = new double[new_biWidth];
	double origin_x_increment = 0.0;
	if (origin_bmp.bmp_info_header.biWidth > new_biWidth) //原宽度更大
	{
		origin_x_increment = 1.0;
		for (int i = 0; i < origin_bmp.bmp_info_header.biWidth; i++)
			origin_x[i] = i;
		for (int i = 0; i < new_biWidth; i++)
			new_x[i] = (double(origin_bmp.bmp_info_header.biWidth) - 1) * i / (double(new_biWidth) - 1);
	}
	else
	{
		origin_x_increment = (double(new_biWidth) - 1) / (double(origin_bmp.bmp_info_header.biWidth) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biWidth; i++)
			origin_x[i] = origin_x_increment * i;
		for (int i = 0; i < new_biWidth; i++)
			new_x[i] = i;
	}
	//*纵坐标（行）
	double* origin_y = new double[origin_bmp.bmp_info_header.biHeight];
	double* new_y = new double[new_biHeight];
	double origin_y_increment = 0.0;
	if (origin_bmp.bmp_info_header.biHeight > new_biHeight) //原高度更大
	{
		origin_y_increment = 1.0;
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			origin_y[i] = i;
		for (int i = 0; i < new_biHeight; i++)
			new_y[i] = (double(origin_bmp.bmp_info_header.biHeight) - 1) * i / (double(new_biHeight) - 1);
	}
	else
	{
		origin_y_increment = (double(new_biHeight) - 1) / (double(origin_bmp.bmp_info_header.biHeight) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			origin_y[i] = origin_y_increment * i;
		for (int i = 0; i < new_biHeight; i++)
			new_y[i] = i;
	}

	//处理新图每一个像素
	uint8_t* new_pixel = new uint8_t[new_biWidth_actual * new_biHeight];
	for (int i = 0; i < new_biHeight; i++)
		for (int j = 0; j < new_biWidth; j++)
		{
			//新像素点坐标(new_x[j],new_y[i])
			//找到最邻近的4个像素在原图中坐标；存储先下后上时，(k1,k2)是左下角
			//*横向搜索直到横坐标差分值正负号反转
			int k1 = 0;
			if (origin_bmp.bmp_info_header.biWidth >= 2)				//原图像宽大于或等于2时
				while (((origin_x[k1] - new_x[j]) * (origin_x[k1 + 1] - new_x[j])) > 0)
					k1++;
			//*纵向搜索直到纵坐标差分值正负号反转
			int k2 = 0;
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//原图像高大于或等于2时
				while (((origin_y[k2] - new_y[i]) * (origin_y[k2 + 1] - new_y[i])) > 0)
					k2++;
			//使用临近的4个像素的值进行双线性插值，赋给新像素
			//找到最邻近的4个点——若正好位于原像素阵行/列上，则另一行/列插值权重为零，不需特殊处理
			//*8位256色位图
			if (origin_bmp.bmp_info_header.biBitCount == 8)
			{
				if (origin_bmp.biWidth_actual >= 2 && origin_bmp.bmp_info_header.biHeight >= 2)
					new_pixel[i * new_biWidth_actual + j] = uint8_t(
						((new_y[i] - origin_y[k2]) * (new_x[j] - origin_x[k1]) *
							origin_bmp.bmp_pixel8[(k2 + 1) * origin_bmp.biWidth_actual + (k1 + 1)]		//v22
							- (new_y[i] - origin_y[k2]) * (new_x[j] - origin_x[k1 + 1]) *
							origin_bmp.bmp_pixel8[(k2 + 1) * origin_bmp.biWidth_actual + (k1)]			//v12
							- (new_y[i] - origin_y[k2 + 1]) * (new_x[j] - origin_x[k1]) *
							origin_bmp.bmp_pixel8[(k2)*origin_bmp.biWidth_actual + (k1 + 1)]			//v21
							+ (new_y[i] - origin_y[k2 + 1]) * (new_x[j] - origin_x[k1 + 1]) *
							origin_bmp.bmp_pixel8[(k2)*origin_bmp.biWidth_actual + (k1)])				//v11
						/ (origin_x_increment * origin_y_increment));
				//else 3种情况：高=1，宽=1，只有1个像素
			}
		}
	//结果图
	bmp_file result_bmp;

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
	result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter
		/ origin_bmp.bmp_info_header.biWidth * new_biWidth;
	result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter
		/ origin_bmp.bmp_info_header.biHeight * new_biHeight;

	result_bmp.bmp_pixel8 = new_pixel;
	result_bmp.biWidth_actual = new_biWidth_actual;

	delete[] origin_x, origin_y, new_x, new_y;
	return result_bmp;
}



//双三次插值
bmp_file image_bicubic_interpolation(bmp_file origin_bmp, int new_biWidth, int new_biHeight)
{
	//四字节对齐问题
	int new_biWidth_actual = new_biWidth;
	//*8位256色位图
	if (origin_bmp.bmp_info_header.biBitCount == 8)
	{
		new_biWidth_actual = (8 * new_biWidth + 31) / 32 * 4;
	}

	//像素坐标归一化（用较大一方）并建立可查的表（表从0开始）
	//*横坐标（列）
	double* origin_x = new double[origin_bmp.bmp_info_header.biWidth];
	double* new_x = new double[new_biWidth];
	double origin_x_increment = 0.0;
	if (origin_bmp.bmp_info_header.biWidth > new_biWidth) //原宽度更大
	{
		origin_x_increment = 1.0;
		for (int i = 0; i < origin_bmp.bmp_info_header.biWidth; i++)
			origin_x[i] = i;
		for (int i = 0; i < new_biWidth; i++)
			new_x[i] = (double(origin_bmp.bmp_info_header.biWidth) - 1) * i / (double(new_biWidth) - 1);
	}
	else
	{
		origin_x_increment = (double(new_biWidth) - 1) / (double(origin_bmp.bmp_info_header.biWidth) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biWidth; i++)
			origin_x[i] = origin_x_increment * i;
		for (int i = 0; i < new_biWidth; i++)
			new_x[i] = i;
	}
	//*纵坐标（行）
	double* origin_y = new double[origin_bmp.bmp_info_header.biHeight];
	double* new_y = new double[new_biHeight];
	double origin_y_increment = 0.0;
	if (origin_bmp.bmp_info_header.biHeight > new_biHeight) //原高度更大
	{
		origin_y_increment = 1.0;
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			origin_y[i] = i;
		for (int i = 0; i < new_biHeight; i++)
			new_y[i] = (double(origin_bmp.bmp_info_header.biHeight) - 1) * i / (double(new_biHeight) - 1);
	}
	else
	{
		origin_y_increment = (double(new_biHeight) - 1) / (double(origin_bmp.bmp_info_header.biHeight) - 1);
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			origin_y[i] = origin_y_increment * i;
		for (int i = 0; i < new_biHeight; i++)
			new_y[i] = i;
	}

	//*归一化系数矩阵逆阵
	ComplexMatrix B_inv(4, 4, true);
	B_inv.c[0][0].re = -1.0 / 6;	B_inv.c[0][1].re = 1.0 / 2;		B_inv.c[0][2].re = -1.0 / 2; B_inv.c[0][3].re = 1.0 / 6;
	B_inv.c[1][0].re = 1.0;			B_inv.c[1][1].re = -5.0 / 2;	B_inv.c[1][2].re = 2.0;		 B_inv.c[1][3].re = -1.0 / 2;
	B_inv.c[2][0].re = -11.0 / 6;	B_inv.c[2][1].re = 3.0;			B_inv.c[2][2].re = -3.0 / 2; B_inv.c[2][3].re = 1.0 / 3;
	B_inv.c[3][0].re = 1.0;			B_inv.c[3][1].re = 0.0;			B_inv.c[3][2].re = 0.0;		 B_inv.c[3][3].re = 0.0;
	ComplexMatrix B_inv_T = (!B_inv);

	//处理新图每一个像素
	uint8_t* new_pixel = new uint8_t[new_biWidth_actual * new_biHeight];
	for (int i = 0; i < new_biHeight; i++)
		for (int j = 0; j < new_biWidth; j++)
		{
			//新像素点坐标(new_x[j],new_y[i])
			//找到最邻近的16个像素在原图中坐标；存储先下后上时，(k1,k2)是左下角
			//*横向搜索直到横坐标差分值正负号反转
			int k1 = 0;
			if (origin_bmp.bmp_info_header.biWidth >= 2)	//原图像高大于或等于2时
				while (((origin_x[k1] - new_x[j]) * (origin_x[k1 + 1] - new_x[j])) > 0)
					k1++;
			//*纵向搜索直到纵坐标差分值正负号反转
			int k2 = 0;
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//原图像宽大于或等于2时
				while (((origin_y[k2] - new_y[i]) * (origin_y[k2 + 1] - new_y[i])) > 0)
					k2++;

			//使用临近的16个像素的值进行双三次插值，赋给新像素
			//找到最邻近的16个点时若正好位于原像素阵行/列上，则另一行/列插值权重为零，不需特殊处理
			//*取得16个像素的思路：看临近2*2=4点的左下角最后停在哪里，分横/纵坐标看是否触边，是的话就改起始点的横/纵坐标
			//*最后起始点固定为要找的4*4点阵的左下角，根据起始点可以找到16个像素
			int x_start = k1;
			if (k1 > 0 && k1 < origin_bmp.bmp_info_header.biWidth - 1) x_start = k1 - 1;
			else if (k1 == 0) x_start = 0;
			else x_start = k1 - 2;
			int y_start = k2;
			if (k2 > 0 && k2 < origin_bmp.bmp_info_header.biHeight - 1) y_start = k2 - 1;
			else if (k2 == 0) y_start = 0;
			else y_start = k2 - 2;
			ComplexMatrix V(4, 4, true);
			for (int ii = 0; ii < 4; ii++)
				for (int jj = 0; jj < 4; jj++)
					V.c[ii][jj].re = origin_bmp.bmp_pixel8[(y_start + jj) * origin_bmp.biWidth_actual + x_start + ii];
			ComplexMatrix A = B_inv * V;
			A = A * B_inv_T;
			//*8位256色位图
			if (origin_bmp.bmp_info_header.biBitCount == 8)
			{
				if (origin_bmp.bmp_info_header.biWidth >= 4 && origin_bmp.bmp_info_header.biHeight >= 4)
				{
					double x = (new_x[j] - origin_x[x_start]) / origin_x_increment;
					ComplexMatrix xx(1, 4, true);
					xx.c[0][0].re = pow(x, 3); xx.c[0][1].re = pow(x, 2); xx.c[0][2].re = x; xx.c[0][3].re = 1.0;
					//xx.c[0][0].re = 1.0; xx.c[0][1].re = x; xx.c[0][2].re = pow(x, 2); xx.c[0][3].re = pow(x, 3);
					double y = (new_y[i] - origin_y[y_start]) / origin_y_increment;
					ComplexMatrix yy(4, 1, true);
					yy.c[0][0].re = pow(y, 3); yy.c[1][0].re = pow(y, 2); yy.c[2][0].re = y; yy.c[3][0].re = 1.0;
					//yy.c[0][0].re = 1.0; yy.c[1][0].re = y; yy.c[2][0].re = pow(y, 2); yy.c[3][0].re = pow(y, 3);
					ComplexMatrix result = xx * A;
					result = result * yy;
					//new_pixel[i * new_biWidth_actual + j] = BYTE(result.c[0][0].re);
					if (result.c[0][0].re >= 0.0 && result.c[0][0].re <= 255)new_pixel[i * new_biWidth_actual + j] = BYTE(result.c[0][0].re);
					else if (result.c[0][0].re < 0)new_pixel[i * new_biWidth_actual + j] = 0;
					else new_pixel[i * new_biWidth_actual + j] = 255;
				}
				//else 3种情况：高<4，宽<4，都<4
			}
		}
	//结果图
	bmp_file result_bmp;

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
	result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter
		/ origin_bmp.bmp_info_header.biWidth * new_biWidth;
	result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter
		/ origin_bmp.bmp_info_header.biHeight * new_biHeight;

	result_bmp.bmp_pixel8 = new_pixel;
	result_bmp.biWidth_actual = new_biWidth_actual;

	delete[] origin_x, origin_y, new_x, new_y;
	return result_bmp;
}