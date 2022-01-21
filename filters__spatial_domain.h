#pragma once

#include"read_write_bmp.h"

constexpr auto UNIT = 1.0;

//���ߣ�Ѱ����λֵ��ð������
//*uint8_t
uint8_t median_value(uint8_t* sequence, int length)
{
	uint8_t temp = 0;
	//ð������
	for (int i = length - 1; i > 0; i--)
		for (int j = 0; j < i; j++)
			if (sequence[j] > sequence[j + 1])
			{
				temp = sequence[j];
				sequence[j] = sequence[j + 1];
				sequence[j + 1] = temp;
			}
	//���
	return length % 2 == 1 ? sequence[length / 2] : (sequence[length / 2 - 1] + sequence[length / 2]) / 2;
}

//���ߣ���˹����
double Gaussian_func(double x, double mu, double sigma, double K)
{
	return exp(-pow(x - mu, 2) / 2 / pow(sigma, 2)) * K;
}

//�ռ����˹��ͨ�˲�����ɫ��ͼ��
bmp_file spatial_domain_Gaussian_LPF(bmp_file origin_bmp, double sigma)
{
	//��˹�ˣ��ɷ���
	int filter_width = int(ceil(sigma * 6));		//3sigmaԭ��
	if (filter_width < 3) filter_width = 3;			//ʹ������3*3�ĺ�
	else if (filter_width % 2 == 0) filter_width++;	//ʹ��������ȵĺ�
	double sqrt_K = UNIT;
	//*�˷ֽ�����������������������ڸ�˹����Բ�ԳƵģ�����������������Ԫ����ͬ��
	double* Gaussian_core_row = new double[filter_width];
	for (int i = 0; i < filter_width; i++)Gaussian_core_row[i] = Gaussian_func(double(i), (double(filter_width) - 1) / 2, sigma, sqrt_K);
	//*ϵ���͵Ĺ�һ��
	//**ϵ���������
	double temp_sum = 0.0;
	//***���Խ��� x4
	for (int i = 0; i < filter_width / 2; i++)temp_sum += pow(Gaussian_core_row[i], 2) * 4;
	//***���ߴ�ֱƽ���� x4
	for (int i = 0; i < filter_width / 2; i++)temp_sum += Gaussian_core_row[i] * 4;	//ϵ��Ϊ1
	//***��Χ�� x8
	for (int i = 0; i < filter_width / 2; i++)
		for (int j = i + 1; j < filter_width / 2; j++)
			temp_sum += Gaussian_core_row[i] * Gaussian_core_row[j] * 8;
	//***���� x1
	temp_sum += 1.0;
	//**��һ������Ϊ�Ƿֽ����������������ƽ������
	for (int i = 0; i < filter_width; i++)Gaussian_core_row[i] /= sqrt(temp_sum);

	//��ȡ���أ������ų������ֽڶ�����������������
	int padding_length = filter_width / 2;
	int padded_biWidth = origin_bmp.bmp_info_header.biWidth + padding_length * 2;
	int padded_biHeight = origin_bmp.bmp_info_header.biHeight + padding_length * 2;
	uint8_t* new_pixels = new uint8_t[padded_biWidth * padded_biHeight];
	memset(new_pixels, 0, sizeof(uint8_t) * padded_biWidth * padded_biHeight);
	if (padded_biHeight >= 3 && padded_biWidth >= 3)
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				new_pixels[(i + padding_length) * padded_biWidth + padding_length + j] = origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];

	//�������
	if (padded_biHeight >= 3 && padded_biWidth >= 3)
	{
		int padded_biHeight_half = origin_bmp.bmp_info_header.biHeight + padding_length;
		//*���ұ�
		for (int i = padding_length; i < padded_biHeight_half; i++)
		{
			int left = i * padded_biWidth + padding_length;
			int right = i * padded_biWidth + padding_length + origin_bmp.bmp_info_header.biWidth - 1;	//**ע���һ����Ϊǰ�沿�ֵõ�������Ѿ���ԭͼ��һ����**
			for (int j = 1; j <= padding_length; j++)
			{
				new_pixels[left - j] = new_pixels[left + j];	//���
				new_pixels[right + j] = new_pixels[right - j];	//�ұ�
			}
		}
		//*���±ߣ������Ľǣ�
		for (int i = 1; i <= padding_length; i++)
		{
			int origin_bottom = (padding_length + i) * padded_biWidth;
			int padding_bottom = (padding_length - i) * padded_biWidth;
			int origin_top = (padding_length + origin_bmp.bmp_info_header.biHeight - 1 - i) * padded_biWidth;	//**ע���һ����Ϊǰ�沿�ֵõ�������Ѿ���ԭͼ��һ����**
			int padding_top = (padding_length + origin_bmp.bmp_info_header.biHeight - 1 + i) * padded_biWidth;	//**ע���һ����Ϊǰ�沿�ֵõ�������Ѿ���ԭͼ��һ����**
			for (int j = 0; j < padded_biWidth; j++)
			{
				new_pixels[padding_bottom + j] = new_pixels[origin_bottom + j];	//�±�
				new_pixels[padding_top + j] = new_pixels[origin_top + j];		//�ϱ�
			}
		}
	}

	//�ռ��˲�
	int temp_index = 0, temp_index_2 = 0;
	double temp_result = 0.0;
	//*������������˹��Բ�Գƣ�����Ѻ���ת180�ȣ����˴���ת180�ȴ����ˣ�
	uint8_t* new_pixels_2 = new uint8_t[origin_bmp.bmp_info_header.biWidth * padded_biHeight];
	memset(new_pixels_2, 0, sizeof(uint8_t) * origin_bmp.bmp_info_header.biWidth * padded_biHeight);
	for (int i = 0; i < padded_biHeight; i++)
		for (int j = padding_length; j < padded_biWidth - padding_length; j++)
		{
			temp_index = i * padded_biWidth + j + padding_length;	//��ǰ�������������ͼ�����أ���ͬһ�У��е����һ������
			temp_index_2 = i * origin_bmp.bmp_info_header.biWidth + j - padding_length;	//��ǰ��Ҫ������м�������
			temp_result = 0.0;
			for (int k = 0; k < filter_width; k++)
				temp_result += Gaussian_core_row[k] * new_pixels[temp_index - k];
			new_pixels_2[temp_index_2] = CLAMP_0_255(temp_result);
		}
	//*����������
	uint8_t* result_pixels = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
	for (int i = padding_length; i < padded_biHeight - padding_length; i++)
		for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
		{
			temp_index = (i + padding_length) * origin_bmp.bmp_info_header.biWidth + j;	//��ǰ�������������ͼ�����أ���ͬһ�У��е����Ϸ�����
			temp_index_2 = (i - padding_length) * origin_bmp.biWidth_actual + j;	//��ǰ��Ҫ����Ľ������
			temp_result = 0.0;
			for (int k = 0; k < filter_width; k++)
				temp_result += Gaussian_core_row[k] * new_pixels_2[temp_index - k * origin_bmp.bmp_info_header.biWidth];
			result_pixels[temp_index_2] = CLAMP_0_255(temp_result);
		}

	//��bmp
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

	result_bmp.bmp_pixel8 = result_pixels;
	result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

	delete[] Gaussian_core_row, new_pixels, new_pixels_2;
	return result_bmp;
}

