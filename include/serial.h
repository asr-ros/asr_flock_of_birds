/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef SERIAL_H
#define SERIAL_H


/******************************************************************************
 *** INCLUDES
 ******************************************************************************
 */

#include <termio.h>
#include <sys/termios.h>

/******************************************************************************
 *** PROTOTYPES der von sick.c verwendeten Funktionen
 ******************************************************************************
 */

/******************************************************************************
 *** MY DEFINES
 ******************************************************************************
 */
#define STATUS_OK    1
#define STATUS_ERROR 0


class SerialDevice 
{ 
  int tty_fd_slot;
  char device_name[255];

 public:
  SerialDevice();
  ~SerialDevice();

  int  open_serial (const char *p_tty_name);
  long change_baud_serial (int tty_fd, speed_t speed);
  long write_serial(int tty_fd, unsigned char *p_buffer, long nb_byte);
  long read_serial(int tty_fd, unsigned char *p_buffer, long nb_bytes_max);
  long wait_for_serial (int tty_fd, long max_time_secs);
  void close_serial (int tty_fd);
  long empty_serial (int tty_fd);
  
 protected:
 
};
#endif













