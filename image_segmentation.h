#pragma once

#include"read_write_bmp.h"

//-ͼ��ָ�-

//	��Ե��⹤�ߣ��õ�������˹����ֵ�����ͼ��
bmp_file Laplacian_threshold_image_of(bmp_file origin_bmp, int threshold_T)
{
	bmp_file result_bmp;
	int filter_width = 3;	// ������˹���Ӻ˵Ŀ��
	int padding_length = filter_width / 2;	// ��䳤�ȣ�ʹ�������
	switch (origin_bmp.bmp_info_header.biBitCount)	// ����biBitCount����������
	{
	case 8:	// 8λ��ÿ������1�ֽ�
	{
		//��ȡ���أ������ų������ֽڶ�����������������
		int padded_biWidth = origin_bmp.bmp_info_header.biWidth + padding_length * 2;
		int padded_biHeight = origin_bmp.bmp_info_header.biHeight + padding_length * 2;
		uint8_t* padded_pixels = new uint8_t[padded_biWidth * padded_biHeight];
		std::memset(padded_pixels, 0, sizeof(uint8_t) * padded_biWidth * padded_biHeight);
		if (padded_biHeight >= 3 && padded_biWidth >= 3)	// ��ֹԽ�羯��
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					padded_pixels[(i + padding_length) * padded_biWidth + padding_length + j] = origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];
				}
			}
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
					padded_pixels[left - j] = padded_pixels[left + j];	//���
					padded_pixels[right + j] = padded_pixels[right - j];	//�ұ�
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
					padded_pixels[padding_bottom + j] = padded_pixels[origin_bottom + j];	//�±�
					padded_pixels[padding_top + j] = padded_pixels[origin_top + j];		//�ϱ�
				}
			}
		}
		//������˹���Ӽ���
		uint8_t* Laplacian_abs_value = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
		std::memset(Laplacian_abs_value, 0, sizeof(uint8_t) * origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight);
		int row_index = 0, index_1 = 0, padded_row_index = 0, padded_index = 0;	// ����������������
		if (padded_biHeight >= 3 && padded_biWidth >= 3 && origin_bmp.biWidth_actual >= 3 && origin_bmp.bmp_info_header.biHeight >= 3)	// ��ֹԽ�羯��
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				padded_row_index = (i + padding_length) * padded_biWidth + padding_length;
				row_index = i * origin_bmp.biWidth_actual;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					padded_index = padded_row_index + j;	//�������ͼ���Ӧ������
					//Laplacian_abs_value[row_index + j] = CLAMP_0_255(abs(
					//	(int)padded_pixels[padded_index + 1] + (int)padded_pixels[padded_index - 1]
					//	+ (int)padded_pixels[padded_index + padded_biWidth] + (int)padded_pixels[padded_index - padded_biWidth]
					//	+ (int)padded_pixels[padded_index + padded_biWidth + 1] + (int)padded_pixels[padded_index - padded_biWidth + 1]
					//	+ (int)padded_pixels[padded_index + padded_biWidth - 1] + (int)padded_pixels[padded_index - padded_biWidth - 1]
					//	- 8 * (int)padded_pixels[padded_index]));
					Laplacian_abs_value[row_index + j] = CLAMP_0_255(abs(
						(int)padded_pixels[padded_index + 1] + (int)padded_pixels[padded_index - 1]
						+ (int)padded_pixels[padded_index + padded_biWidth] + (int)padded_pixels[padded_index - padded_biWidth]
						- 4 * (int)padded_pixels[padded_index]));
				}
			}
		//������˹���ͼ��ֵ��������������ֽڶ��룩
		uint8_t* Laplacian_threshold = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
		std::memset(Laplacian_threshold, 0, sizeof(uint8_t) * origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight);
		if (origin_bmp.biWidth_actual >= 3 && origin_bmp.bmp_info_header.biHeight >= 3)	// ��ֹԽ�羯��
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				row_index = i * origin_bmp.biWidth_actual;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					index_1 = row_index + j;
					Laplacian_threshold[index_1] = uint8_t((Laplacian_abs_value[index_1] > threshold_T) ? 255 : 0);
				}
			}
		//���bmp
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

		result_bmp.bmp_pixel8 = Laplacian_threshold;
		//result_bmp.bmp_pixel8 = Laplacian_abs_value;
		result_bmp.biWidth_actual = origin_bmp.biWidth_actual;

		//int* histogram = hist_0_255(Laplacian_abs_value, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, origin_bmp.biWidth_actual);

		delete[] padded_pixels, Laplacian_abs_value;
	}
	break;
	default:
	break;
	}

	return result_bmp;
}

