#pragma once

#ifndef READ_WRITE_BMP_H
#define READ_WRITE_BMP_H

#include<cmath>
#include"read_write_bmp.h"
#include"complex_matrix_operators.h" // �����任�п��ܳ��ָ���

//��ɢ����Ҷ�任������䣩
bmp_file DFT_zero_padding(const bmp_file& origin_bmp)
{
	bmp_file result_bmp;

	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:	// 8λ256ɫ��ɫ��ͼ
	{
		//*��ȡ�����ֽڶ������ص�һ��������չͼ��
		int new_biWidth = origin_bmp.bmp_info_header.biWidth * 2;
		int new_biHeight = origin_bmp.bmp_info_header.biHeight * 2;
		int new_num_pixels = new_biHeight * new_biWidth;
		int new_biWidth_actual = ((((origin_bmp.bmp_info_header.biBitCount * new_biWidth) + 31) & ~31) >> 3);
		ComplexMatrix origin_bmp_extended(new_biHeight, new_biWidth); // �ռ�����м���
		if (new_num_pixels > 1)	// ��ֹ����Խ��
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					//**�����кŴ洢���򣬼��ȵ��к��У���ͼ��;����x�����෴��ͬʱy������ͬ�������ж�ȡ����ʹ��ͼ�񲻷�ת��λ�ھ������Ͻ�
					//**ǰ���ϵ����Ϊ��ʹDFTƵ�����ĵ�λ��P*Q��С��Ƶ�ʾ��ε�����
					origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = double((i + j) % 2 == 0 ? 1 : (-1)) * origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];
				}

		//*�����������������͸ı�DFT���ĵ�ͼ��DFTƵ�ף���άDFT�ɲ��Ϊ����һάDFT
		//**����ÿ�е�һάDFT
			//*** ���Ż��������ʱ��һ���벿����û��Ҫ�㣬��Ϊ����0
		ComplexMatrix DFT_each_row(new_biHeight, new_biWidth);
		Complex temp_complex_1(.0, .0), temp_complex_2(.0, .0);
		for (int x = 0; x < new_biHeight; x++)			// ��㣬Ŀ����������
			for (int v = 0; v < new_biWidth; v++)		// ��㣬Ŀ���ĺ�����
				for (int y = 0; y < new_biWidth; y++)	// �ڲ㣬��һ����Ԫ�ر������
				{
					temp_complex_1.re = cos(-2.0 * pi * v * y / new_biWidth);
					temp_complex_1.im = sin(-2.0 * pi * v * y / new_biWidth);
					temp_complex_2 = origin_bmp_extended.c[x][y] * temp_complex_1;
					DFT_each_row.c[x][v].re += temp_complex_2.re;
					DFT_each_row.c[x][v].im += temp_complex_2.im;
				}
		//**�����е�һάDFT
		ComplexMatrix DFT_result(new_biHeight, new_biWidth);
		for (int u = 0; u < new_biHeight; u++)			// ��㣬Ŀ����������
			for (int v = 0; v < new_biWidth; v++)		// ��㣬Ŀ���ĺ�����
				for (int x = 0; x < new_biHeight; x++)	// �ڲ㣬��һ����Ԫ�ر������
				{
					temp_complex_1.re = cos(-2.0 * pi * u * x / new_biHeight);
					temp_complex_1.im = sin(-2.0 * pi * u * x / new_biHeight);
					temp_complex_2 = DFT_each_row.c[x][v] * temp_complex_1;
					DFT_result.c[u][v].re += temp_complex_2.re;
					DFT_result.c[u][v].im += temp_complex_2.im;
				}
		//**����Ƶ�ף�ģֵ��
		//***DFT���Ƶ��������������
		ComplexMatrix DFT_frequency_spectrum(new_biHeight, new_biWidth);
		double temp_modulus = .0, temp_max = .0;	// ��¼��ȫ�������е����ֵ���Ա�궨�����������Сֵ��0��
		for (int u = 0; u < new_biHeight; u++)
			for (int v = 0; v < new_biWidth; v++)
			{
				temp_modulus = sqrt(DFT_result.c[u][v].re * DFT_result.c[u][v].re + DFT_result.c[u][v].im * DFT_result.c[u][v].im);

				temp_modulus = log(1 + abs(temp_modulus));	// ת��Ϊ�����߶��Ը��õؿ��ӻ�

				DFT_frequency_spectrum.c[u][v].re = temp_modulus;
				if (temp_max < temp_modulus)temp_max = temp_modulus;
			}
		//***�궨��[0,255]
		uint8_t* result_frequency_spectrum = new uint8_t[new_biWidth_actual * new_biHeight];
		if (new_biWidth_actual * new_biHeight > 1) //��ֹ����Խ��
			for (int u = 0; u < new_biHeight; u++)
				for (int v = 0; v < new_biWidth; v++)
				{
					// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
					result_frequency_spectrum[u * new_biWidth_actual + v] = uint8_t(round(DFT_frequency_spectrum.c[new_biHeight - 1 - u][v].re / temp_max * 255));
				}


		//*���bmp���ߴ��Ϊ2��
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

		result_bmp.bmp_pixel8 = result_frequency_spectrum;	// ���������任Ƶ�ף�ģֵ���궨��[0,255]���������
		result_bmp.biWidth_actual = new_biWidth_actual;
	}
	break;
	default:
	break;
	}
	return result_bmp;
}

