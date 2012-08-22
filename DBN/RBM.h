//
//  RBM.h
//  DBN
//
//  Created by Devon Hjelm on 7/19/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef DBN_RBM_h
#define DBN_RBM_h
#include "Connections.h"
#include "SupportFunctions.h"
#include "Viz.h"

class ContrastiveDivergence;

class RBM {
public:
   float freeEnergy_, reconstructionCost_;
   Connection *c1_, *c2_;
   Activator *up_act_;
   Activator *down_act_;
   
   DataSet *ds1_;
   DataSet *ds2_;
   
   ~RBM(){}
   RBM(){}
   RBM(Connection*);
   RBM(Connection*, Connection*);
   
   void getFreeEnergy();
   void gibbs_HV();
   void gibbs_VH();
   void getReconstructionCost(Input_t *input);
   void sample(DataSet *data, Visualizer *viz);
   
   void makeBatch(int batchsize);
   void expandBiases();
   void get_dims(float *topdim, float *botdim);
   
   void update(ContrastiveDivergence*);
   
   void catch_stats(Stat_flag_t s){
      c1_->catch_stats(s);
      if (c2_ != NULL) c2_->catch_stats(s);
      
   }
   
   void load_DS(DataSet*, DataSet*);
   void load_input_batch(int index);
   void init_DS();
   void visualize(float st1, float st2);
};





#endif
