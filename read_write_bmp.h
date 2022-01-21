#pragma once

#include<stdio.h>
#include<iostream>
#include<string>
#include<Windows.h>
#include<malloc.h>
#include<wingdi.h>

using namespace std;

constexpr auto pi = 3.141592653589793;

struct bmp_file
{
	BITMAPFILEHEADER bmp_file_header;
	BITMAPINFOHEADER bmp_info_header;
	uint8_t* bmp_pixel8 = NULL;
	double* bmp_pixel8_double = NULL;
	tagRGBTRIPLE* bmp_pixel24_RGB = NULL;
	double* bmp_pixel24_H_double = NULL;
	double* bmp_pixel24_S_double = NULL;
	double* bmp_pixel24_I_double = NULL;
	uint8_t* bmp_pixel1 = NULL;
	int biWidth_actual = 0;
};

//工具：限幅到0和255间
uint8_t CLAMP_0_255(double data)
{
	if (data >= 0 && data <= 255)return uint8_t(round(data));
	else if (data < 0)return 0;
	else return 255;
}
//工具：限幅到0和255间
uint8_t CLAMP_0_255(int data)
{
	if (data >= 0 && data <= 255)return uint8_t(round(data));
	else if (data < 0)return 0;
	else return 255;
}
//工具：限幅到0和x间
uint8_t CLAMP_0_x(double data, uint8_t top)
{
	if (data >= 0 && data <= top)return uint8_t(round(data));
	else if (data < 0)return 0;
	else return top;
}


//写入图像四字节对齐问题
//*方案：填充全0像素，但不计入显示宽度
void writing_four_byte_alignment(bmp_file& bmp_1)
{
	switch (bmp_1.bmp_info_header.biBitCount)
	{
	case 1:	// 由于临时存储初始化为零，且写入采用加的方式，无需单独四字节对齐
	break;
	case 8:
	{
		if (bmp_1.bmp_info_header.biWidth % 4 != 0)
			for (int i = 0; i < bmp_1.bmp_info_header.biHeight; i++)
				for (int j = bmp_1.bmp_info_header.biWidth; j < bmp_1.biWidth_actual; j++)
					bmp_1.bmp_pixel8[i * bmp_1.biWidth_actual + j] = 0;
	}
	break;
	case 24:
	{
		if ((bmp_1.bmp_info_header.biWidth * 3) % 4 != 0)
			for (int i = 0; i < bmp_1.bmp_info_header.biHeight; i++)
				for (int j = bmp_1.bmp_info_header.biWidth * 3; j < bmp_1.biWidth_actual; j++)
					bmp_1.bmp_pixel8[i * bmp_1.biWidth_actual + j] = 0;
	}
	break;
	default:
	break;
	}
	return;
}

//工具：0-255间整数直方图计数
int* hist_0_255(const uint8_t* data, int size)
{
	int* count = new int[256];
	for (int i = 0; i < 256; i++)count[i] = 0;
	uint8_t temp = 0;
	for (int i = 0; i < size; i++)
	{
		temp = data[i];
		if (temp >= 0 && temp <= 255)
			count[temp]++;
		else if (temp < 0)count[0]++;
		else count[255]++;
	}
	return count;
}
//工具：0-255间整数直方图计数——二维方式组织的数据
int* hist_0_255(const uint8_t* data, int width, int height)
{
	int* count = new int[256];
	for (int i = 0; i < 256; i++)count[i] = 0;
	uint8_t temp = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			temp = data[i * width + j];
			if (temp >= 0 && temp <= 255)
				count[temp]++;
			else if (temp < 0)count[0]++;
			else count[255]++;
		}
	return count;
}
//工具：0-255间整数直方图计数——二维方式组织的数据；每行末尾可能有空
int* hist_0_255(const uint8_t* data, int width, int height, int width_actual_total)
{
	int* count = new int[256];
	for (int i = 0; i < 256; i++)count[i] = 0;
	uint8_t temp = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			temp = data[i * width_actual_total + j];
			if (temp >= 0 && temp <= 255)
				count[temp]++;
			else if (temp < 0)count[0]++;
			else count[255]++;
		}
	return count;
}
//工具：0-x间整数直方图计数——二维方式组织的数据
int* hist_0_x(const uint8_t* data, int width, int height, int top)
{
	int* count = new int[top + 1];
	for (int i = 0; i < top + 1; i++)count[i] = 0;
	uint8_t temp = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			temp = data[i * width + j];
			if (temp >= 0 && temp <= top)
				count[temp]++;
			else if (temp < 0)count[0]++;
			else count[top]++;
		}
	return count;
}
int* hist_0_x(const int* data, int width, int height, int top)
{
	int* count = new int[top + 1];
	for (int i = 0; i < top + 1; i++)count[i] = 0;
	int temp = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			temp = data[i * width + j];
			if (temp >= 0 && temp <= top)
				count[temp]++;
			else if (temp < 0)count[0]++;
			else count[top]++;
		}
	return count;
}

