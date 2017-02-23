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

#include "bird_track_impl.h"
#include "tracker_impl.h"
#include "transform_coords.h"

#define DOUBLE_VECTOR_LENGTH 6

#define NOTE_KEY_1 "X"
#define NOTE_KEY_2 "Y"
#define NOTE_KEY_3 "Z"
#define NOTE_KEY_4 "RX"
#define NOTE_KEY_5 "RY"
#define NOTE_KEY_6 "RZ"

extern unsigned int debugLevel;
typedef boost::array< ::geometry_msgs::Point_<std::allocator<void> > , 8> BoundingBox;
tracker_impl::tracker_impl(trackerID t, BirdTrack_impl* b, std::string trackerName) : myID(t), myBird(b), seqId(0)
{
  //! thread initialize
  threadRunning = false;
  myTransformer = new transformCoords();
  generalPublisher = node.advertise<asr_msgs::AsrObject>("fob_objects", 1000);
}

tracker_impl::~tracker_impl(){
  threadRunning = false;

  // clearing ros interface
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
  
  while(threadRunning)
    {
    //! get data from gloveDevice
    for (unsigned int i=0; i<MAX_SETS; i++)
      {
	//! get data from fob
	myBird->get_posangles(myID, vAcc[i]);
	 
	if (i+1 < MAX_SETS)
	  usleep(PAUSE_SET);
      }
    
    for (unsigned int i=0; i<6; i++)
      {
	v[i] = getMedian(vAcc, i);
      }
    
    //apply calibration
    myTransformer->transform(v);   
    
    //post calibrated data for regular usage
    if (!this->writeNotification(myID, vAcc[0], v)){
      printf( "Couldn't write Notification to NotificationManager. Error!\n");
      exit(-1);
    }
    
    if (debugLevel > 0 )
      {
	if (myID == rightTracker)
	  printf("\n");
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
  std::size_t readSize = 0;
  if (res)
    {
      readSize = fread(&fcountLeft, sizeof(fcountLeft),1,f);
      readSize = fread(&fcountRight, sizeof(fcountRight),1,f);

      if (debugLevel > 0)  {
	printf("loadCalibFile %s\n",srcFileName);
	printf("fcountLeft:%i, fcountRight:%i\n",fcountLeft,fcountRight);
      }
      readSize = fread(&fulcrumarrayLeft,sizeof(fulcrumarrayLeft[0]),fcountLeft,f);
      readSize = fread(&fulcrumarrayRight,sizeof(fulcrumarrayRight[0]),fcountRight,f);

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

void tracker_impl::getPbdObject(std::string name, double *data, asr_msgs::AsrObject &object)
{
  
  // header
  object.header.frame_id = "";
  object.header.seq = seqId;
  object.header.stamp = ros::Time::now();

  //Identification of tracker results.
  object.providedBy = "asr_flock_of_birds";
  

  // Object location distribution
  geometry_msgs::Pose help_pose;
  help_pose.position.x = data[0] / 1000.0;
  help_pose.position.y = data[1] / 1000.0;
  help_pose.position.z = data[2] / 1000.0;
  
  tf::Quaternion orientation = tf::createQuaternionFromRPY(data[5] * M_PI/180.0, data[4] * M_PI/180.0, data[3] * M_PI/180.0);
  help_pose.orientation.x = orientation.x();
  help_pose.orientation.y = orientation.y();
  help_pose.orientation.z = orientation.z();
  help_pose.orientation.w = orientation.w();

  geometry_msgs::PoseWithCovariance help_pose_with_c;
  help_pose_with_c.pose = help_pose;
  if (object.sampledPoses.size() > 0)
  {
      object.sampledPoses.pop_back();
  }
  object.sampledPoses.push_back(help_pose_with_c);

  // bounding box collapses to 1 point
  for (unsigned int i=0; i<8; i++)
    object.boundingBox[i] = help_pose.position;
  object.sizeConfidence = 1.0;

  //Some misc information
  object.type = "tracker";
  object.typeConfidence = 1.0;
  
  object.identifier = name;

  object.meshResourcePath = "";

}

void tracker_impl::getTransform(double *data, tf::Transform &transform)
{
  transform.setOrigin( tf::Vector3(data[0] / 1000.0, data[1] / 1000.0, data[2] / 1000.0) );
  transform.setRotation( tf::createQuaternionFromRPY(data[5] * M_PI/180.0, data[4] * M_PI/180.0, data[3] * M_PI/180.0));
}

bool
tracker_impl::writeNotification(trackerID id, double *raw, double *calibrated)
{ 
  unsigned int i;
  unsigned int err;
  // input is X Y Z RZ RY RX
  // output X Y Z RX RY RZ

  // create ROS msg
  std::string name = id == leftTracker ? "tracker_left" : "tracker_right";
  
  if (debugLevel > 2 )
    printf( "%s_raw %3.3f %3.3f %3.3f %3.3f° %3.3f° %3.3f° ", name.c_str(),
              raw[0], raw[1], raw[2], raw[3], raw[4], raw[5]);
  if (debugLevel > 0 )
    printf( "%s %3.3f %3.3f %3.3f %3.3f° %3.3f° %3.3f° ", name.c_str(),
              calibrated[0], calibrated[1], calibrated[2], calibrated[3], calibrated[4], calibrated[5]);
  
  
  // write calibrated data to ROS
  tf::Transform transform;
  asr_msgs::AsrObject object;
  
  // as AsrObject
  getPbdObject(name, calibrated, object);
  generalPublisher.publish(object);
  
  // as TF frame
  getTransform(calibrated, transform);
  transformPublisher.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "tracker_base", name));

  // write calibratedraw data to ROS
  name += "_raw";
  getPbdObject(name, raw, object);
  generalPublisher.publish(object);
  
  getTransform(raw, transform);
  transformPublisher.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "Root", name));

  this->seqId++;
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

