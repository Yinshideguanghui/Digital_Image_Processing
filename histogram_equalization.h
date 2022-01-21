#pragma once

#include<iostream>
#include<cmath>

#include"read_write_bmp.h"

//直方图均衡化
bmp_file histogram_equalize(bmp_file origin_bmp)
{
	int num_pixel = origin_bmp.bmp_info_header.biWidth * origin_bmp.bmp_info_header.biHeight;
	int num_pixel_actual = origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight;
	bmp_file result_bmp;
	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:		//* 8位（256色）bmp
	{
		//提取像素
		uint8_t* pixel = new uint8_t[num_pixel_actual];
		long temp_index = 0;
		if (num_pixel_actual > 1)	//以防pixel访问越界
			for (long i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				for (long j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
					pixel[i * origin_bmp.biWidth_actual + j] = origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];
				//for (long j= origin_bmp.bmp_info_header.biWidth;j<origin_bmp.biWidth_actual;j++)
				//	pixel[i * origin_bmp.bmp_info_header.biWidth + j]
			}
		int* hist_counting = hist_0_255(pixel, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, origin_bmp.biWidth_actual);
		uint8_t* transformation_result = new uint8_t[256];
		int total_counting = 0;
		//*L-1 = 256-1 = 255
		for (int i = 0; i < 256; i++)
		{
			total_counting += hist_counting[i];
			transformation_result[i] = uint8_t(round(255.0 * total_counting / num_pixel));
		}
		if (num_pixel_actual > 1)	//以防pixel访问越界
			for (long i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
				for (long j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					temp_index = i * origin_bmp.bmp_info_header.biWidth + j;
					uint8_t temp = transformation_result[pixel[temp_index]];
					pixel[temp_index] = temp;
				}
		//结果bmp
		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp.bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight;
		result_bmp.bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth;
		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp.bmp_info_header.biSizeImage = origin_bmp.bmp_info_header.biSizeImage;
		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp.bmp_pixel8 = pixel;
		result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

		delete[] transformation_result;
	}
	break;
	case 24:	//* 24位（真彩色）bmp
	{
		//**转换到HSI域
		origin_bmp.bmp_pixel24_H_double = new double[num_pixel];
		origin_bmp.bmp_pixel24_S_double = new double[num_pixel];
		origin_bmp.bmp_pixel24_I_double = new double[num_pixel];
		memset(origin_bmp.bmp_pixel24_H_double, 0, sizeof(double) * num_pixel);
		memset(origin_bmp.bmp_pixel24_S_double, 0, sizeof(double) * num_pixel);
		memset(origin_bmp.bmp_pixel24_I_double, 0, sizeof(double) * num_pixel);
		double temp_theta = 0.0;
		double temp_R = 0.0, temp_G = 0.0, temp_B = 0.0;
		double R_G_B_sum = 0.0;
		double R_minus_G = 0.0, R_minus_B = 0.0, G_minus_B = 0.0;
		int temp_row_index = 0, temp_index = 0;
		if (num_pixel > 1)	//以防访问越界
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				temp_row_index = i * origin_bmp.bmp_info_header.biWidth;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					temp_index = temp_row_index + j;
					temp_B = double(origin_bmp.bmp_pixel24_RGB[temp_index].rgbtBlue) / 255;
					temp_G = double(origin_bmp.bmp_pixel24_RGB[temp_index].rgbtGreen) / 255;
					temp_R = double(origin_bmp.bmp_pixel24_RGB[temp_index].rgbtRed) / 255;
					R_minus_G = temp_R - temp_G;
					R_minus_B = temp_R - temp_B;
					G_minus_B = temp_G - temp_B;
					R_G_B_sum = temp_B + temp_G + temp_R;
					//***色调
					temp_theta = acos(0.5 * ((R_minus_G)+(R_minus_B)) / sqrt(R_minus_G * R_minus_G + R_minus_B * G_minus_B + 1e-300)); //避免除以0
					temp_theta = abs(temp_theta * 180 / pi); //弧度转换成角度
					origin_bmp.bmp_pixel24_H_double[temp_index] = temp_B > temp_G ? (360.0 - temp_theta) : temp_theta;
					//***饱和度
					if (origin_bmp.bmp_pixel24_RGB[temp_index].rgbtBlue == 0 && origin_bmp.bmp_pixel24_RGB[temp_index].rgbtGreen == 0
						&& origin_bmp.bmp_pixel24_RGB[temp_index].rgbtRed == 0)	// 原R=G=B=0，即纯黑时，饱和度为0
						origin_bmp.bmp_pixel24_S_double[temp_index] = 0.0;
					else
						origin_bmp.bmp_pixel24_S_double[temp_index] = 1.0 - 3.0 * min(min(temp_R, temp_G), temp_B) / (R_G_B_sum);

					//***亮度
					origin_bmp.bmp_pixel24_I_double[temp_index] = R_G_B_sum / 3;
				}
			}
		//**对亮度直方图均衡化处理
		int top = 255;
		int* I_integer = new int[num_pixel];
		for (int i = 0; i < num_pixel; i++)I_integer[i] = CLAMP_0_x(origin_bmp.bmp_pixel24_I_double[i] * top, top);
		int* hist_counting = hist_0_x(I_integer, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, top);
		int* transformation_result = new int[top + 1];
		int total_counting = 0;
		//***L-1 = 256-1 = 255 或 L-1 = top
		for (int i = 0; i <= top; i++)
		{
			total_counting += hist_counting[i];
			transformation_result[i] = (int)(double(top) * total_counting / num_pixel + 0.5);
		}
		if (num_pixel > 1)	//以防访问越界
			for (int i = 0; i < num_pixel; i++)
				{
					int temp = I_integer[i];
					I_integer[i] = transformation_result[temp];
				}

		//**转换回RGB域
		double* I_result = new double[num_pixel];
		for (int i = 0; i < num_pixel; i++)I_result[i] = double(I_integer[i]) / top;
		tagRGBTRIPLE* new_pixel24_RGB = new tagRGBTRIPLE[origin_bmp.bmp_info_header.biWidth * origin_bmp.bmp_info_header.biHeight];
		double temp_H = 0.0;
		if (num_pixel > 1)	//以防访问越界
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				temp_row_index = i * origin_bmp.bmp_info_header.biWidth;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					temp_index = temp_row_index + j;
					temp_H = origin_bmp.bmp_pixel24_H_double[temp_index];
					if (temp_H < 120)
					{
						temp_H = temp_H * pi / 180; // 变为弧度制以便<cmath>的cos处理
						temp_B = I_result[temp_index] * (1.0 - origin_bmp.bmp_pixel24_S_double[temp_index]);
						temp_R = I_result[temp_index] * (1.0 + (origin_bmp.bmp_pixel24_S_double[temp_index] * cos(temp_H) / cos(pi / 3 - temp_H)));
						temp_G = 3.0 * I_result[temp_index] - (temp_R + temp_B);
						new_pixel24_RGB[temp_index].rgbtBlue = CLAMP_0_255(temp_B * 255);
						new_pixel24_RGB[temp_index].rgbtRed = CLAMP_0_255(temp_R * 255);
						new_pixel24_RGB[temp_index].rgbtGreen = CLAMP_0_255(temp_G * 255);
					}
					else if (temp_H >= 120 && temp_H < 240)
					{
						temp_H -= 120;
						temp_H = temp_H * pi / 180; // 变为弧度制以便<cmath>的cos处理
						temp_R = I_result[temp_index] * (1.0 - origin_bmp.bmp_pixel24_S_double[temp_index]);
						temp_G = I_result[temp_index] * (1.0 + (origin_bmp.bmp_pixel24_S_double[temp_index] * cos(temp_H) / cos(pi / 3 - temp_H)));
						temp_B = 3.0 * I_result[temp_index] - (temp_R + temp_G);
						new_pixel24_RGB[temp_index].rgbtBlue = CLAMP_0_255(temp_B * 255);
						new_pixel24_RGB[temp_index].rgbtRed = CLAMP_0_255(temp_R * 255);
						new_pixel24_RGB[temp_index].rgbtGreen = CLAMP_0_255(temp_G * 255);
					}
					else if (temp_H >= 240 && temp_H <= 360)
					{
						temp_H -= 240;
						temp_H = temp_H * pi / 180; // 变为弧度制以便<cmath>的cos处理
						temp_G = I_result[temp_index] * (1.0 - origin_bmp.bmp_pixel24_S_double[temp_index]);
						temp_B = I_result[temp_index] * (1.0 + (origin_bmp.bmp_pixel24_S_double[temp_index] * cos(temp_H) / cos(pi / 3 - temp_H)));
						temp_R = 3.0 * I_result[temp_index] - (temp_G + temp_B);
						new_pixel24_RGB[temp_index].rgbtBlue = CLAMP_0_255(temp_B * 255);
						new_pixel24_RGB[temp_index].rgbtRed = CLAMP_0_255(temp_R * 255);
						new_pixel24_RGB[temp_index].rgbtGreen = CLAMP_0_255(temp_G * 255);
					}
				}
			}

		//**结果bmp
		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp.bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight;
		result_bmp.bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth;
		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp.bmp_info_header.biSizeImage = origin_bmp.bmp_info_header.biSizeImage;
		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp.biWidth_actual = origin_bmp.biWidth_actual;
		result_bmp.bmp_pixel24_RGB = new_pixel24_RGB;

		delete[] I_integer, I_result, transformation_result;
	}
	break;
	default:
	break;
	}

	return result_bmp;
}

