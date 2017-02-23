/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TRACKERCONSTANTS_H
#define TRACKERCONSTANTS_H

const double PI = 3.14159;

const int MIN_JOINT_RESOLUTION = 10;
const unsigned int TRACKERRAW_MAP2_TRACKERINVENTOR_DEFAULT[22] = {  /*0*/21, // wrist left/right
								/*1*/20, // wrist up/down
						       
								/*2*/3,  // thumb abdunction
								/*3*/0,  // thumb root
								/*4*/1,  // thumb middle
								/*5*/2,  // thumb top
						       
								/*6*/4,  // index root 
								/*7*/10, // index abdunction
								/*8*/5,  // index middle
								/*9*/6,  // index top
						       
								/*10*/7, // middle root
								/*11*/19,// palm abdunction,ignored
								/*12*/8, // middle middle
								/*13*/9, // middle top
						       
								/*14*/11,// ring root
								/*15*/14,// ring abdunction
								/*16*/12,// ring middle
								/*17*/13,// ring top
						       
								/*18*/15,// pinky root
								/*19*/18,// pinky abdunction
								/*20*/16,// pinky middle
								/*21*/17 // pinky top
};


//! for debug output
const char *JOINT_NAME[] = {"wrist-left-right",
			     "wrist-up-down",

			     "thumb-abdunction",
			     "thumb-root",
			     "thumb-middle",
			    "thumb-top",

			     "index-root",
			     "index-abdunction",
			     "index-middle",
			     "index-top",

			     "middle-root",
			     "middle-abdunction",
			     "middle-middle",
			     "middle-top",

			     "ring-root",
			     "ring-abdunction",
			     "ring-middle",
			     "ring-top",

			     "pinky-root",
			     "pinky-abdunction",
			     "pinky-middle",
			     "pinky-top"
};

//! Limits of joint angles. Notation in inventor model order, use
//! after permutation and before sign!!! [radiant]
const double RADLIMIT[22][2] = { /*0*/ {0, PI/2},  /*1*/ {0, PI/2},  /*2*/ {0, PI/2},  /*3*/ {0, PI/4},
				 /*4*/ {0, PI/2},  /*5*/ {0, PI/2},  /*6*/ {0, PI/2},  /*7*/ {0, PI/2},
				 /*8*/ {0, PI/2},  /*9*/ {0, PI/2}, /*10*/ {0, PI/4}, /*11*/ {0, PI/2},
				/*12*/ {0, PI/2}, /*13*/ {0, PI/2}, /*14*/ {0, PI/4}, /*15*/ {0, PI/2},
				/*16*/ {0, PI/2}, /*17*/ {0, PI/2}, /*18*/ {0, PI/4}, /*19*/ {0, PI/2},
				/*20*/ {0, PI/2}, /*21*/ {0, PI/2}
};

//! Sign of angle. Notation in inventor model order, use after permutation!!!
const int RADSIGN[22] = { /*0*/ 1,  /*1*/ 1,  /*2*/ 1,  /*3*/ 1,
			  /*4*/ 1,  /*5*/ 1,  /*6*/ 1,  /*7*/ 1,
			  /*8*/ 1,  /*9*/ 1, /*10*/ 1, /*11*/ 1,
			 /*12*/ 1, /*13*/ 1, /*14*/ 1, /*15*/ 1,
			 /*16*/ 1, /*17*/ 1, /*18*/ 1, /*19*/ 1,
			 /*20*/ 1, /*21*/ 1
};

#endif
