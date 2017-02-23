/**

Copyright (c) 2016, Bernhardt Andre, Engelmann Stephan, Giesler Björn, Heller Florian, Jäkel Rainer, Nguyen Trung, Pardowitz Michael, Weckesser Peter, Yi Xie, Zöllner Raoul
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include "serial.h"
#include <string.h>
#include <errno.h>

SerialDevice::~SerialDevice(){
  if (tty_fd_slot != -1) {
    empty_serial (tty_fd_slot);
    close_serial (tty_fd_slot);
  }
}
SerialDevice::SerialDevice(){
  tty_fd_slot = -1;
  strcpy(device_name,"");
}


/******************************************************************************
 ***
 *** Name:       open_serial
 ***
 *** Purpose:    oeffnet ein serielles Device
 ***
 *** Parameters: devicename (z.B. "/dev/ttyS4") und flags (z.B. CS8|B9600 ) 
 ***
 *** Returns:    filedescriptor, bzw. -1 im Fehlerfall
 ***
 ******************************************************************************
 */

int  SerialDevice::open_serial (const char *p_tty_name)
{
   struct termio tty_set;
   int tty_fd = -1;
    
   if ( (tty_fd = open(p_tty_name, O_RDWR |O_NOCTTY)) < 0 )//NONBLOCK
     {
       printf("ERROR: Couldn't open port %s!\n", p_tty_name);
       perror(NULL);
       return (-1);
     }
   
   if (ioctl(tty_fd, TCGETA, &tty_set ) == -1 )
     {
       printf("ERROR: ioctl() failed to read settings!\n");
       perror(NULL);
       return (-1);
     }
   
   tty_set.c_cflag = (B115200 | CS8 | CLOCAL | CREAD);
   tty_set.c_iflag = IXOFF;
   tty_set.c_lflag = 0;
   tty_set.c_oflag = 0;
   tty_set.c_cc[VTIME]    = 20;
   tty_set.c_cc[VMIN]     = 0;

   if (ioctl(tty_fd, TCSETAW, &tty_set ) == -1 ) 
     {
       printf("ERROR: ioctl() failed to set settings!\n");
       perror(NULL);
       return (-1);
     }

   tty_fd_slot = tty_fd;
   strcpy(device_name,p_tty_name);
   return (tty_fd);
}

/******************************************************************************
 ***
 *** Name:       change_baud_serial
 ***
 *** Purpose:    stellt die Geschwindigkeit der seriellen Schnittstelle um
 ***
 *** Parameters: filedescriptor und neue Geschwindigkeit (z.B. B38400)
 ***
 *** Returns:    STATUS_OK, bzw. STATUS_ERROR im Fehlerfall
 ***
 ******************************************************************************
 */

long SerialDevice::change_baud_serial (int tty_fd, speed_t speed)
 {
   struct termio tty_set;

   if (ioctl(tty_fd, TCGETA, &tty_set ) == -1 ) 
     {
       printf("ERROR: ioctl() failed to read settings!\n");
       perror(NULL);
       return (STATUS_ERROR);
     }
   
   tty_set.c_cflag &= ~CBAUD;
   tty_set.c_cflag |= speed;
   
   if (ioctl(tty_fd, TCSETAW, &tty_set ) == -1 ) 
     {
       printf("ERROR: ioctl() failed to set settings!\n");
       perror(NULL);
       return (STATUS_ERROR);
     }

   return (STATUS_OK);
 }

/******************************************************************************
 ***
 *** Name:       write_serial
 ***
 *** Purpose:    Schickt Daten an die serielle Schnittstelle
 ***
 *** Parameters: filedescriptor, Pointer auf Speicher, Zahl der zu versendenden bytes
 ***
 *** Returns:    STATUS_OK, bzw. STATUS_ERROR im Fehlerfall
 ***
 ******************************************************************************
 */

long SerialDevice::write_serial(int tty_fd, unsigned char *p_buffer, long nb_byte)
 {
   int bytes_out;

   if (( bytes_out = write(tty_fd, p_buffer, nb_byte)) < nb_byte)
     {
       printf("ERROR: write() failed!\n");
       perror(NULL);
       return (STATUS_ERROR);
     };

   return (STATUS_OK);
 }

/******************************************************************************
 ***
 *** Name:       read_serial
 ***
 *** Purpose:    Liest Daten von der seriellen Schnittstelle. Liegen gerade
 ***             keine Daten an, wird eine 20tel Sekunde darauf gewartet!
 ***
 *** Parameters: filedescriptor, Pointer auf Speicherbereich, maximale 
 ***             Zahl der zu lesenden bytes
 ***
 *** Returns:    Zahl der gelesenen bytes
 ***
 ******************************************************************************
 */

long SerialDevice::read_serial(int tty_fd, unsigned char *p_buffer, long nb_bytes_max)
 {
   struct timeval tz;
   fd_set fds;
   int tries       = 0;
   int qu_new      = 0;
   int bytes_read  = 0;
   int ready       = STATUS_ERROR;
  
   /* read max nb_bytes_max bytes */

   while (tries < 2 && !ready)
     {
       tz.tv_sec  = 1;
       tz.tv_usec = 0;
       
       FD_ZERO(&fds);
       FD_SET(tty_fd, &fds);
       if (select(FD_SETSIZE, &fds, 0, 0, &tz) > 0)
	 {
	   qu_new = read(tty_fd, &p_buffer[bytes_read], (nb_bytes_max - bytes_read));
	   if (qu_new > 0)
	     bytes_read += qu_new;
	   else
	     tries++;
	   ready = (bytes_read == nb_bytes_max);
	 }
       else
	 tries++;
     }

   return(bytes_read);
 }

/******************************************************************************
 ***
 *** Name:      close_serial
 ***
 *** Purpose:   schliesst das serielle device
 ***
 *** Parameter: filedescriptor
 ***
 ******************************************************************************
 */

void SerialDevice::close_serial (int tty_fd)
 {
   close(tty_fd);
 }

/******************************************************************************
 ***
 *** Name:       wait_for_serial
 ***
 *** Purpose:    Wartet maximal eine vorgegebene Zeit auf das Eintreffen von 
 ***             Daten an der seriellen Schnittstelle
 ***
 *** Parameters: filedescriptor, maximale Wartezeit
 ***
 *** Returns:    STATUS_OK wenn Daten anliegen, bzw. STATUS_ERROR wenn nicht
 ***
 ******************************************************************************
 */

long SerialDevice::wait_for_serial(int tty_fd, long max_time_secs)
 {
   struct timeval tz;
   fd_set fds;

   while (max_time_secs--)
     {
       tz.tv_sec  = 1;
       tz.tv_usec = 0;
       
       FD_ZERO (&fds);
       FD_SET(tty_fd, &fds);
       
       if (select(FD_SETSIZE, &fds, 0, 0, &tz) > 0)
	 return (STATUS_OK);
     }  

   return (STATUS_ERROR);
 }


/******************************************************************************
 ***
 *** Name:       empty_serial
 ***
 *** Purpose:    Loescht den Eingangspuffer der seriellen Schnittstelle
 ***
 *** Caveats:    Noch nicht getestet... (see manpage zu termios)
 ***
 *** Parameters: filedescriptor
 ***
 *** Returns:    STATUS_OK, bzw. STATUS_ERROR im Fehlerfall
 ***
 ******************************************************************************
 */

long SerialDevice::empty_serial(int tty_fd)
 {
   if (-1 == tcflush (tty_fd, TCIFLUSH))
     return (STATUS_ERROR);
   else
     return (STATUS_OK);
 }

