//直方图均衡化：可指定图片偏亮或偏暗
bmp_file histogram_equalize(bmp_file origin_bmp, bool dark)
{
	int num_pixel = origin_bmp.bmp_info_header.biWidth * origin_bmp.bmp_info_header.biHeight;
	int num_pixel_actual = origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight;
	bmp_file result_bmp;
	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:		//* 8位（256色）bmp
	{
		//提取像素
		uint8_t* pixel = new uint8_t[num_pixel_actual];
		long temp_index = 0;
		if (num_pixel_actual > 1)	//以防pixel访问越界
			for (long i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				for (long j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
					pixel[i * origin_bmp.biWidth_actual + j] = origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];
				//for (long j= origin_bmp.bmp_info_header.biWidth;j<origin_bmp.biWidth_actual;j++)
				//	pixel[i * origin_bmp.bmp_info_header.biWidth + j]
			}
		int* hist_counting = hist_0_255(pixel, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, origin_bmp.biWidth_actual);
		uint8_t* transformation_result = new uint8_t[256];
		int total_counting = 0;
		//*L-1 = 256-1 = 255
		for (int i = 0; i < 256; i++)
		{
			total_counting += hist_counting[i];
			transformation_result[i] = uint8_t(round(255.0 * total_counting / num_pixel));
		}
		if (num_pixel_actual > 1)	//以防pixel访问越界
			for (long i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
				for (long j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					temp_index = i * origin_bmp.bmp_info_header.biWidth + j;
					uint8_t temp = transformation_result[pixel[temp_index]];
					pixel[temp_index] = temp;
				}
		//结果bmp
		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp.bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight;
		result_bmp.bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth;
		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp.bmp_info_header.biSizeImage = origin_bmp.bmp_info_header.biSizeImage;
		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp.bmp_pixel8 = pixel;
		result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

		delete[] transformation_result;
	}
	break;
	case 24:	//* 24位（真彩色）bmp
	{
		//**转换到HSI域
		origin_bmp.bmp_pixel24_H_double = new double[num_pixel];
		origin_bmp.bmp_pixel24_S_double = new double[num_pixel];
		origin_bmp.bmp_pixel24_I_double = new double[num_pixel];
		memset(origin_bmp.bmp_pixel24_H_double, 0, sizeof(double) * num_pixel);
		memset(origin_bmp.bmp_pixel24_S_double, 0, sizeof(double) * num_pixel);
		memset(origin_bmp.bmp_pixel24_I_double, 0, sizeof(double) * num_pixel);
		double temp_theta = 0.0;
		double temp_R = 0.0, temp_G = 0.0, temp_B = 0.0;
		double R_G_B_sum = 0.0;
		double R_minus_G = 0.0, R_minus_B = 0.0, G_minus_B = 0.0;
		int temp_row_index = 0, temp_index = 0;
		if (num_pixel > 1)	//以防访问越界
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				temp_row_index = i * origin_bmp.bmp_info_header.biWidth;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					temp_index = temp_row_index + j;
					temp_B = double(origin_bmp.bmp_pixel24_RGB[temp_index].rgbtBlue) / 255;
					temp_G = double(origin_bmp.bmp_pixel24_RGB[temp_index].rgbtGreen) / 255;
					temp_R = double(origin_bmp.bmp_pixel24_RGB[temp_index].rgbtRed) / 255;
					R_minus_G = temp_R - temp_G;
					R_minus_B = temp_R - temp_B;
					G_minus_B = temp_G - temp_B;
					R_G_B_sum = temp_B + temp_G + temp_R;
					//***色调
					temp_theta = acos(0.5 * ((R_minus_G)+(R_minus_B)) / sqrt(R_minus_G * R_minus_G + R_minus_B * G_minus_B + 1e-300)); //避免除以0
					temp_theta = abs(temp_theta * 180 / pi); //弧度转换成角度
					origin_bmp.bmp_pixel24_H_double[temp_index] = temp_B > temp_G ? (360.0 - temp_theta) : temp_theta;
					//***饱和度
					if (origin_bmp.bmp_pixel24_RGB[temp_index].rgbtBlue == 0 && origin_bmp.bmp_pixel24_RGB[temp_index].rgbtGreen == 0
						&& origin_bmp.bmp_pixel24_RGB[temp_index].rgbtRed == 0)	// 原R=G=B=0，即纯黑时，饱和度为0
						origin_bmp.bmp_pixel24_S_double[temp_index] = 0.0;
					else
						origin_bmp.bmp_pixel24_S_double[temp_index] = 1.0 - 3.0 * min(min(temp_R, temp_G), temp_B) / (R_G_B_sum);

					//***亮度
					origin_bmp.bmp_pixel24_I_double[temp_index] = R_G_B_sum / 3;
				}
			}
		//**对亮度直方图均衡化处理
		int top = 255;
		int* I_integer = new int[num_pixel];
		for (int i = 0; i < num_pixel; i++)I_integer[i] = CLAMP_0_x(origin_bmp.bmp_pixel24_I_double[i] * top, top);
		int* hist_counting = hist_0_x(I_integer, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, top);
		int* transformation_result = new int[top + 1];
		int total_counting = 0;
		//***L-1 = 256-1 = 255
		//***L-1 = top
		for (int i = 0; i <= top; i++)
		{
			total_counting += hist_counting[i];
			transformation_result[i] = (int)(double(top) * total_counting / num_pixel + 0.5);
		}
		if (num_pixel > 1)	//以防访问越界
			for (int i = 0; i < num_pixel; i++)
			{
				int temp = I_integer[i];
				I_integer[i] = transformation_result[temp];
			}

		////**饱和度均衡化处理
		if (!dark)
			for (int i = 0; i < num_pixel; i++)
			{
				if (origin_bmp.bmp_pixel24_I_double[i] <= 0.5)
				{
					if (origin_bmp.bmp_pixel24_S_double[i] <= origin_bmp.bmp_pixel24_I_double[i] * 2)
						origin_bmp.bmp_pixel24_S_double[i] = origin_bmp.bmp_pixel24_S_double[i] * 6;
					else
						origin_bmp.bmp_pixel24_S_double[i] = 0.0;
				}
				else
				{
					if (origin_bmp.bmp_pixel24_S_double[i] <= (1.0 - origin_bmp.bmp_pixel24_I_double[i]) * 2)
						origin_bmp.bmp_pixel24_S_double[i] = origin_bmp.bmp_pixel24_S_double[i] * 6;
					else
						origin_bmp.bmp_pixel24_S_double[i] = 0.0;
				}
				//origin_bmp.bmp_pixel24_S_double[i] = 6 * origin_bmp.bmp_pixel24_S_double[i] * (1 - origin_bmp.bmp_pixel24_S_double[i]);
				if (origin_bmp.bmp_pixel24_S_double[i] > 1)origin_bmp.bmp_pixel24_S_double[i] = 1.0;
			}

		//**转换回RGB域
		double* I_result = new double[num_pixel];
		for (int i = 0; i < num_pixel; i++)I_result[i] = double(I_integer[i]) / top;
		tagRGBTRIPLE* new_pixel24_RGB = new tagRGBTRIPLE[origin_bmp.bmp_info_header.biWidth * origin_bmp.bmp_info_header.biHeight];
		double temp_H = 0.0;
		if (num_pixel > 1)	//以防访问越界
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				temp_row_index = i * origin_bmp.bmp_info_header.biWidth;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					temp_index = temp_row_index + j;
					temp_H = origin_bmp.bmp_pixel24_H_double[temp_index];
					if (temp_H < 120)
					{
						temp_H = temp_H * pi / 180; // 变为弧度制以便<cmath>的cos处理
						temp_B = I_result[temp_index] * (1.0 - origin_bmp.bmp_pixel24_S_double[temp_index]);
						temp_R = I_result[temp_index] * (1.0 + (origin_bmp.bmp_pixel24_S_double[temp_index] * cos(temp_H) / cos(pi / 3 - temp_H)));
						temp_G = 3.0 * I_result[temp_index] - (temp_R + temp_B);
						new_pixel24_RGB[temp_index].rgbtBlue = CLAMP_0_255(temp_B * 255);
						new_pixel24_RGB[temp_index].rgbtRed = CLAMP_0_255(temp_R * 255);
						new_pixel24_RGB[temp_index].rgbtGreen = CLAMP_0_255(temp_G * 255);
					}
					else if (temp_H >= 120 && temp_H < 240)
					{
						temp_H -= 120;
						temp_H = temp_H * pi / 180; // 变为弧度制以便<cmath>的cos处理
						temp_R = I_result[temp_index] * (1.0 - origin_bmp.bmp_pixel24_S_double[temp_index]);
						temp_G = I_result[temp_index] * (1.0 + (origin_bmp.bmp_pixel24_S_double[temp_index] * cos(temp_H) / cos(pi / 3 - temp_H)));
						temp_B = 3.0 * I_result[temp_index] - (temp_R + temp_G);
						new_pixel24_RGB[temp_index].rgbtBlue = CLAMP_0_255(temp_B * 255);
						new_pixel24_RGB[temp_index].rgbtRed = CLAMP_0_255(temp_R * 255);
						new_pixel24_RGB[temp_index].rgbtGreen = CLAMP_0_255(temp_G * 255);
					}
					else if (temp_H >= 240 && temp_H <= 360)
					{
						temp_H -= 240;
						temp_H = temp_H * pi / 180; // 变为弧度制以便<cmath>的cos处理
						temp_G = I_result[temp_index] * (1.0 - origin_bmp.bmp_pixel24_S_double[temp_index]);
						temp_B = I_result[temp_index] * (1.0 + (origin_bmp.bmp_pixel24_S_double[temp_index] * cos(temp_H) / cos(pi / 3 - temp_H)));
						temp_R = 3.0 * I_result[temp_index] - (temp_G + temp_B);
						new_pixel24_RGB[temp_index].rgbtBlue = CLAMP_0_255(temp_B * 255);
						new_pixel24_RGB[temp_index].rgbtRed = CLAMP_0_255(temp_R * 255);
						new_pixel24_RGB[temp_index].rgbtGreen = CLAMP_0_255(temp_G * 255);
					}
				}
			}

		//**结果bmp
		result_bmp.bmp_file_header.bfType = origin_bmp.bmp_file_header.bfType;
		result_bmp.bmp_file_header.bfReserved1 = origin_bmp.bmp_file_header.bfReserved1;
		result_bmp.bmp_file_header.bfReserved2 = origin_bmp.bmp_file_header.bfReserved2;
		result_bmp.bmp_file_header.bfSize = origin_bmp.bmp_file_header.bfSize;
		result_bmp.bmp_file_header.bfOffBits = origin_bmp.bmp_file_header.bfOffBits;

		result_bmp.bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight;
		result_bmp.bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth;
		result_bmp.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
		result_bmp.bmp_info_header.biClrImportant = origin_bmp.bmp_info_header.biClrUsed;
		result_bmp.bmp_info_header.biClrUsed = origin_bmp.bmp_info_header.biClrImportant;
		result_bmp.bmp_info_header.biCompression = origin_bmp.bmp_info_header.biCompression;
		result_bmp.bmp_info_header.biPlanes = origin_bmp.bmp_info_header.biPlanes;
		result_bmp.bmp_info_header.biSize = origin_bmp.bmp_info_header.biSize;
		result_bmp.bmp_info_header.biSizeImage = origin_bmp.bmp_info_header.biSizeImage;
		result_bmp.bmp_info_header.biXPelsPerMeter = origin_bmp.bmp_info_header.biXPelsPerMeter;
		result_bmp.bmp_info_header.biYPelsPerMeter = origin_bmp.bmp_info_header.biYPelsPerMeter;

		result_bmp.biWidth_actual = origin_bmp.biWidth_actual;
		result_bmp.bmp_pixel24_RGB = new_pixel24_RGB;

		delete[] I_integer, I_result, transformation_result;
	}
	break;
	default:
		break;
	}

	return result_bmp;
}
