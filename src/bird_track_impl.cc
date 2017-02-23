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
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fstream>
#include <math.h>

/* my includes */
#include "bird_track_impl.h"

/* my defines */
#define ANGK (double)(180.0/32758.0)

// Achtung: nicht umdefinieren, ein enum interessiert sich dafür!!
#define RIGHT_TRACKER_ID 1
#define LEFT_TRACKER_ID  2

// Emitter unter dem Tisch auf nem Wasserkasten
#define POLHEMUS_POS_X 1284.6
#define POLHEMUS_POS_Y 1319.2
#define POLHEMUS_POS_Z -553.0
#define KOORD_OFFSET_X -16.0
#define KOORD_OFFSET_Y 65.0
#define KOORD_OFFSET_Z 0.0


//#define _DEBUG_

struct data {
    unsigned char lSigB;
    unsigned char mSigB;
};
	 

union myUnion {
    data point;
    short shortPoint;
};

BirdTrack_impl::BirdTrack_impl(std::string serial_port)
{
  printf( "try to open serialdev\n");
  serial_object = new SerialDevice();
  
  fd = serial_object->open_serial(serial_port.c_str());

  pthread_mutex_init(&fobMutex,NULL);

  initialize_Tracker();
}

BirdTrack_impl::~BirdTrack_impl()
{
    while (refcount != 0)
	stop();
    serial_object->empty_serial(fd);
    serial_object->close_serial(fd);	
    delete serial_object;
    pthread_mutex_destroy(&fobMutex);
}

void
BirdTrack_impl::initialize_Tracker(void){
    _erc = (unsigned char)(0xF0 | 1);
    _sensor1 = (unsigned char)(0xF0 | 2);
    _sensor2 = (unsigned char)(0xF0 | 3);

    // see pdf
    static unsigned char birdchangevaluecmd[] = {'P',50,3}; // auto configuration, 1 master at address 1 and 2 slaves at address 2 and 3
    //static unsigned char hemispherecmd[] = {'L',0x0C,0x01}; // tracker have to be above the sensor
    static unsigned char hemispherecmd[] = {'L',0x00,0x00}; // tracker have to be in front of the sensor
    //static unsigned char hemispherecmd[] = {'L',0x00,0x00}; // tracker have to be in front of the sensor
    static unsigned char align2cmd[] = {'q',0xFF,0x3F, 0x00,0x00, 0x00,0x80};
    static unsigned char reframecmd[] = {'P',0x18, 0x00,0x00, 0x00,0x80, 0x00,0x00};
    static unsigned char birdsleep[] = {'G'};
   

    cout << "Autoconfig command wird an den ERC gesendet." << endl;
    cout << "Tracker wird (werden) kurz gestartet..." << endl;
    serial_object->write_serial(fd,&_erc, 1);
    serial_object->write_serial(fd,birdchangevaluecmd,3);
    sleep(1);

    cout << "Hemisphere command wird an die Snsrs gesendet." << endl;
    serial_object->write_serial(fd,&_sensor1, 1);
    serial_object->write_serial(fd,hemispherecmd, 3);
    serial_object->write_serial(fd,&_sensor2, 1);
    serial_object->write_serial(fd,hemispherecmd, 3);
    sleep(1);
    
    cout << "ref frame command wird an die Snsrs gesendet." << endl;
    serial_object->write_serial(fd,&_sensor1, 1);
    serial_object->write_serial(fd,align2cmd, 7);
    serial_object->write_serial(fd,&_sensor2, 1);
    serial_object->write_serial(fd,align2cmd, 7);
    sleep(1);
    

    cout << "ref frame command wird an den ERC gesendet." << endl;
    serial_object->write_serial(fd,reframecmd, 8);
    sleep(1);
    

    cout << "... und wieder schlafen gelegt, damit wir später nur noch sleep und wakeup machen müssen." << endl;
    serial_object->empty_serial(fd);
    serial_object->write_serial(fd,birdsleep,1);
	
    refcount = 0;
}

int
BirdTrack_impl::start(void) 
{
  pthread_mutex_lock(&fobMutex);
  if (refcount == 0)
    {
      static unsigned char birdwakeup[] = {'F'};
      serial_object->empty_serial(fd);
      usleep(10000);
      serial_object->write_serial(fd,birdwakeup,1);
      printf("Schicke wakeup.\n");
    }
  refcount++;
  
  pthread_mutex_unlock(&fobMutex);

  return refcount;
}

int
BirdTrack_impl::stop(void)
{
    pthread_mutex_lock(&fobMutex);
    cout << "Stopping Tracker" << endl;	

    if (refcount == 0){
	pthread_mutex_unlock(&fobMutex);
	return 0;
    }
    if ((refcount-1) == 0){
	static unsigned char birdsleep[] = {'G'};
	serial_object->empty_serial(fd);
	serial_object->write_serial(fd,birdsleep,1);
	printf("Schicke sleep.\n");
    }
    refcount--;
  
    pthread_mutex_unlock(&fobMutex);
    return refcount;
}

void
BirdTrack_impl::write_to_file(double *v, char *argv)
{
    double x, y, z;
    ofstream inf(argv, ios::ate | ios::app);
    cout << "rele koordinaten [x y z]: ";
    cin >> x;
    cin >> y;
    cin >> z;
    if (inf.is_open())
    {

	inf << x << '\t'
	    << y << '\t'
	    << z << '\t'
	    << v[0] << "\t"
	    << v[1] << "\t"
	    << v[2] << "\t"
	    << endl;
	//	  << v[3] << " "
	//	  << v[4] << " "
	//	  << v[5] << endl;
 
	inf.close();
    }
}

