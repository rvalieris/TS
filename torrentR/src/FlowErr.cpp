/* Copyright (C) 2012 Ion Torrent Systems, Inc. All Rights Reserved */

#include <Rcpp.h>
#include "hdf5.h"

using namespace std;

// Read in H5 file generated by FlowErrTest, and return contents to R.

RcppExport SEXP LoadFlowErr(SEXP RFileName)
{
    // Open the H5 file, and get to the dataset:
    const char* fileName = Rcpp::as<const char*>(RFileName);

    hid_t file = H5Fopen(fileName, H5F_ACC_RDONLY, H5P_DEFAULT);
    if(file<0){
      cerr << "H5Fopen unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    hid_t dset = H5Dopen(file, "/dset", H5P_DEFAULT);
    if(dset<0){
      cerr << "H5Dopen unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    // Get dimensions for the data set:
    hid_t space = H5Dget_space(dset);
    if(space<0){
      cerr << "H5Dget_space unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    int ndims = H5Sget_simple_extent_ndims(space);
    if(ndims!=3){
      cerr << "Error: H5Sget_simple_extent_ndims returned " << ndims << endl;
      exit(EXIT_FAILURE);
    }

    hsize_t dims[ndims];
    hsize_t maxDims[ndims];
    int result = H5Sget_simple_extent_dims(space, dims, maxDims);
    if(result==0){
      cerr << "H5Sget_simple_extent_dims unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    int numFlows  = dims[0];
    int maxReadHP = dims[1];
    int maxRefHP  = dims[2];

    // Read in the data set:
    int counts[numFlows][maxReadHP][maxRefHP];
    size_t nelem  = numFlows * maxReadHP * maxRefHP;
    herr_t status = H5Dread(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, counts);
    if (status < 0){
      cout << "H5Dread unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    // Change to R-style column-major order:
    std::vector<int> Rcounts(nelem);
    for(int i=0; i<numFlows; ++i){
        for(int j=0; j<maxReadHP; ++j){
            for(int k=0; k<maxRefHP; ++k){
                Rcounts[k*numFlows*maxReadHP + j*numFlows + i] = counts[i][j][k];
            }
        }
    }

    // Close file:
    status = H5Dclose(dset);
    if (status < 0){
      cout << "H5Dclose unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    status = H5Fclose(file);
    if (status < 0){
      cout << "H5Fclose unsuccessful." << endl;
      exit(EXIT_FAILURE);
    }

    // Package results for return to R:
    std::map<std::string,SEXP> map;
    map["numFlows"]  = Rcpp::wrap( numFlows );
    map["maxReadHP"] = Rcpp::wrap( maxReadHP );
    map["maxRefHP"]  = Rcpp::wrap( maxRefHP );
    map["counts"]    = Rcpp::wrap( Rcounts );

    return Rcpp::wrap( map );
}

