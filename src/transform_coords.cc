/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/* system includes */
/* (none) */

/* my includes */
#include "transform_coords.h"
#include "bird_track_impl.h"
#include "math.h"
#include "complex.h"
#include "polynomial.h"
#include "quaternion.h"

// Emitter unter dem Tisch auf nem Wasserkasten - Wird nur verwendet falls nix anderes zur Verfgung steht
#define POLHEMUS_POS_X 1284.6
#define POLHEMUS_POS_Y 1319.2
#define POLHEMUS_POS_Z -553.0
#define KOORD_OFFSET_X -16.0
#define KOORD_OFFSET_Y 65.0
#define KOORD_OFFSET_Z 0.0

bool calibrated;

double fobCoordList[256][6];
double worldCoordList[256][6];
int countCoord;

void
transformCoords::resetCalibration()
{
  calibrated=false;
  countCoord=0;
}

transformCoords::transformCoords()
{
  calibrated=false;
  countCoord=0;
}

transformCoords::~transformCoords()
{
}

/*
bool
transformCoords::loadCalibFile(const char *srcFileName)
{ //Load and Save is not implemented yet, since the Calibration GUI is able to load and save the
  //Coord lists anyway
  return true;
}

bool
transformCoords::saveCalibFile(const char *srcFileName)
{ //Same here
  return true;
}
*/
double S[3][3];
double N[4][4];
double Cm[3];
double Cs[3];
double c2,c1,c0;
double l;
int ERR=0;

quaternion solution;//Rotation
double scale;//Scale
double trans[3];//
quaternion transq;//Translation

void calcC(){
	for (int x=0;x<3;x++)
		Cs[x]=Cm[x]=0;
	for (int i=0;i<countCoord;i++)
	{
		for (int x=0;x<3;x++)
		{
			Cs[x]+=worldCoordList[i][x];
			Cm[x]+=fobCoordList[i][x];
		}
	}
	for (int x=0;x<3;x++)
	{
		Cs[x]=Cs[x]/countCoord;
		Cm[x]=Cm[x]/countCoord;
	}

}

double sqr(double a){
	return a*a;
}

void calcS(){
  double distf=0,distw=0;

  for (int x=0;x<3;x++)
		for (int y=0;y<3;y++)
			S[x][y]=0;
	
	for (int i=0;i<countCoord;i++)
	  {distw+=sqrt(sqr(worldCoordList[i][0]-Cs[0])+sqr(worldCoordList[i][1]-Cs[1])+sqr(worldCoordList[i][2]-Cs[2]));
	   distf+=sqrt(sqr(fobCoordList[i][0]-Cs[0])+sqr(fobCoordList[i][1]-Cs[1])+sqr(fobCoordList[i][2]-Cs[2]));
	  }
	distf=distf*distw/countCoord/countCoord/10; //??? Was ist dieser distf-Faktor?
	
	for (int i=0;i<countCoord;i++)
		for (int x=0;x<3;x++)
			for (int y=0;y<3;y++)
				S[x][y]+=(worldCoordList[i][x]-Cs[x])*(fobCoordList[i][y]-Cm[y])/distf;
}

void calcN(){
	N[0][0]=S[0][0]+S[1][1]+S[2][2]; N[0][1]=S[1][2]-S[2][1];         N[0][2]=S[2][0]-S[0][2];          N[0][3]=S[0][1]-S[1][0]; 
	N[1][0]=S[1][2]-S[2][1];         N[1][1]=S[0][0]-S[1][1]-S[2][2]; N[1][2]=S[0][1]+S[1][0];          N[1][3]=S[2][0]+S[0][2]; 
	N[2][0]=S[2][0]-S[0][2];         N[2][1]=S[0][1]+S[1][0];         N[2][2]=-S[0][0]+S[1][1]-S[2][2]; N[2][3]=S[1][2]+S[2][1]; 
	N[3][0]=S[0][1]-S[1][0];         N[3][1]=S[2][0]+S[0][2];         N[3][2]=S[1][2]+S[2][1];          N[3][3]=-S[0][0]-S[1][1]+S[2][2]; 
}

