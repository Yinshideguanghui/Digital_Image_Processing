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

//���ߣ��޷���0��255��
uint8_t CLAMP_0_255(double data)
{
	if (data >= 0 && data <= 255)return uint8_t(round(data));
	else if (data < 0)return 0;
	else return 255;
}
//���ߣ��޷���0��255��
uint8_t CLAMP_0_255(int data)
{
	if (data >= 0 && data <= 255)return uint8_t(round(data));
	else if (data < 0)return 0;
	else return 255;
}
//���ߣ��޷���0��x��
uint8_t CLAMP_0_x(double data, uint8_t top)
{
	if (data >= 0 && data <= top)return uint8_t(round(data));
	else if (data < 0)return 0;
	else return top;
}


//д��ͼ�����ֽڶ�������
//*���������ȫ0���أ�����������ʾ���
void writing_four_byte_alignment(bmp_file& bmp_1)
{
	switch (bmp_1.bmp_info_header.biBitCount)
	{
	case 1:	// ������ʱ�洢��ʼ��Ϊ�㣬��д����üӵķ�ʽ�����赥�����ֽڶ���
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

//���ߣ�0-255������ֱ��ͼ����
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
//���ߣ�0-255������ֱ��ͼ����������ά��ʽ��֯������
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
//���ߣ�0-255������ֱ��ͼ����������ά��ʽ��֯�����ݣ�ÿ��ĩβ�����п�
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
//���ߣ�0-x������ֱ��ͼ����������ά��ʽ��֯������
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

//����bmp
bmp_file read_bmp(string file_path)
{
	bmp_file bmp_1{};
	//* �����Ʒ�ʽֻ����
	FILE* file_ptr_1 = fopen(file_path.c_str(), "rb");
	if (file_ptr_1)
	{
		//** ��ȡ�ļ�����
		fseek(file_ptr_1, 0L, SEEK_END);
		int file_length = ftell(file_ptr_1);
		//** ��ȡ�ļ�����
		//*** BITMAPFILEHEADER
		fseek(file_ptr_1, 0L, SEEK_SET);
		fread(&bmp_1.bmp_file_header, sizeof(BITMAPFILEHEADER), 1, file_ptr_1);
		//*** BITMAPINFOHEADER
		fseek(file_ptr_1, sizeof(BITMAPFILEHEADER), SEEK_SET);
		fread(&bmp_1.bmp_info_header, sizeof(BITMAPINFOHEADER), 1, file_ptr_1);
		//*** ������Ϣ
		fseek(file_ptr_1, bmp_1.bmp_file_header.bfOffBits, SEEK_SET);
		//**** �����ֽڶ��벹��ʱʵ��ÿ���ֽ�����������8*��ʾ��ȶ�32�����ֽڱ��س�������ȡ���������ٿ��ж��ٸ����ֽڣ�Ҳ������ʽ��
		bmp_1.biWidth_actual = ((((bmp_1.bmp_info_header.biBitCount * bmp_1.bmp_info_header.biWidth) + 31) & ~31) >> 3);
		//**** ��ȡ����
		switch (bmp_1.bmp_info_header.biBitCount)
		{
		case 1:		//**** ÿ������1λʱ��ÿ���ֽڴ���8������
		{
			int pixel_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.bmp_info_header.biWidth;
			bmp_1.bmp_pixel1 = new uint8_t[pixel_total_num];
			uint8_t* temp_byte = new uint8_t[4];	// �����ֽ�Ϊ��λ����
			memset(temp_byte, 0, sizeof(uint8_t) * 4);
			int times_of_read_per_row = bmp_1.biWidth_actual / 4 - 1;	// ÿ�ж��������ֽڵĴ��������һ�β������ڣ���Ϊ���ܲ���������������
			int times_of_read_bit_last = bmp_1.bmp_info_header.biWidth - times_of_read_per_row * 32; // ���һ��Ӧ�ö�ȡ�ı�����
			uint8_t* temp_bit_last = new uint8_t[32];	// ��ʱ������һ�ζ��������б���
			int temp_index = 0; // ���������м����1
			int temp_row_index = 0; // ���������м����2
			for (int ordinal_of_row = 0; ordinal_of_row < bmp_1.bmp_info_header.biHeight; ordinal_of_row++)	// ���ж���
			{
				temp_row_index = ordinal_of_row * bmp_1.bmp_info_header.biWidth;
				// ���һ�����ֽ�֮ǰ���ض��������ֽ�
				for (int ordinal_of_read_in_column = 0; ordinal_of_read_in_column < times_of_read_per_row; ordinal_of_read_in_column++)
				{
					temp_index = temp_row_index + ordinal_of_read_in_column * 32;
					fread(temp_byte, sizeof(uint8_t), 4, file_ptr_1);
					for (int i = 0; i < 4; i++)	// ���ֽڷֱ��ֳɱ��أ�����uint8_t����
						for (int j = 0; j < 8; j++)
							bmp_1.bmp_pixel1[temp_index + i * 8 + j] = (temp_byte[i] >> (7 - j)) % 2;
				}
				// ���һ�����ֽ�
				temp_index = temp_row_index + times_of_read_per_row * 32;
				fread(temp_byte, sizeof(uint8_t), 4, file_ptr_1);
				for (int i = 0; i < 4; i++)	// ���ֽڷֱ��ֳɱ��أ�����uint8_t����
					for (int j = 0; j < 8; j++)
						temp_bit_last[i * 8 + j] = (temp_byte[i] >> (7 - j)) % 2;
				for (int k = 0; k < times_of_read_bit_last; k++)	// �������ı���
				{
					bmp_1.bmp_pixel1[temp_index + k] = temp_bit_last[k];
				}
			}
			fseek(file_ptr_1, 0, SEEK_SET);
			delete[] temp_byte;
		}
		break;
		case 8:		//**** ÿ������8λʱ�����ֽڶ��벹���ֽ�һ����ȡ
		{
			int pixel_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual;	// ��������Ŀ���������벹���ֽڣ�
			//bmp_1.bmp_pixel8 = (uint8_t*)malloc(pixel_total_num * sizeof(uint8_t));
			bmp_1.bmp_pixel8 = new uint8_t[pixel_total_num];
			fread(bmp_1.bmp_pixel8, sizeof(uint8_t), pixel_total_num, file_ptr_1);
			fseek(file_ptr_1, 0, SEEK_SET);
		}
		break;
		case 24:	//**** ÿ������24λʱ���޳����ֽڶ��벹���ֽ�
		{
			int byte_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual;	// ��RGB�ֽ������������벹���ֽڣ�
			uint8_t* color_index = new uint8_t[bmp_1.bmp_info_header.biHeight * bmp_1.biWidth_actual];	// ��ʱ�������RGB�ֽ�
			int pixel_total_num = bmp_1.bmp_info_header.biHeight * bmp_1.bmp_info_header.biWidth;	// ��������Ŀ�����������벹���ֽڣ�
			bmp_1.bmp_pixel24_RGB = new tagRGBTRIPLE[pixel_total_num];
			// �ȶ�ȡ����RGB�ֽڣ��ٷ��丳ֵ��tagRGBTRIPLE
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
		//** �ر��ļ�
		fclose(file_ptr_1);
	}
	else
	{
		cout << "�ļ������ڣ�" << endl;
		system("pause");
	}
	return bmp_1;
}

//д��bmp
void write_bmp(bmp_file bmp_content, string file_path)
{
	//* �����Ʒ�ʽд���
	FILE* file_ptr_1 = fopen(file_path.c_str(), "wb");
	if (file_ptr_1)
	{
		//** д���ļ�����
		//*** BITMAPFILEHEADER
		fwrite(&bmp_content.bmp_file_header, sizeof(BITMAPFILEHEADER), 1, file_ptr_1);
		//*** BITMAPINFOHEADER
		fwrite(&bmp_content.bmp_info_header, sizeof(BITMAPINFOHEADER), 1, file_ptr_1);
		//*** ��ɫ�壨��ɫ��
		switch (bmp_content.bmp_info_header.biBitCount)
		{
		case 1: //**** 1λ��ֵͼ �ĵ�ɫ��
		{
			BYTE* temp_BYTE = new BYTE[sizeof(RGBQUAD)];
			temp_BYTE[3] = 0x00;
			// ��һ����0��ǰ��������ɫ
			temp_BYTE[0] = 0xff;
			temp_BYTE[1] = 0xff;
			temp_BYTE[2] = 0xff;
			fwrite(temp_BYTE, sizeof(RGBQUAD), 1, file_ptr_1);
			// �ڶ�����1������������ɫ
			temp_BYTE[0] = 0x00;
			temp_BYTE[1] = 0x00;
			temp_BYTE[2] = 0x00;
			fwrite(temp_BYTE, sizeof(RGBQUAD), 1, file_ptr_1);
			delete[] temp_BYTE;
		}
		break;
		case 8: //**** 8λ��256ɫ���Ҷ�ͼ �ĵ�ɫ��
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
		//*** ������Ϣ
		switch (bmp_content.bmp_info_header.biBitCount)
		{
		case 1:
		{
			//****����bmp_pixel8���ֽ���Ϣ����ʱ�洢
			int byte_total_num = bmp_content.bmp_info_header.biHeight * bmp_content.biWidth_actual;
			bmp_content.bmp_pixel8 = new uint8_t[byte_total_num];
			memset(bmp_content.bmp_pixel8, 0, byte_total_num);	// ��ʼ��Ϊ�㣻����д����üӵķ�ʽ�����赥�����ֽڶ�����0
			//****��bmp_pixel8¼���ֵ��Ϣ
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
					for (int k = 0; k < 8; k++)	// ���ֽ�д�룬ÿλ�����ö�Ӧλ������λ����ӵ��ֽ���
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
			//****����bmp_pixel8���ֽ���Ϣ����ʱ�洢
			int byte_total_num = bmp_content.bmp_info_header.biHeight * bmp_content.biWidth_actual;
			bmp_content.bmp_pixel8 = new uint8_t[byte_total_num];
			writing_four_byte_alignment(bmp_content);		// ���ֽڶ���
			//****��bmp_pixel8¼��RGB��Ϣ
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
		//** �ر��ļ�
		fclose(file_ptr_1);
	}
	else
	{
		cout << "���ļ�ʧ�ܣ�" << endl;
		system("pause");
	}
	return;
}