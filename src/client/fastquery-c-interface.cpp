/**
 * *** Copyright Notice ***
 * SDS - Scientific Data Services framework, Copyright (c) 2015, The Regents of the University of California, through Lawrence Berkeley National Laboratory (subject to receipt of any required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * If you have questions about your rights to use or distribute this software, 
 * please contact Berkeley Lab's Technology Transfer Department at TTD@lbl.gov.
 * 
 * NOTICE.  This software was developed under funding from the 
 * U.S. Department of Energy.  As such, the U.S. Government has been granted 
 * for itself and others acting on its behalf a paid-up, nonexclusive, 
 * irrevocable, worldwide license in the Software to reproduce, prepare 
 * derivative works, and perform publicly and display publicly.  
 * Beginning five (5) years after the date permission to assert copyright is 
 * obtained from the U.S. Department of Energy, and subject to any subsequent 
 * five (5) year renewals, the U.S. Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide license
 * in the Software to reproduce, prepare derivative works, distribute copies to
 * the public, perform publicly and display publicly, and to permit others to
 * do so.
 *
*/

/**
 *
 * Email questions to {dbin, sbyna, kwu}@lbl.gov
 * Scientific Data Management Research Group
 * Lawrence Berkeley National Laboratory
 *
*/
#include "fastquery-c-interface.h"
#include "queryProcessor.h"
#include <iostream>

char *condstring = 0;
std::string datafile;
std::string logfile;
std::string conffile;
std::string indexfile;
std::string dsetfile;
char *fileFormat = 0;
std::string varPath;
std::vector<const char *> varNames;
bool useBoxSelection = false;
bool equalityTest = false;
bool xport = false;
int mpi_len = 1000;
int mpi_dim = 0;

