#include<stdlib.h>
#include"read_write_bmp.h"
#include"image_interpolation.h"
#include"histogram_equalization.h"
#include"filters__spatial_domain.h"
#include"orthogonal_transformation.h"
#include"morphology.h"
#include"image_segmentation.h"
using namespace std;

int main()
{
	string bmp_file_name;
	int order = 0;
	double scaler = 1.5;
	cout << "测试模式：1[lena缩放]；2[ceshi.bmp缩放]；3[输入文件名 缩放]" << endl;
	cout << "          4[data2直方图均衡化]；5[ceshi.bmp直方图均衡化]；6[输入文件名 直方图均衡化]" << endl;
	cout << "          7[girl256-pepper&salt.bmp低通高斯滤波器消除椒盐噪声]；8[girl256-pepper&salt.bmp中值滤波器消除椒盐噪声]" << endl;
	cout << "          9[自定义图片低通高斯滤波器消除椒盐噪声]；10[自定义图片中值滤波器消除椒盐噪声]" << endl;
	cout << "          11[data4.bmp直方图均衡化]；12[自定义24位真彩色bmp图片直方图均衡化（两种）]" << endl;
	cout << "          13[自定义偏暗24位真彩色bmp图片直方图均衡化]；14[自定义偏亮24位真彩色bmp图片直方图均衡化（含饱和度处理）]" << endl;
	cout << "          15[lena512.bmp的DFT频谱]；16[lena512.bmp的DCT变换]；17[lena512.bmp的DWT(Haar小波)变换]" << endl;
	cout << "          （15/16已转换为对数尺度以更好地可视化）" << endl;
	cout << "          18[binary-triangle-distorted.bmp的形态学边界提取]" << endl;
	cout << "          19[yeast-cells.bmp的Otsu方法全局阈值处理]；20[yeast-cells.bmp的边缘信息改进全局阈值处理]" << endl;
	cout << " >> ";
	cin >> order;
	switch (order)
	{
	case 1:
	{
		bmp_file_name = "lena512.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_nearest = image_nearest_neighbor_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		bmp_file my_bmp_bilinear = image_bilinear_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		bmp_file my_bmp_bicubic = image_bicubic_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		write_bmp(my_bmp_nearest, "lena768_nearest.bmp");
		write_bmp(my_bmp_bilinear, "lena768_bilinear.bmp");
		write_bmp(my_bmp_bicubic, "lena768_bicubic.bmp");
	}
	break;
	case 2:
	{
		bmp_file_name = "ceshi.bmp";
		cout << "缩放系数 >> ";
		cin >> scaler;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_nearest = image_nearest_neighbor_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		bmp_file my_bmp_bilinear = image_bilinear_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		bmp_file my_bmp_bicubic = image_bicubic_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		write_bmp(my_bmp_nearest, "ceshi_nearest.bmp");
		write_bmp(my_bmp_bilinear, "ceshi_bilinear.bmp");
		write_bmp(my_bmp_bicubic, "ceshi_bicubic.bmp");
	}
	break;
	case 3:
	{
		cout << "请输入要打开的位图文件 >> ";
		cin >> bmp_file_name;
		cout << "缩放系数 >> ";
		cin >> scaler;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_nearest = image_nearest_neighbor_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		bmp_file my_bmp_bilinear = image_bilinear_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		bmp_file my_bmp_bicubic = image_bicubic_interpolation(my_bmp, int(my_bmp.bmp_info_header.biWidth * scaler), int(my_bmp.bmp_info_header.biHeight * scaler));
		write_bmp(my_bmp_nearest, "1_nearest.bmp");
		write_bmp(my_bmp_bilinear, "1_bilinear.bmp");
		write_bmp(my_bmp_bicubic, "1_bicubic.bmp");
	}
	break;
	case 4:
	{
		bmp_file_name = "data2.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp);
		write_bmp(my_bmp_equalized, "data2_equalized.bmp");
	}
	break;
	case 5:
	{
		bmp_file_name = "ceshi.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp);
		write_bmp(my_bmp_equalized, "ceshi_equalized.bmp");
	}
	break;
	case 6:
	{
		cout << "请输入要打开的位图文件 >> ";
		cin >> bmp_file_name;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp);
		write_bmp(my_bmp_equalized, "bmp_1_equalized.bmp");
	}
	break;
	case 7:
	{
		bmp_file_name = "girl256-pepper&salt.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		double sigma = 7.0;
		cout << "sigma = ";
		cin >> sigma;
		bmp_file my_bmp_LPFed = spatial_domain_Gaussian_LPF(my_bmp, sigma);
		//自适应文件名
		string result_file_name;
		int filter_width = int(ceil(sigma * 6));		//3sigma原则
		if (filter_width < 3) filter_width = 3;			//使用至少3*3的核
		else if (filter_width % 2 == 0) filter_width++;	//使用奇数宽度的核
		char filterWidth[16] = { 0 };
		_itoa(filter_width, filterWidth, 10);
		result_file_name = "girl256_GaussianLPFed_width_" + string(filterWidth) + ".bmp";
		write_bmp(my_bmp_LPFed, result_file_name);
	}
	break;
	case 8:
	{
		bmp_file_name = "girl256-pepper&salt.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		int filter_width = 3;
		cout << "filter_width = ";
		cin >> filter_width;
		bmp_file my_bmp_median_filtered = spatial_domain_median_filter(my_bmp, filter_width);
		//自适应文件名
		string result_file_name;
		char filterWidth[16] = { 0 };
		_itoa(filter_width, filterWidth, 10);
		result_file_name = "girl256_medianFiltered_width_" + string(filterWidth) + ".bmp";
		write_bmp(my_bmp_median_filtered, result_file_name);
	}
	break;
	case 9:
	{
		cout << "请输入文件名 >> ";
		cin >> bmp_file_name;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		double sigma = 7.0;
		cout << "sigma = ";
		cin >> sigma;
		bmp_file my_bmp_LPFed = spatial_domain_Gaussian_LPF(my_bmp, sigma);
		//自适应文件名
		string result_file_name;
		int filter_width = int(ceil(sigma * 6));		//3sigma原则
		if (filter_width < 3) filter_width = 3;			//使用至少3*3的核
		else if (filter_width % 2 == 0) filter_width++;	//使用奇数宽度的核
		char filterWidth[16] = { 0 };
		_itoa(filter_width, filterWidth, 10);
		result_file_name = bmp_file_name + "_GaussianLPFed_width_" + string(filterWidth) + ".bmp";
		write_bmp(my_bmp_LPFed, result_file_name);
	}
	break;
	case 10:
	{
		cout << "请输入文件名 >> ";
		cin >> bmp_file_name;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		int filter_width = 3;
		cout << "filter_width = ";
		cin >> filter_width;
		bmp_file my_bmp_median_filtered = spatial_domain_median_filter(my_bmp, filter_width);
		//自适应文件名
		string result_file_name;
		char filterWidth[16] = { 0 };
		_itoa(filter_width, filterWidth, 10);
		result_file_name = bmp_file_name + "_medianFiltered_width_" + string(filterWidth) + ".bmp";
		write_bmp(my_bmp_median_filtered, result_file_name);
	}
	break;
	case 11:
	{
		bmp_file_name = "data4.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp);
		write_bmp(my_bmp_equalized, "data4_equalized.bmp");
		bmp_file my_bmp_equalized2 = histogram_equalize(my_bmp, false);
		write_bmp(my_bmp_equalized2, "data4_equalized_saturaion.bmp");
		
	}
	break;
	case 12:
	{
		cout << "请输入文件名 >> ";
		cin >> bmp_file_name;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp);
		bmp_file my_bmp_equalized2 = histogram_equalize(my_bmp, false);
		//自适应文件名
		string result_file_name = bmp_file_name + "_equalized.bmp";
		write_bmp(my_bmp_equalized, result_file_name);
		string result_file_name2 = bmp_file_name + "_equalized_saturation.bmp";
		write_bmp(my_bmp_equalized2, result_file_name2);
	}
	break;
	case 13:
	{
		cout << "请输入文件名 >> ";
		cin >> bmp_file_name;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp);
		//自适应文件名
		string result_file_name = bmp_file_name + "_equalized.bmp";
		write_bmp(my_bmp_equalized, result_file_name);
	}
	break;
	case 14:
	{
		cout << "请输入文件名 >> ";
		cin >> bmp_file_name;
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_equalized = histogram_equalize(my_bmp, false);
		//自适应文件名
		string result_file_name = bmp_file_name + "_equalized_saturation.bmp";
		write_bmp(my_bmp_equalized, result_file_name);
	}
	break;
	case 15:
	{
		bmp_file_name = "lena512.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_orthogonal_transformed = DFT_zero_padding(my_bmp);
		//自适应文件名
		string result_file_name = bmp_file_name + "_DFT.bmp";
		write_bmp(my_bmp_orthogonal_transformed, result_file_name);
	}
	break;
	case 16:
	{
		bmp_file_name = "lena512.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_orthogonal_transformed = DCT_zero_padding(my_bmp);
		//自适应文件名
		string result_file_name = bmp_file_name + "_DCT.bmp";
		write_bmp(my_bmp_orthogonal_transformed, result_file_name);
	}
	break;
	case 17:
	{
		bmp_file_name = "lena512.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file* my_bmp_orthogonal_transformed = DWT_Haar_1_and_2_scale(my_bmp, true);
		//自适应文件名
		char currentScale[16] = { 0 };
		string result_file_name;
		for (int i = 0; i <= 1; i++)
		{
			_itoa(i + 1, currentScale, 10);
			result_file_name = bmp_file_name + "_DWT_Haar_" + string(currentScale) + "_scale.bmp";
			write_bmp(my_bmp_orthogonal_transformed[i], result_file_name);
		}
	}
	break;
	case 18:
	{
		bmp_file_name = "binary-triangle-distorted.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		unsigned int element_length = 1;	//结构元尺寸
		cout << "结构元尺寸 >> ";
		cin >> element_length;
		bmp_file my_bmp_erosion = erosion(my_bmp, element_length);
		bmp_file my_bmp_boundary = boundary_extraction(my_bmp, element_length);
		//自适应文件名
		string result_file_name;
		char current_element_length[16] = { 0 };
		_itoa(element_length, current_element_length, 10);
		result_file_name = bmp_file_name + string(current_element_length) + "x" + string(current_element_length) + "element" + "_erosion_.bmp";
		write_bmp(my_bmp_erosion, result_file_name);
		result_file_name = bmp_file_name + string(current_element_length) + "x" + string(current_element_length) + "element" + "_boundary_.bmp";
		write_bmp(my_bmp_boundary, result_file_name);
	}
	break;
	case 19:
	{
		bmp_file_name = "yeast-cells.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		bmp_file my_bmp_Otsu = Otsu_optimum_golbal_thresholding(my_bmp, true);
		//自适应文件名
		string result_file_name;
		result_file_name = bmp_file_name + "_Otsu.bmp";
		write_bmp(my_bmp_Otsu, result_file_name);
	}
	break;
	case 20:
	{
		bmp_file_name = "yeast-cells.bmp";
		bmp_file my_bmp;
		my_bmp = read_bmp(bmp_file_name);
		int threshold_T = 115;
		cout << "拉普拉斯模板图像阈值处理阈值 >> ";	//经验值：4邻域拉普拉斯用6，8邻域拉普拉斯用15左右（8邻域没挨个测）
		cin >> threshold_T;
		bmp_file my_bmp_Laplacian_threshold = Laplacian_threshold_image_of(my_bmp, threshold_T);
		bmp_file my_bmp_Otsu_Laplacian = Otsu_optimum_golbal_thresholding_Laplacian_edge_info(my_bmp, threshold_T, true);
		//自适应文件名
		char currentThreshold[16] = { 0 };
		string result_file_name;
		_itoa(threshold_T, currentThreshold, 10);
		result_file_name = bmp_file_name + "_Laplacian_threshold_" + string(currentThreshold) + ".bmp";
		write_bmp(my_bmp_Laplacian_threshold, result_file_name);
		result_file_name = bmp_file_name + "_Otsu_with_Laplacian_threshold_" + string(currentThreshold) + ".bmp";
		write_bmp(my_bmp_Otsu_Laplacian, result_file_name);
	}
	break;
	default:
	break;
	}

	return 0;
}