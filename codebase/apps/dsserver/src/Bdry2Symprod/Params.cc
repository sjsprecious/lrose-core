// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research(UCAR)
// ** National Center for Atmospheric Research(NCAR)
// ** Boulder, Colorado, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////
// Params.cc
//
// TDRP C++ code file for class 'Params'.
//
// Code for program Bdry2Symprod
//
// This file has been automatically
// generated by TDRP, do not modify.
//
/////////////////////////////////////////////

/**
 *
 * @file Params.cc
 *
 * @class Params
 *
 * This class is automatically generated by the Table
 * Driven Runtime Parameters (TDRP) system
 *
 * @note Source is automatically generated from
 *       paramdef file at compile time, do not modify
 *       since modifications will be overwritten.
 *
 *
 * @author Automatically generated
 *
 */
using namespace std;

#include "Params.hh"
#include <cstring>

  ////////////////////////////////////////////
  // Default constructor
  //

  Params::Params()

  {

    // zero out table

    memset(_table, 0, sizeof(_table));

    // zero out members

    memset(&_start_, 0, &_end_ - &_start_);

    // class name

    _className = "Params";

    // initialize table

    _init();

    // set members

    tdrpTable2User(_table, &_start_);

    _exitDeferred = false;

  }

  ////////////////////////////////////////////
  // Copy constructor
  //

  Params::Params(const Params& source)

  {

    // sync the source object

    source.sync();

    // zero out table

    memset(_table, 0, sizeof(_table));

    // zero out members

    memset(&_start_, 0, &_end_ - &_start_);

    // class name

    _className = "Params";

    // copy table

    tdrpCopyTable((TDRPtable *) source._table, _table);

    // set members

    tdrpTable2User(_table, &_start_);

    _exitDeferred = false;

  }

  ////////////////////////////////////////////
  // Destructor
  //

  Params::~Params()

  {

    // free up

    freeAll();

  }

  ////////////////////////////////////////////
  // Assignment
  //

  void Params::operator=(const Params& other)

  {

    // sync the other object

    other.sync();

    // free up any existing memory

    freeAll();

    // zero out table

    memset(_table, 0, sizeof(_table));

    // zero out members

    memset(&_start_, 0, &_end_ - &_start_);

    // copy table

    tdrpCopyTable((TDRPtable *) other._table, _table);

    // set members

    tdrpTable2User(_table, &_start_);

    _exitDeferred = other._exitDeferred;

  }

  ////////////////////////////////////////////
  // loadFromArgs()
  //
  // Loads up TDRP using the command line args.
  //
  // Check usage() for command line actions associated with
  // this function.
  //
  //   argc, argv: command line args
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   char **params_path_p:
  //     If this is non-NULL, it is set to point to the path
  //     of the params file used.
  //
  //   bool defer_exit: normally, if the command args contain a 
  //      print or check request, this function will call exit().
  //      If defer_exit is set, such an exit is deferred and the
  //      private member _exitDeferred is set.
  //      Use exidDeferred() to test this flag.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadFromArgs(int argc, char **argv,
                           char **override_list,
                           char **params_path_p,
                           bool defer_exit)
  {
    int exit_deferred;
    if (_tdrpLoadFromArgs(argc, argv,
                          _table, &_start_,
                          override_list, params_path_p,
                          _className,
                          defer_exit, &exit_deferred)) {
      return (-1);
    } else {
      if (exit_deferred) {
        _exitDeferred = true;
      }
      return (0);
    }
  }

  ////////////////////////////////////////////
  // loadApplyArgs()
  //
  // Loads up TDRP using the params path passed in, and applies
  // the command line args for printing and checking.
  //
  // Check usage() for command line actions associated with
  // this function.
  //
  //   const char *param_file_path: the parameter file to be read in
  //
  //   argc, argv: command line args
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   bool defer_exit: normally, if the command args contain a 
  //      print or check request, this function will call exit().
  //      If defer_exit is set, such an exit is deferred and the
  //      private member _exitDeferred is set.
  //      Use exidDeferred() to test this flag.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadApplyArgs(const char *params_path,
                            int argc, char **argv,
                            char **override_list,
                            bool defer_exit)
  {
    int exit_deferred;
    if (tdrpLoadApplyArgs(params_path, argc, argv,
                          _table, &_start_,
                          override_list,
                          _className,
                          defer_exit, &exit_deferred)) {
      return (-1);
    } else {
      if (exit_deferred) {
        _exitDeferred = true;
      }
      return (0);
    }
  }

  ////////////////////////////////////////////
  // isArgValid()
  // 
  // Check if a command line arg is a valid TDRP arg.
  //

  bool Params::isArgValid(const char *arg)
  {
    return (tdrpIsArgValid(arg));
  }

  ////////////////////////////////////////////
  // load()
  //
  // Loads up TDRP for a given class.
  //
  // This version of load gives the programmer the option to load
  // up more than one class for a single application. It is a
  // lower-level routine than loadFromArgs, and hence more
  // flexible, but the programmer must do more work.
  //
  //   const char *param_file_path: the parameter file to be read in.
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   expand_env: flag to control environment variable
  //               expansion during tokenization.
  //               If TRUE, environment expansion is set on.
  //               If FALSE, environment expansion is set off.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::load(const char *param_file_path,
                   char **override_list,
                   int expand_env, int debug)
  {
    if (tdrpLoad(param_file_path,
                 _table, &_start_,
                 override_list,
                 expand_env, debug)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // loadFromBuf()
  //
  // Loads up TDRP for a given class.
  //
  // This version of load gives the programmer the option to
  // load up more than one module for a single application,
  // using buffers which have been read from a specified source.
  //
  //   const char *param_source_str: a string which describes the
  //     source of the parameter information. It is used for
  //     error reporting only.
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   const char *inbuf: the input buffer
  //
  //   int inlen: length of the input buffer
  //
  //   int start_line_num: the line number in the source which
  //     corresponds to the start of the buffer.
  //
  //   expand_env: flag to control environment variable
  //               expansion during tokenization.
  //               If TRUE, environment expansion is set on.
  //               If FALSE, environment expansion is set off.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadFromBuf(const char *param_source_str,
                          char **override_list,
                          const char *inbuf, int inlen,
                          int start_line_num,
                          int expand_env, int debug)
  {
    if (tdrpLoadFromBuf(param_source_str,
                        _table, &_start_,
                        override_list,
                        inbuf, inlen, start_line_num,
                        expand_env, debug)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // loadDefaults()
  //
  // Loads up default params for a given class.
  //
  // See load() for more detailed info.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadDefaults(int expand_env)
  {
    if (tdrpLoad(NULL,
                 _table, &_start_,
                 NULL, expand_env, FALSE)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // sync()
  //
  // Syncs the user struct data back into the parameter table,
  // in preparation for printing.
  //
  // This function alters the table in a consistent manner.
  // Therefore it can be regarded as const.
  //

  void Params::sync(void) const
  {
    tdrpUser2Table(_table, (char *) &_start_);
  }

  ////////////////////////////////////////////
  // print()
  // 
  // Print params file
  //
  // The modes supported are:
  //
  //   PRINT_SHORT:   main comments only, no help or descriptions
  //                  structs and arrays on a single line
  //   PRINT_NORM:    short + descriptions and help
  //   PRINT_LONG:    norm  + arrays and structs expanded
  //   PRINT_VERBOSE: long  + private params included
  //

  void Params::print(FILE *out, tdrp_print_mode_t mode)
  {
    tdrpPrint(out, _table, _className, mode);
  }

  ////////////////////////////////////////////
  // checkAllSet()
  //
  // Return TRUE if all set, FALSE if not.
  //
  // If out is non-NULL, prints out warning messages for those
  // parameters which are not set.
  //

  int Params::checkAllSet(FILE *out)
  {
    return (tdrpCheckAllSet(out, _table, &_start_));
  }

  //////////////////////////////////////////////////////////////
  // checkIsSet()
  //
  // Return TRUE if parameter is set, FALSE if not.
  //
  //

  int Params::checkIsSet(const char *paramName)
  {
    return (tdrpCheckIsSet(paramName, _table, &_start_));
  }

  ////////////////////////////////////////////
  // freeAll()
  //
  // Frees up all TDRP dynamic memory.
  //

  void Params::freeAll(void)
  {
    tdrpFreeAll(_table, &_start_);
  }

  ////////////////////////////////////////////
  // usage()
  //
  // Prints out usage message for TDRP args as passed
  // in to loadFromArgs().
  //

  void Params::usage(ostream &out)
  {
    out << "TDRP args: [options as below]\n"
        << "   [ -params/--params path ] specify params file path\n"
        << "   [ -check_params/--check_params] check which params are not set\n"
        << "   [ -print_params/--print_params [mode]] print parameters\n"
        << "     using following modes, default mode is 'norm'\n"
        << "       short:   main comments only, no help or descr\n"
        << "                structs and arrays on a single line\n"
        << "       norm:    short + descriptions and help\n"
        << "       long:    norm  + arrays and structs expanded\n"
        << "       verbose: long  + private params included\n"
        << "       short_expand:   short with env vars expanded\n"
        << "       norm_expand:    norm with env vars expanded\n"
        << "       long_expand:    long with env vars expanded\n"
        << "       verbose_expand: verbose with env vars expanded\n"
        << "   [ -tdrp_debug] debugging prints for tdrp\n"
        << "   [ -tdrp_usage] print this usage\n";
  }

  ////////////////////////////////////////////
  // arrayRealloc()
  //
  // Realloc 1D array.
  //
  // If size is increased, the values from the last array 
  // entry is copied into the new space.
  //
  // Returns 0 on success, -1 on error.
  //

  int Params::arrayRealloc(const char *param_name, int new_array_n)
  {
    if (tdrpArrayRealloc(_table, &_start_,
                         param_name, new_array_n)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // array2DRealloc()
  //
  // Realloc 2D array.
  //
  // If size is increased, the values from the last array 
  // entry is copied into the new space.
  //
  // Returns 0 on success, -1 on error.
  //

  int Params::array2DRealloc(const char *param_name,
                             int new_array_n1,
                             int new_array_n2)
  {
    if (tdrpArray2DRealloc(_table, &_start_, param_name,
                           new_array_n1, new_array_n2)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // _init()
  //
  // Class table initialization function.
  //
  //

  void Params::_init()

  {

    TDRPtable *tt = _table;

    // Parameter 'Comment 0'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 0");
    tt->comment_hdr = tdrpStrDup("DEBUGGING");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'debug'
    // ctype is '_debug_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("debug");
    tt->descr = tdrpStrDup("Debug option");
    tt->help = tdrpStrDup("If set, debug messages will be printed appropriately");
    tt->val_offset = (char *) &debug - &_start_;
    tt->enum_def.name = tdrpStrDup("debug_t");
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("DEBUG_OFF");
      tt->enum_def.fields[0].val = DEBUG_OFF;
      tt->enum_def.fields[1].name = tdrpStrDup("DEBUG_NORM");
      tt->enum_def.fields[1].val = DEBUG_NORM;
      tt->enum_def.fields[2].name = tdrpStrDup("DEBUG_VERBOSE");
      tt->enum_def.fields[2].val = DEBUG_VERBOSE;
    tt->single_val.e = DEBUG_OFF;
    tt++;
    
    // Parameter 'Comment 1'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 1");
    tt->comment_hdr = tdrpStrDup("PROCESS CONTROL");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'instance'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("instance");
    tt->descr = tdrpStrDup("Process instance.");
    tt->help = tdrpStrDup("Used for procmap registration and auto restarting.");
    tt->val_offset = (char *) &instance - &_start_;
    tt->single_val.s = tdrpStrDup("");
    tt++;
    
    // Parameter 'no_threads'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("no_threads");
    tt->descr = tdrpStrDup("Option to prevent server from using a thread per client.");
    tt->help = tdrpStrDup("For debugging purposes it it sometimes useful to suppress the use of threads. Set no_threads to TRUE for this type of debugging.");
    tt->val_offset = (char *) &no_threads - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'Comment 2'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 2");
    tt->comment_hdr = tdrpStrDup("Server details for support from ServerMgr");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'port'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("port");
    tt->descr = tdrpStrDup("Port number.");
    tt->help = tdrpStrDup("The server listens on this port for client requests.");
    tt->val_offset = (char *) &port - &_start_;
    tt->single_val.i = 5452;
    tt++;
    
    // Parameter 'qmax'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("qmax");
    tt->descr = tdrpStrDup("Max quiescent period (secs).");
    tt->help = tdrpStrDup("If the server does not receive requests for this time period, it will die gracefully. If set to -1, the server never exits.");
    tt->val_offset = (char *) &qmax - &_start_;
    tt->single_val.i = -1;
    tt++;
    
    // Parameter 'max_clients'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("max_clients");
    tt->descr = tdrpStrDup("Maximum number of clients");
    tt->help = tdrpStrDup("This is the maximum number of threads the application will produce to handle client requests.  If the maximum is reached, new clients will receive a SERVICE_DENIED error message and will have to request the data again.  If set to -1, no maximum is enforced.");
    tt->val_offset = (char *) &max_clients - &_start_;
    tt->single_val.i = 64;
    tt++;
    
    // Parameter 'Comment 3'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 3");
    tt->comment_hdr = tdrpStrDup("RENDERING PARAMETERS");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'display_detections'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("display_detections");
    tt->descr = tdrpStrDup("display detection flag");
    tt->help = tdrpStrDup("If FALSE, the detections will not be displayed.");
    tt->val_offset = (char *) &display_detections - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'detection_color'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("detection_color");
    tt->descr = tdrpStrDup("detection color");
    tt->help = tdrpStrDup("Name of color suggested for rendering the detection polyline.");
    tt->val_offset = (char *) &detection_color - &_start_;
    tt->single_val.s = tdrpStrDup("purple");
    tt++;
    
    // Parameter 'display_extrapolations'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("display_extrapolations");
    tt->descr = tdrpStrDup("display extrapolation flag");
    tt->help = tdrpStrDup("If FALSE, the extrapolations will not be displayed.");
    tt->val_offset = (char *) &display_extrapolations - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'extrapolation_color'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("extrapolation_color");
    tt->descr = tdrpStrDup("extrapolation color");
    tt->help = tdrpStrDup("Name of color suggested for rendering the extrapolation polyline.");
    tt->val_offset = (char *) &extrapolation_color - &_start_;
    tt->single_val.s = tdrpStrDup("blue");
    tt++;
    
    // Parameter 'display_line_type'
    // ctype is '_line_type_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("display_line_type");
    tt->descr = tdrpStrDup("display line type");
    tt->help = tdrpStrDup("Line type suggested for using on the display.");
    tt->val_offset = (char *) &display_line_type - &_start_;
    tt->enum_def.name = tdrpStrDup("line_type_t");
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("LINETYPE_SOLID");
      tt->enum_def.fields[0].val = LINETYPE_SOLID;
      tt->enum_def.fields[1].name = tdrpStrDup("LINETYPE_DASH");
      tt->enum_def.fields[1].val = LINETYPE_DASH;
      tt->enum_def.fields[2].name = tdrpStrDup("LINETYPE_DOT_DASH");
      tt->enum_def.fields[2].val = LINETYPE_DOT_DASH;
    tt->single_val.e = LINETYPE_SOLID;
    tt++;
    
    // Parameter 'display_line_width'
    // ctype is 'long'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = LONG_TYPE;
    tt->param_name = tdrpStrDup("display_line_width");
    tt->descr = tdrpStrDup("display line width");
    tt->help = tdrpStrDup("Line width suggested for using on the display.");
    tt->val_offset = (char *) &display_line_width - &_start_;
    tt->single_val.l = 1;
    tt++;
    
    // Parameter 'display_capstyle'
    // ctype is '_capstyle_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("display_capstyle");
    tt->descr = tdrpStrDup("display capstyle");
    tt->help = tdrpStrDup("Capstyle suggested for using on the display.");
    tt->val_offset = (char *) &display_capstyle - &_start_;
    tt->enum_def.name = tdrpStrDup("capstyle_t");
    tt->enum_def.nfields = 4;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("CAPSTYLE_BUTT");
      tt->enum_def.fields[0].val = CAPSTYLE_BUTT;
      tt->enum_def.fields[1].name = tdrpStrDup("CAPSTYLE_NOT_LAST");
      tt->enum_def.fields[1].val = CAPSTYLE_NOT_LAST;
      tt->enum_def.fields[2].name = tdrpStrDup("CAPSTYLE_PROJECTING");
      tt->enum_def.fields[2].val = CAPSTYLE_PROJECTING;
      tt->enum_def.fields[3].name = tdrpStrDup("CAPSTYLE_ROUND");
      tt->enum_def.fields[3].val = CAPSTYLE_ROUND;
    tt->single_val.e = CAPSTYLE_BUTT;
    tt++;
    
    // Parameter 'display_joinstyle'
    // ctype is '_joinstyle_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("display_joinstyle");
    tt->descr = tdrpStrDup("display joinstyle");
    tt->help = tdrpStrDup("Joinstyle suggested for using on the display.");
    tt->val_offset = (char *) &display_joinstyle - &_start_;
    tt->enum_def.name = tdrpStrDup("joinstyle_t");
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("JOINSTYLE_BEVEL");
      tt->enum_def.fields[0].val = JOINSTYLE_BEVEL;
      tt->enum_def.fields[1].name = tdrpStrDup("JOINSTYLE_MITER");
      tt->enum_def.fields[1].val = JOINSTYLE_MITER;
      tt->enum_def.fields[2].name = tdrpStrDup("JOINSTYLE_ROUND");
      tt->enum_def.fields[2].val = JOINSTYLE_ROUND;
    tt->single_val.e = JOINSTYLE_BEVEL;
    tt++;
    
    // Parameter 'display_label'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("display_label");
    tt->descr = tdrpStrDup("display label flag");
    tt->help = tdrpStrDup("If TRUE, the boundary label will be displayed with the boundary.");
    tt->val_offset = (char *) &display_label - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'label_source'
    // ctype is '_LabelSource'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("label_source");
    tt->descr = tdrpStrDup("label source flag");
    tt->help = tdrpStrDup("Label indicating where to get the label for the boundary.  For COLIDE boundaries, the label should come from the description field in the shape.  For other boundaries, I would expect the label to come from within the polyline itself since each shape is allowed to contain more than one polyline.");
    tt->val_offset = (char *) &label_source - &_start_;
    tt->enum_def.name = tdrpStrDup("LabelSource");
    tt->enum_def.nfields = 2;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("LABEL_DESCRIP");
      tt->enum_def.fields[0].val = LABEL_DESCRIP;
      tt->enum_def.fields[1].name = tdrpStrDup("LABEL_POLYLINE");
      tt->enum_def.fields[1].val = LABEL_POLYLINE;
    tt->single_val.e = LABEL_POLYLINE;
    tt++;
    
    // Parameter 'label_font'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("label_font");
    tt->descr = tdrpStrDup("label font name");
    tt->help = tdrpStrDup("The name of the X font to be used when rendering the label.");
    tt->val_offset = (char *) &label_font - &_start_;
    tt->single_val.s = tdrpStrDup("8x13");
    tt++;
    
    // Parameter 'display_vector'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("display_vector");
    tt->descr = tdrpStrDup("display vector flag");
    tt->help = tdrpStrDup("If TRUE, the motion vector will be displayed with the boundary.");
    tt->val_offset = (char *) &display_vector - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'vector_color'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("vector_color");
    tt->descr = tdrpStrDup("motion vector color");
    tt->help = tdrpStrDup("Name of color suggested for rendering the motion vector.");
    tt->val_offset = (char *) &vector_color - &_start_;
    tt->single_val.s = tdrpStrDup("purple");
    tt++;
    
    // Parameter 'extrap_secs'
    // ctype is 'long'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = LONG_TYPE;
    tt->param_name = tdrpStrDup("extrap_secs");
    tt->descr = tdrpStrDup("extrapolation seconds to use when rendering the motion vector");
    tt->help = tdrpStrDup("Since we are now extrapolating the boundaries upon request, we don't know the extrapolation time period anymore.  So, we must now supply it.");
    tt->val_offset = (char *) &extrap_secs - &_start_;
    tt->has_min = TRUE;
    tt->min_val.l = 0;
    tt->single_val.l = 0;
    tt++;
    
    // Parameter 'head_length'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("head_length");
    tt->descr = tdrpStrDup("Vector arrowhead length in km");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &head_length - &_start_;
    tt->single_val.d = 5;
    tt++;
    
    // Parameter 'head_half_angle'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("head_half_angle");
    tt->descr = tdrpStrDup("Vector arrowhead half angle in degrees");
    tt->help = tdrpStrDup("This is the angle between the arrowhead line and the vector line.");
    tt->val_offset = (char *) &head_half_angle - &_start_;
    tt->single_val.d = 45;
    tt++;
    
    // Parameter 'calc_extrapolations'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("calc_extrapolations");
    tt->descr = tdrpStrDup("calculate extrapolations flag");
    tt->help = tdrpStrDup("If TRUE, the server will calculate the extrapolation polylines rather than using the extrapolations in the database.");
    tt->val_offset = (char *) &calc_extrapolations - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'point_extrapolations'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("point_extrapolations");
    tt->descr = tdrpStrDup("use point motion extrapolation method");
    tt->help = tdrpStrDup("If TRUE, and calc_extrapolations is TRUE, extrapolate each point of the boundary based on speed of the boundary at that point.  If FALSE, the average motion vector will be used to calculate the extrapolation");
    tt->val_offset = (char *) &point_extrapolations - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'latitude_shift'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("latitude_shift");
    tt->descr = tdrpStrDup("vertex latitude shift");
    tt->help = tdrpStrDup("This value is added to the latitude of each vertex in each boundary to shift the boundaries relative to a new location.");
    tt->val_offset = (char *) &latitude_shift - &_start_;
    tt->single_val.d = 0;
    tt++;
    
    // Parameter 'longitude_shift'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("longitude_shift");
    tt->descr = tdrpStrDup("vertex longitude shift");
    tt->help = tdrpStrDup("This value is added to the longitude of each vertex in each boundary to shift the boundaries relative to a new location.");
    tt->val_offset = (char *) &longitude_shift - &_start_;
    tt->single_val.d = 0;
    tt++;
    
    // Parameter 'take_color_from_id'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("take_color_from_id");
    tt->descr = tdrpStrDup("Option to code the boundary colors from the boundary ID's");
    tt->help = tdrpStrDup("May be useful for human entered boundaries. It allows CIDD\nto essentially be used as a drawing tool. If the ID is not\nspecified in the color look up table (LUT) then the detection\nor extrapolation color is used as appropriate.");
    tt->val_offset = (char *) &take_color_from_id - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'color_lut'
    // ctype is '_color_lut_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("color_lut");
    tt->descr = tdrpStrDup("Look up table for colors based on boundary ID's.\nRelevant only if take_color_from_id is TRUE.");
    tt->help = tdrpStrDup("");
    tt->array_offset = (char *) &_color_lut - &_start_;
    tt->array_n_offset = (char *) &color_lut_n - &_start_;
    tt->is_array = TRUE;
    tt->array_len_fixed = FALSE;
    tt->array_elem_size = sizeof(color_lut_t);
    tt->array_n = 10;
    tt->struct_def.name = tdrpStrDup("color_lut_t");
    tt->struct_def.nfields = 2;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("int");
      tt->struct_def.fields[0].fname = tdrpStrDup("bdryID");
      tt->struct_def.fields[0].ptype = INT_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &_color_lut->bdryID - (char *) _color_lut;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("color");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &_color_lut->color - (char *) _color_lut;
    tt->n_struct_vals = 20;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].i = 0;
      tt->struct_vals[1].s = tdrpStrDup("white");
      tt->struct_vals[2].i = 1;
      tt->struct_vals[3].s = tdrpStrDup("cyan");
      tt->struct_vals[4].i = 2;
      tt->struct_vals[5].s = tdrpStrDup("purple");
      tt->struct_vals[6].i = 3;
      tt->struct_vals[7].s = tdrpStrDup("red");
      tt->struct_vals[8].i = 4;
      tt->struct_vals[9].s = tdrpStrDup("yellow");
      tt->struct_vals[10].i = 5;
      tt->struct_vals[11].s = tdrpStrDup("gold");
      tt->struct_vals[12].i = 6;
      tt->struct_vals[13].s = tdrpStrDup("orange");
      tt->struct_vals[14].i = 7;
      tt->struct_vals[15].s = tdrpStrDup("lightgreen");
      tt->struct_vals[16].i = 8;
      tt->struct_vals[17].s = tdrpStrDup("pink");
      tt->struct_vals[18].i = 9;
      tt->struct_vals[19].s = tdrpStrDup("lightblue");
    tt++;
    
    // Parameter 'display_shear_info'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("display_shear_info");
    tt->descr = tdrpStrDup("Flag indicating whether to display shear information");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &display_shear_info - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'shear_field_to_display'
    // ctype is '_shear_field_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("shear_field_to_display");
    tt->descr = tdrpStrDup("Parameter indicating which shear field to display");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &shear_field_to_display - &_start_;
    tt->enum_def.name = tdrpStrDup("shear_field_t");
    tt->enum_def.nfields = 6;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("SHEAR_FIELD_NUM_PTS");
      tt->enum_def.fields[0].val = SHEAR_FIELD_NUM_PTS;
      tt->enum_def.fields[1].name = tdrpStrDup("SHEAR_FIELD_ZBAR_CAPE");
      tt->enum_def.fields[1].val = SHEAR_FIELD_ZBAR_CAPE;
      tt->enum_def.fields[2].name = tdrpStrDup("SHEAR_FIELD_MAX_SHEAR");
      tt->enum_def.fields[2].val = SHEAR_FIELD_MAX_SHEAR;
      tt->enum_def.fields[3].name = tdrpStrDup("SHEAR_FIELD_MEAN_SHEAR");
      tt->enum_def.fields[3].val = SHEAR_FIELD_MEAN_SHEAR;
      tt->enum_def.fields[4].name = tdrpStrDup("SHEAR_FIELD_KMIN");
      tt->enum_def.fields[4].val = SHEAR_FIELD_KMIN;
      tt->enum_def.fields[5].name = tdrpStrDup("SHEAR_FIELD_KMAX");
      tt->enum_def.fields[5].val = SHEAR_FIELD_KMAX;
    tt->single_val.e = SHEAR_FIELD_MAX_SHEAR;
    tt++;
    
    // Parameter 'shear_field_format_string'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("shear_field_format_string");
    tt->descr = tdrpStrDup("Format string for shear information");
    tt->help = tdrpStrDup("For information on see \"man sprintf\".");
    tt->val_offset = (char *) &shear_field_format_string - &_start_;
    tt->single_val.s = tdrpStrDup("%f");
    tt++;
    
    // Parameter 'shear_field_color'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("shear_field_color");
    tt->descr = tdrpStrDup("Shear field text color");
    tt->help = tdrpStrDup("Name of color suggested for rendering the shear field.");
    tt->val_offset = (char *) &shear_field_color - &_start_;
    tt->single_val.s = tdrpStrDup("purple");
    tt++;
    
    // Parameter 'shear_min_distance_btwn_fields'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("shear_min_distance_btwn_fields");
    tt->descr = tdrpStrDup("Minimum distance, in km, between shear points for displaying shear values.");
    tt->help = tdrpStrDup("When displaying shear information, we'll start at one end of the boundary and display the specified value. We'll then follow the points through the boundary until we reach a point that is further than this distance from the previous point where we displayed a value, and we'll display the next value there.");
    tt->val_offset = (char *) &shear_min_distance_btwn_fields - &_start_;
    tt->single_val.d = 0;
    tt++;
    
    // trailing entry has param_name set to NULL
    
    tt->param_name = NULL;
    
    return;
  
  }
