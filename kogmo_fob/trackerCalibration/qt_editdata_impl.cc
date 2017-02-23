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
#include <qdialog.h>
#include <qtable.h>
#include <qmessagebox.h>
#include <qheader.h>

/* my includes */
#include "qt_editdata_impl.h"

qt_editdata_impl::qt_editdata_impl()
{
  
}

qt_editdata_impl::qt_editdata_impl(QWidget* parent,
				   const char* name,
				   bool modal,
				   WFlags fl )
  : qt_editdata(parent, name, modal, fl)
{
  updateCal();
}


qt_editdata_impl::~qt_editdata_impl()
{
}


bool
qt_editdata_impl::setTrackerCal(cData::trackerCal t){

  tcal = t;
  updateCal();
  return true;
}


cData::trackerCal
qt_editdata_impl::getTrackerCal(){

  return tcal;
}

bool
qt_editdata_impl::updateCal(){

  // determine number of lines (sensors)
  unsigned int numrows = tcal.size();
  unsigned int numcols = 0;
  // number of cols is MAX(all line lenghts)
  for (unsigned int i=0; i<numrows; i++)
    numcols = tcal[i].size() > numcols ? tcal[i].size() : numcols;

  datatable->setNumRows(numrows);
  datatable->setNumCols(2*numcols);

  // loop through all cols
  for (unsigned int j=0; j<numcols; j+=1){
    char outp[12], inp[12];
    sprintf(outp, "Output %d", j+1);
    sprintf(inp, "Input %d", j+1);
    datatable->horizontalHeader()->setLabel( j*2, outp);
    datatable->horizontalHeader()->setLabel( j*2+1, inp);
  }
  
  // loop through all lines
  for (unsigned int i=0; i<numrows; i++){
    // loop through all cols
    for (unsigned int j=0; j<tcal[i].size(); j++){
      // enter numbers
      datatable->setItem( i, 2*j,
			  new QTableItem( datatable,
					  QTableItem::WhenCurrent,
					  QString::number( tcal[i][j].first ) ) );
      datatable->setItem( i, 2*j+1,
			  new QTableItem( datatable,
					  QTableItem::WhenCurrent,
					  QString::number( tcal[i][j].second ) ) );
    }
  }
  return true;
}

void
qt_editdata_impl::accept(){

  cData::trackerCal t;
  bool ok = true;
  unsigned int numrows = datatable->numRows();
  unsigned int numcols = datatable->numCols();

  // loop through all lines
  for (unsigned int i=0; i<numrows; i++){
    cData::sensorCal scal;
    // loop through all cols
    for (unsigned int j=0; j<numcols; j+=2){
      bool oktmp;
      cData::fulcrum f;
      f.first = datatable->text(i,j).toDouble(&oktmp);
      ok = ok && oktmp;
      f.second = datatable->text(i,j+1).toDouble(&oktmp);
      ok = ok && oktmp;
      scal.push_back(f);
    }
    t.push_back(scal);
  }
  if(ok){
    tcal = t;
    QDialog::accept();
  }
  else{
    QMessageBox::critical(this,
			  "Calibration data",
			  "Data is corrupt. Cannot store it.\nOnly numbers are allowed.",
			  "Ok");
  }
}



#if qt_editdata_test
#include <stdio.h>
int main(int argc, char **argv)
{
  // This is a module-test block. You can put code here that tests
  // just the contents of this C file, and build it by saying
  //             make qt_editdata_test
  // Then, run the resulting executable (qt_editdata_test).
  // If it works as expected, the module is probably correct. ;-)

  fprintf(stderr, "Testing qt_editdata\n");

  return 0;
}
#endif /* qt_editdata_test */
