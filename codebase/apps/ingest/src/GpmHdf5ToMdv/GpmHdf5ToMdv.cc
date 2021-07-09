// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// GpmHdf5ToMdv.cc
//
// GpmHdf5ToMdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////
//
// GpmHdf5ToMdv reads McIdas data in NetCDF format, and
// converts to MDV
//
////////////////////////////////////////////////////////////////

#include <algorithm>
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <Mdv/MdvxField.hh>
#include <dsserver/DsLdataInfo.hh>
#include <euclid/search.h>
#include "GpmHdf5ToMdv.hh"
using namespace std;

const fl32 GpmHdf5ToMdv::_missingFloat = -9999.0;

// Constructor

GpmHdf5ToMdv::GpmHdf5ToMdv(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name

  _progName = "GpmHdf5ToMdv";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that file list set in archive mode
  
  if (_params.mode == Params::FILELIST && _args.inputFileList.size() == 0) {
    cerr << "ERROR: GpmHdf5ToMdv" << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.reg_interval);

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file,
			     _params.process_latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    if (_params.debug) {
      for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
        cerr << "Adding path: " << _args.inputFileList[ii] << endl;
      }
    }
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  if (strlen(_params.file_name_ext) > 0) {
    _input->setSearchExt(_params.file_name_ext);
  }

  if (strlen(_params.file_name_substr) > 0) {
    _input->setSubString(_params.file_name_substr);
  }

}

// destructor

GpmHdf5ToMdv::~GpmHdf5ToMdv()