void calc_c(){
c2= (N[0][0]*N[2][2] - sqr(N[0][2])) + (N[1][1]*N[2][2] - sqr(N[1][2])) + 
		(N[0][0]*N[3][3] - sqr(N[0][3])) + (N[1][1]*N[3][3] - sqr(N[1][3])) +
	    (N[2][2]*N[3][3] + sqr(N[2][3])) + (N[0][0]*N[1][1] - sqr(N[0][1]));

c1 = -N[1][1]*(N[2][2]*N[3][3] - sqr(N[2][3])) + N[1][2]*(N[3][3]*N[1][2] - N[2][3]*N[1][3]) - N[1][3]*(N[1][2]*N[2][3] - N[2][2]*N[1][3])
	 -N[0][0]*(N[2][2]*N[3][3] - sqr(N[2][3])) + N[0][2]*(N[3][3]*N[0][2] - N[2][3]*N[0][3]) - N[0][3]*(N[2][3]*N[0][2] - N[2][2]*N[0][3])
	 -N[0][0]*(N[1][1]*N[3][3] - sqr(N[1][3])) + N[0][1]*(N[3][3]*N[0][1] - N[1][3]*N[0][3]) - N[0][3]*(N[0][1]*N[1][3] - N[1][1]*N[0][3])
	 -N[0][0]*(N[1][1]*N[2][2] - sqr(N[1][2])) + N[0][1]*(N[2][2]*N[0][1] - N[1][2]*N[0][2]) - N[0][2]*(N[0][1]*N[1][2] - N[1][1]*N[0][2]);

c0 =  (N[0][0]*N[1][1] - sqr(N[0][1]))*(N[2][2]*N[3][3] - sqr(N[2][3])) + (N[0][1]*N[0][2] - N[0][0]*N[1][2])*(N[1][2]*N[3][3] - N[2][3]*N[1][3])
	+ (N[0][0]*N[1][3] - N[0][1]*N[0][3])   *   (N[1][2]*N[2][3] - N[2][2]*N[1][3])   + (N[0][1]*N[1][2] - N[1][1]*N[0][2])*(N[0][2]*N[3][3] - N[2][3]*N[0][3])
	+ (N[1][1]*N[0][3] - N[0][1]*N[1][3])   *   (N[0][2]*N[2][3] - N[2][2]*N[0][3])   +   sqr(N[0][2]*N[1][3] - N[1][2]*N[0][3]);
//printf("x^4+%fx^2+%fx+%f\n",c2,c1,c0);
}

void find_max_lambda(){
	double a[4];
	double x1,x2,x3,x4;
	int c;
	int i;
	int b=0;
	c=solve_biquadratic(1,0,c2,c1,c0,&x1,&x2,&x3,&x4);
	a[0]=x1;a[1]=x2;a[2]=x3;a[3]=x4;
	printf("Solutions for Lamda (%d): ",c);
	for (i=0;i<c;i++) printf("%f ",a[i]);
	printf("\n");
	if (c!=0) l=a[0]; else l=-HUGE_VAL;
	if (c>1)
		for (i=1;i<c;i++)
			if (a[i]>l) l=a[i];
		for (i=1;i<c;i++)
			if (abs(a[i]-l)<epsilon) b++;
	if (b>1) ERR=1;	//Zwei gleiche maximale Eigenwerte -> Der dazugeh�ige Eigenraum ist nicht eindimensional
    printf ("Error after find_max_lambda: %d\n", ERR);
}

void flipLines(int a,int b){//Flips two lines within the matrix
	for (int i=0;i<4;i++)
	{
		double c=N[a][i];
		N[a][i]=N[b][i];
		N[b][i]=c;
	}
}

void mAddLines(int to,int from,double value){//Adds line "from" to line "to muliplied with "value"
		for (int i=0;i<4;i++)
			N[to][i]+=N[from][i]*value;
}

void setQuat(int pos,quaternion* q,double value)//If you ever need to access a quaternion like an array
{
	switch (pos)
	{
	case 0:q->r=value;break;
	case 1:q->i=value;break;
	case 2:q->j=value;break;
	case 3:q->k=value;break;
	}
}

double NB[4][4];

