/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __QUATERNION_H
#define __QUATERNION_H

#include "complex.h"
#include "quaternion.h"
#include "math.h"

struct quaternion 
{
	double r,i,j,k;
};		

quaternion qzero= {0,0,0,0};

quaternion qneg(quaternion a){
	quaternion r;
	r.r=-a.r;
	r.i=-a.i;
	r.j=-a.j;
	r.k=-a.k;
	return r;
}

quaternion qmul(quaternion a,quaternion b)
{ 
	quaternion r;
	r.r=a.r*b.r-a.i*b.i-a.j*b.j-a.k*b.k;
	r.i=a.r*b.i+a.i*b.r+a.j*b.k-a.k*b.j;
	r.j=a.r*b.j-a.i*b.k+a.j*b.r+a.k*b.i;
	r.k=a.r*b.k+a.i*b.j-a.j*b.i+a.k*b.r;
	return r;
}

quaternion qadd(quaternion a,quaternion b)
{
	quaternion r;
	r.r=a.r+b.r;
	r.i=a.i+b.i;
	r.j=a.j+b.j;
	r.k=a.k+b.k;
	return r;
}

quaternion qsub(quaternion a,quaternion b)
{
	quaternion r;
	r.r=a.r-b.r;
	r.i=a.i-b.i;
	r.j=a.j-b.j;
	r.k=a.k-b.k;
	return r;
}

quaternion r2q(double x)
{
	quaternion r;
	r.r=x;
	r.i=0;
	r.j=0;
	r.k=0;
	return r;
}

bool isEqual(quaternion a,quaternion b)
{
	return ((a.r==b.r)&&(a.i==b.i)&&(a.j==b.j)&&(a.k=b.k));
}

bool isEqualE(quaternion a,quaternion b)
{
	return ((abs(a.r-b.r)<epsilon)&&(abs(a.i-b.i)<epsilon)&&(abs(a.j-b.j)<epsilon)&&(abs(a.k-b.k)<epsilon));
}

quaternion cpow(quaternion a,int b)
{
	quaternion r;
    r=a;
	for (int i=1;i<b;i++) r=qmul(r,a);
	return r;
}

double abs(quaternion a)
{
	return sqrt(a.r*a.r+a.i*a.i+a.j*a.j+a.k*a.k);
}

quaternion qabs(quaternion a)
{
	quaternion r=qzero;
	r.r=abs(a);
	return r;
}

quaternion qscale(double s,quaternion a)
{
	quaternion r;
	r.r=a.r*s;
	r.i=a.i*s;
	r.j=a.j*s;
	r.k=a.k*s;
	return r;
}

quaternion qNormalize(quaternion q)
{
	return qscale(1/abs(q),q);
}

quaternion qconj(quaternion a)
{
	quaternion r=qneg(a);
	r.r=a.r;
	return r;
}


quaternion qinv(quaternion a)
{
	return qscale(1/abs(a),qconj(qscale(1/abs(a),a)));
	
}
/*
quaternion cdiv(quaternion a,complex b)
{
	quaternion r;
	double B=b.r*b.r+b.i*b.i;
	r.r=(a.r*b.r+a.i*b.i)/B;
	r.i=(a.i*b.r-a.r*b.i)/B;
	return r;
}

quaternion cexp(quaternion a){
	quaternion r;
	double length,angle;
	length=exp(a.r);
	angle=a.i;
	r.r=cos(angle)*length;
	r.i=sin(angle)*length;
	return r;
}

quaternion cln(quaternion a){
	quaternion r;
	double length,angle;
	length=sqrt(a.r*a.r+a.i*a.i);
	angle=atan2(a.i,a.r);
	r.r=log(length);
	r.i=angle;
	return r;
}
*/

quaternion qpos(quaternion a){
	return a;
}
/*
quaternion csqrt(quaternion a)
{
	quaternion r;
	double length,angle;
	length=sqrt(sqrt(a.r*a.r+a.i*a.i));
	angle=atan2(a.i,a.r)/2;
	r.r=cos(angle)*length;
	r.i=sin(angle)*length;
	return r;
}
*/

void qprint(quaternion c)
{
	printf("%f",c.r);
	if (c.i>0) printf("+%fi",c.i);
	if (c.i<0)  printf("%fi",c.i);
	if (c.j>0) printf("+%fi",c.j);
	if (c.j<0)  printf("%fi",c.j);
	if (c.k>0) printf("+%fi",c.k);
	if (c.k<0)  printf("%fi",c.k);
	printf("\n");
}

bool isReal(quaternion x)
{
		return ((abs(x.i)<=epsilon)&&(abs(x.j)<=epsilon)&&(abs(x.k)<=epsilon));
}

void rotate(double* x,double* y,double* z,quaternion q){
	quaternion r;
	r.r=0;
	r.i=*x;
	r.j=*y;
	r.k=*z;
	r=qmul(qmul(q,r),qinv(q));
	*x=r.i;
	*y=r.j;
	*z=r.k;
}

#endif
