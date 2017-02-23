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
#include <sstream>
#include <fstream>
#include <cmath>

/* my includes */
#include "cData.h"
#include "trackerConstants.h"

#define NUM_SENSORS 22
#define NUM_TOTAL_STEPS 8
#define MIN_JOINT_RESOLUTION 20

const char *CAL_DEFAULT_TEXT = "Data tracker calibration (c) 2004 by Steffen.";
const char *CAL_STEP_TEXT[] = {"Press your hand against a flat surface, all fingers pointing parallel,  and press \"Record Sample\" when ready.",
			       "Press your hand against a flat surface, open all fingers, and press \"Record Sample\" when ready.",
			       "Close your index, middle, ring, pinky finger as much as possible, and press \"Record Sample\" when ready.",
			       "Do an ok gesture by touching the thumb tip with the index tip, and press \"Record Sample\" when ready.",
			       "Bend your wrist up, and press Enter when ready.",
			       "Bend your wrist down, and press Enter when ready.",
			       "Bend your wrist inside, with your thumb towards the forearm. Press Enter when ready.",
			       "Bend your wrist outside, and press Enter when ready.",
			       "Calibration finished. Now save and/or load it to the server."
};

const char *CAL_DEFAULT_PIC = "images/00glove-calibration.jpg"; 
const char *CAL_STEP_PIC[] = {"images/0hand-flat-closed.jpg",
			      "images/1hand-flat-open.jpg",
			      "images/2hand-fingers-closed.jpg",
			      "images/3hand-ok-gesture.jpg",
			      "images/4hand-wrist-up.jpg",
			      "images/5hand-wrist-down.jpg",
			      "images/6hand-wirst-in.jpg",
			      "images/7hand-wirst-out.jpg",
			      "images/00glove-calibration.jpg"
};


cData::cData()
{
  numTotalSteps = NUM_TOTAL_STEPS;
  reset();
}

cData::~cData()
{
}

bool
cData::connectToTrackerServer(std::string trackerServerName){

  CORBA::Object_ptr newObjectPointer = bgcorba::resolveObject(trackerServerName);
  myTracker = tracker::trackerServer::_narrow(newObjectPointer);
  //myTracker = trackerServer::_narrow(newObjectPointer);
  if (CORBA::is_nil(myTracker)) {
    BGDBG(0, "Could not find a trackerServer.\n");
    return false;
  }
  else
    BGDBG(0, "trackerServer found!\n");

  return true;
}

void
cData::reset(){

  data.clear();
  rawData.clear();
  currStep = 0;
}

unsigned int
cData::getNumTotalSteps(){

  return numTotalSteps;
}

unsigned int
cData::getCurrentStep(){

  return currStep;
}

std::string
cData::getTextForStep(int step){

  if(step <= NUM_TOTAL_STEPS && step >= 0)
    return CAL_STEP_TEXT[step];
  else{
    return CAL_DEFAULT_TEXT;
  }
}

std::string
cData::getPicForStep(int step){

  if(step <= NUM_TOTAL_STEPS && step >= 0)
    return CAL_STEP_PIC[step];
  else{
    return CAL_DEFAULT_PIC;
  }
}

std::vector<double>
cData::calibrateNextStep(){

  currStep++;
  std::vector<double> c;
  IPRFilterModule::TimeDataVector measure;

  //! receive data
  try{
    measure = *(myTracker->getRawData());
  }catch(...){
    BGDBG(0, "Glove server dead? Could not connect.\n");
    return c;
  }

  for (unsigned int i=0; i<measure.v.vector.length(); i++)
    c.push_back(measure.v.vector[i]);

  rawData.push_back(c);
  
  return c;
}

cData::trackerCal
cData::getCalibration(){

  return data;
}

bool
cData::setCalibration(cData::trackerCal g){

  data = g;
  return true;
}

bool
cData::calibrate(){

  data = computeCalibration(rawData);
  if(data.size() != NUM_SENSORS)
    return false;

  return true;
}

bool
cData::saveCalibration(std::string fileName){

  FILE *f;

  // open file
  if ((f=fopen(fileName.c_str(), "w")) == NULL)
    {
      BGDBG(0, "save calibration file - error: open file !\n");  
      return false;
    }

  for (unsigned int i=0; i<data.size(); i++){

    for (unsigned int j=0; j<data[i].size(); j++){
      fprintf(f, "%f:%f", (data[i])[j].first, (data[i])[j].second);
      if(j < data[i].size()-1)
	fprintf(f, " ");
      else
	fprintf(f, "\n");
    }
  }
  fclose(f);

  BGDBG(0, "Saved calibration successfully!\n");
  
  return true;
}


