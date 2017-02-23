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
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sstream>
#include <vector>
#include <algorithm>
/* my includes */

#include "BirdTrack_impl.h"
#include "tracker_impl.h"
#include "transformCoords.h"

#define DOUBLE_VECTOR_LENGTH 6

#define NOTE_KEY_1 "X"
#define NOTE_KEY_2 "Y"
#define NOTE_KEY_3 "Z"
#define NOTE_KEY_4 "RX"
#define NOTE_KEY_5 "RY"
#define NOTE_KEY_6 "RZ"

extern unsigned int debugLevel;

tracker_impl::tracker_impl(trackerID t, BirdTrack_impl* b, std::string trackerName)
{
  unsigned int err;
  //! initialize rtdb connection
  printf("connecting to rtdb...\n");
  //prepare dbinfo structure
  err = kogmo_rtdb_connect_initinfo (&dbinfo, "", trackerName.c_str(), 0.02); DIEonERR(err);
  //use dbinfo structure to connect and save connection handle in dbc
  pid = kogmo_rtdb_connect (&dbc, &dbinfo); DIEonERR(pid);
  //prepare the dataobj_info structure
  err = kogmo_rtdb_obj_initinfo (dbc, &dataobj_info_data,trackerName.c_str(), KOGMO_RTDB_OBJTYPE_E1_FOBTRACKER,
				 sizeof (kogmo_rtdb_obj_e1_fobtracker_t)); DIEonERR(err);
  std::string trackerName_raw = trackerName;
  trackerName_raw+="_raw";
  err = kogmo_rtdb_obj_initinfo (dbc, &dataobj_info_raw,trackerName_raw.c_str(), KOGMO_RTDB_OBJTYPE_E1_FOBTRACKER_RAW,
				 sizeof (kogmo_rtdb_obj_e1_fobtracker_t)); DIEonERR(err);

  // send it to the database -> tell the database what kind of data we send
  oid = kogmo_rtdb_obj_insert (dbc, &dataobj_info_data); DIEonERR(oid);
  oid = kogmo_rtdb_obj_insert (dbc, &dataobj_info_raw); DIEonERR(oid);
  // prepare the actual data object
  dataobj_data = (kogmo_rtdb_obj_e1_fobtracker_t *) malloc(dataobj_info_data.size_max);
   err = kogmo_rtdb_obj_initdata (dbc, &dataobj_info_data, dataobj_data); DIEonERR(err);


  printf("done!\n");
   
  //! tracker
  myID = t;
  myBird = b;

  //! thread initialize
  threadRunning = false;
  myTransformer =new transformCoords();


}

tracker_impl::~tracker_impl(){
  threadRunning = false;
  // disconnect database connection and remove the data object
  kogmo_rtdb_obj_delete (dbc, &dataobj_info_data);
  kogmo_rtdb_obj_delete (dbc, &dataobj_info_raw); 
  kogmo_rtdb_disconnect (dbc, NULL);

  free(dataobj_raw);
  free(dataobj_data);
}


void
*tracker_impl::s_workerThread(void* arg)
{
  tracker_impl* myobj = (tracker_impl*)arg;
  printf("static WorkerThread method called!\n");

  myobj->workerThread();
  printf( "Worker thread exit.\n");
  return NULL;
}



#define MAX_SETS 1
#define PAUSE_SET 30000
#define PAUSE_WRITE 30000

double getMedian(double (&vAcc)[MAX_SETS][6], unsigned int index){
  if (MAX_SETS == 1)
    return vAcc[0][index];

  std::vector<double> data;
  for (unsigned int i=0; i<MAX_SETS; i++)
    data.push_back(vAcc[i][index]);
  std::sort(data.begin(), data.end());
  
  if (MAX_SETS % 2 == 0)
    return ((data[MAX_SETS / 2 - 1] + data[MAX_SETS / 2]) / 2);
  else
    return data[(MAX_SETS - 1) / 2];
}


void
tracker_impl::workerThread(){

  double v[6];
  double vAcc[MAX_SETS][6];
  
  while(threadRunning){

    
    //! get data from gloveDevice
    for (unsigned int i=0; i<MAX_SETS; i++){
      //! get data from fob
      myBird->get_posangles(myID, vAcc[i]);

      if (i == 0)
	{
	  //post raw data for the calibration app
	  if (debugLevel > 2)
	    printf("RAW ");
	  writeNotification(dataobj_info_raw, vAcc[0]);
	}
      
      if (i+1 < MAX_SETS)
	usleep(PAUSE_SET);
    }
    
    for (unsigned int i=0; i<6; i++){
      v[i] = getMedian(vAcc, i);
    }
    
    //apply calibration
    myTransformer->transform(v);
      
      
    if (debugLevel > 0 )
      {
	printf( "%s %3.3lf %3.3lf %3.3lf %3.3lf %3.3lf %3.3lf ",
		myID == leftTracker ? "LEFT" : "RIGHT", 
		v[0], v[1], v[2], v[3], v[4], v[5]);

	if (myID == rightTracker)
	  printf("\n");
      }
    //Vertauschen der beiden Achsen - ist halt so ;-)
    double temp = v[5];
    v[5] = v[3];
    v[3] = temp;

    
   
   
    //post calibrated data for regular usage
    if (debugLevel > 2)
      printf("CAL ");
    if (!writeNotification(dataobj_info_data, v)){
      printf( "Couldn't write Notification to NotificationManager. Error!\n");
      exit(-1);
    }
    
    usleep(PAUSE_WRITE);
  }
  threadRunning = false;
}

