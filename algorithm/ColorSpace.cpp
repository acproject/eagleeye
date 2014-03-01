#include "ColorSpace.h"
#include <math.h>

namespace eagleeye
{
Matrix<Array<float,3>> rgb2xyz(const Matrix<ERGB>& rgb)
{
	int rows=rgb.rows();
	int cols=rgb.cols();
	Matrix<Array<float,3>> xyz(rows,cols,Array<float,3>(0));

	for (int i=0;i<rows;++i)
	{
		const ERGB* rgb_data=rgb.row(i);
		Array<float,3>* xyz_data=xyz.row(i);
		for (int j=0;j<cols;++j)
		{
			float R = rgb[j][0]/255.0f;
			float G = rgb[j][1]/255.0f;
			float B = rgb[j][2]/255.0f;

			float r, g, b;

			if(R <= 0.04045f)	r = R/12.92f;
			else				r = pow((R+0.055f)/1.055f,2.4f);
			if(G <= 0.04045f)	g = G/12.92f;
			else				g = pow((G+0.055f)/1.055f,2.4f);
			if(B <= 0.04045f)	b = B/12.92f;
			else				b = pow((B+0.055f)/1.055f,2.4f);

			xyz_data[j][0] = r*0.4124564f + g*0.3575761f + b*0.1804375f;
			xyz_data[j][1] = r*0.2126729f + g*0.7151522f + b*0.0721750f;
			xyz_data[j][2] = r*0.0193339f + g*0.1191920f + b*0.9503041f;
		}
	}

	return xyz;
}

Matrix<Array<float,3>> rgb2lab(const Matrix<ERGB>& rgb)
{
	int rows=rgb.rows();
	int cols=rgb.cols();
	Matrix<Array<float,3>> lab(rows,cols,Array<float,3>(0));

	for (int i=0;i<rows;++i)
	{
		const ERGB* rgb_data=rgb.row(i);
		Array<float,3>* lab_data=lab.row(i);
		for (int j=0;j<cols;++j)
		{
			//------------------------
			// sRGB to XYZ conversion
			//------------------------
			float X, Y, Z;
			float R = rgb[j][0]/255.0f;
			float G = rgb[j][1]/255.0f;
			float B = rgb[j][2]/255.0f;

			float r, g, b;

			if(R <= 0.04045f)	r = R/12.92f;
			else				r = pow((R+0.055f)/1.055f,2.4f);
			if(G <= 0.04045f)	g = G/12.92f;
			else				g = pow((G+0.055f)/1.055f,2.4f);
			if(B <= 0.04045f)	b = B/12.92f;
			else				b = pow((B+0.055f)/1.055f,2.4f);

			X = r*0.4124564f + g*0.3575761f + b*0.1804375f;
			Y = r*0.2126729f + g*0.7151522f + b*0.0721750f;
			Z = r*0.0193339f + g*0.1191920f + b*0.9503041f;

			//------------------------
			// XYZ to LAB conversion
			//------------------------
			float epsilon = 0.008856f;	//actual CIE standard
			float kappa   = 903.3f;		//actual CIE standard

			float Xr = 0.950456f;	//reference white
			float Yr = 1.0f;		//reference white
			float Zr = 1.088754f;	//reference white

			float xr = X/Xr;
			float yr = Y/Yr;
			float zr = Z/Zr;

			float fx, fy, fz;
			if(xr > epsilon)	fx = pow(xr, 1.0f/3.0f);
			else				fx = (kappa*xr + 16.0f)/116.0f;
			if(yr > epsilon)	fy = pow(yr, 1.0f/3.0f);
			else				fy = (kappa*yr + 16.0f)/116.0f;
			if(zr > epsilon)	fz = pow(zr, 1.0f/3.0f);
			else				fz = (kappa*zr + 16.0f)/116.0f;

			lab_data[j][0] = 116.0f*fy-16.0f;
			lab_data[j][1] = 500.0f*(fx-fy);
			lab_data[j][2] = 200.0f*(fy-fz);
		}
	}

	return lab;
}

Matrix<ERGB> autoBuildColorTable(int class_num)
{
	Matrix<ERGB> colortable;
	return colortable;
}
}