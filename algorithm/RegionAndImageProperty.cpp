#include "RegionAndImageProperty.h"
namespace eagleeye
{
int find( int set[], int x )  
{  
	int r = x;  
	while ( set[r] != r )  
		r = set[r];  
	return r;  
}  

void bwlabel(const Matrix<unsigned char>& binary_img, Matrix<int>& label,int& label_num, int neighborhood)
{
	if (neighborhood != 4 && neighborhood != 8)
	{
		neighborhood = 4;
	}

	int nr = binary_img.rows();
	int nc = binary_img.cols();

	int total = nr * nc;  

	label = Matrix<int>(nr,nc);
	int* label_data = label.dataptr();

	// results  
	memset(label_data, 0, total * sizeof(int));  

	int nobj = 0;                               // number of objects found in image  

	// other variables                               
	int* lset = new int[total];					// label table  
	memset(lset, 0, total * sizeof(int));  

	int ntable = 0;  
	for( int r = 0; r < nr; r++ )   
	{  
		const unsigned char* binary_img_data=binary_img.row(r);
		for( int c = 0; c < nc; c++ )   
		{              
			if ( binary_img_data[c] )   // if A is an object  (current pos)
			{                 
				// get the neighboring pixels B, C, D, and E  
				int B, C, D, E;  
				if ( c == 0 )   
					B = 0;   
				else   
					B = find( lset, ELEM(label_data, r, c - 1, nc) );  

				if ( r == 0 )   
					C = 0;   
				else   
					C = find( lset, ELEM(label_data, r - 1, c, nc) );  

				if ( r == 0 || c == 0 )   
					D = 0;   
				else   
					D = find( lset, ELEM(label_data, r - 1, c - 1, nc) );  

				if ( r == 0 || c == nc - 1 )   
					E = 0;  
				else   
					E = find( lset, ELEM(label_data, r - 1, c + 1, nc) );  

				switch(neighborhood)
				{
				case 4:
					{
						// apply 4 connectedness  
						if ( B && C )   
						{   
							// B and C are labeled  
							if ( B == C )  
								ELEM(label_data, r, c, nc) = B;  
							else 
							{  
								lset[C] = B;  
								ELEM(label_data, r, c, nc) = B;  
							}  
						}   
						else if ( B )				// B is object but C is not  
							ELEM(label_data, r, c, nc) = B;  
						else if ( C )               // C is object but B is not  
							ELEM(label_data, r, c, nc) = C;  
						else   
						{	
							//	B, C, D not object - new object  
							//  label and put into table  
							ntable++;  
							ELEM(label_data, r, c, nc) = lset[ ntable ] = ntable;  
						}  

						break;
					}
				case 8:
					{
						// apply 8 connectedness  
						if ( B || C || D || E )   
						{  
							int tlabel = B;  
							if ( B )   
								tlabel = B;  
							else if ( C )   
								tlabel = C;  
							else if ( D )   
								tlabel = D;  
							else if ( E )   
								tlabel = E;  

							ELEM(label_data, r, c, nc) = tlabel;  
							if ( B && B != tlabel )   
								lset[B] = tlabel;  
							if ( C && C != tlabel )   
								lset[C] = tlabel;  
							if ( D && D != tlabel )   
								lset[D] = tlabel;  
							if ( E && E != tlabel )   
								lset[E] = tlabel;  
						}   
						else   
						{  
							//   label and put into table  
							ntable++;  
							ELEM(label_data, r, c, nc) = lset[ ntable ] = ntable;  
						}  
						break;
					}
				}
			}   
			else   
			{  
				ELEM(label_data, r, c, nc) = 0;      // A is not an object so leave it  
			}  
		}  
	}  

	// consolidate component table  
	for( int i = 0; i <= ntable; i++ )  
		lset[i] = find( lset, i );          

	// run image through the look-up table  
	for( int r = 0; r < nr; r++ )  
		for( int c = 0; c < nc; c++ )  
			ELEM(label_data, r, c, nc) = lset[ ELEM(label_data, r, c, nc) ];  

	// count up the objects in the image  
	for( int i = 0; i <= ntable; i++ )  
		lset[i] = 0;  
	for( int r = 0; r < nr; r++ )  
		for( int c = 0; c < nc; c++ )  
			lset[ ELEM(label_data, r, c, nc) ]++;  

	// number the objects from 1 through n objects  
	nobj = 0;  
	lset[0] = 0;  
	for( int i = 1; i <= ntable; i++ )  
		if ( lset[i] > 0 )  
			lset[i] = ++nobj;  

	// run through the look-up table again  
	for( int r = 0; r < nr; r++ )  
		for( int c = 0; c < nc; c++ )  
			ELEM(label_data, r, c, nc) = lset[ ELEM(label_data, r, c, nc) ];  
	//  
	delete[] lset;  

	label_num=nobj;
}
}