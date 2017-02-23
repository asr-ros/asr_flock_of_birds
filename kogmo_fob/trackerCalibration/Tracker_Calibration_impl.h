/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TRACKER_CALIBRATION_H
#define TRACKER_CALIBRATION_H

/* system includes */
#include <vector>
/* my includes */
#include "qt_calibration.uic.h"

// KOGMO_RTDB standard includes
#define KOGMO_RTDB_DONT_INCLUDE_ALL_OBJECTS
#include "kogmo_rtdb.h"
#include "kogmo_rtdb_obj_typeids.h"
//KOGMO_RTDB special includes for our project with the id E1
#include "itec/kogmo_rtdb_obj_e1_itec.h"
using namespace KogniMobil;


/*!
  \class Tracker_Calibration_impl
  \brief


*/
class Tracker_Calibration_impl
  : public qt_Tracker_Calibration
{
 public:
  Tracker_Calibration_impl();
  ~Tracker_Calibration_impl();

  virtual void connectSlot();
  virtual void saveSlot();
  virtual void loadSlot();
  virtual void helpSlot();
  virtual void GetNAdd_clicked();
  virtual void ResetCalib_clicked();
  virtual void SendDataToTracker_clicked();
  virtual void saveCoord();
  virtual void loadCoord();
  virtual void clearAllData();

  struct fulcrum{
    double sensorVal[3];
    double sensorAngle[3];
    double worldVal[3];
    double worldAngle[3];
  };

  typedef std::pair<double, double> fulcrumPair;
  typedef std::vector<fulcrumPair> sensorCal;
  typedef std::vector<sensorCal> trackerCal;
  
private:
  int openDevice(std::string &trackLeftId, std::string &trackRightId);
  int closeDevice();
  void updateCounter();

  void printFulcrum(fulcrum fulc);
  bool displayCal(trackerCal cal);

  bool trackLeft;
  bool trackRight;
  
  int fcountLeft;
  int fcountRight;
  int coordCountLeft;
  int coordCountRight;

  fulcrum fulcrumarrayLeft[255];
  fulcrum fulcrumarrayRight[255];

   /* variables that handle the database connection */
  kogmo_rtdb_handle_t *dbc; /*< the database connection handle */
  kogmo_rtdb_connect_info_t dbinfo; /*< information about the database connection */
  kogmo_rtdb_obj_info_t dataobj_left_info; /*< information about the type of the dataobject */
  kogmo_rtdb_obj_info_t dataobj_right_info; /*< information about the type of the dataobject */
  kogmo_rtdb_obj_e1_fobtracker_t *dataobj_left; /*< the dataobject specific to the sensor */
  kogmo_rtdb_obj_e1_fobtracker_t *dataobj_right; /*< the dataobject specific to the sensor */
  kogmo_rtdb_objid_t oidLeft; /*< id of our object within the database */
  kogmo_rtdb_objid_t oidRight; /*< another id */
};


#endif /* TRACKER_CALIBRATION_H */
