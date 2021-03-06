//
//  IO.h
//  DBN
//
//  Created by Devon Hjelm on 7/23/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef DBN_IO_h
#define DBN_IO_h
#include "Types.h"
#include <stdint.h>
#include <arpa/inet.h>

class Layer;

class DataSet{
public:
   std::string name;
   int height, width, number, masksize;
   Input_t *train, *test, *validation, *extra;
   
   Layer *input_layer;
   
   bool applymask;
   bool denorm;
   
   gsl_vector_float *meanImage;
   gsl_vector_float *mask;
   gsl_vector_float *norm;
   
   DataSet(){
      masksize = 0;
      mask = NULL;
      meanImage = NULL;
   }
   
   void loadMNIST();
   void loadfMRI(bool,bool,bool);
   void loadSPM();
   void loadstim();
   void splitValidate(float percentage = .1);
   void removeMeanImage();
   void getMask();
   void removeMask();
   void normalize();
   gsl_vector_float *applyMask(gsl_vector_float *v);
};

#endif
