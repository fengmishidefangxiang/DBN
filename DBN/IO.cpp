//
//  IO.cpp
//  DBN
//
//  Created by Devon Hjelm on 7/23/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "IO.h"
#include "SupportFunctions.h"

void DataSet::loadMNIST(){
   
   std::cout << "Loading MNIST" << std::endl;
   
   name = "MNIST";
   
   meanImage = NULL;
   mask = NULL;
   
   std::string trainfile = MNISTpath + "train-images.idx3-ubyte";
   std::string testfile = MNISTpath + "t10k-images.idx3-ubyte";
   FILE *file_handle;
   file_handle = fopen(trainfile.c_str(), "rb");
   
   unsigned char pixel; 
   uint32_t magicnumber, imageNum, rowNum, colNum;
   
   fread(&magicnumber, sizeof(magicnumber), 1, file_handle);
   fread(&imageNum, sizeof(imageNum), 1, file_handle);
   fread(&rowNum, sizeof(rowNum), 1, file_handle);
   fread(&colNum, sizeof(colNum), 1, file_handle);
   
   magicnumber = ntohl(magicnumber);
   imageNum = ntohl(imageNum);
   rowNum = ntohl(rowNum);
   colNum = ntohl(colNum);
   
   train = gsl_matrix_float_alloc(imageNum, colNum*rowNum);
   
   height=rowNum, width = colNum, number = imageNum;
   
   for (int i = 0; i < imageNum; ++i)
      for (int j = 0; j < rowNum*colNum; ++j){
         fread(&pixel, sizeof(pixel), 1, file_handle);
         int val = (int)pixel;
         gsl_matrix_float_set(train, i, j, (float)val);
      }
   
   fclose(file_handle);
   file_handle = fopen(testfile.c_str(), "rb");
   
   fread(&magicnumber, sizeof(magicnumber), 1, file_handle);
   fread(&imageNum, sizeof(imageNum), 1, file_handle);
   fread(&rowNum, sizeof(rowNum), 1, file_handle);
   fread(&colNum, sizeof(colNum), 1, file_handle);
   
   magicnumber = ntohl(magicnumber);
   imageNum = ntohl(imageNum);
   rowNum = ntohl(rowNum);
   colNum = ntohl(colNum);
   
   test = gsl_matrix_float_alloc(imageNum, colNum*rowNum);
   
   for (int i = 0; i < imageNum; ++i)
      for (int j = 0; j < rowNum*colNum; ++j){
         fread(&pixel, sizeof(pixel), 1, file_handle);
         int val = (int)pixel;
         gsl_matrix_float_set(test, i, j, (float)val);
      }
   
   fclose(file_handle);
   
   applymask = false;
   
   std::cout << "Done loading" << std::endl;
   
   denorm = false;
}

void DataSet::loadfMRI(bool d1, bool d2, bool d3){
   
   number = (d1+d2+d3)*220;
   width = 53;
   height = 63;
   
   train = gsl_matrix_float_alloc(number, height*width);
   
   std::cout << "Loading fMRI" << std::endl;
   
   name = "fMRI";
   
   std::string filename, pathname;
   
   struct dirent *filep;
   struct stat filestat;
   std::ifstream file;
   DIR *dir;
   int sample = 0;
   
   std::string path;
   extra = gsl_matrix_float_calloc(220, width*height);
   meanImage = gsl_vector_float_calloc(train->size2);
   norm = gsl_vector_float_calloc(train->size2);
   for (int s = 1; s <=3; ++s) {
      if ((s == 1 && d1) || (s == 2 && d2) || (s==3 && d3) ) {
         
         std::stringstream n;
         n << s;
         path = fMRIpath + n.str() + "/";
         dir = opendir(path.c_str());
         while ((filep = readdir(dir))){
            filename = filep->d_name;
            pathname = path + filep->d_name;
            
            // If the file is a directory (or is in some way invalid) we'll skip it
            if (stat( pathname.c_str(), &filestat )) continue;
            if (S_ISDIR( filestat.st_mode ))         continue;
            if (filename == ".DS_Store")             continue;
            
            //cout << "Loading " << filename << endl;
            
            file.open(pathname.c_str());
            
            std::string line;
            int index = 0;
            while (getline(file, line)){
               float value;
               std::istringstream iss(line);
               while (iss >> value) {
                  gsl_matrix_float_set(extra, sample%220, index, value);
                  ++index;
               }
            }
            file.close();
            ++sample;
         }
         closedir(dir);
         removeMeanImage();
         normalize();
         for (int i = 0; i < extra->size1; ++i)
            for (int j = 0; j < extra->size2; ++j) {
               float val = gsl_matrix_float_get(extra, i, j);
               gsl_matrix_float_set(train, sample-220+i, j, val);
            }
      }
   }
   //removeMeanImage();
   removeMask();
   applymask = true;
   gsl_matrix_float_free(extra);
   
   extra = gsl_matrix_float_alloc(train->size1, train->size2);
   gsl_matrix_float_memcpy(extra, train); //This is for time courses since I don't preserve data order in training.
   denorm = true;
}

