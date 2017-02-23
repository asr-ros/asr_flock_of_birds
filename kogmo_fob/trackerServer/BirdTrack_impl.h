/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#ifndef BIRDTRACK_H
#define BIRDTRACK_H

/* system includes */
#include <pthread.h>

/* my includes */
#include "serialClass/serial.h"

using namespace std;
//using namespace Observer;

class BirdTrack_impl
{
public:
  BirdTrack_impl(std::string serial_device);
  ~BirdTrack_impl();
  
  int get_posangles(int whichtracker, double *v);
  int get_rawposangles(int whichtracker, int *raw);
  void initialize_Tracker();
  int start();
  int stop();
  void write_to_file(double *v, char *argv);
  int polhemus_coord_to_world_coord(double *vec);
  
private:
  SerialDevice *serial_object;
  int fd;
  double doubledata[12]; // [0..5]: Pos.werte für ersten Sensor; [6..12]: Pos.werte für zweiten Sensor
  unsigned char _erc, _sensor1, _sensor2;
  int refcount;
  //void writeNotification(int whichtracker, double *v);//double *data);

  //! protect all methods that are called by the corba interface classes.
  pthread_mutex_t fobMutex;

  
};

#endif /* BIRDTRACK_H */