{

  // unregister process

  PMU_auto_unregister();

  // free up

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int GpmHdf5ToMdv::Run ()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  
  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Reading file");
    ta_file_uncompress(inputPath);
    if (_processFile(inputPath)) {
      cerr << "ERROR = GpmHdf5ToMdv::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int GpmHdf5ToMdv::_processFile(const char *input_path)

{

  PMU_auto_register("Processing file");
  
  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // use try block to catch any exceptions
  
  try {
    
    // open file
    
    H5File file(input_path, H5F_ACC_RDONLY);
    if (_params.debug) {
      cerr << "  file size: " << file.getFileSize() << endl;
    }
    
    // get the root group
    
    Group root(file.openGroup("/"));

    // root attributes

    _fileHeader = _readStringAttribute(root, "FileHeader", "RootAttr");
    _fileInfo = _readStringAttribute(root, "FileInfo", "RootAttr");
    _inputRecord = _readStringAttribute(root, "InputRecord", "RootAttr");
    _jaxaInfo = _readStringAttribute(root, "JAXAInfo", "RootAttr");
    _navigationRecord = _readStringAttribute(root, "NavigationRecord", "RootAttr");
    _history = _readStringAttribute(root, "history", "RootAttr");
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "FileHeader: " << endl << "===================" << endl
           << _fileHeader << "===================" << endl;
      cerr << "FileInfo: " << endl << "===================" << endl
           << _fileInfo << "===================" << endl;
      cerr << "InputRecord: " << endl << "===================" << endl
           << _inputRecord << "===================" << endl;
      cerr << "JAXAInfo: " << endl << "===================" << endl
           << _jaxaInfo << "===================" << endl;
      cerr << "NavigationRecord: " << endl << "===================" << endl
           << _navigationRecord << "===================" << endl;
      cerr << "history: " << endl << "===================" << endl
           << _history << "===================" << endl;
    }

    // open the NS group
    
    Group ns(root.openGroup("NS"));
    if (_readGroupNs(ns)) {
      return -1;
    }

  } // try
  
  catch (H5x::Exception &e) {
    // _addErrStr("ERROR - reading GPM HDF5 file");
    // _addErrStr(e.getDetailMsg());
    return -1;
  }

  // interpolate dbz field onto latlon grid

  _interpDbz();

  // invert the gate levels because the radar gate data is stored
  // top-down instead of bottom-up

  _invertDbzGateLevels();
  
  // optionally remap onto specified output grid
  
  if (_params.remap_gates_to_vert_levels) {
    _remapVertLevels();
  }

  // create output Mdvx file object
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
    
  // set master header
  
  if (_setMasterHeader(mdvx)) {
    return -1;
  }
  
  // add the data fields
  
  MdvxField *dbzField = _createMdvxField("DBZ",
                                         "reflectivity",
                                         "dBZ",
                                         _nx, _ny, _nz,
                                         _minxDeg, _minyDeg, _minzKm,
                                         _dxDeg, _dyDeg, _dzKm,
                                         _missingDbz,
                                         _dbzOutput.data());
  
  mdvx.addField(dbzField);

  // write output file
  
  if (_params.debug) {
    cerr << "Writing file to url: " << _params.output_url << endl;
  }
  
  if (mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - GpmHdf5ToMdv" << endl;
    cerr << "  Cannot write file to url: " << _params.output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "  Wrote output file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

/////////////////////////////////////////////
// read string attribute

string GpmHdf5ToMdv::_readStringAttribute(Group &group,
                                          const string &attrName,
                                          const string &context)
  
{
  Hdf5xx::DecodedAttr decodedAttr;
  _utils.loadAttribute(group, attrName, context, decodedAttr);
  string attr(decodedAttr.getAsString());
  return attr;
}

//////////////////////////////////////////////
// read the NS group

int GpmHdf5ToMdv::_readGroupNs(Group &ns)
  
{

  _qualAvailable = false;

  string swathHeader = _readStringAttribute(ns, "SwathHeader", "ns-attr");
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "SwathHeader: " << endl << "===================" << endl
         << swathHeader << "===================" << endl;
  }

  if (_readTimes(ns)) {
    return -1;
  }

  if (_readLatLon(ns)) {
    return -1;
  }
  
  if (_readSpaceCraftPos(ns)) {
    return -1;
  }

  if (_readQcFlags(ns)) {
    return -1;
  }

  // read reflectivity

  if (_readDbz(ns)) {
    return -1;
  }

  // read quality flag

  if (_readField2D(ns, "FLG", "qualityFlag",
                   _qualInput, _missingQual, _qualUnits) == 0) {
    _qualAvailable = true;
  }
  
  return 0;

}

//////////////////////////////////////////////
// read the scan times

int GpmHdf5ToMdv::_readTimes(Group &ns)
  
{

  Group scanTime(ns.openGroup("ScanTime"));
  vector<size_t> dims;
  string units;
  NcxxPort::si32 missingVal;
  
  vector<NcxxPort::si32> years, months, days, hours, mins, secs, msecs;
  Hdf5xx hdf5;
  if (hdf5.readSi32Array(scanTime, "Year", "ScanTime",
                         dims, missingVal, years, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Year variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Month", "ScanTime",
                         dims, missingVal, months, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Month variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "DayOfMonth", "ScanTime",
                         dims, missingVal, days, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read DayOfMonth variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Hour", "ScanTime",
                         dims, missingVal, hours, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Hour variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Minute", "ScanTime",
                         dims, missingVal, mins, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Minute variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "Second", "ScanTime",
                         dims, missingVal, secs, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read Second variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanTime, "MilliSecond", "ScanTime",
                         dims, missingVal, msecs, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  Cannot read MilliSecond variable" << endl;
    return -1;
  }

  _times.clear();
  for (size_t ii = 0; ii < years.size(); ii++) {
    DateTime dtime(years[ii], months[ii], days[ii],
                   hours[ii], mins[ii], secs[ii], msecs[ii] / 1000.0);
    _times.push_back(dtime);
  }
  if (_times.size() < 1) {
    cerr << "ERROR - GpmHdf5ToMdv::_readTimes()" << endl;
    cerr << "  No scan times found" << endl;
    return -1;
  }
  
  _startTime = _times[0];
  _endTime = _times[_times.size()-1];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Reading scan times <<====" << endl;
    cerr << "  nTimes: " << _times.size() << endl;
    cerr << "  startTime: " << _startTime.asString(3) << endl;
    cerr << "  endTime: " << _endTime.asString(3) << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t ii = 0; ii < _times.size(); ii++) {
        cerr << "  ii, time: " << ii << ", " << _times[ii].asString(3) << endl;
      }
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// read the quality flags

int GpmHdf5ToMdv::_readQcFlags(Group &ns)
  
{

  Group scanStatus(ns.openGroup("scanStatus"));
  vector<size_t> dims;
  string units;
  NcxxPort::si32 missingVal;

  _dataQuality.resize(_times.size());
  _dataWarning.resize(_times.size());
  _geoError.resize(_times.size());
  _geoWarning.resize(_times.size());
  _limitErrorFlag.resize(_times.size());
  _missingScan.resize(_times.size());
  
  Hdf5xx hdf5;
  if (hdf5.readSi32Array(scanStatus, "dataQuality", "scanStatus",
                         dims, missingVal, _dataQuality, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read dataQuality flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "dataWarning", "scanStatus",
                         dims, missingVal, _dataWarning, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read dataWarning flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "geoError", "scanStatus",
                         dims, missingVal, _geoError, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read geoError flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "geoWarning", "scanStatus",
                         dims, missingVal, _geoWarning, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read geoWarning flag variable" << endl;
  }
  if (hdf5.readSi32Array(scanStatus, "limitErrorFlag", "scanStatus",
                         dims, missingVal, _limitErrorFlag, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read limitErrorFlag variable" << endl;
    return -1;
  }
  if (hdf5.readSi32Array(scanStatus, "missing", "scanStatus",
                         dims, missingVal, _missingScan, units)) {
    cerr << "WARNING - GpmHdf5ToMdv::_readQcFlags()" << endl;
    cerr << "  Cannot read missing flag variable" << endl;
    return -1;
  }

  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "====>> Reading QC flags <<====" << endl;
    for (size_t ii = 0; ii < _times.size(); ii++) {
      cerr << "  ii, dQ, dW, geoE, geoW, limE, miss: "
           << ii << ", "
           << _dataQuality[ii] << ", "
           << _dataWarning[ii] << ", "
           << _geoError[ii] << ", "
           << _geoWarning[ii] << ", "
           << _limitErrorFlag[ii] << ", "
           << _missingScan[ii] << endl;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// read the lat/lon arrays

int GpmHdf5ToMdv::_readLatLon(Group &ns)
  
{
  
  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> latDims;
  string latUnits;
  if (hdf5.readFl64Array(ns, "Latitude", "NS",
                         latDims, _missingLat, _lats, latUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Cannot read Latitude variable" << endl;
    return -1;
  }

  // read Longitude
  
  vector<size_t> lonDims;
  string lonUnits;
  if (hdf5.readFl64Array(ns, "Longitude", "NS",
                         lonDims, _missingLon, _lons, lonUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Cannot read Longitude variable" << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (latDims.size() != 2 || lonDims.size() != 2) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Latitude/Longitude must have 2 dimensions" << endl;
    cerr << "  latDims.size(): " << latDims.size() << endl;
    cerr << "  lonDims.size(): " << lonDims.size() << endl;
    return -1;
  }
  if (latDims[0] != lonDims[0] || latDims[1] != lonDims[1]) {
    cerr << "ERROR - GpmHdf5ToMdv::_readLatLon()" << endl;
    cerr << "  Latitude/Longitude must have same dimensions" << endl;
    cerr << "  latDims[0]: " << latDims[0] << endl;
    cerr << "  lonDims[0]: " << lonDims[0] << endl;
    cerr << "  latDims[1]: " << latDims[1] << endl;
    cerr << "  lonDims[1]: " << lonDims[1] << endl;
    return -1;
  }
  _nScans = latDims[0];
  _nRays = latDims[1];

  _latLons.clear();
  _minLat = 90.0;
  _maxLat = -90.0;
  _minLon = 360.0;
  _maxLon = -360.0;
  for (size_t iscan = 0; iscan < _nScans; iscan++) {
    vector<Point_d> pts;
    for (size_t iray = 0; iray < _nRays; iray++) {
      size_t ipt = iscan * _nRays + iray;
      Point_d pt;
      pt.y = _lats[ipt];
      pt.x = _lons[ipt];
      pts.push_back(pt);
      if (pt.y != _missingLat) {
        _minLat = min(_minLat, pt.y);
        _maxLat = max(_maxLat, pt.y);
      }
      if (pt.x != _missingLon) {
        _minLon = min(_minLon, pt.x);
        _maxLon = max(_maxLon, pt.x);
      }
    } // iray
    _latLons.push_back(pts);
  } // iscan

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Reading lat/lon <<====" << endl;
    cerr << "  missingLat: " << _missingLat << endl;
    cerr << "  missingLon: " << _missingLon << endl;
    cerr << "  minLat, maxLat: " << _minLat << ", " << _maxLat << endl;
    cerr << "  minLon, maxLon: " << _minLon << ", " << _maxLon << endl;
    cerr << "  nScans, nRays: " << _nScans << ", " << _nRays << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        for (size_t iray = 0; iray < _nRays; iray++) {
          cerr << "  iscan, iray, lat, lon: "
               << iscan << ", "
               << iray << ", "
               << _latLons[iscan][iray].y << ", "
               << _latLons[iscan][iray].x << endl;
        } // iray
      } // iscan
    }
  }

  // set up (x, y) grid details

  _dxDeg = _params.output_grid.dLon;
  _dyDeg = _params.output_grid.dLat;

  if (_params.set_output_grid_limits_from_data) {
    _minxDeg = ((int) floor(_minLon / _dxDeg) - 5) * _dxDeg;
    _minyDeg = ((int) floor(_minLat / _dyDeg) - 5) * _dyDeg;
    _nx = (_maxLon - _minLon) / _dxDeg + 10;
    _ny = (_maxLat - _minLat) / _dyDeg + 10;
  } else {
    _nx = _params.output_grid.nLon;
    _ny = _params.output_grid.nLat;
    _minxDeg = _params.output_grid.minLon;
    _minyDeg = _params.output_grid.minLat;
  }

  _maxxDeg = _minxDeg + _nx * _dxDeg;
  _maxyDeg = _minyDeg + _ny * _dyDeg;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Output grid details <<====" << endl;
    cerr << "  _nx, _ny: " << _nx << ", " << _ny << endl;
    cerr << "  _dxDeg, _dyDeg: " << _dxDeg << ", " << _dyDeg << endl;
    cerr << "  _minxDeg, _minyDeg: " << _minxDeg << ", " << _minyDeg << endl;
    cerr << "  _maxxDeg, _maxyDeg: " << _maxxDeg << ", " << _maxyDeg << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the spacecraft lat/lon/alt

int GpmHdf5ToMdv::_readSpaceCraftPos(Group &ns)
  
{
  
  Hdf5xx hdf5;
  Group nav(ns.openGroup("navigation"));

  // read spacecraft longitude
  
  vector<size_t> lonDims;
  NcxxPort::fl64 missingLon;
  string lonUnits;
  if (hdf5.readFl64Array(nav, "scLon", "NS/navigation",
                         lonDims, missingLon, _scLon, lonUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readSpaceCraftPos()" << endl;
    cerr << "  Cannot read scLon variable" << endl;
    return -1;
  }

  // read spacecraft latitude
  
  vector<size_t> latDims;
  NcxxPort::fl64 missingLat;
  string latUnits;
  if (hdf5.readFl64Array(nav, "scLat", "NS/navigation",
                         latDims, missingLat, _scLat, latUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readSpaceCraftPos()" << endl;
    cerr << "  Cannot read scLat variable" << endl;
    return -1;
  }

  // read spacecraft altitude
  
  vector<size_t> altDims;
  NcxxPort::fl64 missingAlt;
  string altUnits;
  if (hdf5.readFl64Array(nav, "scAlt", "NS/navigation",
                         altDims, missingAlt, _scAlt, altUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readSpaceCraftPos()" << endl;
    cerr << "  Cannot read scAlt variable" << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (lonDims[0] != _nScans || latDims[0] != _nScans || altDims[0] != _nScans) {
    cerr << "ERROR - GpmHdf5ToMdv::_readSpaceCraftPos()" << endl;
    cerr << "  Lat/Lon/Alt dims must match _nScans" << endl;
    cerr << "  nScans: " << _nScans << endl;
    cerr << "  latDims[0]: " << latDims[0] << endl;
    cerr << "  lonDims[0]: " << lonDims[0] << endl;
    cerr << "  altDims[0]: " << altDims[0] << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Reading spacecraft position <<====" << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        cerr << "  iscan, scLat, scLon, scAlt: "
             << iscan << ", "
             << _scLat[iscan] << ", "
             << _scLon[iscan] << ", "
             << _scAlt[iscan] << endl;
      } // iray
    } // iscan
  }

  return 0;

}

//////////////////////////////////////////////
// read the refectivity

int GpmHdf5ToMdv::_readDbz(Group &ns)
  
{
  
  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> dbzDims;
  
  Group slv(ns.openGroup("SLV"));
  if (hdf5.readFl32Array(slv, "zFactorCorrected", "NS/SLV",
                         dbzDims, _missingDbz, _dbzInput, _dbzUnits)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readDbz()" << endl;
    cerr << "  Cannot read zFactorCorrected variable" << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (dbzDims.size() != 3) {
    cerr << "ERROR - GpmHdf5ToMdv::_readDbz()" << endl;
    cerr << "  zFactorCorrected must have 3 dimensions" << endl;
    cerr << "  dbzDims.size(): " << dbzDims.size() << endl;
    return -1;
  }
  if (dbzDims[0] != _nScans || dbzDims[1] != _nRays) {
    cerr << "ERROR - GpmHdf5ToMdv::_readDbz()" << endl;
    cerr << "  DBZ dimensions must match nScans and nRays" << endl;
    cerr << "  dbzDims[0]: " << dbzDims[0] << endl;
    cerr << "  dbzDims[1]: " << dbzDims[1] << endl;
    cerr << "  _nScans: " << _nScans << endl;
    cerr << "  _nRays: " << _nRays << endl;
    return -1;
  }

  _nGates = dbzDims[2];
  _nz = _nGates;
  _minzKm = _params.radar_min_z_km;
  _dzKm = _params.radar_delta_z_km;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Read DBZ <<====" << endl;
    cerr << "  nScans, nRays, nGates: " 
         << _nScans << ", " << _nRays << ", " << _nGates << endl;
    cerr << "  missingDbz: " << _missingDbz << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        for (size_t iray = 0; iray < _nRays; iray++) {
          for (size_t igate = 0; igate < _nGates; igate++) {
            size_t ipt = iscan * _nRays * _nGates + iray * _nGates + igate;
            if (_dbzInput[ipt] != _missingDbz) {
              cerr << "  iscan, iray, igate, dbz: "
                   << iscan << ", "
                   << iray << ", "
                   << igate << ", "
                   << _dbzInput[ipt] << endl;
            }
          } // igate
        } // iray
      } // iscan
    } // extra
  } // verbose
  
  return 0;

}

//////////////////////////////////////////////
// read a 2D field - floats

int GpmHdf5ToMdv::_readField2D(Group &ns,
                               const string &groupName,
                               const string &fieldName,
                               vector<NcxxPort::fl32> &vals,
                               NcxxPort::fl32 &missingVal,
                               string &units)
  
{
  
  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> dims;
  
  string context("NS/");
  context += groupName;
  Group grp(ns.openGroup(groupName));
  if (hdf5.readFl32Array(grp, fieldName, context,
                         dims, missingVal, vals, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/field: " << groupName << "/" << fieldName << endl;
    return -1;
  }

  // check dimensions for consistency
  
  if (dims.size() != 2) {
    cerr << "ERROR - GpmHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/field: " << groupName << "/" << fieldName << endl;
    cerr << "  2D fields must have 2 dimensions" << endl;
    cerr << "  dims.size(): " << dims.size() << endl;
    return -1;
  }
  if (dims[0] != _nScans || dims[1] != _nRays) {
    cerr << "ERROR - GpmHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/field: " << groupName << "/" << fieldName << endl;
    cerr << "  dimensions must match nScans and nRays" << endl;
    cerr << "  dims[0]: " << dims[0] << endl;
    cerr << "  dims[1]: " << dims[1] << endl;
    cerr << "  _nScans: " << _nScans << endl;
    cerr << "  _nRays: " << _nRays << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Read field: " << groupName << "/" << fieldName << " <<====" << endl;
    cerr << "  nScans, nRays: " 
         << _nScans << ", " << _nRays << endl;
    cerr << "  missingVal: " << missingVal << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        for (size_t iray = 0; iray < _nRays; iray++) {
          size_t ipt = iscan * _nRays + iray;
          if (vals[ipt] != missingVal) {
            cerr << "  iscan, iray, val: "
                 << iscan << ", "
                 << iray << ", "
                 << vals[ipt] << endl;
          }
        } // iray
      } // iscan
    } // extra
  } // verbose
  
  return 0;

}

//////////////////////////////////////////////
// read a 2D field - int16s

int GpmHdf5ToMdv::_readField2D(Group &ns,
                               const string &groupName,
                               const string &fieldName,
                               vector<NcxxPort::si16> &vals,
                               NcxxPort::si16 &missingVal,
                               string &units)
  
{
  
  Hdf5xx hdf5;

  // read Latitude
  
  vector<size_t> dims;
  
  string context("NS/");
  context += groupName;
  Group grp(ns.openGroup(groupName));
  vector<NcxxPort::si32> vals32;
  NcxxPort::si32 missing32;
  
  if (hdf5.readSi32Array(grp, fieldName, context,
                         dims, missing32, vals32, units)) {
    cerr << "ERROR - GpmHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/field: " << groupName << "/" << fieldName << endl;
    return -1;
  }
  missingVal = (NcxxPort::si16) missing32;
  vals.resize(vals32.size());
  for (size_t ii = 0; ii < vals32.size(); ii++) {
    vals[ii] = (NcxxPort::si16) vals32[ii];
  }

  // check dimensions for consistency
  
  if (dims.size() != 2) {
    cerr << "ERROR - GpmHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/field: " << groupName << "/" << fieldName << endl;
    cerr << "  2D fields must have 2 dimensions" << endl;
    cerr << "  dims.size(): " << dims.size() << endl;
    return -1;
  }
  if (dims[0] != _nScans || dims[1] != _nRays) {
    cerr << "ERROR - GpmHdf5ToMdv::_readField2D()" << endl;
    cerr << "  Cannot read group/field: " << groupName << "/" << fieldName << endl;
    cerr << "  dimensions must match nScans and nRays" << endl;
    cerr << "  dims[0]: " << dims[0] << endl;
    cerr << "  dims[1]: " << dims[1] << endl;
    cerr << "  _nScans: " << _nScans << endl;
    cerr << "  _nRays: " << _nRays << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====>> Read field: " << groupName << "/" << fieldName << " <<====" << endl;
    cerr << "  nScans, nRays: " 
         << _nScans << ", " << _nRays << endl;
    cerr << "  missingVal: " << missingVal << endl;
    if (_params.debug >= Params::DEBUG_EXTRA) {
      for (size_t iscan = 0; iscan < _nScans; iscan++) {
        for (size_t iray = 0; iray < _nRays; iray++) {
          size_t ipt = iscan * _nRays + iray;
          if (vals[ipt] != missingVal) {
            cerr << "  iscan, iray, val: "
                 << iscan << ", "
                 << iray << ", "
                 << vals[ipt] << endl;
          }
        } // iray
      } // iscan
    } // extra
  } // verbose
  
  return 0;

}

//////////////////////////////////////////////
// interpolate the reflectivity

void GpmHdf5ToMdv::_interpDbz()
  
{

  _interpField(_dbzInput, _missingDbz, _dbzInterp,
               _params.interp_using_nearest_neighbor);

}

//////////////////////////////////////////////
// interpolate float field

void GpmHdf5ToMdv::_interpField(vector<NcxxPort::fl32> &valsInput,
                                NcxxPort::fl32 missingVal,
                                vector<NcxxPort::fl32> &valsInterp,
                                bool nearestNeighbor)
  
{

  // initialize dbz grid

  size_t nOutput = _nx * _ny * _nz;
  valsInterp.resize(nOutput);
  for (size_t ii = 0; ii < nOutput; ii++) {
    valsInterp[ii] = missingVal;
  }

  // loop through the vertical levels

  for (size_t iz = 0; iz < _nz; iz++) {

    double zM = (_minzKm + iz * _dzKm) * 1000.0;
    
    // load input vals for this level
    
    vector<vector<NcxxPort::fl32> > valsIn;
    for (size_t iscan = 0; iscan < _nScans; iscan++) {
      vector<NcxxPort::fl32> valsScan;
      for (size_t iray = 0; iray < _nRays; iray++) {
        size_t ipt = iscan * _nRays * _nGates + iray * _nGates + iz;
        valsScan.push_back(valsInput[ipt]);
      } // iray
      valsIn.push_back(valsScan);
    } // iscan
    
    // loop through the input (scan, ray) grid

    for (size_t iscan = 0; iscan < _nScans - 1; iscan++) {
      for (size_t iray = 0; iray < _nRays - 1; iray++) {
        
        // corner locations adjusted for radar slant
        Point_d corners[4];
        corners[0] = _getCornerLatLon(iscan, iray, zM);
        corners[1] = _getCornerLatLon(iscan, iray + 1, zM);
        corners[2] = _getCornerLatLon(iscan + 1, iray + 1, zM);
        corners[3] = _getCornerLatLon(iscan + 1, iray, zM);

        // dbz vals at corners
        NcxxPort::fl32 vals[4];
        vals[0] = valsIn[iscan][iray];
        vals[1] = valsIn[iscan][iray + 1];
        vals[2] = valsIn[iscan + 1][iray + 1];
        vals[3] = valsIn[iscan + 1][iray];

        // check for valid VAL
        int nGood = 0;
        for (int ii = 0; ii < 4; ii++) {
          if (vals[ii] != missingVal) {
            nGood++;
          }
        }
        if (nGood < 1) {
          continue;
        }
        
        // interp for the output grid points inside the polygon
        
        _interpInsidePolygon(corners, vals, missingVal, 
                             iz, valsInterp, nearestNeighbor);
        
      } // iray
    } // iscan

  } // iz

}

//////////////////////////////////////////////
// interpolate int field

void GpmHdf5ToMdv::_interpField(vector<NcxxPort::si32> &valsInput,
                                NcxxPort::si32 missingVal,
                                vector<NcxxPort::si32> &valsInterp)
  
{

  // initialize dbz grid

  size_t nOutput = _nx * _ny * _nz;
  valsInterp.resize(nOutput);
  for (size_t ii = 0; ii < nOutput; ii++) {
    valsInterp[ii] = missingVal;
  }

  // loop through the vertical levels

  for (size_t iz = 0; iz < _nz; iz++) {

    double zM = (_minzKm + iz * _dzKm) * 1000.0;
    
    // load input vals for this level
    
    vector<vector<NcxxPort::si32> > valsIn;
    for (size_t iscan = 0; iscan < _nScans; iscan++) {
      vector<NcxxPort::si32> valsScan;
      for (size_t iray = 0; iray < _nRays; iray++) {
        size_t ipt = iscan * _nRays * _nGates + iray * _nGates + iz;
        valsScan.push_back(valsInput[ipt]);
      } // iray
      valsIn.push_back(valsScan);
    } // iscan
    
    // loop through the input (scan, ray) grid

    for (size_t iscan = 0; iscan < _nScans - 1; iscan++) {
      for (size_t iray = 0; iray < _nRays - 1; iray++) {
        
        // corner locations adjusted for radar slant
        Point_d corners[4];
        corners[0] = _getCornerLatLon(iscan, iray, zM);
        corners[1] = _getCornerLatLon(iscan, iray + 1, zM);
        corners[2] = _getCornerLatLon(iscan + 1, iray + 1, zM);
        corners[3] = _getCornerLatLon(iscan + 1, iray, zM);

        // dbz vals at corners
        NcxxPort::si32 vals[4];
        vals[0] = valsIn[iscan][iray];
        vals[1] = valsIn[iscan][iray + 1];
        vals[2] = valsIn[iscan + 1][iray + 1];
        vals[3] = valsIn[iscan + 1][iray];

        // check for valid VAL
        int nGood = 0;
        for (int ii = 0; ii < 4; ii++) {
          if (vals[ii] != missingVal) {
            nGood++;
          }
        }
        if (nGood < 1) {
          continue;
        }
        
        // interp for the output grid points inside the polygon
        
        _interpInsidePolygon(corners, vals, missingVal, iz, valsInterp);
        
      } // iray
    } // iscan

  } // iz

}

//////////////////////////////////////////////
// interpolate points within polygon - floats

void GpmHdf5ToMdv::_interpInsidePolygon(const Point_d *corners,
                                        const NcxxPort::fl32 *vals,
                                        NcxxPort::fl32 missingVal,
                                        size_t iz,
                                        vector<NcxxPort::fl32> &valsInterp,
                                        bool nearestNeighbor)
 
{

  // compute lat/lon bounding box
  
  double minLat = corners[0].y;
  double maxLat = corners[0].y;
  double minLon = corners[0].x;
  double maxLon = corners[0].x;
  
  for (int ii = 1; ii < 4; ii++) {
    minLat = min(minLat, corners[ii].y);
    maxLat = max(maxLat, corners[ii].y);
    minLon = min(minLon, corners[ii].x);
    maxLon = max(maxLon, corners[ii].x);
  }
  
  // compute the output grid limits for the bounding box
  
  int minIx = (int) ((minLon - _minxDeg) / _dxDeg);
  int maxIx = (int) ((maxLon - _minxDeg) / _dxDeg + 1.0);
  
  int minIy = (int) ((minLat - _minyDeg) / _dyDeg);
  int maxIy = (int) ((maxLat - _minyDeg) / _dyDeg + 1.0);
  
  minIx = max(minIx, 0);
  minIy = max(minIy, 0);
  maxIx = min(maxIx, (int) _nx - 1);
  maxIy = min(maxIy, (int) _ny - 1);
  
  // loop through the output grid points, finding if they are inside the polygon

  for (int iy = minIy; iy <= maxIy; iy++) {
    for (int ix = minIx; ix <= maxIx; ix++) {
      Point_d pt;
      pt.x = _minxDeg + ix * _dxDeg;
      pt.y = _minyDeg + iy * _dyDeg;
      if (EGS_point_in_polygon(pt, (Point_d *) corners, 4)) {
        double interpVal = _interpPt(pt, corners, vals, missingVal, nearestNeighbor);
        size_t outputIndex = iz * _nx * _ny + iy * _nx + ix;
        valsInterp[outputIndex] = interpVal;
      }
    } // ix
  } // iy

}

//////////////////////////////////////////////
// interpolate points within polygon - int

void GpmHdf5ToMdv::_interpInsidePolygon(const Point_d *corners,
                                        const NcxxPort::si32 *vals,
                                        NcxxPort::si32 missingVal,
                                        size_t iz,
                                        vector<NcxxPort::si32> &valsInterp)
 
{

  // compute lat/lon bounding box
  
  double minLat = corners[0].y;
  double maxLat = corners[0].y;
  double minLon = corners[0].x;
  double maxLon = corners[0].x;
  
  for (int ii = 1; ii < 4; ii++) {
    minLat = min(minLat, corners[ii].y);
    maxLat = max(maxLat, corners[ii].y);
    minLon = min(minLon, corners[ii].x);
    maxLon = max(maxLon, corners[ii].x);
  }
  
  // compute the output grid limits for the bounding box
  
  int minIx = (int) ((minLon - _minxDeg) / _dxDeg);
  int maxIx = (int) ((maxLon - _minxDeg) / _dxDeg + 1.0);
  
  int minIy = (int) ((minLat - _minyDeg) / _dyDeg);
  int maxIy = (int) ((maxLat - _minyDeg) / _dyDeg + 1.0);
  
  minIx = max(minIx, 0);
  minIy = max(minIy, 0);
  maxIx = min(maxIx, (int) _nx - 1);
  maxIy = min(maxIy, (int) _ny - 1);
  
  // loop through the output grid points, finding if they are inside the polygon

  for (int iy = minIy; iy <= maxIy; iy++) {
    for (int ix = minIx; ix <= maxIx; ix++) {
      Point_d pt;
      pt.x = _minxDeg + ix * _dxDeg;
      pt.y = _minyDeg + iy * _dyDeg;
      if (EGS_point_in_polygon(pt, (Point_d *) corners, 4)) {
        NcxxPort::si32 interpVal = _interpPt(pt, corners, vals, missingVal);
        size_t outputIndex = iz * _nx * _ny + iy * _nx + ix;
        valsInterp[outputIndex] = interpVal;
      }
    } // ix
  } // iy

}

//////////////////////////////////////////////
// interpolate point within polygon - double

NcxxPort::fl32 GpmHdf5ToMdv::_interpPt(const Point_d &pt,
                               const Point_d *corners,
                               const NcxxPort::fl32 *vals,
                               NcxxPort::fl32 missingVal,
                               bool nearestNeighbor)
  
{
  
  // compute the distances from the pt to the corners
  
  double dist[4];
  for (int ii = 0; ii < 3; ii++) {
    double dx = pt.x - corners[ii].x;
    double dy = pt.y - corners[ii].y;
    dist[ii] = sqrt(dx * dx + dy * dy);
  }
  
  // nearest neighbor?

  if (nearestNeighbor) {
    double minDist = 1.0e99;
    int minIndex = -1;
    for (int ii = 0; ii < 3; ii++) {
      if (dist[ii] < minDist) {
        minDist = dist[ii];
        if (vals[ii] != missingVal) {
          minIndex = ii;
        }
      }
    }
    return vals[minIndex];
  }
  
  // inverse distance weighted interpolation
  
  double weight[4];
  double sumWt = 0.0;
  double sumInterp = 0.0;
  for (int ii = 0; ii < 3; ii++) {
    if (vals[ii] != missingVal) {
      weight[ii] = pow(1.0 / dist[ii], _params.interp_power_parameter);
      sumWt += weight[ii];
      sumInterp += weight[ii] * vals[ii];
    }
  }
  double interpVal = sumInterp / sumWt;

  return interpVal;

}
  
//////////////////////////////////////////////
// interpolate point within polygon - int
// always uses nearest neighbor

int GpmHdf5ToMdv::_interpPt(const Point_d &pt,
                            const Point_d *corners,
                            const int *vals,
                            int missingVal)
  
{
  
  // compute the distances from the pt to the corners
  
  double dist[4];
  for (int ii = 0; ii < 3; ii++) {
    double dx = pt.x - corners[ii].x;
    double dy = pt.y - corners[ii].y;
    dist[ii] = sqrt(dx * dx + dy * dy);
  }

  // find nearest

  double minDist = 1.0e99;
  int minIndex = -1;
  for (int ii = 0; ii < 3; ii++) {
    if (dist[ii] < minDist) {
      minDist = dist[ii];
      if (vals[ii] != missingVal) {
        minIndex = ii;
      }
    }
  }

  return vals[minIndex];
  
}
  
/////////////////////////////////////////////////////////////////
// get lat/lon of a grid corner, for given height (zM)
// adjusting the corner locations for slant from radar to this point

Point_d GpmHdf5ToMdv::_getCornerLatLon(int iscan,
                                       int iray,
                                       double zM)

{

  // ht as a fraction of the spacecraft height
  
  double zFraction = zM / _scAlt[iscan];
  
  // get the point
  
  Point_d pt = _latLons[iscan][iray];
  
  double cornerLat = pt.y;
  double slantDeltaLat = (_scLat[iscan] - cornerLat) * zFraction;
  cornerLat += slantDeltaLat;
  
  double cornerLon = pt.x;
  double slantDeltaLon = (_scLon[iscan] - cornerLon) * zFraction;
  cornerLon += slantDeltaLon;

  Point_d corner;
  corner.y = cornerLat;
  corner.x = cornerLon;
  
  return corner;

}

/////////////////////////////////////////////////////////////////
// get lat/lon of a grid corner, at surface

Point_d GpmHdf5ToMdv::_getCornerLatLon(int iscan,
                                       int iray)

{
  return _latLons[iscan][iray];
}

////////////////////////////////////////////////////////
// invert the height levels because the data is stored
// with the top first and decreasing in height 

void GpmHdf5ToMdv::_invertDbzGateLevels()
{

  // prepare output grid
  
  _dbzOutput.resize(_dbzInterp.size());
  _zLevels.resize(_nz);
  for (size_t iz = 0; iz < _nz; iz++) {
    _zLevels[iz] = _minzKm + iz * _dzKm;
  }

  // invert vlevels

  size_t nptsPlane = _nx * _ny;
  for (size_t iz = 0; iz < _nz; iz++) {
    for (size_t iy = 0; iy < _ny; iy++) {
      for (size_t ix = 0; ix < _nx; ix++) {
        size_t interpIndex = iz * nptsPlane + iy * _nx + ix;
        size_t outputIndex = (_nz - iz - 1) * nptsPlane + iy * _nx + ix;
        _dbzOutput[outputIndex] = _dbzInterp[interpIndex];
      } // ix
    } // iy
  } // iz

}
  
////////////////////////////////////////////////////////
// remap the gates onto specified vertical levels
// compute the max for the remapping

void GpmHdf5ToMdv::_remapVertLevels()
{

  _dbzInterp = _dbzOutput;

  // zlevels are specified

  _zLevels.resize(_params.output_z_levels_km_n);
  for (int iz = 0; iz < _params.output_z_levels_km_n; iz++) {
    _zLevels[iz] = _params._output_z_levels_km[iz];
  }

  // prepare output grid

  size_t nPtsOutput = _nx * _ny * _zLevels.size();
  _dbzOutput.resize(nPtsOutput);

  // compute heights of mid pt between specified levels

  vector<double> zMid;
  for (size_t ii = 0; ii < _zLevels.size() - 1; ii++) {
    zMid.push_back((_zLevels[ii] + _zLevels[ii+1]) / 2.0);
  }
  vector<int> lowIndex, highIndex;
  lowIndex.resize(_zLevels.size());
  highIndex.resize(_zLevels.size());
  lowIndex[0] = 0;
  highIndex[_zLevels.size()-1] = _nz - 1;
  for (size_t iz = 1; iz < _zLevels.size(); iz++) {
    lowIndex[iz] = (int) ((zMid[iz-1] - _minzKm) / _dzKm) + 1;
  }
  for (size_t iz = 0; iz < _zLevels.size() - 1; iz++) {
    highIndex[iz] = (int) ((zMid[iz] - _minzKm) / _dzKm);
  }
  
  // compute max dbz for specified vlevels

  size_t nptsPlane = _nx * _ny;
  for (size_t iy = 0; iy < _ny; iy++) {
    for (size_t ix = 0; ix < _nx; ix++) {
      for (size_t iz = 0; iz < _zLevels.size(); iz++) {
        NcxxPort::fl32 maxDbz = _missingDbz;
        for (int jz = lowIndex[iz]; jz <= highIndex[iz]; jz++) {
          size_t interpIndex = jz * nptsPlane + iy * _nx + ix;
          if (_dbzInterp[interpIndex] != _missingDbz) {
            maxDbz = max(maxDbz, _dbzInterp[interpIndex]);
          }
        } // jz
        size_t outputIndex = iz * nptsPlane + iy * _nx + ix;
        _dbzOutput[outputIndex] = maxDbz;
      } // iz
    } // ix
  } // iy

}
  
/////////////////////////////////////////////////
// Set the master header from the NCF file
//
// Returns 0 on success, -1 on failure

int GpmHdf5ToMdv::_setMasterHeader(DsMdvx &mdvx)

{

  mdvx.clearMasterHeader();

  // time

  if (_params.debug) {
    cerr << "===========================================" << endl;
    cerr << "Found data set at start time: " << _startTime.asString(3) << endl;
  }

  mdvx.setBeginTime(_startTime.utime());
  mdvx.setEndTime(_endTime.utime());
  mdvx.setValidTime(_endTime.utime());
  
  // data collection type
  
  mdvx.setDataCollectionType(Mdvx::DATA_MEASURED);

  // data set name, source and info

  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource("NASA-GPM");
  mdvx.setDataSetInfo(_history.c_str());

  return 0;

}

///////////////////////////////////
// Create an Mdvx field - floats

MdvxField *GpmHdf5ToMdv::_createMdvxField(const string &fieldName,
                                          const string &longName,
                                          const string &units,
                                          size_t nx, size_t ny, size_t nz,
                                          double minx, double miny, double minz,
                                          double dx, double dy, double dz,
                                          NcxxPort::fl32 missingVal,
                                          NcxxPort::fl32 *vals)

{

  // check max levels

  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  
  // _inputProj.syncToFieldHdr(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = true;
  fhdr.data_dimension = 3;
  
  fhdr.scale = 1.0;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  
  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = _zLevels.size();

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = minz;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (size_t ii = 0; ii < _zLevels.size(); ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zLevels[ii];
  }

  // create MdvxField object
  // converting data to encoding and compression types
  
  MdvxField *field = new MdvxField(fhdr, vhdr, vals);
  field->convertType((Mdvx::encoding_type_t) _params.output_encoding_type,
                     Mdvx::COMPRESSION_GZIP);

  // set names etc
  
  field->setFieldName(fieldName);
  field->setFieldNameLong(longName);
  field->setUnits(units);
  field->setTransform("");

  return field;

}
  
///////////////////////////////////
// Create an Mdvx field - ints

MdvxField *GpmHdf5ToMdv::_createMdvxField(const string &fieldName,
                                          const string &longName,
                                          const string &units,
                                          size_t nx, size_t ny, size_t nz,
                                          double minx, double miny, double minz,
                                          double dx, double dy, double dz,
                                          NcxxPort::si16 missingVal,
                                          NcxxPort::si16 *vals)

{

  // check max levels

  if (nz > MDV_MAX_VLEVELS) {
    nz = MDV_MAX_VLEVELS;
  }

  // set up MdvxField headers

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  
  // _inputProj.syncToFieldHdr(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = true;
  fhdr.data_dimension = 3;
  
  fhdr.scale = 1.0;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.encoding_type = Mdvx::ENCODING_INT16;
  fhdr.data_element_nbytes = sizeof(si16);
  
  fhdr.nx = nx;
  fhdr.ny = ny;
  fhdr.nz = _zLevels.size();

  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(si32);

  fhdr.grid_minx = minx;
  fhdr.grid_miny = miny;
  fhdr.grid_minz = minz;

  fhdr.grid_dx = dx;
  fhdr.grid_dy = dy;
  fhdr.grid_dz = dz;
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  for (size_t ii = 0; ii < _zLevels.size(); ii++) {
    vhdr.type[ii] = Mdvx::VERT_TYPE_Z;
    vhdr.level[ii] = _zLevels[ii];
  }

  // create MdvxField object
  // converting data to encoding and compression types
  
  MdvxField *field = new MdvxField(fhdr, vhdr, vals);
  field->convertType((Mdvx::encoding_type_t) _params.output_encoding_type,
                     Mdvx::COMPRESSION_GZIP);

  // set names etc
  
  field->setFieldName(fieldName);
  field->setFieldNameLong(longName);
  field->setUnits(units);
  field->setTransform("");

  return field;

}
  
