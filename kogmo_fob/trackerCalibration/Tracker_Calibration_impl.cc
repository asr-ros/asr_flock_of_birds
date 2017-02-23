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
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtextbrowser.h>
#include <qprogressbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qtable.h>
#include <qcheckbox.h>

#include <sstream>

/* my includes */
#include "qt_editdata_impl.h"
#include "qt_calibration.uic.h"
#include "Tracker_Calibration_impl.h"

#define DIEonERR(x) if (x < 0) {printf("Fehler %u in %s:%u Datenbankbefehl\n",x,__FILE__,__LINE__);}


Tracker_Calibration_impl::Tracker_Calibration_impl()
{
  trackLeft=false;
  trackRight=false;
  fcountLeft=0;
  fcountRight=0;
  coordCountLeft=0;
  coordCountRight=0;
  pushButton10->setEnabled(false);
  logBrowser->clear();
  logBrowser->append("Started tracker calibration.\n");
}

Tracker_Calibration_impl::~Tracker_Calibration_impl()
{
  closeDevice();
}

void
Tracker_Calibration_impl::updateCounter()
{  std::stringstream s;
  s  << "World coordinates: " << (coordCountLeft+coordCountRight) << " Tracker coordinates: " << (fcountLeft+fcountRight);
  textFulcrumCount->setText(s.str().c_str());
}


void
Tracker_Calibration_impl::connectSlot(){

  
  std::string serverNameLeft((trackerName->text()).latin1());
  std::string serverNameRight((trackerName_2->text()).latin1());

  trackLeft = (Use_Left_Tracker->isChecked());
  trackRight= (Use_Right_Tracker->isChecked());

  openDevice(serverNameLeft, serverNameRight);


  if(!(trackLeft|trackRight)){
    QMessageBox::critical(this, "TrackerCalibration", "Could not connect to any tracker server.", "OK");
    logBrowser->append("Could not connect to any tracker server.\n");
    connectedLabel->setText("Not connected");
    connectedLabel->setPaletteForegroundColor(QColor("red"));
    buttonConnectToTracker->setDefault(true);
  }
  else{
    std::stringstream s;
    if (trackLeft) s << "Connected to left server " << trackerName->text() << "\n";
    if (trackRight) s << "Connected to right server " << trackerName->text() << "\n";
    logBrowser->append(s.str().c_str());
    if (trackLeft&trackRight) connectedLabel->setText("Both connected"); else
      if (trackLeft) connectedLabel->setText("Left connected"); else
	connectedLabel->setText("Right connected");
    connectedLabel->setPaletteForegroundColor(QColor("green"));
    pushButton14->setDefault(true);
  }
  logBrowser->scrollToBottom();
}


int Tracker_Calibration_impl::openDevice(std::string &trackLeftId, std::string &trackRightId) {
  int err;
    std::string temp;
    
  /* establish database connection
   * the cycle time is 0,03s this means this client does not ask for new data more than 30 times per second,
   * this has to be measured or estimated! The database stalls read until this delay is met! */
  err = kogmo_rtdb_connect_initinfo (&dbinfo, "", "TrackerCalibrationClient", 0.03); DIEonERR(err);
  oidLeft = kogmo_rtdb_connect (&dbc, &dbinfo); DIEonERR(oidLeft);
  
  //search for our objecttype. Wait until it is registered
  
  if (trackLeft){

    temp = trackLeftId + "_raw";
    oidLeft = kogmo_rtdb_obj_searchinfo_wait (dbc, temp.c_str(), KOGMO_RTDB_OBJTYPE_E1_FOBTRACKER_RAW, 0, 0);  DIEonERR(oidLeft);
    if (err <0) 
      trackLeft=false;
    err = kogmo_rtdb_obj_readinfo (dbc, oidLeft, 0, &dataobj_left_info); DIEonERR(err);
    if (err <0) 
      trackLeft=false;
	
    dataobj_left = (kogmo_rtdb_obj_e1_fobtracker_t *) malloc(dataobj_left_info.size_max);
    err = kogmo_rtdb_obj_initdata (dbc, &dataobj_left_info, dataobj_left); DIEonERR(err);
    if (err <0) 
      trackLeft=false;

  }
  if (trackRight) {
    temp = trackRightId + "_raw";
    oidRight = kogmo_rtdb_obj_searchinfo_wait (dbc, temp.c_str(), KOGMO_RTDB_OBJTYPE_E1_FOBTRACKER_RAW, 0, 0);  DIEonERR(oidRight);
    if (err <0) 
      trackRight=false;
    err = kogmo_rtdb_obj_readinfo (dbc, oidRight, 0, &dataobj_right_info);
    if (err <0) 
      trackRight=false;
    dataobj_right = (kogmo_rtdb_obj_e1_fobtracker_t *) malloc(dataobj_right_info.size_max);
    err = kogmo_rtdb_obj_initdata (dbc, &dataobj_right_info, dataobj_right);
    if (err <0) 
      trackRight=false;
  }

  return 0;
}