void calc_eigenVec(){
//Calculation of N-Lambda*I
	solution.r=0;
	solution.i=0;
	solution.j=0;
	solution.k=0;
	for (int i=0;i<4;i++)
		N[i][i]-=l;
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
			NB[i][j]=N[i][j];
	{
	int j=-1;
	for (int i=0;i<4;i++)
	{int l=0;
		for (int k=0;k<4;k++)
		{
			if (abs(N[k][i])>epsilon) l++;
		}
		if (l==1) j=i;
	}
	if (j>=0) {setQuat(j,&solution,1);return;}	//One row is zero, so there is a solution with one value not equal zero
	}
	for (int i1=0;i1<4;i1++)	//there is a solution with two value not equal zero
		for (int j=0;j<3;j++)
			if ((i1!=j)&&(abs(N[0][i1])>epsilon))
			{	int s=0;
				for (int k=1;k<4;k++)
					if ((abs(N[k][i1])<epsilon)&&(abs(N[k][j])<epsilon)) s++;
					else if (abs(N[k][i1])>epsilon)
						if (abs(N[k][j]/N[k][i1]-N[0][j]/N[0][i1])<epsilon) s++;
				if (s==3)
				{
					setQuat(j,&solution,N[0][i1]);
					setQuat(i1,&solution,-N[0][j]);
					return;
				}
			}
	//try to find a solution with (a,b,1,0) and a,b!=0
	{	
		int m=-1;
		int n,p;
		for (int i2=1;i2<4;i2++)
			if (abs(N[0][0]*N[i2][1]-N[i2][0]*N[0][1])>=epsilon)	//Sind Zeile 1 und m linear abh�gig?
			{m=i2;break;}
		double a=-(N[m][2]*N[0][0]-N[0][2]*N[m][0])/(N[0][0]*N[m][1]-N[m][0]*N[0][1]);
		double b= (N[m][2]*N[0][1]-N[0][2]*N[m][1])/(N[0][0]*N[m][1]-N[m][0]*N[0][1]);
		if (m>0)			//Es gibt eine zu m linear unabh�gige Zeile
		{
			switch (m) //Bestimmung der beiden bisher nichtbenutzten Zeilen
			{
				case 1:n=2;p=3;break;
				case 2:n=1;p=3;break;
				case 3:n=1;p=2;break;
			}
			if ((abs(a*N[n][0]+b*N[n][1]+N[n][2])<epsilon)&&(abs(a*N[p][0]+b*N[p][1]+N[p][2])<epsilon))
			{
					setQuat(0,&solution,a);
					setQuat(1,&solution,b);
					setQuat(2,&solution,1);
					return;
			}
		}
		
	}
//Gibt es wenigstens eine Lösung mit (a,b,c,1)
	solution.k=1;
	for (int i=0;i<4;i++)	//
		N[i][3]*=-1;	
	if (abs(N[0][0])<epsilon) flipLines(0,1);
	if (abs(N[0][0])<epsilon) flipLines(0,2);
	if (abs(N[0][0])<epsilon) flipLines(0,3);
	if (abs(N[0][0])<epsilon) {ERR=2;return;}//Darf eigentlich nicht passieren
	
	for (int i=1;i<4;i++)
		mAddLines(i,0,-N[i][0]/N[0][0]);//Erste Spalte auf Null setzen	
	if (abs(N[1][1])<epsilon) flipLines(1,2);
	if (abs(N[1][1])<epsilon) flipLines(1,3);
	if (abs(N[1][1])<epsilon) {ERR=2;return;}//Darf eigentlich auch nicht passieren
	
	for (int i=2;i<4;i++)
		mAddLines(i,1,-N[i][1]/N[1][1]);//Zweite Spalte auf Null setzen
		if (abs(N[2][2])<epsilon) flipLines(2,3);
	if (abs(N[2][2])<epsilon) {ERR=2;return;}//Darf auch nicht passieren
	if ((abs(N[2][2])<epsilon)&&(abs(N[2][3])>=epsilon)) {ERR=3;return;}	//Es gibt keine Lösung
	if ((abs(N[3][2])<epsilon)&&(abs(N[3][3])>=epsilon)) {ERR=3;return;}	//Es gibt keine Lösung
	for (int i=0;i<2;i++)
		if (abs(N[i][i])<epsilon) {ERR=4;return;}	//Der Lösungsraum ist nicht eindimensional
		
	if ((abs(N[3][3])<epsilon)&&(abs(N[4][4])<epsilon)) {ERR=4;return;}	//Der Lösungsraum ist nicht eindimensional
	
	solution.k=1;
	solution.j=(0.5*N[2][3]/N[2][2]+0.5*N[3][3]/N[3][2]);
	solution.i=((N[1][3]-solution.j*N[1][2])/N[1][1]);
	solution.r=((N[0][3]-solution.j*N[0][2]-solution.i*N[0][1])/N[0][0]);
}