bool
cData::openCalibration(std::string fileName){

  std::ifstream inputFile (fileName.c_str());
  if (!inputFile.is_open()){
    BGDBG(0, "open calibration file - error while opening file !\n");  
    return false;
  }


  // split into lines
  bg::strlist lines = bg::string(inputFile).split("\n");
  trackerCal tcal;

  // parse lines
  for (unsigned int lineNo=0; lineNo<lines.size(); lineNo++){

    // split into fulcri
    bg::strlist fulcs = lines[lineNo].split(" ");
    sensorCal scal;

    // parse fulcri
    for(unsigned int fulcNo=0; fulcNo<fulcs.size(); fulcNo++){

      fulcrum f;
      bg::strlist oneF = fulcs[fulcNo].split(":");
      if(oneF.size() != 2){
	BGDBG(0, "Read calibration file - error while reading file. Incomplete data.\n");  
	return false;
      }
      f.first = oneF[0].toDouble();
      f.second = oneF[1].toDouble();
      scal.push_back(f);
    }
    tcal.push_back(scal);
  }
  
  
  BGDBG(0, "Opened calibration file successfully!\n");
  BGDBG(3, "File contained %d lines, first line contained %d entries.\n", tcal.size(), tcal[0].size());

  data = tcal;
  
  return true;
}

bool
cData::checkConsistency(std::string &print){

  std::vector<unsigned int> bad;
  std::stringstream s;
  
  for (unsigned int i=0; i<data.size(); i++){

    if(fabs(data[i][1].second - data[i][0].second) < MIN_JOINT_RESOLUTION){

      bad.push_back(i);
    }
  }

  if(!bad.empty()){

    BGDBG(3, "Calibration data not ok.\n");
    s << "********************************************************************\n";
    s << "Bad calibration data.\n";
    for(unsigned int i=0; i<bad.size(); i++){
      s << "Joint " << JOINT_NAME[bad[i]] << " does not achieve minimum resolution.\n";
      s << "Joint resolution: " << (int)fabs(data[bad[i]][1].second - data[bad[i]][0].second)
	<< " Minimum resolution: " << MIN_JOINT_RESOLUTION << "\n";
      if(fabs(data[bad[i]][1].second - data[bad[i]][0].second) < 2)
	s << "GloveServer will bail out when reading this data!\n";
      s << "\n";
    }
    s << "It is strongly recommended to repeat the calibration.\n";
    s << "********************************************************************\n";

    print = std::string(s.str());
    return false;
  }
  else{
    s << "Calibration data ok.\n";
    print = std::string(s.str());
    BGDBG(3, "Calibration data ok.\n");
  }
  
  return true;
}

bool
cData::tellServerLoadCurrentCalibration(){
  /*
  tracker::allSensorCal calib;
  calib.length(data.size());

  //! loop durch alle sensoren
  for (unsigned int i=0; i<data.size(); i++){
    
    tracker::oneSensorCal osc;
    osc.length(data[i].size());
    
    //! loop durch alle Stützpunkte
    for (unsigned int j=0; j<data[i].size(); j++){

      tracker::fulcrum f;
      f.angle = data[i][j].first;
      f.sensorVal = data[i][j].second;
      osc[j] = f;
    }
    calib[i] = osc;
  }
  bool ret;
  try{
    ret = myTracker->loadCalibration(calib);
  }
  catch(...){
    BGDBG(0, "TracerServer dead? Could not connect.\n");
    ret = false;
  }
  return ret;*/
  return false;
}