//��ɢ���ұ任������䣩
bmp_file DCT_zero_padding(const bmp_file& origin_bmp)
{
	bmp_file result_bmp;

	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:	// 8λ256ɫ��ɫ��ͼ
	{
		//*��ȡ�����ֽڶ������ص�һ��������չͼ��
		int new_biWidth = origin_bmp.bmp_info_header.biWidth * 2;
		int new_biHeight = origin_bmp.bmp_info_header.biHeight * 2;
		int new_num_pixels = new_biHeight * new_biWidth;
		int new_biWidth_actual = ((((origin_bmp.bmp_info_header.biBitCount * new_biWidth) + 31) & ~31) >> 3);
		ComplexMatrix origin_bmp_extended(new_biHeight, new_biWidth); // �ռ�����м���
		if (new_num_pixels > 1)	// ��ֹ����Խ��
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					//**�����кŴ洢���򣬼��ȵ��к��У���ͼ��;����x�����෴��ͬʱy������ͬ�������ж�ȡ����ʹ��ͼ�񲻷�ת��λ�ھ������Ͻ�
					origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = double(origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j]);
				}

		//*������������������ͼ��DCTƵ�ף���άDCT�ɲ��Ϊ����һάDCT
		//**����ÿ�е�һάDCT
		ComplexMatrix DCT_each_row(new_biHeight, new_biWidth);
		double temp_double = .0;
		for (int x = 0; x < new_biHeight; x++)			// ��㣬Ŀ����������
		{
			for (int v = 0; v < new_biWidth; v++)		// ��㣬Ŀ���ĺ�����
			{
				for (int y = 0; y < new_biWidth; y++)	// �ڲ㣬��һ����Ԫ�ر������
				{
					temp_double = cos((2.0 * y + 1) * pi * v / (2.0 * new_biWidth));
					DCT_each_row.c[x][v].re += origin_bmp_extended.c[x][y].re * temp_double;
				}
				DCT_each_row.c[x][v].re *= sqrt(2.0 / new_biWidth);
			}
			// v = 0 ʱ��ÿ�е�һ����ϵ������
			DCT_each_row.c[x][0].re *= (1 / sqrt(2.0));
		}
		//**�����е�һάDCT
		ComplexMatrix DCT_result(new_biHeight, new_biWidth);
		for (int v = 0; v < new_biWidth; v++)			// ��㣬Ŀ���ĺ�����
		{
			for (int u = 0; u < new_biHeight; u++)		// ��㣬Ŀ����������
			{
				for (int x = 0; x < new_biHeight; x++)	// �ڲ㣬��һ����Ԫ�ر������
				{
					temp_double = cos((2.0 * x + 1) * pi * u / (2.0 * new_biHeight));
					DCT_result.c[u][v].re += DCT_each_row.c[x][v].re * temp_double;
				}
				DCT_result.c[u][v].re *= sqrt(2.0 / new_biHeight);
			}
			// u = 0 ʱ��ÿ�е�һ����ϵ������
			DCT_result.c[0][v].re *= (1 / sqrt(2.0));
		}

		//**����Ƶ�ף�ģֵ��
		//***DCT���Ƶ��������������
		ComplexMatrix DCT_frequency_spectrum(new_biHeight, new_biWidth);
		double temp_modulus = .0, temp_max = .0;	// ��¼��ȫ�������е����ֵ���Ա�궨�����������Сֵ��0��
		for (int u = 0; u < new_biHeight; u++)
			for (int v = 0; v < new_biWidth; v++)
			{
				temp_modulus = DCT_result.c[u][v].re;

				temp_modulus = log(1 + abs(temp_modulus));	// ת��Ϊ�����߶��Ը��õؿ��ӻ�

				DCT_frequency_spectrum.c[u][v].re = temp_modulus;
				if (temp_max < temp_modulus)temp_max = temp_modulus;
			}
		//***�궨��[0,255]
		uint8_t* result_frequency_spectrum = new uint8_t[new_biWidth_actual * new_biHeight];
		if (new_biWidth_actual * new_biHeight > 1) //��ֹ����Խ��
			for (int u = 0; u < new_biHeight; u++)
				for (int v = 0; v < new_biWidth; v++)
				{
					// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
					result_frequency_spectrum[u * new_biWidth_actual + v] = uint8_t(round(DCT_frequency_spectrum.c[new_biHeight - 1 - u][v].re / temp_max * 255));
				}

		//*���bmp���ߴ��Ϊ2��
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

		result_bmp.bmp_pixel8 = result_frequency_spectrum;	// ���������任Ƶ�ף�ģֵ���궨��[0,255]���������
		result_bmp.biWidth_actual = new_biWidth_actual;
	}
	break;
	default:
		break;
	}
	return result_bmp;
}