/*
 * close the db connection
 */
int Tracker_Calibration_impl::closeDevice(){
  signed int err;
  err = kogmo_rtdb_obj_delete (dbc, &dataobj_left_info); DIEonERR(err);
  err = kogmo_rtdb_obj_delete (dbc, &dataobj_right_info); DIEonERR(err);
  err = kogmo_rtdb_disconnect(dbc, NULL); DIEonERR(err);
  free(dataobj_left);
  free(dataobj_right);
  return 0;
}


bool
Tracker_Calibration_impl::displayCal(trackerCal cal){

  std::stringstream s;
  s << "Calibration data:\n";
  for (unsigned int i=0; i<cal.size(); i++){
    
    for (unsigned int j=0; j<cal[i].size(); j++){
      s << (cal[i])[j].first << ":" << (cal[i])[j].second;
      if(j < cal[i].size()-1)
	s << " ";
      else
	s << "\n";
    }
  }
  logBrowser->append(s.str().c_str());
  logBrowser->scrollToBottom();
  
  return true;
}


void
Tracker_Calibration_impl::saveSlot(){

  QString qfile = QFileDialog::getSaveFileName(QString::null,
					       "Tracker calibrations (*.calib)",
					       this,
					       "save file dialog",
					       "Choose a filename to save under" );

  bool res;
  std::string filename;
  std::stringstream s;
  
  if(qfile != NULL){
    filename = std::string(qfile.latin1());
    FILE *f;
    res = ((f=fopen(filename.c_str(), "w"))!=NULL);
    if (res)
      {
	printf("fcountLeft:%i, fcountRight:%i\n",fcountLeft,fcountRight);
	fwrite(&fcountLeft,sizeof(fcountLeft),1,f);
	fwrite(&fcountRight,sizeof(fcountRight),1,f);
	printf("fulcrumyArrayLeft(%i):\n",fcountRight);
	for(int i=0;i < fcountRight;i++) {
	  printf("\nentry %i:\n",i);
	  printFulcrum(fulcrumarrayLeft[i]);
	}
	printf("fulcrumyArrayRight(%i):\n",fcountLeft);
	for(int i=0;i < fcountRight;i++) {
	  printf("\nentry %i:\n",i);
	  printFulcrum(fulcrumarrayRight[i]);
	}
	fwrite(&fulcrumarrayLeft,sizeof(fulcrumarrayLeft[0]),fcountLeft,f);
	fwrite(&fulcrumarrayRight,sizeof(fulcrumarrayRight[0]),fcountRight,f);
	s << "Saved " << (fcountLeft+fcountRight) << " fulcrums\n";
	fclose(f);
      }
  }
  else
    res = false;
  
  if(res)
    s  << "Saved data to file " << filename << "\n";
  else
    s << "File " << filename << " not saved.\n";
  logBrowser->append(s.str().c_str());
  logBrowser->scrollToBottom();
}

  void Tracker_Calibration_impl::printFulcrum(fulcrum fulc) {
    printf("sensorVal: %3.3f, %3.3f, %3.3f\n",fulc.sensorVal[0] ,fulc.sensorVal[1], fulc.sensorVal[2]);
    printf("sensorAngle: %3.3f, %3.3f, %3.3f\n",fulc.sensorAngle[0] ,fulc.sensorAngle[1], fulc.sensorAngle[2]);
    printf("worldVal: %3.3f, %3.3f, %3.3f\n",fulc.worldVal[0] ,fulc.worldVal[1], fulc.worldVal[2]);
    printf("worldAngle: %3.3f, %3.3f, %3.3f\n",fulc.worldAngle[0] ,fulc.worldAngle[1], fulc.worldAngle[2]);
  }
  