int
BirdTrack_impl::get_rawposangles(int whichtracker, int *raw){

  //DEBUG
  //cout << "Starting with get_rawposangles" <<endl;
  pthread_mutex_lock(&fobMutex);

    if (refcount == 0){
	cout << "get_rawposangles sagt refcount = " << refcount << endl;
	pthread_mutex_unlock(&fobMutex);
	return -1;
    }
	
    short i;
    short realdata[6];
    short shortdata[12];
    float floatdata[6];
    unsigned char rs232tofbbcmd, pos_angl_cmd[4];
    char birddata[12]; //Bird-Data-Buffer
	
    for (i=0; i<12; i++) {
	birddata[i] = 0;
	shortdata[i] = 0;
    }
    for (i=0; i<6; i++) {
	realdata[i] = 0;
	floatdata[i] = 0.0;
    }
	
	
    if (whichtracker == LEFT_TRACKER_ID)
	rs232tofbbcmd = (unsigned char)(0xF0 | LEFT_TRACKER_ID + 1);
		
    else if (whichtracker == RIGHT_TRACKER_ID)
	rs232tofbbcmd = (unsigned char)(0xF0 | RIGHT_TRACKER_ID + 1);
	
    else{
	cout << "Falsche id abgefragt: " << whichtracker << endl;
	pthread_mutex_unlock(&fobMutex);
	return (-1);
    }
		
    pos_angl_cmd[0] = rs232tofbbcmd;
    pos_angl_cmd[1] = 'Y';
    pos_angl_cmd[2] = rs232tofbbcmd;
    pos_angl_cmd[3] = 0x42;

    /* Es wird solange abgefragt, bis das erste Bit des ersten Bytes eine 1 hat.
       Erst dann sind Daten vom Flock gesendet worden!
    */
    while(1)
    {
	serial_object->write_serial(fd,pos_angl_cmd,4);
	serial_object->empty_serial(fd);
	serial_object->read_serial(fd, (unsigned char*)birddata, 12);
	if (birddata[0] & 0x80) break;  
	cout << "waiting..." <<endl;  
    }
    birddata[0] &= 0x7F;  // setze die erste 1 auf 0; jetzt steht vor jedem Byte als erstes eine Null
    myUnion myPoint;

    for (i=0; i<12; i=i+2)
	birddata[i] <<= 1;  // verschiebe alle LS Byte um eins nach links!

    for (i=0; i<=10; i=i+2) {
	myPoint.point.mSigB = birddata[i+1];
	myPoint.point.lSigB = birddata[i];
	shortdata[i/2] = myPoint.shortPoint;  // mache LS Byte und MS Byte
    }
	
    for (i=0; i<6; i++){
        //das ganze nochmal um eins nach links, da sonst an erster Stelle eine Null stehen würde
	shortdata[i] <<= 1; 
	raw[i] = (int) shortdata[i];  // caste alles als int
    }
    
    pthread_mutex_unlock(&fobMutex);
    return 1;

}


int
BirdTrack_impl::get_posangles(int whichtracker, double *v){

    short i;
    int intdata[6];
    //DEBUG
    //cout <<  "get_posangles called" <<endl;

    get_rawposangles(whichtracker, intdata);

	
    for (i=0; i<3; i++){
	//siehe Dokumentation von FloB (Umrechnung ist individuell) // millimeter
      v[i] = ( (double)intdata[i]) * 0.11162109375;//( ( ( (double)intdata[i])*144.0) / 32768.0) * 2.54;  
    }

    for (i=3; i<6; i++)
    {
      // grad
      v[i] = ( (double)intdata[i]) * 0.0054931640625;//(double) ((intdata[i] * 180) / 32768);
    }

    
    pthread_mutex_unlock(&fobMutex);
    //DEBUG
    //cout << "get_posangles finished" <<endl;
    return 1;
}

int
BirdTrack_impl::polhemus_coord_to_world_coord(double *vec)
{
    double x, y, z, rx, ry, rz;
//  double InterTabelleX[1000];
    x = (-1.0)*vec[0] * 10;
    y = (+1.0)*vec[1] * 10;
    z = (-1.0)*vec[2] * 10;
    vec[0] = x; //+ KOORD_OFFSET_X;
    vec[1] = y; // + KOORD_OFFSET_Y;
    vec[2] = z; // + KOORD_OFFSET_Z;
   
    rx = vec[5];
    ry = vec[4];
    rz = vec[3];
    
    /*   if (transform_transformation(&rx, &ry, &rz) == 0)
	 {
	 printf("Transform Transformation abkack - not my problem. Bye, Steffen.\n");
	 exit(1);
      }
    */
    
    vec[3] = rx *  M_PI / 180.0;
    vec[4] = ry *  M_PI / 180.0;
    vec[5] = rz *  M_PI / 180.0;
    

    
    //Interpolation
    
    //	interTabelleX
    
    return(1);
}

  

/* end of not implemented yet :-) *g*  */



#if BirdTrack_impl_test

#include <bgcorba_impl.h>
#include "IPRFilter.hh"
#include "NotificationManager.hh"
#include "NotificationReceiver.hh"

int main(int argc, char **argv)
{
    bgcorba::init(argc, argv);
  
    // Create a new instance of the implementation 
    BirdTrack_impl *impl = new BirdTrack_impl;

    int retval =  bgcorba::main(impl);

    delete impl;

    return retval;
}
#endif /* BirdTrack_impl_test */