//С���任������С����
bmp_file* DWT_Haar_1_and_2_scale(const bmp_file& origin_bmp, bool from_extern)
{
	bmp_file* result_bmp = new bmp_file[2];
	switch (origin_bmp.bmp_info_header.biBitCount)
	{
	case 8:	// 8λ256ɫ��ɫ��ͼ
	{
		//*��ȡ�����ֽڶ������ص�һ��������չͼ��
		ComplexMatrix origin_bmp_extended(origin_bmp.bmp_info_header.biHeight, origin_bmp.bmp_info_header.biWidth); // �ռ�����м���
		if (origin_bmp.bmp_info_header.biHeight * origin_bmp.bmp_info_header.biWidth > 1)	// ��ֹ����Խ��
		{
			if(from_extern)	//��������С���任
				for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
					for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
					{
						//**�����кŴ洢���򣬼��ȵ��к��У���ͼ��;����x�����෴��ͬʱy������ͬ�������ж�ȡ����ʹ��ͼ�񲻷�ת��λ�ھ������Ͻ�
						origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = double(origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j]);
					}
			else // С���任������һ�߶�С���任
				for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
					for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
					{
						//**�����кŴ洢���򣬼��ȵ��к��У���ͼ��;����x�����෴��ͬʱy������ͬ�������ж�ȡ����ʹ��ͼ�񲻷�ת��λ�ھ������Ͻ�
						origin_bmp_extended.c[origin_bmp.bmp_info_header.biHeight - 1 - i][j].re = origin_bmp.bmp_pixel8_double[i * origin_bmp.biWidth_actual + j];
					}
		}

		//*������������������ͼ��DWT����άDWT�ɲ��Ϊ����һάDWT
		//*1�߶ȹ���FWT��ϵ����1/2����һ����������߶�FWTʱ��1/sqrt(2)
		//**����ÿ�е�һάDWT����ˮƽ������㣬�ִ�ֱ����ʹ�ֱϸ�ڣ����ش�ֱ�����ϸ�ڣ�
		ComplexMatrix DWT_each_row(origin_bmp.bmp_info_header.biHeight, origin_bmp.bmp_info_header.biWidth);
		for (int x = 0; x < origin_bmp.bmp_info_header.biHeight; x++)			// Ŀ����������
		{
			for (int y = 0; y < origin_bmp.bmp_info_header.biWidth / 2; y++)	// ��һ����Ԫ�ر���
			{
				//***��Ƶ����
				DWT_each_row.c[x][y].re = (origin_bmp_extended.c[x][2 * y].re + origin_bmp_extended.c[x][2 * y + 1].re) / (2.0);
				//***��Ƶ��������ֱϸ�ڣ�
				DWT_each_row.c[x][y + origin_bmp.bmp_info_header.biWidth / 2].re = (origin_bmp_extended.c[x][2 * y].re - origin_bmp_extended.c[x][2 * y + 1].re) / (2.0);
			}
		}
		//**����ÿ�е�һάDWT���ش�ֱ������㣬��ˮƽ�����ˮƽϸ�ڣ�����ˮƽ�����ϸ�ڣ�
		ComplexMatrix DWT_result(origin_bmp.bmp_info_header.biHeight, origin_bmp.bmp_info_header.biWidth);
		for (int y = 0; y < origin_bmp.bmp_info_header.biWidth; y++)			// Ŀ���ĺ�����
		{
			for (int x = 0; x < origin_bmp.bmp_info_header.biHeight / 2; x++)	// ��һ����Ԫ�ر���
			{
				//***��Ƶ����
				DWT_result.c[x][y].re = (DWT_each_row.c[2 * x][y].re + DWT_each_row.c[2 * x + 1][y].re) / (2.0);
				//***��Ƶ������ˮƽϸ�ڣ�
				DWT_result.c[x + origin_bmp.bmp_info_header.biHeight / 2][y].re = (DWT_each_row.c[2 * x][y].re - DWT_each_row.c[2 * x + 1][y].re) / (2.0);
			}
		}

		//*���ͼ
		uint8_t* result_frequency_spectrum = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
		//**����ͼcA���߶ȿռ䣩
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //��ֹ����Խ��
			for (int u = 0; u < origin_bmp.bmp_info_header.biHeight / 2; u++)
				for (int v = 0; v < origin_bmp.bmp_info_header.biWidth / 2; v++)
				{
					// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight - 1 - u) * origin_bmp.biWidth_actual + v] = CLAMP_0_255(DWT_result.c[u][v].re);
				}
		//**�������ͼ��С���ռ䣩�ֱ�궨��[0,255]
		double temp_double = .0, temp_max = .0, temp_min = .0, temp_diff = .0;
		int temp_row_index = 0;
		//***����ͼcV����ֱϸ�ڣ������������ ���� ������ŵ�����ȥ
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
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //��ֹ����Խ��
			for (int u = 0; u < origin_bmp.bmp_info_header.biHeight / 2; u++)
				for (int v = origin_bmp.bmp_info_header.biWidth / 2; v < origin_bmp.bmp_info_header.biWidth; v++)
				{
					// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight / 2 - 1 - u) * origin_bmp.biWidth_actual + v - origin_bmp.bmp_info_header.biWidth / 2] 
						= uint8_t(round(abs(DWT_result.c[u][v].re - temp_min) / temp_diff * 255));
				}
		//***cH��ˮƽ����ϸ�ڣ������������ ���� ������ŵ�����ȥ
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
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //��ֹ����Խ��
			for (int u = origin_bmp.bmp_info_header.biHeight / 2; u < origin_bmp.bmp_info_header.biHeight; u++)
				for (int v = 0; v < origin_bmp.bmp_info_header.biWidth / 2; v++)
				{
					// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight + origin_bmp.bmp_info_header.biHeight / 2 - 1 - u) * origin_bmp.biWidth_actual + v + origin_bmp.bmp_info_header.biWidth / 2]
						= uint8_t(round(abs(DWT_result.c[u][v].re - temp_min) / temp_diff * 255));
				}
		//***����ͼcD���Խ�ϸ�ڣ�
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
		if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //��ֹ����Խ��
			for (int u = origin_bmp.bmp_info_header.biHeight / 2; u < origin_bmp.bmp_info_header.biHeight; u++)
				for (int v = origin_bmp.bmp_info_header.biWidth / 2; v < origin_bmp.bmp_info_header.biWidth; v++)
				{
					// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
					result_frequency_spectrum[(origin_bmp.bmp_info_header.biHeight - 1 - u) * origin_bmp.biWidth_actual + v] = uint8_t(round(abs(DWT_result.c[u][v].re - temp_min) / temp_diff * 255));
				}

		//*���bmp���ߴ��Ϊ2��
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

		result_bmp[0].bmp_pixel8 = result_frequency_spectrum;	// ���������任��ģֵ���궨��[0,255]���������
		result_bmp[0].biWidth_actual = origin_bmp.biWidth_actual;

		//*2�߶ȱ任
		if (from_extern)
		{
			//**��1�߶ȱ任�����ȡԭͼ
			bmp_file origin_for_2_scale;
			origin_for_2_scale.bmp_info_header.biHeight = origin_bmp.bmp_info_header.biHeight / 2;
			origin_for_2_scale.bmp_info_header.biWidth = origin_bmp.bmp_info_header.biWidth / 2;
			origin_for_2_scale.bmp_info_header.biBitCount = origin_bmp.bmp_info_header.biBitCount;
			origin_for_2_scale.biWidth_actual = origin_for_2_scale.bmp_info_header.biWidth;
			origin_for_2_scale.bmp_pixel8_double = new double[origin_for_2_scale.bmp_info_header.biHeight * origin_for_2_scale.bmp_info_header.biWidth];
			if (origin_for_2_scale.bmp_info_header.biHeight * origin_for_2_scale.bmp_info_header.biWidth > 1) //��ֹ����Խ��
				for (int u = 0; u < origin_for_2_scale.bmp_info_header.biHeight; u++)
					for (int v = 0; v < origin_for_2_scale.bmp_info_header.biWidth; v++)
					{
						// ���ڶ���ʱ�ķ���д��ʱҲ��Ҫ����
						origin_for_2_scale.bmp_pixel8_double[(origin_for_2_scale.bmp_info_header.biHeight - 1 - u) * origin_for_2_scale.biWidth_actual + v]
							= DWT_result.c[u][v].re;
					}
			//**2�߶ȱ任
			bmp_file* ptr_for_2_scale = DWT_Haar_1_and_2_scale(origin_for_2_scale, false);

			//*ƴ���γ�2�߶ȵ��ܱ任ͼ
			//**��ȡ2�߶ȱ任���
			uint8_t* result_2_scale = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
			if (origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight > 1) //��ֹ����Խ��
			{
				for (int u = origin_for_2_scale.bmp_info_header.biHeight; u < origin_bmp.bmp_info_header.biHeight; u++)
				{
					for (int v = 0; v < origin_for_2_scale.bmp_info_header.biWidth; v++)	//2�߶ȱ任����������Ͻǣ����ಿ��ȫ�հ�1�߶Ƚ��
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

			//**���bmp
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

			result_bmp[1].bmp_pixel8 = result_2_scale;	// ���������任��ģֵ���궨��[0,255]���������
			result_bmp[1].biWidth_actual = origin_bmp.biWidth_actual;
		}
	}
	break;
	default:
	break;
	}

	return result_bmp;
}

//��ɢ���ұ任������䣩��ͨ������Գ���չͼ��DFT������DCT����Ч��̫�Ͷ�������
/*bmp_file DCT_zero_padding(const bmp_file& origin_bmp, bool flag)
//{
//	bmp_file result_bmp;
//
//	switch (origin_bmp.bmp_info_header.biBitCount)
//	{
//	case 8:	// 8λ256ɫ��ɫ��ͼ
//	{
//		//*��ȡ�����ֽڶ������ص�һ��������չͼ��
//		int new_biWidth = origin_bmp.bmp_info_header.biWidth * 2;
//		int new_biWidth_extended = new_biWidth * 2;
//		int new_biHeight = origin_bmp.bmp_info_header.biHeight * 2;
//		int new_biHeight_extended = new_biHeight * 2;
//		int new_num_pixels = new_biHeight * new_biWidth;
//		int new_biWidth_actual = ((((origin_bmp.bmp_info_header.biBitCount * new_biWidth) + 31) & ~31) >> 3);
//		double temp_double = .0;
//		//*ͨ������Գ���չͼ��DFT������DCT��������ͼ�������İ����뱾P345������άDCT�ɲ��Ϊ����һάDCT
//		//**ϵ������s��ֻ�е�һ����Ҫ*1/sqrt(2.0)��ֱ�ӳ˼���
//		//**ϵ������h
//		ComplexMatrix h_row(1, new_biWidth);		//�����е�DCT
//		for (int u = 0; u < new_biWidth; u++)
//		{
//			h_row.c[0][u].re = cos(-pi * u / 2 / new_biWidth);
//			h_row.c[0][u].im = sin(-pi * u / 2 / new_biWidth);
//			//h_row.c[0][u].re = cos(-pi * u / new_biWidth);
//			//h_row.c[0][u].im = sin(-pi * u / new_biWidth);
//		}
//		ComplexMatrix h_column(new_biHeight, 1);	//�����е�DCT
//		for (int u = 0; u < new_biHeight; u++)
//		{
//			h_column.c[u][0].re = cos(-pi * u / 2 / new_biHeight);
//			h_column.c[u][0].im = sin(-pi * u / 2 / new_biHeight);
//			//h_column.c[u][0].re = cos(-pi * u / new_biHeight);
//			//h_column.c[u][0].im = sin(-pi * u / new_biHeight);
//		}
//		//**��һ�ζ���DCT
//		//***��ȡ���أ�ͬʱ�жԳ���չ��ÿ�б���������
//		ComplexMatrix origin_bmp_extended(new_biHeight, new_biWidth_extended); // ׼������һ��һάDCT
//		if (new_num_pixels > 1)	// ��ֹ����Խ��
//			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
//				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
//				{
//					temp_double = double(origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j]);
//					origin_bmp_extended.c[i][new_biWidth_extended - 1 - j].re = temp_double;
//					origin_bmp_extended.c[i][j].re = temp_double;
//				}
//		//***����ÿ�е�һάDFT
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
//		//***���������ֻ��ȡ�����ǰ�벿����ΪDCT������ʺ�벿���в��óˣ�
//		for (int i = 0; i < new_biHeight; i++)
//		{
//			DFT_each_row.c[i][0].re *= (1 / sqrt(2.0));
//			DFT_each_row.c[i][0].im *= (1 / sqrt(2.0));
//			for (int j = 0; j < new_biWidth; j++)
//			{
//				DFT_each_row.c[i][j] = DFT_each_row.c[i][j] * h_row.c[0][j];
//			}
//		}
//		//***��ȡ�����ǰ�벿����ΪDCT�����ͬʱ�жԳ���չ��ÿ�б���������
//		ComplexMatrix DCT_each_row_result(new_biHeight_extended, new_biWidth); // ׼�����ڶ���һάDCT
//		for (int i = 0; i < new_biHeight; i++)
//			for (int j = 0; j < new_biWidth; j++)
//			{
//				temp_double = DFT_each_row.c[i][j].re;	// ֻȡʵ��
//				DCT_each_row_result.c[new_biHeight_extended - 1 - i][j].re = temp_double;
//				DCT_each_row_result.c[i][j].re = temp_double;
//			}
//		//***����ÿ�е�һάDFT
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
//		//***���������ֻ��ȡ�����ǰ�벿����ΪDCT������ʺ�벿���в��óˣ�
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
//		//**��ȡ�����ǰ�벿����ΪDCT�����������Ƶ�ף�ģֵ��
//		//***DCT���Ƶ��������������
//		ComplexMatrix DCT_frequency_spectrum(new_biHeight, new_biWidth);
//		double temp_modulus = .0, temp_max = .0;	// ��¼��ȫ�������е����ֵ���Ա�궨�����������Сֵ��0��
//		for (int u = 0; u < new_biHeight; u++)
//			for (int v = 0; v < new_biWidth; v++)
//			{
//				temp_modulus = DFT_each_column.c[u][v].re;	// DCTֻȡʵ��
//
//				temp_modulus = log(1 + abs(temp_modulus));	// ת��Ϊ�����߶��Ը��õؿ��ӻ�
//
//				DCT_frequency_spectrum.c[u][v].re = temp_modulus;
//				if (temp_max < temp_modulus)temp_max = temp_modulus;
//			}
//		//***�궨��[0,255]
//		uint8_t* result_frequency_spectrum = new uint8_t[new_biWidth_actual * new_biHeight];
//		if (new_biWidth_actual * new_biHeight > 1) //��ֹ����Խ��
//			for (int u = 0; u < new_biHeight; u++)
//				for (int v = 0; v < new_biWidth; v++)
//				{
//					//****ע�⣺����õ���DCT���ͼ���кŷ�ת��
//					result_frequency_spectrum[u * new_biWidth_actual + v] = uint8_t(round(DCT_frequency_spectrum.c[new_biHeight - 1 - u][v].re / temp_max * 255));
//				}
//
//		//*���bmp���ߴ��Ϊ2��
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
//		result_bmp.bmp_pixel8 = result_frequency_spectrum;	// ���������任Ƶ�ף�ģֵ���궨��[0,255]���������
//		result_bmp.biWidth_actual = new_biWidth_actual;
//	}
//	break;
//	default:
//		break;
//	}
//	return result_bmp;
//}*/

#endif