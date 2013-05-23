/*
    Copyright (c) 2012 <benemorius@gmail.com>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
*/



#ifndef E36KOMBI_H
#define E36KOMBI_H
#include <DS2.h>

const uint8_t coolant_temp_table[256] = {200, 199, 196, 192, 189, 185, 182, 179, 175, 172, 169, 165, 162, 158, 155, 152, 148, 145, 142, 138, 135, 131, 
130, 128, 126, 125, 123, 121, 120, 118, 116, 115, 114, 113, 112, 111, 110, 108, 107, 106, 104, 103, 102, 101, 101, 99, 99, 98, 97, 97, 96, 95, 94, 93, 93, 
92, 91, 90, 89, 88, 88, 87, 86, 85, 85, 84, 84, 83, 83, 82, 81, 81, 80, 80, 79, 79, 78, 77, 76, 76, 75, 75, 74, 74, 73, 72, 72, 71, 71, 71, 70, 70, 69, 69, 69, 68, 
67, 67, 67, 66, 66, 65, 65, 65, 64, 63, 63, 62, 62, 62, 61, 61, 60, 60, 60, 59, 58, 58, 57, 57, 57, 56, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 48, 
47, 47, 46, 46, 45, 44, 44, 43, 43, 42, 42, 41, 40, 40, 40, 39, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 30, 29, 29, 28, 28, 27, 26, 26, 
25, 25, 24, 24, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 11, 11, 11, 11, 
11, 10, 10, 10, 10, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0};

class E36Kombi
{
public:
	E36Kombi(DS2& diagnosticInterface);

	bool query();
	float getCoolantTemperature(); //returns coolant temperature in degrees celsius
	
private:
	DS2& diag;
	int address;
	DS2PacketType packetType;
};

#endif // E36KOMBI_H