//读入bmp
bmp_file read_bmp(string file_path)
{
	bmp_file bmp_1{};
	//* 二进制方式只读打开
	FILE* file_ptr_1 = fopen(file_path.c_str(), "rb");
	if (file_ptr_1)
	{
		//** 获取文件长度
		fseek(file_ptr_1, 0L, SEEK_END);
		int file_length = ftell(file_ptr_1);
		//** 读取文件内容
		//*** BITMAPFILEHEADER
		fseek(file_ptr_1, 0L, SEEK_SET);
		fread(&bmp_1.bmp_file_header, sizeof(BITMAPFILEHEADER), 1, file_ptr_1);
		//*** BITMAPINFOHEADER
		fseek(file_ptr_1, sizeof(BITMAPFILEHEADER), SEEK_SET);
		fread(&bmp_1.bmp_info_header, sizeof(BITMAPINFOHEADER), 1, file_ptr_1);
		//*** 像素信息
		fseek(file_ptr_1, bmp_1.bmp_file_header.bfOffBits, SEEK_SET);
		//**** 有四字节对齐补齐时实际每行字节数（可先算8*显示宽度对32即四字节比特长度向上取“整”，再看有多少个四字节，也可用下式）
		bmp_1.biWidth_actual = ((((bmp_1.bmp_info_header.biBitCount * bmp_1.bmp_info_header.biWidth) + 31) & ~31) >> 3);
		//**** 读取像素
		switch (bmp_1.bmp_info_header.biBitCount)
		{
		case 1:		//**** 每个像素1位时，每个字节代表8个像素
		{
			int pixel_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.bmp_info_header.biWidth;
			bmp_1.bmp_pixel1 = new uint8_t[pixel_total_num];
			uint8_t* temp_byte = new uint8_t[4];	// 以四字节为单位读入
			memset(temp_byte, 0, sizeof(uint8_t) * 4);
			int times_of_read_per_row = bmp_1.biWidth_actual / 4 - 1;	// 每行读入整四字节的次数（最后一次不算在内，因为可能不读满，单独处理）
			int times_of_read_bit_last = bmp_1.bmp_info_header.biWidth - times_of_read_per_row * 32; // 最后一次应该读取的比特数
			uint8_t* temp_bit_last = new uint8_t[32];	// 临时存放最后一次读出的所有比特
			int temp_index = 0; // 索引计算中间变量1
			int temp_row_index = 0; // 索引计算中间变量2
			for (int ordinal_of_row = 0; ordinal_of_row < bmp_1.bmp_info_header.biHeight; ordinal_of_row++)	// 分行读入
			{
				temp_row_index = ordinal_of_row * bmp_1.bmp_info_header.biWidth;
				// 最后一个四字节之前，必定读满四字节
				for (int ordinal_of_read_in_column = 0; ordinal_of_read_in_column < times_of_read_per_row; ordinal_of_read_in_column++)
				{
					temp_index = temp_row_index + ordinal_of_read_in_column * 32;
					fread(temp_byte, sizeof(uint8_t), 4, file_ptr_1);
					for (int i = 0; i < 4; i++)	// 四字节分别拆分成比特，读入uint8_t数组
						for (int j = 0; j < 8; j++)
							bmp_1.bmp_pixel1[temp_index + i * 8 + j] = (temp_byte[i] >> (7 - j)) % 2;
				}
				// 最后一个四字节
				temp_index = temp_row_index + times_of_read_per_row * 32;
				fread(temp_byte, sizeof(uint8_t), 4, file_ptr_1);
				for (int i = 0; i < 4; i++)	// 四字节分别拆分成比特，读入uint8_t数组
					for (int j = 0; j < 8; j++)
						temp_bit_last[i * 8 + j] = (temp_byte[i] >> (7 - j)) % 2;
				for (int k = 0; k < times_of_read_bit_last; k++)	// 读入最后的比特
				{
					bmp_1.bmp_pixel1[temp_index + k] = temp_bit_last[k];
				}
			}
			fseek(file_ptr_1, 0, SEEK_SET);
			delete[] temp_byte;
		}
		break;
		case 8:		//**** 每个像素8位时：四字节对齐补充字节一并读取
		{
			int pixel_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual;	// 总像素数目（包含对齐补充字节）
			//bmp_1.bmp_pixel8 = (uint8_t*)malloc(pixel_total_num * sizeof(uint8_t));
			bmp_1.bmp_pixel8 = new uint8_t[pixel_total_num];
			fread(bmp_1.bmp_pixel8, sizeof(uint8_t), pixel_total_num, file_ptr_1);
			fseek(file_ptr_1, 0, SEEK_SET);
		}
		break;
		case 24:	//**** 每个像素24位时：剔除四字节对齐补充字节
		{
			int byte_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual;	// 总RGB字节数（包括对齐补充字节）
			uint8_t* color_index = new uint8_t[bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual];	// 临时存放所有RGB字节
			int pixel_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.bmp_info_header.biWidth;	// 总像素数目（不包含对齐补充字节）
			bmp_1.bmp_pixel24_RGB = new tagRGBTRIPLE[pixel_total_num];
			// 先读取所有RGB字节，再分配赋值到tagRGBTRIPLE
			fread(color_index, sizeof(uint8_t), bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual, file_ptr_1);
			int temp_row_index = 0, temp_row_index_actual = 0;
			for (int i = 0; i < bmp_1.bmp_info_header.biHeight; i++)
			{
				temp_row_index = i * bmp_1.bmp_info_header.biWidth;
				temp_row_index_actual = i * bmp_1.biWidth_actual;
				for (int j = 0; j < bmp_1.bmp_info_header.biWidth; j++)
				{
					bmp_1.bmp_pixel24_RGB[temp_row_index + j].rgbtBlue = color_index[temp_row_index_actual + j * 3];
					bmp_1.bmp_pixel24_RGB[temp_row_index + j].rgbtGreen = color_index[temp_row_index_actual + j * 3 + 1];
					bmp_1.bmp_pixel24_RGB[temp_row_index + j].rgbtRed = color_index[temp_row_index_actual + j * 3 + 2];
				}
			}
			delete[] color_index;
			fseek(file_ptr_1, 0, SEEK_SET);
		}
		break;
		default:
		break;
		}
		//** 关闭文件
		fclose(file_ptr_1);
	}
	else
	{
		cout << "文件不存在！" << endl;
		system("pause");
	}
	return bmp_1;
}

