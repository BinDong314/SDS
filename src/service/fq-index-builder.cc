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
/**
   A simple program to test the capability of building and storing
   indexes to a file.

   binning option is specified in the form of "<binning ... />".  
   NOTE that on most systems, the binning option needs to be quoted 
   because it involved characters that have special meaning to most shells.
*/

#include "indexBuilder.h"
#include <iostream>	

static std::string datafile;
static std::string indexfile;
static char *fileFormat = 0;
static char *binning = 0;
static bool forcerebuild = false;
static std::string conffile;
static std::string varPath;
static std::vector<const char*> varNames;
static std::string logfile;
static int mpi_len = FQ_DEFAULT_MPI_LEN;
static int mpi_dim = FQ_DEFAULT_MPI_DIM;

void parseArgs(int argc, char **argv) {
    static const char *options="b:B:c:C:d:D:f:F:p:P:n:N:i:I:v:V:m:M:l:L:g:G:rR";
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
	case 'm':
	case 'M': fileFormat = optarg; break;
	case 'd':
	case 'D':
	case 'f':
	case 'F': datafile = optarg; break;
	case 'i':
	case 'I': indexfile = optarg; break;
	case 'g':
	case 'G': logfile = optarg; break;
	case 'p':
	case 'P': varPath = optarg; break;
	case 'n':
	case 'N': varNames.push_back(optarg); break;
	case 'v':
	case 'V': ibis::gVerbose = atoi(optarg); break;
	case 'r':
	case 'R': forcerebuild = true; break;
	case 'l':
	case 'L': mpi_len = atoi(optarg);break; 
	case 'b':
	case 'B': binning = optarg; break;
	case 'c':
	case 'C': conffile = optarg; break;
	default: break;
        } // switch
    } // while
} // parseArgs


int main(int argc, char** argv) {
    int ret = 0;
    parseArgs(argc, argv);
    if (datafile.empty()) {
        std::cerr << "Usage:\n" << *argv 
                  << " -f data-file-name [-c conf-file] [-i index-file-name]"
	    " [-g log-file] [-n variable-name] [-p variable-path]"
	    " [-b '<binning nbins=1000 />' (default unbinned)]"
	    " [-r (force-rebuild-index)] [-v verboseness]"
	    " [-m fileFormat [HDF5(default), H5PART, NETCDF, PNETCDF]]"
	    " [-l mpi_subarray_size(default=100000)]\n"
	    "\tIt builds index for a set of variables whose dataset location has the prefix\n"
	    "\tvariable-path and postfix variable-name.\n\n"
	    "\tUse option \"-i\" to specify the output file for storing "
	    "indexes.\n"
	    "\tOtherwise, the indexes are written back to data file "
	    "\"data-file-name\".\n\n"
	    "\tUse option \"-r\" to enforce rebuild and replace the existing "
	    "index.\n\n"
	    "\tUnder parallel mode, use \"-l\" to set the subarray size for "
	    "spitting dataset.\n\n"
	    "\tUse option \"-b\" to specify the binning option to build the "
	    "index.\n"
	    "\tThe available binning option is defined and provided by the "
	    "FastBit.\n"
	    "\tMore information can be found at "
	    "http://sdm.lbl.gov/fastbit/doc/indexSpec.html.\n"
	    "\tBinning option is suggested to be used with large dataset "
	    "to reduce the size of built index.\n"
	    "\tPrecision option is suggested to be used when the query "
	    "involves floating point numbers.\n"
		  << std::endl;
        return -1;
    }
#ifndef FQ_NOMPI 
    MPI_Init(&argc, &argv);
    int mpi_size, mpi_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
#endif

    if (! logfile.empty()){
#ifndef FQ_NOMPI
	std::ostringstream oss;
	oss << logfile << "-" << mpi_rank << ".log";
	logfile = oss.str();
#endif
        std::cout << *argv << " is to redirect log messages to \""
		  << logfile << "\" ..." << std::endl;
	ibis::util::setLogFileName(logfile.c_str());
    }

    if (! conffile.empty())
	ibis::gParameters().read(conffile.c_str());
    ibis::gParameters().add(FQ_REPORT_STATISTIC, "false");
    ibis::gParameters().add("fileManager.maxBytes", "2GB");
    if (forcerebuild) {
	ibis::gParameters().add("FastQuery.forceIndexRebuild", "true");
    } else {
	ibis::gParameters().add("FastQuery.forceIndexRebuild", "false");
    }

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
	lg() << *argv << " data file \"" << datafile << "\"";
	if (! indexfile.empty())
	    lg() << "\tindexfile \"" << indexfile.c_str() << "\"";
	if (binning !=0)
	    lg() << "\tbinning option \"" << binning << "\"";
	if (! varPath.empty())
	    lg() << "\tvariable path \"" << varPath << "\"";
	if (! varNames.empty())
	    lg() << "\twith "  << varNames.size() << " variable name"
		 << (varNames.size()>1?"s":"") << " ...";
    }

    ibis::util::timer totTimer(*argv, 1);
    IndexBuilder* indexBuilder = new
	IndexBuilder(datafile, ffmt, indexfile, ibis::gVerbose);
    bool berr = indexBuilder->isValid();    
    if (! berr) {
	LOGGER(ibis::gVerbose >= 0)
	    << "ERROR: Failed to initialize the IndexBuilder object for file \"" 
	    << datafile << "\"";
	delete(indexBuilder);
#ifndef FQ_NOMPI
        MPI_Finalize();
#endif
	return -2;
    }
    LOGGER(ibis::gVerbose > 1)
	<< *argv << " initiate the IndexBuilder object for file \"" 
	<< datafile << "\"";

    ret = 0;

    for (unsigned j = 0; j < varNames.size(); ++ j) {
	std::string vn(varNames[j]);
	if (mpi_len > 0) {
	    ret += indexBuilder->buildIndexes
                (binning, varPath, vn, mpi_dim, mpi_len);
	}
	else {
	    ret += indexBuilder->buildIndexes(binning, varPath, vn);
	}
    }
    if (varNames.empty()) {
    	ret += indexBuilder->buildIndexes
            (binning, varPath, "", mpi_dim, mpi_len);
/*	if (! varPath.empty()) {
	    ret += indexBuilder->buildIndexes(binning, varPath,);
	} else {
	    ret += indexBuilder->buildIndexes(binning);
	}*/
    }
    delete(indexBuilder);
    LOGGER(ibis::gVerbose >= 0)
        << *argv << " completed with ret = " << ret;
#ifndef FQ_NOMPI
    MPI_Finalize();
#endif
    if(ret > 0){
      return 0;
    }else{
      return -1;
    }
} // main
