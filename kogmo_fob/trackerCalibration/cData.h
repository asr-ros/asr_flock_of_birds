/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef CDATA_H
#define CDATA_H

/* system includes */
#include <vector>

/* my includes */
//#include "gloveServer.hh"

#include "calibrationData.h"

/*!
  \class cData
  \brief


*/

//#define numSensors 22


class cData {
public:
  cData();
  ~cData();

  typedef std::pair<double, double> fulcrum;
  typedef std::vector<fulcrum> sensorCal;
  typedef std::vector<sensorCal> trackerCal;

  bool connectToTrackerServer(std::string trackerServerName);

  void reset();
  
  unsigned int getNumTotalSteps();
  unsigned int getCurrentStep();
  std::string getTextForStep(int);
  std::string getPicForStep(int);
  bool calibrate();
  
  trackerCal getCalibration();
  bool setCalibration(trackerCal g);

  std::vector<double> calibrateNextStep();

  bool checkConsistency(std::string &print);
  bool saveCalibration(std::string fileName);
  bool openCalibration(std::string fileName);
  bool tellServerLoadCurrentCalibration();
  //  tracker::trackerServer_ptr myTracker;

private:

  trackerCal computeCalibration(std::vector<std::vector<double> > r);

  //! recorded raw data
  std::vector<std::vector<double> > rawData;
  
  //! calibration data
  trackerCal data;

  //! current and total step number
  unsigned int numTotalSteps;
  unsigned int currStep;

  //! ptr to tracker server
  
};

#endif /* CDATA_H */
