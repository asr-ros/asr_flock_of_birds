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
#include <boost/program_options.hpp>
#include <iostream>
/* my includes */
/* (none) */
#include "BirdTrack_impl.h"
#include "tracker_impl.h"

/*
  \brief Start the Flock of Bird Server 
*/
unsigned int debugLevel;
const static char* LEFT_DEFAULT_NAME = "TrackLeft";
const static char* RIGHT_DEFAULT_NAME = "TrackRight";
const static char* SERIAL_PORT = "/dev/ttyS0";

namespace po = boost::program_options;


int main(int argc, char **argv)
{
  std::string leftname, rightname, dbName, filename, calibFile, device;
  bool do_loop=true;

  po::options_description desc("Usage : tracker [options]", 120);
  desc.add_options()
    ( "help,h","show help screen")
    ( "dbname",po::value<std::string>(&dbName)->default_value(""),"database name (on localhost) NOT IMPLEMENTED YET")
    ( "device",po::value<std::string>(&device)->default_value(SERIAL_PORT),"serial device, e.g. /dev/ttyS0")
    ( "register-as-left",po::value<std::string>(&leftname)->default_value(LEFT_DEFAULT_NAME),"<left-tracker-name>" )
    ( "register-as-right",po::value<std::string>(&rightname)->default_value(RIGHT_DEFAULT_NAME),"<right-tracker-name>" ) 
    ( "no-left","Don't use left tracker" ) 
    ( "no-right","Don't use right tracker " )
    ( "calib-file",po::value<std::string>(&calibFile)," <filename>")
    ( "debug-level,d",po::value<unsigned int>(&debugLevel)->default_value(0),"the more, the higher ;)")
    //    ( "read-from-file <filename> ",po::value< vector<string> >()->composing(),"to read data from file and NOT from tracker NOT IMPLEMENTED" )
    ;


  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc <<endl;
    return 0;
  }
  
  dbName = vm["dbname"].as<std::string>();
  leftname = vm["register-as-left"].as<std::string>();
  rightname = vm["register-as-right"].as<std::string>();
  
  // Create new instance of your object.
  BirdTrack_impl *fob = new BirdTrack_impl(device);
  //fob->initialize_Tracker();
  //fob->start();

  tracker_impl *lefttracker;
  tracker_impl *righttracker;
  
  if(!vm.count("no-left")){
    lefttracker = new tracker_impl(leftTracker, fob, leftname);
    if(vm.count("calib-file"))
      lefttracker->loadCalibFile(calibFile.c_str());
    lefttracker->start();
  }
    if(!vm.count("no-right")){
      righttracker = new tracker_impl(rightTracker, fob, rightname);
      if(vm.count("calib-file"))
	righttracker->loadCalibFile(calibFile.c_str());
      righttracker->start();
  }
  
  if(!vm.count("no-left")){
    cout << "Published name for left tracker will be " << leftname.c_str() << endl;
  }
  if(!vm.count("no-right")){
    cout << "Published name for right tracker will be " << rightname.c_str() << endl;
  }

  while (do_loop) {
    std::string command;
    bool res;
    cin >> command;
    if (command.find("reload") != command.npos) {
      cout << "reloading calibration from file "<< calibFile <<endl;
      cout << "reloading calibration left:";
      res = lefttracker->loadCalibFile(calibFile.c_str());
      cout << (res?"Success":"Failure!") << endl;
      cout << "reloading calibration right:";
      res = righttracker->loadCalibFile(calibFile.c_str());
      cout << (res?"Success":"Failure!") << endl;
    }

  }
  // Main loop has finished; delete your object.
  
  if(!vm.count("no-left")) {
    lefttracker->stop();
    delete lefttracker;
  }
  if(!vm.count("no-right")) {
    righttracker->stop();
    delete righttracker;
  }
  delete fob;
}
