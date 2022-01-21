#pragma once

#include<iostream>

#include"complex_matrix_operators.h"
#include"read_write_bmp.h"

//���ڽ���ֵ
bmp_file image_nearest_neighbor_interpolation(bmp_file origin_bmp, int new_biWidth, int new_biHeight)
{
	//���ֽڶ�������
	int new_biWidth_actual = new_biWidth;
	//*8λ256ɫλͼ
	if (origin_bmp.bmp_info_header.biBitCount == 8)
	{
		new_biWidth_actual = (8 * new_biWidth + 31) / 32 * 4;
	}

	//���������һ�����ýϴ�һ�����������ɲ�ı����0��ʼ��
	//*�����꣨�У�
	double* origin_x = new double[origin_bmp.bmp_info_header.biWidth];
	double* new_x = new double[new_biWidth];
	if (origin_bmp.biWidth_actual > new_biWidth) //ԭ��ȸ���
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
	//*�����꣨�У�
	double* origin_y = new double[origin_bmp.bmp_info_header.biHeight];
	double* new_y = new double[new_biHeight];
	if (origin_bmp.bmp_info_header.biHeight > new_biHeight) //ԭ�߶ȸ���
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

	//������ͼÿһ������
	uint8_t* new_pixel = new uint8_t[new_biWidth_actual * new_biHeight];
	for (int i = 0; i < new_biHeight; i++)
		for (int j = 0; j < new_biWidth; j++)
		{
			//�����ص�����(new_x[j],new_y[i])
			//�ҵ����ڽ���4��������ԭͼ�����ꣻ�洢���º���ʱ��(k1,k2)�����½�
			//*��������ֱ����������ֵ�����ŷ�ת
			int k1 = 0;
			if (origin_bmp.bmp_info_header.biWidth >= 2)				//ԭͼ�����ڻ����2ʱ
				while (((origin_x[k1] - new_x[j]) * (origin_x[k1 + 1] - new_x[j])) > 0)
					k1++;
			//*��������ֱ����������ֵ�����ŷ�ת
			int k2 = 0;
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//ԭͼ��ߴ��ڻ����2ʱ
				while (((origin_y[k2] - new_y[i]) * (origin_y[k2 + 1] - new_y[i])) > 0)
					k2++;
			//Ѱ��4�����������һ��
			if (origin_bmp.bmp_info_header.biWidth >= 2)				//ԭͼ�����ڻ����2ʱ
				k1 = abs(origin_x[k1] - new_x[j]) <= abs(origin_x[k1 + 1] - new_x[j]) ? (k1) : (k1 + 1);
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//ԭͼ��ߴ��ڻ����2ʱ
				k2 = abs(origin_y[k2] - new_y[i]) <= abs(origin_y[k2 + 1] - new_y[i]) ? (k2) : (k2 + 1);
			//ȡ�����ٽ������ص�ֵ������������
			//*8λ256ɫλͼ
			if (origin_bmp.bmp_info_header.biBitCount == 8)
			{
				new_pixel[i * new_biWidth_actual + j] = origin_bmp.bmp_pixel8[k2 * origin_bmp.biWidth_actual + k1];
			}
		}
	//���ͼ
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


//˫���Բ�ֵ
bmp_file image_bilinear_interpolation(bmp_file origin_bmp, int new_biWidth, int new_biHeight)
{
	//���ֽڶ�������
	int new_biWidth_actual = new_biWidth;
	//*8λ256ɫλͼ
	if (origin_bmp.bmp_info_header.biBitCount == 8)
	{
		new_biWidth_actual = (8 * new_biWidth + 31) / 32 * 4;
	}

	//���������һ�����ýϴ�һ�����������ɲ�ı����0��ʼ��
	//*�����꣨�У�
	double* origin_x = new double[origin_bmp.bmp_info_header.biWidth];
	double* new_x = new double[new_biWidth];
	double origin_x_increment = 0.0;
	if (origin_bmp.bmp_info_header.biWidth > new_biWidth) //ԭ��ȸ���
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
	//*�����꣨�У�
	double* origin_y = new double[origin_bmp.bmp_info_header.biHeight];
	double* new_y = new double[new_biHeight];
	double origin_y_increment = 0.0;
	if (origin_bmp.bmp_info_header.biHeight > new_biHeight) //ԭ�߶ȸ���
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

	//������ͼÿһ������
	uint8_t* new_pixel = new uint8_t[new_biWidth_actual * new_biHeight];
	for (int i = 0; i < new_biHeight; i++)
		for (int j = 0; j < new_biWidth; j++)
		{
			//�����ص�����(new_x[j],new_y[i])
			//�ҵ����ڽ���4��������ԭͼ�����ꣻ�洢���º���ʱ��(k1,k2)�����½�
			//*��������ֱ����������ֵ�����ŷ�ת
			int k1 = 0;
			if (origin_bmp.bmp_info_header.biWidth >= 2)				//ԭͼ�����ڻ����2ʱ
				while (((origin_x[k1] - new_x[j]) * (origin_x[k1 + 1] - new_x[j])) > 0)
					k1++;
			//*��������ֱ����������ֵ�����ŷ�ת
			int k2 = 0;
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//ԭͼ��ߴ��ڻ����2ʱ
				while (((origin_y[k2] - new_y[i]) * (origin_y[k2 + 1] - new_y[i])) > 0)
					k2++;
			//ʹ���ٽ���4�����ص�ֵ����˫���Բ�ֵ������������
			//�ҵ����ڽ���4���㡪��������λ��ԭ��������/���ϣ�����һ��/�в�ֵȨ��Ϊ�㣬�������⴦��
			//*8λ256ɫλͼ
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
				//else 3���������=1����=1��ֻ��1������
			}
		}
	//���ͼ
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



//˫���β�ֵ
bmp_file image_bicubic_interpolation(bmp_file origin_bmp, int new_biWidth, int new_biHeight)
{
	//���ֽڶ�������
	int new_biWidth_actual = new_biWidth;
	//*8λ256ɫλͼ
	if (origin_bmp.bmp_info_header.biBitCount == 8)
	{
		new_biWidth_actual = (8 * new_biWidth + 31) / 32 * 4;
	}

	//���������һ�����ýϴ�һ�����������ɲ�ı����0��ʼ��
	//*�����꣨�У�
	double* origin_x = new double[origin_bmp.bmp_info_header.biWidth];
	double* new_x = new double[new_biWidth];
	double origin_x_increment = 0.0;
	if (origin_bmp.bmp_info_header.biWidth > new_biWidth) //ԭ��ȸ���
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
	//*�����꣨�У�
	double* origin_y = new double[origin_bmp.bmp_info_header.biHeight];
	double* new_y = new double[new_biHeight];
	double origin_y_increment = 0.0;
	if (origin_bmp.bmp_info_header.biHeight > new_biHeight) //ԭ�߶ȸ���
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

	//*��һ��ϵ����������
	ComplexMatrix B_inv(4, 4, true);
	B_inv.c[0][0].re = -1.0 / 6;	B_inv.c[0][1].re = 1.0 / 2;		B_inv.c[0][2].re = -1.0 / 2; B_inv.c[0][3].re = 1.0 / 6;
	B_inv.c[1][0].re = 1.0;			B_inv.c[1][1].re = -5.0 / 2;	B_inv.c[1][2].re = 2.0;		 B_inv.c[1][3].re = -1.0 / 2;
	B_inv.c[2][0].re = -11.0 / 6;	B_inv.c[2][1].re = 3.0;			B_inv.c[2][2].re = -3.0 / 2; B_inv.c[2][3].re = 1.0 / 3;
	B_inv.c[3][0].re = 1.0;			B_inv.c[3][1].re = 0.0;			B_inv.c[3][2].re = 0.0;		 B_inv.c[3][3].re = 0.0;
	ComplexMatrix B_inv_T = (!B_inv);

	//������ͼÿһ������
	uint8_t* new_pixel = new uint8_t[new_biWidth_actual * new_biHeight];
	for (int i = 0; i < new_biHeight; i++)
		for (int j = 0; j < new_biWidth; j++)
		{
			//�����ص�����(new_x[j],new_y[i])
			//�ҵ����ڽ���16��������ԭͼ�����ꣻ�洢���º���ʱ��(k1,k2)�����½�
			//*��������ֱ����������ֵ�����ŷ�ת
			int k1 = 0;
			if (origin_bmp.bmp_info_header.biWidth >= 2)	//ԭͼ��ߴ��ڻ����2ʱ
				while (((origin_x[k1] - new_x[j]) * (origin_x[k1 + 1] - new_x[j])) > 0)
					k1++;
			//*��������ֱ����������ֵ�����ŷ�ת
			int k2 = 0;
			if (origin_bmp.bmp_info_header.biHeight >= 2)	//ԭͼ�����ڻ����2ʱ
				while (((origin_y[k2] - new_y[i]) * (origin_y[k2 + 1] - new_y[i])) > 0)
					k2++;

			//ʹ���ٽ���16�����ص�ֵ����˫���β�ֵ������������
			//�ҵ����ڽ���16����ʱ������λ��ԭ��������/���ϣ�����һ��/�в�ֵȨ��Ϊ�㣬�������⴦��
			//*ȡ��16�����ص�˼·�����ٽ�2*2=4������½����ͣ������ֺ�/�����꿴�Ƿ񴥱ߣ��ǵĻ��͸���ʼ��ĺ�/������
			//*�����ʼ��̶�ΪҪ�ҵ�4*4��������½ǣ�������ʼ������ҵ�16������
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
			//*8λ256ɫλͼ
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
				//else 3���������<4����<4����<4
			}
		}
	//���ͼ
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