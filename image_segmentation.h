#pragma once

#include"read_write_bmp.h"

//-图像分割-

//	边缘检测工具：得到拉普拉斯的阈值处理后图像
bmp_file Laplacian_threshold_image_of(bmp_file origin_bmp, int threshold_T)
{
	bmp_file result_bmp;
	int filter_width = 3;	// 拉普拉斯算子核的宽度
	int padding_length = filter_width / 2;	// 填充长度；使用零填充
	switch (origin_bmp.bmp_info_header.biBitCount)	// 根据biBitCount决定处理方法
	{
	case 8:	// 8位：每个像素1字节
	{
		//提取像素，其中排除因四字节对齐填充的无意义像素
		int padded_biWidth = origin_bmp.bmp_info_header.biWidth + padding_length * 2;
		int padded_biHeight = origin_bmp.bmp_info_header.biHeight + padding_length * 2;
		uint8_t* padded_pixels = new uint8_t[padded_biWidth * padded_biHeight];
		std::memset(padded_pixels, 0, sizeof(uint8_t) * padded_biWidth * padded_biHeight);
		if (padded_biHeight >= 3 && padded_biWidth >= 3)	// 防止越界警告
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					padded_pixels[(i + padding_length) * padded_biWidth + padding_length + j] = origin_bmp.bmp_pixel8[i * origin_bmp.biWidth_actual + j];
				}
			}
	//镜像填充
		if (padded_biHeight >= 3 && padded_biWidth >= 3)
		{
			int padded_biHeight_half = origin_bmp.bmp_info_header.biHeight + padding_length;
			//*左右边
			for (int i = padding_length; i < padded_biHeight_half; i++)
			{
				int left = i * padded_biWidth + padding_length;
				int right = i * padded_biWidth + padding_length + origin_bmp.bmp_info_header.biWidth - 1;	//**注意减一，因为前面部分得到的起点已经是原图第一列了**
				for (int j = 1; j <= padding_length; j++)
				{
					padded_pixels[left - j] = padded_pixels[left + j];	//左边
					padded_pixels[right + j] = padded_pixels[right - j];	//右边
				}
			}
			//*上下边（包括四角）
			for (int i = 1; i <= padding_length; i++)
			{
				int origin_bottom = (padding_length + i) * padded_biWidth;
				int padding_bottom = (padding_length - i) * padded_biWidth;
				int origin_top = (padding_length + origin_bmp.bmp_info_header.biHeight - 1 - i) * padded_biWidth;	//**注意减一，因为前面部分得到的起点已经是原图第一行了**
				int padding_top = (padding_length + origin_bmp.bmp_info_header.biHeight - 1 + i) * padded_biWidth;	//**注意减一，因为前面部分得到的起点已经是原图第一行了**
				for (int j = 0; j < padded_biWidth; j++)
				{
					padded_pixels[padding_bottom + j] = padded_pixels[origin_bottom + j];	//下边
					padded_pixels[padding_top + j] = padded_pixels[origin_top + j];		//上边
				}
			}
		}
		//拉普拉斯算子计算
		uint8_t* Laplacian_abs_value = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
		std::memset(Laplacian_abs_value, 0, sizeof(uint8_t) * origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight);
		int row_index = 0, index_1 = 0, padded_row_index = 0, padded_index = 0;	// 计算索引辅助变量
		if (padded_biHeight >= 3 && padded_biWidth >= 3 && origin_bmp.biWidth_actual >= 3 && origin_bmp.bmp_info_header.biHeight >= 3)	// 防止越界警告
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				padded_row_index = (i + padding_length) * padded_biWidth + padding_length;
				row_index = i * origin_bmp.biWidth_actual;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					padded_index = padded_row_index + j;	//在零填充图里对应的索引
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
		//拉普拉斯结果图阈值处理（结果包含四字节对齐）
		uint8_t* Laplacian_threshold = new uint8_t[origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight];
		std::memset(Laplacian_threshold, 0, sizeof(uint8_t) * origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight);
		if (origin_bmp.biWidth_actual >= 3 && origin_bmp.bmp_info_header.biHeight >= 3)	// 防止越界警告
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				row_index = i * origin_bmp.biWidth_actual;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					index_1 = row_index + j;
					Laplacian_threshold[index_1] = uint8_t((Laplacian_abs_value[index_1] > threshold_T) ? 255 : 0);
				}
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