//	Otsu�����м乤�ߣ�����ֱ��ͼ��������ֵ��
int Otsu_threshold(const int* histogram, WORD bmp_biBitCount, int num_pixel, bool separability_measure_display = false)
{
	int result = 0;
	switch (bmp_biBitCount)	// bmp_biBitCount������histogram�ж�����
	{
	case 8:	// 8λ��ֱ��ͼ��256��
	{
		//*�ۼƺ͡��ۼƾ�ֵ���ۼƾ�ֵ���һ����ȫ�־�ֵ
		double* p1_k = new double[256];	// �ۼƺ�
		double* m_k = new double[256];	// �ۼƾ�ֵ
		std::memset(p1_k, 0, sizeof(double) * 256);
		std::memset(m_k, 0, sizeof(double) * 256);
		for (int k = 0; k < 256; k++)
		{
			for (int i = 0; i <= k; i++)
			{
				p1_k[k] += histogram[i];
				m_k[k] += (double(i) * histogram[i]);
			}
			p1_k[k] /= double(num_pixel);
			m_k[k] /= double(num_pixel);
		}
		//*������䷽���¼���ֵ
		double* sigma2_B = new double[256];	// ��䷽��
		std::memset(sigma2_B, 0, sizeof(double) * 256);
		double max_sigma2_B = 0.0;
		for (int k = 0; k < 256; k++)
		{
			if (p1_k[k] > 0 && p1_k[k] < 1.0)
			{
				sigma2_B[k] = pow((m_k[255] * p1_k[k] - m_k[k]), 2) / p1_k[k] / abs(1.0 - p1_k[k]);
				if (sigma2_B[k] > max_sigma2_B)max_sigma2_B = sigma2_B[k];
			}
		}
		//*Otsu��ֵ������䷽�����ֵλ�ã������ֵ��Ψһ����ȡ��Ӧλ�ûҶȵ�ƽ��ֵ
		int k_star_mean = 0;	// ��¼���ֵƽ���Ҷ�λ��
		int count = 0;			// ��¼���ֵ����
		for (int k = 0; k < 256; k++)
		{
			if (abs(sigma2_B[k] - max_sigma2_B) < 1e-300)
			{
				k_star_mean += k;
				count++;
			}
		}
		k_star_mean /= count;
		//*����Ҫ������㲢��ʾȫ�ַ���Ϳɷ����Բ��
		if (separability_measure_display)
		{
			//**ȫ�ַ���
			double sigma2_G = 0.0;
			for (int k = 0; k < 256; k++)sigma2_G += (pow((k - m_k[255]), 2) * histogram[k]);
			sigma2_G /= num_pixel;
			//**�ɷ����Բ��
			double eta_star = sigma2_B[k_star_mean] / sigma2_G;
			//**��ʾ���
			cout << endl;
			cout << "ȫ�ַ��� = " << sigma2_G << endl;
			cout << "�ɷ����Բ�� = " << eta_star << endl;
			cout << endl;
		}
		//*���������ͷ��ڴ�
		delete[] p1_k, m_k, sigma2_B;
		result = k_star_mean;
	}
	break;
	default:
	break;
	}
	return result;
}

