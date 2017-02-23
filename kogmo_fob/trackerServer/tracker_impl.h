/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TRACKER_H
#define TRACKER_H

//MACROS
#define DIEonERR(value) if (value<0) { \
 fprintf(stderr,"%i DIED in %s line %i with error %i\n",getpid(),__FILE__,__LINE__,-value);exit(1); }

/* system includes */
#include <string>

/* my includes */
#include "transformCoords.h"
//KOGMO_RTDB standard includes
#define KOGMO_RTDB_DONT_INCLUDE_ALL_OBJECTS
#include "kogmo_rtdb.h"
#include "kogmo_rtdb_obj_typeids.h"
//KOGMO_RTDB special includes for our project with the id E1
#include "itec/kogmo_rtdb_obj_e1_itec.h"
using namespace KogniMobil;


typedef enum{rightTracker=1, leftTracker}trackerID;

//for reference purposes: the old data structure
/*
    // Definition of a calibration data set
  struct fulcrum{
    double sensorVal[3];
    double sensorAngle[3];
	double worldVal[3];
	double worldAngle[3];
  };
  //typedef sequence<fulcrum> oneSensorCal;
  typedef sequence<fulcrum> allSensorCal;
*/

class tracker_impl{
 public:
  tracker_impl(trackerID t, BirdTrack_impl* b, std::string trackerName);
  ~tracker_impl();

  void start();
  void stop();
  
  bool loadCalibFile(const char *srcFileName);


private:
  trackerID myID;
  BirdTrack_impl* myBird;
  transformCoords* myTransformer;
  struct fulcrum{
    double sensorVal[3];
    double sensorAngle[3];
    double worldVal[3];
    double worldAngle[3];
  };
 /* variables that handle the database connection */
 kogmo_rtdb_handle_t *dbc; /*< the database connection handle */
 kogmo_rtdb_connect_info_t dbinfo; /*< information about the database connection */

 kogmo_rtdb_obj_info_t dataobj_info_data; /*< information about the type of the dataobject, for calibrated data */
 kogmo_rtdb_obj_info_t dataobj_info_raw; /*< information about the type of the dataobject, for raw data */
 
 kogmo_rtdb_obj_e1_fobtracker_t *dataobj_data; /*< the dataobject specific to the sensor */
 kogmo_rtdb_obj_e1_fobtracker_t *dataobj_raw; /*< the dataobject specific to the sensor */

 kogmo_rtdb_objid_t oid; /*< some id */
 kogmo_rtdb_objid_t pid; /*< another id */
  
  //! this method is called by static method s_workerthread
  void workerThread();
  //! flag switching on/off thread
  bool threadRunning;
  bool writeNotification(kogmo_rtdb_obj_info_t &dataobj_info, double*);

  //! Thread for delivering tracker data to nCenter
  pthread_t workerThreadHandle;
  //! Thread is bound to this method
  static void *s_workerThread(void* arg);

  void printFulcrum(fulcrum fulc);

  bool loadCalibration(const fulcrum cal[255], unsigned int length);
  
};

#endif /* TRACKER_H */
