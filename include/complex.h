/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __COMPLEX_H
#define __COMPLEX_H

#include <cmath>
#include <cstdio>

double epsilon= 0.0000001;

struct complex { double r; double i;};

complex czero = {0,0};

complex cmul(complex a,complex b)
{ 
	complex r;
	r.r=a.r*b.r-a.i*b.i;
	r.i=a.i*b.r+a.r*b.i;
	return r;
}

complex cadd(complex a,complex b)
{
	complex r;
	r.r=a.r+b.r;
	r.i=a.i+b.i;
	return r;
}

complex csub(complex a,complex b)
{
	complex r;
	r.r=a.r-b.r;
	r.i=a.i-b.i;
	return r;
}

complex r2c(double x)
{
	complex r;
	r.r=x;
	r.i=0;
	return r;
}

bool isEqual(complex a,complex b)
{
	return ((a.r==b.r)&&(a.i==b.i));
}

bool isEqualE(complex a,complex b)
{
	return ((abs(a.r-b.r)<epsilon)&&(abs(a.i-b.i)<epsilon));
}

complex cpow(complex a,int b)
{
	complex r;
    r=a;
	for (int i=1;i<b;i++) r=cmul(r,a);
	return r;
}

complex cdiv(complex a,complex b)
{
	complex r;
	double B=b.r*b.r+b.i*b.i;
	r.r=(a.r*b.r+a.i*b.i)/B;
	r.i=(a.i*b.r-a.r*b.i)/B;
	return r;
}

complex cexp(complex a){
	complex r;
	double length,angle;
	length=exp(a.r);
	angle=a.i;
	r.r=cos(angle)*length;
	r.i=sin(angle)*length;
	return r;
}

complex cln(complex a){
	complex r;
	double length,angle;
	length=sqrt(a.r*a.r+a.i*a.i);
	angle=atan2(a.i,a.r);
	r.r=log(length);
	r.i=angle;
	return r;
}

complex cneg(complex a){
	complex r;
	r.r=-a.r;
	r.i=-a.i;
	return r;
}

complex cpos(complex a){
	return a;
}

complex csqrt(complex a)
{
	complex r;
	double length,angle;
	length=sqrt(sqrt(a.r*a.r+a.i*a.i));
	angle=atan2(a.i,a.r)/2;
	r.r=cos(angle)*length;
	r.i=sin(angle)*length;
	return r;
}

void cprint(complex c)
{
	printf("%f",c.r);
	if (c.i>0) printf("+%fi",c.i);
	if (c.i<0) printf("%fi",c.i);
	printf("\n");
}

bool isReal(complex x)
{
		return (abs(x.i)<=epsilon);
}
#endif
