/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __POLYNOMIAL_H
#define __POLYNOMIAL_H
#include "math.h"
#include "complex.h"
double cbrt(double x){//Kubikwurzel
	if (x<0) return -pow(-x,1.0/3.0);
	return pow(x,1.0/3.0);
}

//	double epsilon=0.0000001;

//x1/2=(-b+-sqrt(b^2-4ac))/(2a)
int solve_quadratic(double a,double b,double c,double* x1,double* x2)
{
	if (a==0)//Lineares Gleichungssystem
	{
		if (b==0) return 0;//c ist konstant
		*x1=-c/b;
		return 1;
	}
	double d=b*b-4*a*c;
	if (d<0) return 0;
	*x1=(-b+sqrt(d))/2*a;
	*x2=(-b-sqrt(d))/2*a;
	return 2;


}

//Solve cubic nach http://www.montgelas-gymnasium.de/mathe/kubfa/leitkubgleich.html
int solve_cubic(double a,double b,double c,double d,double* x1,double* x2,double* x3)
{	if (a==0) return solve_quadratic(b,c,d,x1,x2);
	double pi=3.141592653589793;
	double p=(3*a*c-b*b)/(3*a*a);
	double q=(2*b*b*b)/(27*a*a*a)-(b*c)/(3*a*a)+d/a;
	double D=(q*q/4)+(p*p*p/27);
	if (D>epsilon) 
	{ //Nur eine reelle L�ung
		*x1=cbrt(-q/2+sqrt(D))+cbrt(-q/2-sqrt(D))-b/(3*a);
		*x2=HUGE_VAL;
		*x3=HUGE_VAL;
		return 1;
	}
	else if (D<-epsilon)
	{
		double alpha=acos(-q/(2*sqrt(-p*p*p/27)));
		*x1=2*sqrt(-p/3)*cos(alpha/3+0.0/3.0*pi)-b/(3*a);
		*x2=2*sqrt(-p/3)*cos(alpha/3+2.0/3.0*pi)-b/(3*a);
		*x3=2*sqrt(-p/3)*cos(alpha/3+4.0/3.0*pi)-b/(3*a);
		return 3;
	}
	else //D is nearby zero
	{
		if (abs(p)<epsilon)
		{
			*x1=*x2=*x3=-b/(3*a);
			return 3;
		} else
		{	*x1=cbrt(-4*q)-b/(3*a);
			*x2=*x3=cbrt(q/2)-b/(3*a);
			return 3;
		}
	}
}

double sqrt2(double x)
{
	return sqrt(abs(x));
}


void setpos(int p,complex a,double* x1,double* x2,double* x3,double* x4)
{
switch (p)
{
case 1:*x1=a.r;break;
case 2:*x2=a.r;break;
case 3:*x3=a.r;break;
case 4:*x4=a.r;
}
}