unsigned int fq_multi_dsets(char *qstr, char *filename, char *group, const char **dataset, int datasetcount, char *indexfilename, int mpi_length, SDS_Value_union **buf, unsigned int *fq_hits, char *other){
  int i, mpi_rank;

  //Here we assume the objects in "coll" are datasets in the same HDF5 group
  datafile   = filename;
  indexfile  = indexfilename;
  condstring = qstr;
  varPath    = group;
  mpi_len    = mpi_length;

  //MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
#ifndef FQ_NOMPI
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  std::cout << "FastQuery MPI version" << std::endl;
#else
  mpi_rank = 0;
  std::cout << "FastQuery No MPI version" << std::endl;
#endif

  ibis::gVerbose = 0;
  
  //for(i = 0; i < datasetcount; i++){
  //dsetfile = dataset[i];
  // varNames.push_back(dataset[i]);
  //if (ibis::gVerbose >=0)
  //   std::cout << "vname : " << varNames[i] << " parameter :  "  <<  dataset[i] << " # of dsets"<< datasetcount << std::endl;
  //}
  
  if(ibis::gVerbose > 0){
    std::cout << "dfile : " << datafile << std::endl;
    std::cout << "pth   : " << varPath  << std::endl;
    
    std::cout << "ifile : " << indexfile << std::endl;
    std::cout << "qstr  : " << qstr  << std::endl;
  }
  
  logfile = "log.query";

  FQ::FileFormat ffmt = FQ::FQ_HDF5;
  if (fileFormat != 0) {
    std::string format = fileFormat;
    if (format.compare("HDF5") == 0) {
      ffmt = FQ::FQ_HDF5;
    } else if (format.compare("H5PART") == 0) {
      ffmt = FQ::FQ_H5Part;
    } else if (format.compare("NETCDF") == 0) {
      ffmt = FQ::FQ_NetCDF;
    } else if (format.compare("PNETCDF") == 0) {
      ffmt = FQ::FQ_pnetCDF;
    }
  }

  if (ibis::gVerbose > 1) {
    ibis::util::logger lg;
    lg() << "fq_query" << " data file \"" << datafile << "\"";
    if (! indexfile.empty())
      lg() << "\tindexfile \"" << indexfile.c_str() << "\"";
    if (! varPath.empty())
      lg() << "\tvariable path \"" << varPath << "\"";
    if (! varNames.empty())
      lg() << "\twith "  << varNames.size() << " variable name"
           << (varNames.size()>1?"s":"") << " ...";
  }

  ibis::util::timer totTimer("fq_query", 1);
  // open the named file

#ifndef FQ_NOMPI
  double t0, t1, t2; 
  MPI_Barrier(MPI_COMM_WORLD);
  t0 = MPI_Wtime();
  t1 = MPI_Wtime(); 
#endif

  //printf("Init queryProcessor !\n");
  QueryProcessor* queryProcessor =
      new QueryProcessor(datafile, ffmt, indexfile, ibis::gVerbose);
  if (queryProcessor->isValid() == false) {
    LOGGER(ibis::gVerbose > 0)
        << "ERROR: failed to initiate the QueryProcessor object for file \"" 
        << datafile.c_str() << "\" ...\n";

    delete(queryProcessor);
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    return -2;
  }

  //printf("After queryProcessor !\n");

  unsigned int hits = 0;
  // getNumHits
  hits = queryProcessor->getNumHits(condstring, varPath, mpi_dim, mpi_len);
  LOGGER(ibis::gVerbose > 1)
      << "fastquery " << " processed conditions \"" << condstring 
      << "\" and produced " << hits << " hit" << (hits>1?"s":"");

  if (hits <= 0) {
    LOGGER(ibis::gVerbose > 1)
        << "Warning -- No element is seleteced by \"" << condstring << "\"";

    delete(queryProcessor);
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    return -3;
  }
  LOGGER(ibis::gVerbose >= 0)
      << "fastquery " << " successfully evaluated query \"" << condstring
      << "\" with "<< hits << " hitsss" << (hits>1?"s":"");

#ifndef FQ_NOMPI
  MPI_Barrier(MPI_COMM_WORLD);
  t2 = MPI_Wtime(); 
  if (mpi_rank == 0)
    std::cout << "new QueryProcessor, time: " << t2 - t1 << std::endl;
  t1 = MPI_Wtime();   
#endif

  //printf("Here we are before evaluate !");

  //if (varNames.empty()) return 0;
  if(datasetcount == 0) 
    return 0;
  std::string variable;
  std::vector<uint64_t> dims;
  FQ::DataType type;
  //if (!queryProcessor->getVariableInfo(varNames[0], variable, dims, &type, varPath)) {
  if (!queryProcessor->getVariableInfo(dataset[0], variable, dims, &type, varPath)) {
    if (ibis::gVerbose >= 0) {
      std::cout << "ERROR: Failed to get the information for variable \"" << variable.c_str()
                << "\" from file \"" << datafile.c_str() << "\"" << std::endl;
    }
    delete(queryProcessor);
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    return -3;
  }

  //printf("Here we are before evaluate !");

  // executeQuery
  uint64_t hits1=0;
  //printf("dims.size() %d \n", dims.size());
  std::vector<uint64_t> coords;
  if (useBoxSelection) {
    coords.reserve(hits*2*dims.size());
    hits1 = queryProcessor->executeQuery
        ((char*)condstring, coords, varPath, FQ::BOXES_SELECTION,
         mpi_dim, mpi_len);
  } else {
    coords.reserve(hits*dims.size());
    hits1 = queryProcessor->executeQuery
        ((char*)condstring, coords, varPath, FQ::POINTS_SELECTION,
         mpi_dim, mpi_len);
  }
   
  //printf("Here we are after evaluate !");
  if (hits != hits1) {
    LOGGER(ibis::gVerbose >= 0)
        << "Warning -- number of hits does not match";
    
    delete(queryProcessor);
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    return -4;
  }
  
#ifndef FQ_NOMPI
  MPI_Barrier(MPI_COMM_WORLD);
  t2 = MPI_Wtime(); 
  if (mpi_rank == 0)
    std::cout << "queryProcessor->executeQuery, time: " << t2 - t1 << std::endl;
  t1 = MPI_Wtime(); 
#endif

  //printf(" varNames.size %d \n ", varNames.size());
  //for (unsigned j = 0; j < varNames.size(); ++ j) {
  for (unsigned j = 0; j < datasetcount; ++ j) {
    std::string variable, vn;
    std::vector<uint64_t> dims;
    FQ::DataType type;
    bool berr = true;
    //vn = varNames[j];
    vn = dataset[j];
    berr = queryProcessor->getVariableInfo
        (vn, variable, dims, &type, varPath);
    if (! berr) {
      LOGGER(ibis::gVerbose > 0)
          << "Warning -- Failed to get the information for variable \""
          << variable << "\" from file \"" << datafile << "\"";

      delete(queryProcessor);
#ifndef FQ_NOMPI
      MPI_Finalize();
#endif
      return -5;
    }

    // getSelectedData
    //std::vector <double> values;
    //coll->object_array[j]->data_buffer = 
    //SDS_Value_union *result_buf;
    //result_buf= (SDS_Value_union *)(coll->object_array[j]->data_buffer);
    buf[j] = (SDS_Value_union*) malloc(sizeof(SDS_Value_union) * hits); 
    //printf("here we are before read!");
    switch(type) {
      case FQ::FQT_BYTE: {
        std::vector<char> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          std::cout << "Force [char] to [int] in fastquery-c-interface()" << std::endl;
          buf[j][i].i = data[i];
        }
        break;}
      case FQ::FQT_FLOAT: {
        std::vector<float> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          buf[j][i].f = data[i];
        }
        break;}
      case FQ::FQT_DOUBLE: {
        std::vector<double> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          buf[j][i].d = data[i];
        }
        break;}
      case FQ::FQT_INT: {
        std::vector<int32_t> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //printf("here we are !");

        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          buf[j][i].i = data[i];
        }
        break;}
      case FQ::FQT_LONG: {
        std::vector<int64_t> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          std::cout << "Force [long int] to [int] in fastquery-c-interface()" << std::endl;
          buf[j][i].i = data[i];
        }
        break;}
      case FQ::FQT_UINT: {
        std::vector<uint32_t> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          std::cout << "Force [unsigned int] to [int] in fastquery-c-interface()" << std::endl;
          buf[j][i].i = data[i];
        }
        break;}
      case FQ::FQT_ULONG: {
        std::vector<uint64_t> data;
        data.resize(hits);
        if (useBoxSelection) {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath, FQ::BOXES_SELECTION);
        } else {
          berr = queryProcessor->getSelectedData
              (vn, coords, &data[0], varPath);
        }
        //values.resize(data.size());
        for(unsigned int i=0; i<data.size(); i++) {
          std::cout <<"Force [unsigned long] to [int] in fastquery-c-interface()"<<std::endl;
          buf[j][i].i = data[i];
        }
        break;}
      default:
        LOGGER(ibis::gVerbose > 0)
            << "Warning -- Data type " << type << " is not supported";
        delete(queryProcessor);
