#pragma once

#include"read_write_bmp.h"

//-形态学图像处理-

//腐蚀（二值图像，使用正方形全前景结构元）
bmp_file erosion(bmp_file& origin_bmp, int element_length)
{
	bmp_file result_bmp;
	//*用前景像素填充，填充长度为(element_length/2)
	int temp_biWidth = origin_bmp.bmp_info_header.biWidth + element_length / 2 * 2;
	int temp_biHeight = origin_bmp.bmp_info_header.biHeight + element_length / 2 * 2;
	int total_temp_pixel = temp_biHeight * temp_biWidth;
	uint8_t* temp_pixel = new uint8_t[total_temp_pixel];
	memset(temp_pixel, 0, total_temp_pixel);
	int temp_row_index = 0;
	int row_index = 0;
	if (total_temp_pixel > 1)	// 防止访问越界警告
	{
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			temp_row_index = (i + element_length / 2) * temp_biWidth + element_length / 2;
			row_index = i * origin_bmp.bmp_info_header.biWidth;
			for (int j = 0; j < origin_bmp.bmp_info_header.biHeight; j++)
				temp_pixel[temp_row_index + j] = origin_bmp.bmp_pixel1[row_index + j];
		}
	}
	//*腐蚀运算
	int total_pixel = origin_bmp.bmp_info_header.biHeight * origin_bmp.bmp_info_header.biWidth;
	uint8_t* result_pixel = new uint8_t[total_pixel];	// 存储腐蚀运算结果
	memset(result_pixel, 0, total_pixel);
	//**使用正方形全前景结构元时，可以直接判断：设结果为前景，然后如果周围像素有一个不是前景，则变为背景
	bool flag = false;
	if (total_pixel > 1 && total_temp_pixel > 1)	// 防止访问越界警告
	{
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			row_index = i * origin_bmp.bmp_info_header.biWidth;
			for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
			{
				for (int u = 0; u < element_length; u++)
				{
					temp_row_index = (i + u) * temp_biWidth + j;
					flag = false;
					for (int v = 0; v < element_length; v++)
					{
						if (temp_pixel[temp_row_index + v] != 0)
						{
							result_pixel[row_index + j] = 1;
							flag = true;
							break;
						}
					}
					if (flag)break;
				}
			}
		}
	}

	//*结果bmp
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

	result_bmp.bmp_pixel1 = result_pixel;
	result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

	delete[] temp_pixel;
	return result_bmp;
}

//边界提取（二值图像，使用正方形全前景结构元）
bmp_file boundary_extraction(bmp_file& origin_bmp, int element_length)
{
	//*先算腐蚀
	bmp_file erosion_bmp = erosion(origin_bmp, element_length);
	//*边界提取结果 = 原图 - 腐蚀
	int total_pixel = origin_bmp.bmp_info_header.biHeight * origin_bmp.bmp_info_header.biWidth;
	uint8_t* result_pixel = new uint8_t[total_pixel];	// 存储腐蚀运算结果
	memset(result_pixel, 0, total_pixel);
	int row_index = 0, index = 0;
	if (total_pixel > 1)	// 防止访问越界警告
	{
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			row_index = i * origin_bmp.bmp_info_header.biWidth;
			for (int j = 0; j < origin_bmp.bmp_info_header.biHeight; j++)
			{
				index = row_index + j;
				result_pixel[index] = (origin_bmp.bmp_pixel1[index] - erosion_bmp.bmp_pixel1[index] == 0) ? 1 : 0;
			}
		}
	}

	//*结果bmp
	bmp_file result_bmp;
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

	result_bmp.bmp_pixel1 = result_pixel;
	result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

	return result_bmp;
}