//Nach http://de.wikipedia.org/wiki/Biquadratische_Gleichung
int solve_biquadratic(double a4,double a3,double a2,double a1,double a0,double* x1,double* x2,double* x3,double* x4)
{
	if (a4==0) return solve_cubic(a3,a2,a1,a0,x1,x2,x3);
	if ((a3==0) && (a1==0))
	{
		if (solve_quadratic(a4,a2,a0,x1,x3)==0) return 0;
		int s=0;
		if (*x1>=0)
		{
			*x1=sqrt(*x1);
			*x2=-*x1;
			s+=2;
		}
		if (*x3>=0)
		{
			if (s==0)
			{
			*x1=sqrt(*x3);
			*x2=-*x1;
			s+=2;
			} else
			{
			*x3=sqrt(*x3);
			*x4=-*x3;
			s+=2;
			}
		}
		return s;
	}
	complex A=r2c(a4);
	complex B=r2c(a3);
	complex C=r2c(a2);
	complex D=r2c(a1);
	complex E=r2c(a0);
	complex a = cadd(cdiv (cmul(cmul(r2c(-3),B),B) , cmul(cmul(r2c(8),A),A)) , cdiv(C,A));

	complex b = cadd(csub(cdiv(cpow(B,3),cmul(r2c(8),cpow(A,3))),
		                  cdiv(cmul(B,C),cmul(cmul(r2c(2),A),A))),
		                  cdiv(D,A));

	complex c = cadd(csub(cadd(cdiv(cmul(r2c(-3),cpow(B,4)),cmul(r2c(256),cpow(A,4))),
		        cdiv(cmul(C,cmul(B,B)),cmul(r2c(16),cpow(A,3)))),
				cdiv(cmul(B,D),cmul(cmul(r2c(4),A),A))),
				cdiv(E,A));

	if (isEqualE(b,czero))
	{   if (solve_quadratic(1,a.r,c.r,x1,x3)==0) return 0;
		int s=0;
		if (*x1>=0)
		{
			*x1=sqrt(*x1);
			*x2=-*x1-a3/(4*a4);
			*x1-=a3/(4*a4);
			s+=2;
		}
		if (*x3>=0)
		{
			if (s==0)
			{
			*x1=sqrt(*x3);
			*x2=-*x1-a3/(4*a4);
			*x1-=a3/(4*a4);
			s+=2;
			} else
			{
			*x3=sqrt(*x3);
			*x4=-*x3-a3/(4*a4);
			*x3-=a3/(4*a4);
			s+=2;
			}
		}
		return s;
	}
	complex P = csub(cdiv(cmul(a,a),r2c(-12)),c);
	complex Q = csub(cadd(cdiv(cpow(a,3),r2c(-108)),cdiv(cmul(a,c),r2c(3))),cdiv(cmul(b,b),r2c(8)));
	complex R = cadd(cdiv(Q,r2c(2)),csqrt( cadd(cdiv(cmul(Q,Q),r2c(4)),cdiv(cpow(P,3),r2c(27))) ));
	complex U = czero;
	if (isEqualE(R,czero))
	{
		U=czero;
	} else
	{
		 U = cexp(cdiv(cln(R),r2c(3)));
	}
	complex y=csub(cmul(r2c(-5.0/6.0),a),U);
	if (!isEqualE(U,czero))
	{
		y=cadd(y,cdiv(P,cmul(r2c(3),U)));
	}
	//          -B/(4*A) + (sqrt(a+2*y) + sqrt(-(a+2*y) - 2*(a + b/sqrt(a+2*y) )))/2;
	//            L1            L2       L5/L7   L3      |        L4/L6
	complex a2y = cadd(a,cmul(r2c(2),y));
	complex L1 = cneg(cdiv(B,cmul(r2c(4),A)));
	complex L2 = csqrt(a2y);
	complex L3 = cneg(a2y);
	complex L4 = cmul(r2c(2),cadd(a,cdiv(b,L2)));
	complex L5 = csqrt(csub(L3,L4));
	complex L6 = cmul(r2c(2),csub(a,cdiv(b,L2)));
	complex L7 = csqrt(csub(L3,L6));
	complex X1 = cadd(L1,cdiv(cadd(     L2 ,     L5),r2c(2)));
	complex X2 = cadd(L1,cdiv(cadd(cneg(L2),     L7),r2c(2)));
	complex X3 = cadd(L1,cdiv(cadd(     L2 ,cneg(L5)),r2c(2)));
	complex X4 = cadd(L1,cdiv(cadd(cneg(L2),cneg(L7)),r2c(2)));
	int s=0;
	if (isReal(X1)) {s++;setpos(s,X1,x1,x2,x3,x4);}
	if (isReal(X2)) {s++;setpos(s,X2,x1,x2,x3,x4);}
	if (isReal(X3)) {s++;setpos(s,X3,x1,x2,x3,x4);}
	if (isReal(X4)) {s++;setpos(s,X4,x1,x2,x3,x4);}
	return s;
}
#endif