//	Otsu����ȫ����ֵ����
bmp_file Otsu_optimum_golbal_thresholding(bmp_file origin_bmp, bool separability_measure_display = false)
{
	bmp_file result_bmp;
	switch (origin_bmp.bmp_info_header.biBitCount)	// ����biBitCount����������
	{
	case 8:	// 8λ��ÿ������1�ֽ�
	{
		int num_pixel_actual = origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight;
		//*�Ҷȷֲ�ͳ��
		int num_pixel = origin_bmp.bmp_info_header.biWidth * origin_bmp.bmp_info_header.biHeight;
		int* histogram = hist_0_255(origin_bmp.bmp_pixel8, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, origin_bmp.biWidth_actual);
		//*�õ���ֵ
		int threshold = Otsu_threshold(histogram, origin_bmp.bmp_info_header.biBitCount, num_pixel, separability_measure_display);
		//*��ֵ����
		uint8_t* result_pixels = new uint8_t[num_pixel_actual];
		std::memset(result_pixels, 0, sizeof(uint8_t) * num_pixel_actual);
		int row_index = 0, index_1 = 0;
		if (num_pixel_actual > 1)	//��ֹԽ�羯��
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			row_index = i * origin_bmp.biWidth_actual;
			for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
			{
				index_1 = row_index + j;
				result_pixels[index_1] = (origin_bmp.bmp_pixel8[index_1] > threshold) ? 255 : 0;
			}
		}
		//*���bmp
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
	}
	break;
	default:
	break;
	}

	return result_bmp;
}

//	Otsu��������������˹�ı�Ե��Ϣ�Ľ�ȫ����ֵ����
bmp_file Otsu_optimum_golbal_thresholding_Laplacian_edge_info(bmp_file origin_bmp, int Laplacian_threshold, bool separability_measure_display = false)
{
	bmp_file result_bmp;
	//�õ�������˹����ֵ��ֵ������ͼ��ʹ��8λbmp��ǰ��255������0��
	bmp_file Laplacian_threshold_image = Laplacian_threshold_image_of(origin_bmp, Laplacian_threshold); 
	switch (origin_bmp.bmp_info_header.biBitCount)	// ����biBitCount����������
	{
	case 8:	// 8λ��ÿ������1�ֽ�
	{
		int num_pixel_actual = origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight;
		//ֻ������������˹ͼ����ǰ�����ּ���ֱ��ͼ
		int* histogram = new int[256];
		std::memset(histogram, 0, sizeof(int) * 256);
		uint8_t temp = 0;
		int row_index = 0, index_1 = 0;
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			row_index = i * origin_bmp.biWidth_actual;	// ע�����ֽڶ���
			for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
			{
				index_1 = row_index + j;
				if (Laplacian_threshold_image.bmp_pixel8[index_1] != 0)
				{
					temp = origin_bmp.bmp_pixel8[index_1];
					if (temp >= 0 && temp <= 255)
						histogram[temp]++;
					else if (temp < 0)histogram[0]++;
					else histogram[255]++;
				}
			}
		}
		int num_total_pixel_foreground = 0;	// ǰ��������������
		for (int i = 0; i < 256; i++)num_total_pixel_foreground += histogram[i];
		// �õ���ֵ
		int threshold = Otsu_threshold(histogram, origin_bmp.bmp_info_header.biBitCount, num_total_pixel_foreground, separability_measure_display);
		// ��ֵ����
		uint8_t* result_pixels = new uint8_t[num_pixel_actual];
		std::memset(result_pixels, 0, sizeof(uint8_t) * num_pixel_actual);
		if (num_pixel_actual > 1)	//��ֹԽ�羯��
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				row_index = i * origin_bmp.biWidth_actual;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					index_1 = row_index + j;
					result_pixels[index_1] = (origin_bmp.bmp_pixel8[index_1] > threshold) ? 255 : 0;
				}
			}
		// ���bmp
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
	}
	break;
	default:
	break;
	}

	return result_bmp;
}