#ifndef FQ_NOMPI
        MPI_Finalize();
#endif
        return -5;
    }
    if (! berr) {
      LOGGER(ibis::gVerbose > 0)
          << "Warning -- Failed to get selected data";

      delete(queryProcessor);
#ifndef FQ_NOMPI
      MPI_Finalize();
#endif
      return -6;
    }

#ifndef FQ_NOMPI
    MPI_Barrier(MPI_COMM_WORLD);
    t2 = MPI_Wtime(); 
    if (mpi_rank == 0)
      std::cout << "getSelectedData, time: " << t2 - t1 << std::endl;
    t1 = MPI_Wtime(); 
#endif
  }

  LOGGER(ibis::gVerbose > 0)
      << "fq_query" << " successfully complete processing query with " 
      << hits << " hitsss " << (hits>1?"s":"");
  
#ifndef FQ_NOMPI
  MPI_Barrier(MPI_COMM_WORLD);
  t2 = MPI_Wtime(); 
  if (mpi_rank == 0)
    std::cout << "Over all time: " << t2 - t0 << std::endl;
#endif  

  delete(queryProcessor);
  
  *fq_hits = hits;
  std::cout << *fq_hits << " hits in fastquery " << hits;

  //#ifndef FQ_NOMPI
  //MPI_Finalize();
  //#endif
  return hits;
} // main