void
tracker_impl::start(){
  printf( "Starting myBird\n");
  myBird->start();

  //! start thread
  if(!threadRunning){
    threadRunning = true;
    pthread_create(&workerThreadHandle,NULL, &s_workerThread, (void*)this);
  }
  else
    printf( "Thread started, but was already running.\n");
  printf( "Thread running.\n");
}

void
tracker_impl::stop(){
  printf( "Stopping myBird\n");
  myBird->stop();

  //! stop thread
  threadRunning = false;
}


bool
tracker_impl::loadCalibFile(const char *srcFileName)    
{
  //original: return myTransformer->loadCalibFile(srcFileName);
  //original: myTransformer->loadCalibFile: no implemented
  //this code taken from trackerCalibration
  bool res;
  std::string filename;
  std::stringstream s;

  int fcountLeft, fcountRight;
  fulcrum fulcrumarrayLeft[255], fulcrumarrayRight[255];
  FILE *f;
  res = ((f=fopen(srcFileName, "r"))!=NULL);
  if (res)
    {
      fread(&fcountLeft,sizeof(fcountLeft),1,f);
      fread(&fcountRight,sizeof(fcountRight),1,f);

      if (debugLevel > 0)  {
	printf("loadCalibFile %s\n",srcFileName);
	printf("fcountLeft:%i, fcountRight:%i\n",fcountLeft,fcountRight);
      }
      fread(&fulcrumarrayLeft,sizeof(fulcrumarrayLeft[0]),fcountLeft,f);
      fread(&fulcrumarrayRight,sizeof(fulcrumarrayRight[0]),fcountRight,f);

      if (debugLevel > 1) {
	printf("fulcrumyArrayLeft(%i):\n",fcountLeft);
	for(int i=0;i < fcountLeft;i++) {
	  printf("\nentry %i:\n",i);
	  printFulcrum(fulcrumarrayLeft[i]);
	}
	printf("fulcrumyArrayRight(%i):\n",fcountRight);
	for(int i=0;i < fcountRight;i++) {
	  printf("\nentry %i:\n",i);
	  printFulcrum(fulcrumarrayRight[i]);
	}
      }
//loading from file works. storage in array works
      s << "Loaded " << (fcountLeft+fcountRight) << " fulcrums\n";
      fclose(f);
    }
  else {
    printf("error loading calibration data: 2%s:%u\n",__FILE__,__LINE__);
    return false;
  }

  if (myID == rightTracker) {
    if (debugLevel > 0)
      printf("loading Calibration into rightTracker\n");
    res = loadCalibration(fulcrumarrayRight, fcountRight);
  } else {
    if (debugLevel > 0)
      printf("loading Calibration into leftTracker\n");
    res = loadCalibration(fulcrumarrayLeft, fcountLeft);
  }
  if(res)
    printf("Success! Calibration data loaded\n");
  else
    printf("error loading calibration data: %s:%u\n",__FILE__,__LINE__);

  return res;
}


void tracker_impl::printFulcrum(fulcrum fulc) {
    printf("sensorVal: %3.3f, %3.3f, %3.3f\n",fulc.sensorVal[0] ,fulc.sensorVal[1], fulc.sensorVal[2]);
    printf("sensorAngle: %3.3f, %3.3f, %3.3f\n",fulc.sensorAngle[0] ,fulc.sensorAngle[1], fulc.sensorAngle[2]);
    printf("worldVal: %3.3f, %3.3f, %3.3f\n",fulc.worldVal[0] ,fulc.worldVal[1], fulc.worldVal[2]);
    printf("worldAngle: %3.3f, %3.3f, %3.3f\n",fulc.worldAngle[0] ,fulc.worldAngle[1], fulc.worldAngle[2]);
  }

bool
tracker_impl::loadCalibration(const fulcrum cal[255], unsigned int length)
{
  double world[6],sensor[6];
  myTransformer->resetCalibration();
  for (unsigned int i=0; i < length; i++)
    {
      for (int j=0;j<3;j++)
	{
	  sensor[j]=cal[i].sensorVal[j];
	  sensor[j+3]=cal[i].sensorAngle[j];
	  world[j]=cal[i].worldVal[j];
	  world[j+3]=cal[i].worldAngle[j];
	}
      myTransformer->addCalibrationData(sensor,world);
    }
  return myTransformer->isCalibrated();
}

bool
tracker_impl::writeNotification(kogmo_rtdb_obj_info_t &dataobj_info, double* data) { //double *data) {
  unsigned int i;
  unsigned int err;


  
  dataobj_data->data.trackerID = myID;
  for (i=0; i<6; i++) {
    dataobj_data->data.data[i] = data[i];
  }

  if (myID == leftTracker)
    if (debugLevel > 2 )
      printf( "LEFT  %3.3lf %3.3lf %3.3lf %3.3lf %3.3lf %3.3lf\n", 
	      data[0], data[1], data[2], data[3], data[4], data[5]);
  if (myID == rightTracker)
    if (debugLevel > 2 )
      printf( "RIGHT %3.3lf %3.3lf %3.3lf %3.3lf %3.3lf %3.3lf\n", 
	      data[0], data[1], data[2], data[3], data[4], data[5]);

  // write data to database - if this crashes choose dataobj_raw for dataobj_info_raw
  err = kogmo_rtdb_obj_writedata (dbc, dataobj_info.oid, dataobj_data); DIEonERR(err);
    
  //tell the database that one cycle is finished and that we are still alive. This is important!
  kogmo_rtdb_cycle_done(dbc,0);
    
  return true;
}





#if tracker_impl_test
int main(int argc, char **argv)
{
  bgcorba::init(argc, argv);
  
  // Create a new instance of the implementation 
  tracker_impl *impl = new tracker_impl;

  int retval =  bgcorba::main(impl);

  delete impl;

  return retval;
}
#endif /* tracker_impl_test */

