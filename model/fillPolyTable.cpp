#include "fillPolyTable.h"

//Usage:
//float poly[30];
//fillPolyTable(&poly);

//Coef_j of mode_i is poly[5 * mode_i + coef_j]
//Order 4 polynomial approximation of w0, for xi going from 0.000000 to 20.000000.

void fillPolyTable(float* poly)
{
	poly[0] = -0.000002;
	poly[1] = 0.000108;
	poly[2] = -0.004266;
	poly[3] = 0.141605;
	poly[4] = 2.406022;
	poly[5] = -0.000000;
	poly[6] = 0.000010;
	poly[7] = 0.000265;
	poly[8] = 0.011996;
	poly[9] = 5.520011;
	poly[10] = 0.000000;
	poly[11] = 0.000000;
	poly[12] = 0.000038;
	poly[13] = 0.003085;
	poly[14] = 8.653728;
	poly[15] = 0.000000;
	poly[16] = 0.000000;
	poly[17] = 0.000008;
	poly[18] = 0.001220;
	poly[19] = 11.791534;
	poly[20] = 0.000000;
	poly[21] = 0.000000;
	poly[22] = 0.000003;
	poly[23] = 0.000601;
	poly[24] = 14.930918;
	poly[25] = 0.000000;
	poly[26] = 0.000000;
	poly[27] = 0.000001;
	poly[28] = 0.000339;
	poly[29] = 18.071064;
}