//�ռ�����ֵ�˲��������κˣ�����ɫ��ͼ��
bmp_file spatial_domain_median_filter(bmp_file origin_bmp, int filter_width)
{
	//��ȡ���أ��ų������ֽڶ�����������������
	int padding_length = filter_width / 2;
	int padded_biWidth = origin_bmp.bmp_info_header.biWidth + padding_length * 2;
	int padded_biHeight = origin_bmp.bmp_info_header.biHeight + padding_length * 2;
	uint8_t* new_pixels = new uint8_t[padded_biWidth * padded_biHeight];
	memset(new_pixels, 0, sizeof(uint8_t) * padded_biWidth * padded_biHeight);
	if (padded_biHeight >= 3 && padded_biWidth >= 3)
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				new_pixels[(i + padding_length) * padded_biWidth + padding_length + j] = origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];

	//�������
	if (padded_biHeight >= 3 && padded_biWidth >= 3)
	{
		//*���ұ�
		int padded_biHeight_half = origin_bmp.bmp_info_header.biHeight + padding_length;
		for (int i = padding_length; i < padded_biHeight_half; i++)
		{
			int left = i * padded_biWidth + padding_length;
			int right = i * padded_biWidth + padding_length + origin_bmp.bmp_info_header.biWidth - 1;	//**ע���һ����Ϊǰ�沿�ֵõ�������Ѿ���ԭͼ��һ����**
			for (int j = 1; j <= padding_length; j++)
			{
				new_pixels[left - j] = new_pixels[left + j];	//���
				new_pixels[right + j] = new_pixels[right - j];	//�ұ�
			}
		}
		//*���±ߣ������Ľǣ�
		for (int i = 1; i <= padding_length; i++)
		{
			int origin_bottom = (padding_length + i) * padded_biWidth;
			int padding_bottom = (padding_length - i) * padded_biWidth;
			int origin_top = (padding_length + origin_bmp.bmp_info_header.biHeight - 1 - i) * padded_biWidth;	//**ע���һ����Ϊǰ�沿�ֵõ�������Ѿ���ԭͼ��һ����**
			int padding_top = (padding_length + origin_bmp.bmp_info_header.biHeight - 1 + i) * padded_biWidth;	//**ע���һ����Ϊǰ�沿�ֵõ�������Ѿ���ԭͼ��һ����**
			for (int j = 0; j < padded_biWidth; j++)
			{
				new_pixels[padding_bottom + j] = new_pixels[origin_bottom + j];	//�±�
				new_pixels[padding_top + j] = new_pixels[origin_top + j];		//�ϱ�
			}
		}
	}

	//�ռ��˲�
	uint8_t* result_pixels = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
	uint8_t* temp_pixels = new uint8_t[filter_width * filter_width];
	memset(temp_pixels, 0, sizeof(uint8_t) * filter_width * filter_width);
	for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
		{
			if (padded_biWidth > 1 && padded_biHeight > 1)
			{
				//*��ȡ�������أ���������½�����ǡ��������������ȥ��������ͼ�е�����
				for (int ii = 0; ii < filter_width; ii++)
					for (int jj = 0; jj < filter_width; jj++)
						temp_pixels[ii * filter_width + jj] = new_pixels[(i + ii) * padded_biWidth + j + jj];
				//*Ѱ����ֵ
				result_pixels[i * origin_bmp.biWidth_actual + j] = median_value(temp_pixels, filter_width * filter_width);
			}
		}

	//��bmp
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

	result_bmp.bmp_pixel8 = result_pixels;
	result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

	return result_bmp;
}