void calc_scale()
{
double a=0;
double b=0;
for (int i=0;i<countCoord;i++)
{
	a+=sqr(worldCoordList[i][0]-Cs[0])+sqr(worldCoordList[i][1]-Cs[1])+sqr(worldCoordList[i][2]-Cs[2]);
	b+=sqr(fobCoordList[i][0]-Cm[0])+sqr(fobCoordList[i][1]-Cm[1])+sqr(fobCoordList[i][2]-Cm[2]);
}
scale=sqrt(a/b);
 printf("Scale is %f\n",scale);
}

void calc_trans()
{quaternion tr,cm,cs;
cm.r=0;
cm.i=Cm[0];
cm.j=Cm[1];
cm.k=Cm[2];
cs.r=0;
cs.i=Cs[0];
cs.j=Cs[1];
cs.k=Cs[2];
tr=qsub(cs,qscale(scale,qmul(qmul(solution,cm),qinv(solution))));
trans[0]=tr.i;
trans[1]=tr.j;
trans[2]=tr.k;
transq=tr;
 printf("Transformation Vector is:%f %f %f\n",trans[0],trans[1],trans[2]);
}

void transform_point(double xi, double yi, double zi,double* xo, double* yo, double* zo)
{
quaternion q;
q.r=0;
q.i=xi;
q.j=yi;
q.k=zi;
q=qadd(transq,qscale(scale,qmul(qmul(solution,q),qinv(solution))));
*xo=q.i;
*yo=q.j;
*zo=q.k;
}

bool
transformCoords::addCalibrationData(double *fobCoords,double *worldCoords)
{
  ERR=0;
  //try to compute transformation from all available data and return true if calibration is possible with available data
  if (countCoord<256)
  	{
  	for (int i=0;i<6;i++)
  		{
		worldCoordList[countCoord][i]=worldCoords[i];
		fobCoordList[countCoord][i]=fobCoords[i];
		}
	countCoord++;
	}
  if (countCoord>=3)
  	{
		calcC();
	    calc_scale();
		calcS();
		calcN();
		calc_c();
		find_max_lambda();
		if (ERR==0)
		{
		    calc_eigenVec();
			solution.i=-solution.i;
			solution.j=-solution.j;
			solution.k=-solution.k;
			printf("Rotation Quaternion:");
			qprint(solution);
		}
		if (ERR==0)
			calc_trans();
		calibrated=(ERR==0);
   	}
  //Calibrate from available data and set calibrated=false if there is some unsolveable trouble
  cout << "Calibrated: " << calibrated << "(error: " << ERR << ")\n";
  return calibrated;
}

void
transformCoords::transform(double *v)
{ double x,y,z;
  if (calibrated)
	{ 
		x=v[0];
		y=v[1];
		z=v[2];
		transform_point(x,y,z,&x,&y,&z);
		v[0]=x;
		v[1]=y;
		v[2]=z;
	}
  else
	{
	  printf("Not calibrated call of transform\n");
		//Actually nothing to do, except you want to implement some generic calibration
	}
}



bool
transformCoords::isCalibrated()
{
  return calibrated;
}

#if transformCoords_test
#include <stdio.h>
int main(int argc, char **argv)
{
  // This is a module-test block. You can put code here that tests
  // just the contents of this C file, and build it by saying
  //             make transformCoords_test
  // Then, run the resulting executable (transformCoords_test).
  // If it works as expected, the module is probably correct. ;-)

  fprintf(stderr, "Testing transformCoords\n");

  return 0;
}
#endif /* transformCoords_test */