//"dataset" is in size of "dim"
int fq_hist(char *qstr, char *filename, char *group, const char **dataset, int dimension, char *indexfilename, int mpi_len, char *begin_str, char *stride_str, char *end_str,  int **fq_count, int *fq_count_size)
{

  char *fileModel = 0;
  char *varPath = 0;
  int verboseness = 0;

  ibis::gVerbose = -1;

  char *hist_path = 0;
  int mpi_dim = FASTQUERY_DEFAULT_MPI_DIM;

  char  *begin, *end, *stride;
  double begin1 = 0;	// option y
  double end1 = 0;		// option e
  double stride1 = 0;	// option s
  
  double begin2 = 0;	// option y
  double end2 = 0;		// option e
  double stride2 = 0;	// option s
  
  double begin3 = 0;       // option y
  double end3 = 0;     	// option e
  double stride3 = 0;      // option s

  
  bool easyToShow = true;
  bool verification = true;

  char *varName1 = 0;
  char *varName2 = 0;
  char *varName3 = 0;
  
  std::string varPathStr;
  std::string varNameStr1;
  std::string varNameStr2;
  std::string varNameStr3;
  
  int mpi_rank;

    
  condstring = qstr;
  datafile   = filename;
  varPath    = group;
  //varName    = dataset;
  //dimension  = dim;
  if(indexfilename != NULL)
    indexfile  = indexfilename;
  //std::cout << "data file :  " << datafile << " index file : " << indexfile  << std::endl;
  
  begin      = begin_str;
  end        = end_str;
  stride     = stride_str;
  //std::cout << "begin: " << begin_str << " stride: " << stride_str << " end: "<< end_str << std::endl;
  if(mpi_len <= 0)
    mpi_len = FASTQUERY_DEFAULT_MPI_LEN ;

#ifndef FQ_NOMPI
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  std::cout << "FastQuery MPI version" << std::endl;
#else
  mpi_rank = 0;
  std::cout << "FastQuery No MPI version" << std::endl;
#endif
    
  
  std::vector<double> beginList;
  std::vector<double> endList;
  std::vector<double> strideList;
  beginList.resize(dimension);
  endList.resize(dimension);
  strideList.resize(dimension);
  if (datafile.empty() || condstring == 0 || dimension == 0 ) {
    std::cout << "parameter is enmpry: data file:  " << datafile.empty() << ", cond string (" << condstring << ", dimension ( " << dimension << std::endl;
    return -1;
  }
  

  ibis::gParameters().add(FQ_REPORT_STATISTIC, "true");

  ibis::horometer totTimer;
  totTimer.start();

  FQ::FileFormat model = FQ::FQ_HDF5;
  if (fileModel != 0) {
    std::string format = fileModel;
    if (format.compare("HDF5") == 0) {
      model = FQ::FQ_HDF5;
    }
    else if (format.compare("H5PART") == 0) {
      model = FQ::FQ_H5Part;
    }
    else if (format.compare("NETCDF") == 0) {
      model = FQ::FQ_NetCDF;
    }
    else if (format.compare("PNETCDF") == 0) {
      model = FQ::FQ_pnetCDF;
    }
  }

  if (! indexfile.empty()) {
    if (verboseness > 1)
      std::cout << "using indexfile \"" << indexfile.c_str()
                << "\" ... \n";
  }

  if (varPath != 0) {
    if (verboseness > 1)
      std::cout << "Debug: use variable path \"" << varPath << "\"\n";
    varPathStr = varPath;
  }

  //	std::cout << "varName:" << varName << " begin:" << begin << " end:" << end << " stride:" << stride << std::endl;
  //varName1 = strtok(varName, ",;");
  //if (dimension>1) varName2 = strtok(NULL, ",;");
  //if (dimension>2) varName3 = strtok(NULL, ",;");

  varName1 = strdup(dataset[0]); // strtok(varName, ",;");
  if (dimension>1) varName2 = strdup(dataset[1]) ;//strtok(NULL, ",;");
  if (dimension>2) varName3 = strdup(dataset[2]);//strtok(NULL, ",;");

  begin1 = atof(strtok(begin, ",;"));
  if (dimension>1) begin2 = atof(strtok(NULL, ",;"));
  if (dimension>2) begin3 = atof(strtok(NULL, ",;"));

  end1 = atof(strtok(end, ",;"));
  if (dimension>1) end2 = atof(strtok(NULL, ",;"));
  if (dimension>2) end3 = atof(strtok(NULL, ",;"));

  stride1 = atof(strtok(stride, ",;"));
  if (dimension>1) stride2 = atof(strtok(NULL, ",;"));
  if (dimension>2) stride3 = atof(strtok(NULL, ",;"));

  //std::cout << "varName is " << varName1 << ", " << varName2 << ", " << varName3 << std::endl;
  //std::cout << "begin   is " << begin1 << ", " << begin2 << ", " << begin3 << std::endl;
  //std::cout << "end     is " << end1 << ", " << end2 << ", " << end3 << std::endl;
  //std::cout << "stride  is " << stride1 << ", " << stride2 << ", " << stride3 << std::endl;

  if (varName1!=0) varNameStr1 = varName1;
  if (dimension>1 && varName2!=0) varNameStr2 = varName2;
  if (dimension>2 && varName3!=0) varNameStr3 = varName3;
  /*
    if (mpi_rank==0) {
    unsigned int dims1 = static_cast<uint32_t>(1+floor((end1-begin1)/stride1));
    unsigned int dims2 = static_cast<uint32_t>(1+floor((end2-begin2)/stride2));
    unsigned int dims3 = static_cast<uint32_t>(1+floor((end3-begin3)/stride3));
    std::cout << "dims1 * dims2 * dims3 = " << dims1 << " * " << dims2 << " * " << dims3 << std::endl;
    }
  */


  static std::ostringstream logfilestream;
  if (logfilestream.str().empty() != true) {
#ifndef FQ_NOMPI
    logfilestream << mpi_rank << ".log";
#endif
  }
    
  // open the named file
  QueryProcessor* queryProcessor = new QueryProcessor(datafile, model, indexfile, verboseness, "", logfilestream.str().c_str()); // the file handler

  if (queryProcessor->isValid() == false) {
    if (verboseness > 0) {
      std::cout << "ERROR: failed to initiate the QueryProcessor object for file \""
                << datafile.c_str() << "\" ...\n";
      std::cout << "REPORT: failed to complete processing query" << std::endl;
    }
    delete(queryProcessor);
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    return -1;
  }

  uint64_t hits = 0;
  // getNumHits
  ibis::horometer timer;
  timer.start();
  hits = queryProcessor->getNumHits(condstring, varPathStr, mpi_dim, mpi_len);
  timer.stop();
  if (verboseness > 1)
    std::cout << "Debug: conditions \"" << condstring
              << "\" number of hits " << hits << std::endl;;

  if (hits == 0) {
    if (verboseness > 1) {
      std::cout << "Warning -- No element is seleteced ==>"
                << " the rest of the test is skipped!" << std::endl;
    }
    if (verboseness > 0) {
#ifndef FQ_NOMPI
      if (mpi_rank==0) {
#endif
        std::cout << "REPORT: successfully completed processing query with "
                  << hits << " hits" << std::endl;
#ifndef FQ_NOMPI
      }
#endif
    }
    delete(queryProcessor);
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    return hits;
  }

  // executeQuery
  std::vector<uint64_t> coords;
  std::vector<uint32_t> counts;
  bool herr = true;
  //	if (mpi_rank==0) std::cout<<"histogram starting..."<<std::endl;
  if (varPath != 0) {
    if (dimension==1) {
      herr = queryProcessor->get1DHistogram
          ((char*) condstring, varNameStr1, varPathStr, begin1, end1, stride1,
           counts, mpi_dim, mpi_len);
    }
    else if (dimension==2) {
      herr = queryProcessor->get2DHistogram
          ((char*) condstring, varPathStr,
           varNameStr1, begin1, end1, stride1,
           varNameStr2, begin2, end2, stride2,
           counts, mpi_dim, mpi_len);
    }
    else if (dimension==3) {
      herr = queryProcessor->get3DHistogram
          ((char*) condstring, varPathStr,
           varNameStr1, begin1, end1, stride1,
           varNameStr2, begin2, end2, stride2,
           varNameStr3, begin3, end3, stride3,
           counts, mpi_dim, mpi_len);
    }
    if (! herr) {
      LOGGER(ibis::gVerbose >= 0)
          <<  " Failed to compute the histogram";
      return -2;
    }

    /************************/
    /*  verify part         */
    /************************/

    uint64_t hits1 = 0;
    for (int i=0; i<counts.size(); i++) {
      hits1 += counts[i];
    }
    if (hits1 != hits) {
      std::cout<<"Error:\tcheck sum failed. Num of Hit is " << hits
               << ",and histogram number is " << hits1<<std::endl;
    }else{
      std::cout<<"verification result is correct.\n";
    }
    
    *fq_count_size = counts.size();
    (*fq_count) = (int *) malloc(sizeof(int) * counts.size()); 
    //printf("%d \n", *fq_count_size);
    for (int i=0; i<counts.size(); i++) {
      (*fq_count)[i] = counts[i];
    }
    
  }//end if(!varPath)
  
  if (verboseness > 0) {
#ifndef FQ_NOMPI
    if (mpi_rank==0) {
#endif
      std::cout << "REPORT: successfully completed get1DHistogram with "
                << counts.size() << " histogram size" << std::endl;
#ifndef FQ_NOMPI
    }
#endif
  }
  delete(queryProcessor);
#ifndef FQ_NOMPI
  MPI_Finalize();
#endif
  totTimer.stop();
  LOGGER(FastQuery::reportTiming())
      << "Statistic\thistogram::totTimer\t"
      << totTimer.CPUTime() << "\t" << totTimer.realTime()
      << "\t";
  return hits;
} // main


int get_mpi_length(char *fq_parameter){
  char      *temp_parameter, *lp, *temp;
  int        fq_length;
  
  temp_parameter = strdup(fq_parameter);
  temp           = strdup(temp_parameter); 
  lp  = strstr(temp_parameter, "-l");
  if(lp != NULL){
    lp = lp + sizeof("-l");
    while(*lp == ' ')
      lp++;
    sscanf(lp, "%d%s", &fq_length, temp);
  }else{
    fq_length = 0;
  }
  free(temp);
  free(temp_parameter);

  return fq_length;
}