void DataSet::loadSPM(){
   number = 220;
   width = 2;
   height = 1;
   train = gsl_matrix_float_alloc(number, width*height);
   std::cout << "Loading stimulus" << std::endl;
   
   std::string filename, pathname;
   std::ifstream file;
   
   int sample = 0;
   
   pathname = SPMpath + "SPM.dat";
   
   file.open(pathname.c_str());
   
   std::string line;
   while (getline(file, line)){
      float value;
      int index = 1;
      std::istringstream iss(line);
      while (iss >> value) {
         if (index == 1 || index == 2)
            for (int i = 0; i < width/2; ++i){
               gsl_matrix_float_set(train, sample, index-1+(2*i), value);
            }
         ++index;
      }
      ++sample;
   }
   file.close();
}

void DataSet::loadstim(){
   number = 220;
   width = 2;
   height = 1;
   train = gsl_matrix_float_alloc(number, width*height);
   std::cout << "Loading stimulus" << std::endl;
   
   std::string filename;
   std::ifstream file;
   std::string line;
   int sample = 0;
   
   filename = SPMpath + "l_stim.dat";
   file.open(filename.c_str());
   
   while (getline(file, line)){
      float value;
      std::istringstream iss(line);
      while (iss >> value) gsl_matrix_float_set(train, sample, 0, value);
      ++sample;
   }
   file.close();
   
   sample = 0;
   filename = SPMpath + "r_stim.dat";
   file.open(filename.c_str());
   
   while (getline(file, line)){
      float value;
      std::istringstream iss(line);
      while (iss >> value) gsl_matrix_float_set(train, sample, 1, value);
      ++sample;
   }
   file.close();
   gsl_matrix_float_scale(train, 10);
}

void DataSet::removeMeanImage(){
   Input_t *input = extra;
   gsl_vector_float *sample = gsl_vector_float_alloc(input->size2);
   gsl_vector_float_set_zero(meanImage);
   for (int i = 0; i < input->size1; ++i){
      gsl_matrix_float_get_row(sample, input, i);
      gsl_vector_float_add(meanImage, sample);
   }
   
   gsl_vector_float_scale(meanImage, (float)1/(float)input->size1);
   
   for (int i = 0; i < input->size1; ++i){
      gsl_matrix_float_get_row(sample, input, i);
      gsl_vector_float_sub(sample, meanImage);
      gsl_matrix_float_set_row(input, i, sample);
   }
   float min, max;
   gsl_matrix_float_minmax(input, &min, &max);
   
   gsl_vector_float_free(sample);
   
}

void DataSet::removeMask(){
   int count = 0;
   float mean = gsl_stats_float_mean(meanImage->data, meanImage->stride, meanImage->size);
   
   for (int i = 0; i < meanImage->size; ++i) count += (gsl_vector_float_get(meanImage, i) > mean);
   
   gsl_matrix_float *newtrain = gsl_matrix_float_alloc(number, count);
   
   mask = gsl_vector_float_alloc(height*width);
   masksize = height*width-count;
   
   for (int j = 0; j < height*width; ++j){
      float val = gsl_vector_float_get(meanImage, j);
      if (val > mean) gsl_vector_float_set(mask, j, 1);
      else gsl_vector_float_set(mask, j, 0);
   }

   for (int i = 0; i < number; ++i){
      int jprime = 0;
      for (int j = 0; j < height*width; ++j){
         float maskval = gsl_vector_float_get(mask, j);
         if (maskval == 1) {
            float val = gsl_matrix_float_get(train, i, j);
            gsl_matrix_float_set(newtrain, i, jprime, val);
            ++jprime;
         }
      }
   }
   
   gsl_matrix_float_free(train);
   train = newtrain;
}

gsl_vector_float *DataSet::applyMask(gsl_vector_float *v){
   gsl_vector_float *newv = gsl_vector_float_calloc(v->size + masksize);
   for (int i = 0, iprime = 0; i < newv->size; ++i) {
      float maskval = gsl_vector_float_get(mask, i);
      if (maskval == 1) {
         float val = gsl_vector_float_get(v, iprime);
         gsl_vector_float_set(newv, i, val);
         ++iprime;
      }
   }
   gsl_vector_float_free(newv);
   return newv;
}

void DataSet::normalize(){
   Input_t *input = extra;
   float mean = gsl_stats_float_mean(input->data, 1, input->size1*input->size2);
   float sd = gsl_stats_float_sd(input->data, 1, input->size1*input->size2);
   gsl_matrix_float_add_constant(input, -mean);
   gsl_matrix_float_scale(input, (float)1/sd);
}