//写入bmp
void write_bmp(bmp_file bmp_content, string file_path)
{
	//* 二进制方式写入打开
	FILE* file_ptr_1 = fopen(file_path.c_str(), "wb");
	if (file_ptr_1)
	{
		//** 写入文件内容
		//*** BITMAPFILEHEADER
		fwrite(&bmp_content.bmp_file_header, sizeof(BITMAPFILEHEADER), 1, file_ptr_1);
		//*** BITMAPINFOHEADER
		fwrite(&bmp_content.bmp_info_header, sizeof(BITMAPINFOHEADER), 1, file_ptr_1);
		//*** 调色板（颜色表）
		switch (bmp_content.bmp_info_header.biBitCount)
		{
		case 1: //**** 1位二值图 的调色板
		{
			BYTE* temp_BYTE = new BYTE[sizeof(RGBQUAD)];
			temp_BYTE[3] = 0x00;
			// 第一个（0，前景）：白色
			temp_BYTE[0] = 0xff;
			temp_BYTE[1] = 0xff;
			temp_BYTE[2] = 0xff;
			fwrite(temp_BYTE, sizeof(RGBQUAD), 1, file_ptr_1);
			// 第二个（1，背景）：黑色
			temp_BYTE[0] = 0x00;
			temp_BYTE[1] = 0x00;
			temp_BYTE[2] = 0x00;
			fwrite(temp_BYTE, sizeof(RGBQUAD), 1, file_ptr_1);
			delete[] temp_BYTE;
		}
		break;
		case 8: //**** 8位（256色）灰度图 的调色板
		{
			BYTE* temp_BYTE = new BYTE[sizeof(RGBQUAD)];
			for (int i = 0; i < 256; i++)
			{
				temp_BYTE[0] = i;
				temp_BYTE[1] = i;
				temp_BYTE[2] = i;
				temp_BYTE[3] = 0;
				fwrite(temp_BYTE, sizeof(RGBQUAD), 1, file_ptr_1);
			}
			delete[] temp_BYTE;
		}
		break;
		default:
		break;
		}
		//*** 像素信息
		switch (bmp_content.bmp_info_header.biBitCount)
		{
		case 1:
		{
			//****借用bmp_pixel8当字节信息的临时存储
			int byte_total_num = bmp_content.bmp_info_header.biHeight * bmp_content.biWidth_actual;
			bmp_content.bmp_pixel8 = new uint8_t[byte_total_num];
			memset(bmp_content.bmp_pixel8, 0, byte_total_num);	// 初始化为零；由于写入采用加的方式，无需单独四字节对齐填0
			//****向bmp_pixel8录入二值信息
			int temp_row_index_actual = 0, temp_index_actual = 0;
			int temp_row_index = 0, temp_index = 0;
			for (int i = 0; i < bmp_content.bmp_info_header.biHeight; i++)
			{
				temp_row_index_actual = i * bmp_content.biWidth_actual;
				temp_row_index = i * bmp_content.bmp_info_header.biWidth;
				for (int j = 0; j < bmp_content.biWidth_actual; j++)
				{
					temp_index_actual = temp_row_index_actual + j;
					temp_index = temp_row_index + j * 8;
					for (int k = 0; k < 8; k++)	// 逐字节写入，每位比特用对应位数的移位运算加到字节上
						bmp_content.bmp_pixel8[temp_index_actual] += (bmp_content.bmp_pixel1[temp_index + k] << (7 - k));
				}
			}
			fwrite(bmp_content.bmp_pixel8, sizeof(uint8_t), byte_total_num, file_ptr_1);
		}
		break;
		case 8:
		{
			writing_four_byte_alignment(bmp_content);
			int pixel_total_num = bmp_content.bmp_info_header.biHeight * bmp_content.biWidth_actual;
			fwrite(bmp_content.bmp_pixel8, sizeof(uint8_t), pixel_total_num, file_ptr_1);
		}
		break;
		case 24:
		{
			//****借用bmp_pixel8当字节信息的临时存储
			int byte_total_num = bmp_content.bmp_info_header.biHeight * bmp_content.biWidth_actual;
			bmp_content.bmp_pixel8 = new uint8_t[byte_total_num];
			writing_four_byte_alignment(bmp_content);		// 四字节对齐
			//****向bmp_pixel8录入RGB信息
			int temp_row_index = 0, temp_row_index_actual = 0;
			if (byte_total_num >= 3)
				for (int i = 0; i < bmp_content.bmp_info_header.biHeight; i++)
				{
					temp_row_index = i * bmp_content.bmp_info_header.biWidth;
					temp_row_index_actual = i * bmp_content.biWidth_actual;
					for (int j = 0; j < bmp_content.bmp_info_header.biWidth; j++)
					{
						bmp_content.bmp_pixel8[temp_row_index_actual + j * 3] = bmp_content.bmp_pixel24_RGB[temp_row_index + j].rgbtBlue;
						bmp_content.bmp_pixel8[temp_row_index_actual + j * 3 + 1] = bmp_content.bmp_pixel24_RGB[temp_row_index + j].rgbtGreen;
						bmp_content.bmp_pixel8[temp_row_index_actual + j * 3 + 2] = bmp_content.bmp_pixel24_RGB[temp_row_index + j].rgbtRed;
					}
				}
			fwrite(bmp_content.bmp_pixel8, sizeof(uint8_t), byte_total_num, file_ptr_1);
		}
		break;
		default:
		break;
		}
		//** 关闭文件
		fclose(file_ptr_1);
	}
	else
	{
		cout << "打开文件失败！" << endl;
		system("pause");
	}
	return;
}