//	Otsu方法中间工具（处理直方图，返回阈值）
int Otsu_threshold(const int* histogram, WORD bmp_biBitCount, int num_pixel, bool separability_measure_display = false)
{
	int result = 0;
	switch (bmp_biBitCount)	// bmp_biBitCount决定了histogram有多少项
	{
	case 8:	// 8位：直方图有256项
	{
		//*累计和、累计均值；累计均值最后一项是全局均值
		double* p1_k = new double[256];	// 累计和
		double* m_k = new double[256];	// 累计均值
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
		//*计算类间方差，记录最大值
		double* sigma2_B = new double[256];	// 类间方差
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
		//*Otsu阈值等于类间方差最大值位置，若最大值不唯一，则取对应位置灰度的平均值
		int k_star_mean = 0;	// 记录最大值平均灰度位置
		int count = 0;			// 记录最大值个数
		for (int k = 0; k < 256; k++)
		{
			if (abs(sigma2_B[k] - max_sigma2_B) < 1e-300)
			{
				k_star_mean += k;
				count++;
			}
		}
		k_star_mean /= count;
		//*若需要，则计算并显示全局方差和可分离性测度
		if (separability_measure_display)
		{
			//**全局方差
			double sigma2_G = 0.0;
			for (int k = 0; k < 256; k++)sigma2_G += (pow((k - m_k[255]), 2) * histogram[k]);
			sigma2_G /= num_pixel;
			//**可分离性测度
			double eta_star = sigma2_B[k_star_mean] / sigma2_G;
			//**显示结果
			cout << endl;
			cout << "全局方差 = " << sigma2_G << endl;
			cout << "可分离性测度 = " << eta_star << endl;
			cout << endl;
		}
		//*输出结果，释放内存
		delete[] p1_k, m_k, sigma2_B;
		result = k_star_mean;
	}
	break;
	default:
	break;
	}
	return result;
}

//	Otsu方法全局阈值处理
bmp_file Otsu_optimum_golbal_thresholding(bmp_file origin_bmp, bool separability_measure_display = false)
{
	bmp_file result_bmp;
	switch (origin_bmp.bmp_info_header.biBitCount)	// 根据biBitCount决定处理方法
	{
	case 8:	// 8位：每个像素1字节
	{
		int num_pixel_actual = origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight;
		//*灰度分布统计
		int num_pixel = origin_bmp.bmp_info_header.biWidth * origin_bmp.bmp_info_header.biHeight;
		int* histogram = hist_0_255(origin_bmp.bmp_pixel8, origin_bmp.bmp_info_header.biWidth, origin_bmp.bmp_info_header.biHeight, origin_bmp.biWidth_actual);
		//*得到阈值
		int threshold = Otsu_threshold(histogram, origin_bmp.bmp_info_header.biBitCount, num_pixel, separability_measure_display);
		//*阈值处理
		uint8_t* result_pixels = new uint8_t[num_pixel_actual];
		std::memset(result_pixels, 0, sizeof(uint8_t) * num_pixel_actual);
		int row_index = 0, index_1 = 0;
		if (num_pixel_actual > 1)	//防止越界警告
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			row_index = i * origin_bmp.biWidth_actual;
			for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
			{
				index_1 = row_index + j;
				result_pixels[index_1] = (origin_bmp.bmp_pixel8[index_1] > threshold) ? 255 : 0;
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

		result_bmp.bmp_pixel8 = result_pixels;
		result_bmp.biWidth_actual = origin_bmp.biWidth_actual;
	}
	break;
	default:
	break;
	}

	return result_bmp;
}

//	Otsu方法基于拉普拉斯的边缘信息改进全局阈值处理
bmp_file Otsu_optimum_golbal_thresholding_Laplacian_edge_info(bmp_file origin_bmp, int Laplacian_threshold, bool separability_measure_display = false)
{
	bmp_file result_bmp;
	//得到拉普拉斯绝对值阈值处理后的图像（使用8位bmp，前景255，背景0）
	bmp_file Laplacian_threshold_image = Laplacian_threshold_image_of(origin_bmp, Laplacian_threshold); 
	switch (origin_bmp.bmp_info_header.biBitCount)	// 根据biBitCount决定处理方法
	{
	case 8:	// 8位：每个像素1字节
	{
		int num_pixel_actual = origin_bmp.biWidth_actual * origin_bmp.bmp_info_header.biHeight;
		//只用上述拉普拉斯图像中前景部分计算直方图
		int* histogram = new int[256];
		std::memset(histogram, 0, sizeof(int) * 256);
		uint8_t temp = 0;
		int row_index = 0, index_1 = 0;
		for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
		{
			row_index = i * origin_bmp.biWidth_actual;	// 注意四字节对齐
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
		int num_total_pixel_foreground = 0;	// 前景像素总数计数
		for (int i = 0; i < 256; i++)num_total_pixel_foreground += histogram[i];
		// 得到阈值
		int threshold = Otsu_threshold(histogram, origin_bmp.bmp_info_header.biBitCount, num_total_pixel_foreground, separability_measure_display);
		// 阈值处理
		uint8_t* result_pixels = new uint8_t[num_pixel_actual];
		std::memset(result_pixels, 0, sizeof(uint8_t) * num_pixel_actual);
		if (num_pixel_actual > 1)	//防止越界警告
			for (int i = 0; i < origin_bmp.bmp_info_header.biHeight; i++)
			{
				row_index = i * origin_bmp.biWidth_actual;
				for (int j = 0; j < origin_bmp.bmp_info_header.biWidth; j++)
				{
					index_1 = row_index + j;
					result_pixels[index_1] = (origin_bmp.bmp_pixel8[index_1] > threshold) ? 255 : 0;
				}
			}
		// 结果bmp
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