void
Tracker_Calibration_impl::loadSlot(){

  QString qfile = QFileDialog::getOpenFileName(QString::null,
					       "Tracker calibrations (*.calib)",
					       this,
					       "open file dialog",
					       "Choose a file to open" );

  bool res;
  std::string filename;
  std::stringstream s;
  
  if(qfile != NULL){
    filename = std::string(qfile.latin1());
    FILE *f;
    res = ((f=fopen(filename.c_str(), "r"))!=NULL);
    if (res)
      {
	fread(&fcountLeft,sizeof(fcountLeft),1,f);
	fread(&fcountRight,sizeof(fcountRight),1,f);

	printf("fcountLeft:%i, fcountRight:%i\n",fcountLeft,fcountRight);

	fread(&fulcrumarrayLeft,sizeof(fulcrumarrayLeft[0]),fcountLeft,f);
	fread(&fulcrumarrayRight,sizeof(fulcrumarrayRight[0]),fcountRight,f);

	printf("fulcrumyArrayLeft(%i):\n",fcountRight);
	for(int i=0;i < fcountRight;i++) {
	  printf("\nentry %i:\n",i);
	  printFulcrum(fulcrumarrayLeft[i]);
	}
	printf("fulcrumyArrayRight(%i):\n",fcountLeft);
	for(int i=0;i < fcountRight;i++) {
	  printf("\nentry %i:\n",i);
	  printFulcrum(fulcrumarrayRight[i]);
	}

	s << "Loaded " << (fcountLeft+fcountRight) << " fulcrums\n";
	fclose(f);
	coordCountLeft=fcountLeft;
	coordCountRight=fcountRight;
      }
  }
  else
    res = false;
  
  if(res){
    s  << "Loaded data from file " << filename << "\n";
  }
  else
    s << "File " << filename << " not opened.\n";
  logBrowser->append(s.str().c_str());
  logBrowser->scrollToBottom();
  updateCounter();
}


void
Tracker_Calibration_impl::loadCoord()
{
  loadSlot();
  fcountLeft=0;
  fcountRight=0;
  if (coordCountLeft>0)
    {
      std::stringstream xl,yl,zl;
      xl << fulcrumarrayLeft[fcountLeft].worldVal[0];
      WorldX->setText(xl.str().c_str());
      yl << fulcrumarrayLeft[fcountLeft].worldVal[1];
      WorldY->setText(yl.str().c_str());
      zl << fulcrumarrayLeft[fcountLeft].worldVal[2];
      WorldZ->setText(zl.str().c_str());
    }
  if (coordCountRight>0)
    {
      std::stringstream xr,yr,zr;
      xr << fulcrumarrayRight[fcountRight].worldVal[0];
      WorldX_2->setText(xr.str().c_str());
      yr << fulcrumarrayRight[fcountRight].worldVal[1];
      WorldY_2->setText(yr.str().c_str());
      zr << fulcrumarrayRight[fcountRight].worldVal[2];
      WorldZ_2->setText(zr.str().c_str());
    }
  updateCounter();
}

void
Tracker_Calibration_impl::saveCoord()
{ int l=fcountLeft;
  int r=fcountRight;
  fcountLeft=coordCountLeft;
  fcountRight=coordCountRight;
  saveSlot();
  fcountLeft=l;
  fcountRight=r;
}


void
Tracker_Calibration_impl::helpSlot(){

  QMessageBox::about(this,
		     "TrackerCalibration",
		     "If you really need some help, there should be a readme file in your Tracker Calibration directory.");
  BGDBG(0, "See readme file for help\n");
}


void
Tracker_Calibration_impl::GetNAdd_clicked()
{
  int err;

  Tracker_Calibration_impl::fulcrum fLeft, fRight;
  std::stringstream s;

  //read object from database
  err = kogmo_rtdb_obj_readdata (dbc, oidLeft, 0, dataobj_left, dataobj_left_info.size_max); 
  if (err <0)
        logBrowser->append("Left tracker server dead? Could not connect.\n");
  err = kogmo_rtdb_obj_readdata (dbc, oidRight, 0, dataobj_right, dataobj_right_info.size_max);
  if (err <0)
      logBrowser->append("RightTracker server dead? Could not connect.\n");
  

  if (trackLeft)
    {
      fLeft.sensorVal[0]=dataobj_left->data.data[0];
      fLeft.sensorVal[1]=dataobj_left->data.data[1];
      fLeft.sensorVal[2]=dataobj_left->data.data[2];
      fLeft.worldVal[0]=strtod(WorldX->text(),NULL);
      fLeft.worldVal[1]=strtod(WorldY->text(),NULL);
      fLeft.worldVal[2]=strtod(WorldZ->text(),NULL);
      fulcrumarrayLeft[fcountLeft++]=fLeft;
      if (fcountLeft>coordCountLeft) coordCountLeft++; else
	{
	  std::stringstream x,y,z;
	  x << fulcrumarrayLeft[fcountLeft].worldVal[0];
	  WorldX->setText(x.str().c_str());
	  y << fulcrumarrayLeft[fcountLeft].worldVal[1];
	  WorldY->setText(y.str().c_str());
	  z << fulcrumarrayLeft[fcountLeft].worldVal[2];
	  WorldZ->setText(z.str().c_str());
	}
    }
  if (trackRight)
    {
      fRight.sensorVal[0]=dataobj_right->data.data[0];
      fRight.sensorVal[1]=dataobj_right->data.data[1];
      fRight.sensorVal[2]=dataobj_right->data.data[2];
      fRight.worldVal[0]=strtod(WorldX_2->text(),NULL);
      fRight.worldVal[1]=strtod(WorldY_2->text(),NULL);
      fRight.worldVal[2]=strtod(WorldZ_2->text(),NULL);
      fulcrumarrayRight[fcountRight++]=fRight;
      if (fcountRight>coordCountRight) coordCountRight++; else
	{
	  std::stringstream x,y,z;
	  x << fulcrumarrayRight[fcountRight].worldVal[0];
	  WorldX_2->setText(x.str().c_str());
	  y << fulcrumarrayRight[fcountRight].worldVal[1];
	  WorldY_2->setText(y.str().c_str());
	  z << fulcrumarrayRight[fcountRight].worldVal[2];
	  WorldZ_2->setText(z.str().c_str());
	}
    }
  if (trackLeft)
    {
      s << "Left: ";
      for (int i=0;i<3;i++)
        s << fLeft.sensorVal[i] << " ";
    }
  if (trackRight)
    {
      s << "Right: ";
      for (int i=0;i<3;i++)
	s << fRight.sensorVal[i] << " ";
    }
  logBrowser->append(s.str().c_str());
  updateCounter();
  
}

