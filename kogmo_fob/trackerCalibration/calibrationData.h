/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef CALIBRATIONDATA_H
#define CALIBRATIONDATA_H


const double GLOVE_RIGHT_VEC_1[22] = {/*0*/0.3, // wrist left
				      /*1*/0.9, // wrist up
						       
// 				      /*2*/-0.5, // thumb abd closed
				      /*2*/0.26, // thumb abd closed
				      /*3*/0.0,  // thumb root open
				      /*4*/0.0,  // thumb middle open
				      /*5*/0.0,  // thumb top open
						       
				      /*6*/0.0,  // index root open 
				      /*7*/-0.2, // index abd closed
				      /*8*/0.0,  // index middle open
				      /*9*/0.0,  // index top open
						       
				      /*10*/0.0, // middle root open
				      /*11*/0.0,// palm adb ignored
				      /*12*/0.0, // middle mid open
				      /*13*/0.0, // middle top open
						       
				      /*14*/0.0,// ring root open
				      /*15*/0.27,// ring abd closed
				      /*16*/0.0,// ring middle open
				      /*17*/0.0,// ring top open
						       
				      /*18*/0.0,// pinky root open
				      /*19*/0.5,// pinky abd closed
				      /*20*/0.0,// pinky middle open
				      /*21*/0.0 // pinky top open
};

const double GLOVE_RIGHT_VEC_2[22] = {  /*0*/-0.5, // wrist right
					/*1*/-0.7, // wrist down
						       
// 					/*2*/-0.2,  // thumb abd ok
					/*2*/1.3,  // thumb abd ok
// 					/*3*/-0.4,  // thumb root ok
					/*3*/-1.0,  // thumb root ok
					/*4*/-0.9,  // thumb middle ok
					/*5*/-0.8,  // thumb top ok
						       
					/*6*/-1.5,  // index root closed 
					/*7*/0.2, // index abd open
					/*8*/-1.7,  // index middle closed
					/*9*/-1.3,  // index top closed
						       
					/*10*/-1.5, // middle root closed
					/*11*/0.0,// palm abd ignored
					/*12*/-1.7, // middle mid closed
					/*13*/-1.3, // middle top closed
						       
					/*14*/-1.5,// ring root closed
					/*15*/0.0,// ring abd open
					/*16*/-1.7,// ring middle closed
					/*17*/-1.3,// ring top closed
						       
					/*18*/-1.5,// pinky root closed
					/*19*/-0.2,// pinky abd open
					/*20*/-1.7,// pinky middle closed
					/*21*/-1.3 // pinky top closed
};


#endif