cData::trackerCal
cData::computeCalibration(std::vector<std::vector<double> > r){

  //! construct a calibration data struct
  trackerCal c;
  sensorCal sc;
  fulcrum f;

  //! reference angles
  std::vector<double> a1, a2;
  for (int i=0; i<NUM_SENSORS; i++){
	//    a1.push_back(GLOVE_RIGHT_VEC_1[i]);
	//    a2.push_back(GLOVE_RIGHT_VEC_2[i]);
  }

  //! check if input data is ok
  if(r.size() != NUM_TOTAL_STEPS){

    BGDBG(0, "Error in computeCalibration(). Input data not ok.\n");
    return c;
  }
  
  //! wrist calibration
  //! left/right
  sc.clear();
  f.first = a1[0];
  f.second = r[6][0];
  sc.push_back(f);
  f.first = a2[0];
  f.second = r[7][0];
  sc.push_back(f);
  c.push_back(sc);

  //! up/down
  sc.clear();
  f.first = a1[1];
  f.second = r[4][1];
  sc.push_back(f);
  f.first = a2[1];
  f.second = r[5][1];
  sc.push_back(f);
  c.push_back(sc);

  //! calibration of thumb
  //! spread
  sc.clear();
  f.first = a1[2];
  f.second = r[0][2];
  sc.push_back(f);
  f.first = a2[2];
  f.second = r[3][2];
  sc.push_back(f);
  c.push_back(sc);

  //! calibration of thumb
  sc.clear();
  f.first = a1[3];
  f.second = r[1][3];
  sc.push_back(f);
  f.first = a2[3];
  f.second = r[3][3];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[4];
  f.second = r[1][4];
  sc.push_back(f);
  f.first = a2[4];
  f.second = r[3][4];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[5];
  f.second = r[1][5];
  sc.push_back(f);
  f.first = a2[5];
  f.second = r[3][5];
  sc.push_back(f);
  c.push_back(sc);

  //! calibration of index
  sc.clear();
  f.first = a1[6];
  f.second = r[0][6];
  sc.push_back(f);
  f.first = a2[6];
  f.second = r[2][6];
  sc.push_back(f);
  c.push_back(sc);

  //! spread
  sc.clear();
  f.first = a1[7];
  f.second = r[0][7];
  sc.push_back(f);
  f.first = a2[7];
  f.second = r[1][7];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[8];
  f.second = r[0][8];
  sc.push_back(f);
  f.first = a2[8];
  f.second = r[2][8];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[9];
  f.second = r[0][9];
  sc.push_back(f);
  f.first = a2[9];
  f.second = r[2][9];
  sc.push_back(f);
  c.push_back(sc);

  //! calibration of middle
  sc.clear();
  f.first = a1[10];
  f.second = r[0][10];
  sc.push_back(f);
  f.first = a2[10];
  f.second = r[2][10];
  sc.push_back(f);
  c.push_back(sc);

  //! spread
  sc.clear();
  f.first = a1[11];
  f.second = r[0][11];
  sc.push_back(f);
  f.first = a2[11];
  f.second = r[1][11];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[12];
  f.second = r[0][12];
  sc.push_back(f);
  f.first = a2[12];
  f.second = r[2][12];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[13];
  f.second = r[0][13];
  sc.push_back(f);
  f.first = a2[13];
  f.second = r[2][13];
  sc.push_back(f);
  c.push_back(sc);

  //! calibration of ring
  sc.clear();
  f.first = a1[14];
  f.second = r[0][14];
  sc.push_back(f);
  f.first = a2[14];
  f.second = r[2][14];
  sc.push_back(f);
  c.push_back(sc);

  //! spread
  sc.clear();
  f.first = a1[15];
  f.second = r[0][15];
  sc.push_back(f);
  f.first = a2[15];
  f.second = r[1][15];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[16];
  f.second = r[0][16];
  sc.push_back(f);
  f.first = a2[16];
  f.second = r[2][16];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[17];
  f.second = r[0][17];
  sc.push_back(f);
  f.first = a2[17];
  f.second = r[2][17];
  sc.push_back(f);
  c.push_back(sc);

  //! calibration of pinky
  sc.clear();
  f.first = a1[18];
  f.second = r[0][18];
  sc.push_back(f);
  f.first = a2[18];
  f.second = r[2][18];
  sc.push_back(f);
  c.push_back(sc);

  //! spread
  sc.clear();
  f.first = a1[19];
  f.second = r[0][19];
  sc.push_back(f);
  f.first = a2[19];
  f.second = r[1][19];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[20];
  f.second = r[0][20];
  sc.push_back(f);
  f.first = a2[20];
  f.second = r[2][20];
  sc.push_back(f);
  c.push_back(sc);

  sc.clear();
  f.first = a1[21];
  f.second = r[0][21];
  sc.push_back(f);
  f.first = a2[21];
  f.second = r[2][21];
  sc.push_back(f);
  c.push_back(sc);

  return c;
}



#if cData_test
#include <stdio.h>
int main(int argc, char **argv)
{
  // This is a module-test block. You can put code here that tests
  // just the contents of this C file, and build it by saying
  //             make cData_test
  // Then, run the resulting executable (cData_test).
  // If it works as expected, the module is probably correct. ;-)

  fprintf(stderr, "Testing cData\n");

  return 0;
}
#endif /* cData_test */