void
Tracker_Calibration_impl::ResetCalib_clicked()
{
  fcountLeft=0;
  fcountRight=0;
  if (coordCountLeft>0)
    {
      std::stringstream xl,yl,zl;
      xl << fulcrumarrayLeft[fcountLeft].worldVal[0];
      WorldX->setText(xl.str().c_str());
      yl << fulcrumarrayLeft[fcountLeft].worldVal[1];
      WorldY->setText(yl.str().c_str());
      zl << fulcrumarrayLeft[fcountLeft].worldVal[2];
      WorldZ->setText(zl.str().c_str());
    }
  if (coordCountRight>0)
    {
      std::stringstream xr,yr,zr;
      xr << fulcrumarrayRight[fcountRight].worldVal[0];
      WorldX_2->setText(xr.str().c_str());
      yr << fulcrumarrayRight[fcountRight].worldVal[1];
      WorldY_2->setText(yr.str().c_str());
      zr << fulcrumarrayRight[fcountRight].worldVal[2];
      WorldZ_2->setText(zr.str().c_str());
    }
  updateCounter();
}

void
Tracker_Calibration_impl::clearAllData()
{
  fcountLeft=0;
  fcountRight=0;
  coordCountLeft=0;
  coordCountRight=0;
  updateCounter();
}
 


  // we don'T use CORBA anymore so this is not working
  // calibration is done as usual, results are saved
  // in a file which is loaded by trackerServer
  // (haass)
void
Tracker_Calibration_impl::SendDataToTracker_clicked()
{
  /*
  tracker::allSensorCal dataLeft,dataRight;
  std::stringstream s;
  s << "Sent " << fcountLeft << " fulcrums fo left\n";
  for (int i = 0; i < fcountLeft; i++)
    s <<  fulcrumarrayLeft[i].sensorVal[0] << " "
      <<  fulcrumarrayLeft[i].sensorVal[1] << " "
      <<  fulcrumarrayLeft[i].sensorVal[2] << " -> "
      <<  fulcrumarrayLeft[i].worldVal[0] << " "
      <<  fulcrumarrayLeft[i].worldVal[1] << " "
      <<  fulcrumarrayLeft[i].worldVal[2] << "\n";
      
  s << "Sent " << fcountRight << " fulcrums fo right\n";
  for (int i = 0; i < fcountRight; i++)
    s <<  fulcrumarrayRight[i].sensorVal[0] << " "
      <<  fulcrumarrayRight[i].sensorVal[1] << " "
      <<  fulcrumarrayRight[i].sensorVal[2] << " -> "
      <<  fulcrumarrayRight[i].worldVal[0] << " "
      <<  fulcrumarrayRight[i].worldVal[1] << " "
      <<  fulcrumarrayRight[i].worldVal[2] << "\n";

  if (trackLeft)
    {
      dataLeft.length(fcountLeft);
      for (int i=0;i<fcountLeft;i++)
	dataLeft[i]=fulcrumarrayLeft[i];
      myCalLeft->myTracker->loadCalibration(dataLeft);
    }
  if (trackRight)
    {
      dataRight.length(fcountRight);
      for (int i=0;i<fcountRight;i++)
	dataRight[i]=fulcrumarrayRight[i];
      myCalRight->myTracker->loadCalibration(dataRight);
    }
  logBrowser->append(s.str().c_str());
  */
}


#if Tracker_Calibration_test
#include <stdio.h>
int main(int argc, char **argv)
{
  // This is a module-test block. You can put code here that tests
  // just the contents of this C file, and build it by saying
  //             make Tracker_Calibration_test
  // Then, run the resulting executable (Tracker_Calibration_test).
  // If it works as expected, the module is probably correct. ;-)

  fprintf(stderr, "For testing simply run the application\n");

  return 0;
}
#endif /* Tracker_Calibration_test */
