/*-----------------------------------------------------------------------------
MESSAGE PASSING INTERFACE TEST CASE SUITE

Copyright - 1996 Intel Corporation

Intel Corporation hereby grants a non-exclusive license under Intel's
copyright to copy, modify and distribute this software for any purpose 
and without fee, provided that the above copyright notice and the following
paragraphs appear on all copies.

Intel Corporation makes no representation that the test cases comprising
this suite are correct or are an accurate representation of any standard.

IN NO EVENT SHALL INTEL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT OR
SPECULATIVE DAMAGES, (INCLUDING WITHOUT LIMITING THE FOREGOING, CONSEQUENTIAL,
INCIDENTAL AND SPECIAL DAMAGES) INCLUDING, BUT NOT LIMITED TO INFRINGEMENT,
LOSS OF USE, BUSINESS INTERRUPTIONS, AND LOSS OF PROFITS, IRRESPECTIVE OF
WHETHER INTEL HAS ADVANCE NOTICE OF THE POSSIBILITY OF ANY SUCH DAMAGES.

INTEL CORPORATION SPECIFICALLY DISCLAIMS ANY WARRANTIES INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NON-INFRINGEMENT.  THE SOFTWARE PROVIDED HEREUNDER
IS ON AN "AS IS" BASIS AND INTEL CORPORATION HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS OR MODIFICATIONS.
-----------------------------------------------------------------------------*/
/*******************************************************************************
*                          Error test for MPI_Allgather()
* 
* This test verifies that the correct error is returned if MPI_Allgather is
* called with an invalid argument.
* 
* 1)  Use mis-matched types.
* 2)  Use invalid type.
* 3)  Use invalid size (-1).
* 4)  Use MPI_COMM_NULL.
* 5)  Use a freed communicator.
* 6)  Use an inter-communicator.
*
* In all cases, expect to receive appropriate error.
*
* 
* Test history:
*    1  01/12/96     gt   Created
* 
*******************************************************************************/
#include "mpitest_cfg.h"
#include "mpitest.h"

int main( int argc, char *argv[])
{
  int 
    pass,
    fail,              /*  counts total number of failures */
    verify,            /*  counts total number of verify failures */
    ierr,              /*  return value from MPI calls     */
    errorclass,        /*  error class of ierr             */
    color,             /*  used to split a communicator    */
    size;

  double
    *input,          /* input to gather */
    *output;         /* results of gather */

  char 
    testname[128],     /*  the name of this test           */
    info_buf[256];     /*  for fprintf                     */
    char error_string[MPI_MAX_ERROR_STRING];
  
  
  MPI_Comm comm,       /*  MPI communicators               */
	   comm2;

/*
**  Initialize the MPI environment and test environment.
*/
ierr = MPI_Init(&argc, &argv);
if( ierr!=MPI_SUCCESS)
  {
    sprintf(info_buf, "Non-zero return code (%d) from MPI_Init()", ierr);
    MPITEST_message(MPITEST_FATAL, info_buf);
  }

sprintf( testname, "MPI_Allgather_err5");
MPITEST_init( argc, argv);
if (MPITEST_me==0)
  {
    sprintf(info_buf, "Starting %s test", testname);
    MPITEST_message(MPITEST_INFO0, info_buf);
  }


pass = 0;
fail = 0;
verify = 0;
input = (double *)malloc((MPITEST_nump+1)*sizeof(double));
output = (double *)malloc((MPITEST_nump+1)*sizeof(double));
if ((input == NULL) || (output == NULL))
  MPITEST_message(MPITEST_FATAL, "Couldn't allocate test arrays");

for (size=0; size <= MPITEST_nump; size++) input[size]=0;


/*
**  Set an errorhandler so we get control back.
*/
ierr = MPI_Errhandler_set(MPI_COMM_WORLD,MPI_ERRORS_RETURN);
if (ierr != MPI_SUCCESS)
  {
    fail++;
    sprintf(info_buf, "MPI_Errorhandler_set returned %d", ierr);
    MPITEST_message(MPITEST_FATAL, info_buf);
  }


/*
**  Invalid communicator: Freed Communicator
*/
ierr = MPI_Comm_dup(MPI_COMM_WORLD, &comm);
if (ierr != MPI_SUCCESS)
  {
    fail++;
    sprintf(info_buf, "MPI_Comm_dup(MPI_COMM_WORLD) returned %d", ierr);
    MPITEST_message(MPITEST_FATAL, info_buf);
  }

ierr = MPI_Errhandler_set(comm,MPI_ERRORS_RETURN);
if (ierr != MPI_SUCCESS)
  {
    fail++;
    sprintf(info_buf, "MPI_Errorhandler_set returned %d", ierr);
    MPITEST_message(MPITEST_FATAL, info_buf);
  }

comm2 = comm;
ierr = MPI_Comm_free(&comm2);
if (ierr != MPI_SUCCESS)
  {
    fail++;
    sprintf(info_buf, "MPI_Comm_free(dup) returned %d", ierr);
    MPITEST_message(MPITEST_FATAL, info_buf);
  }

sprintf(info_buf, "about to call MPI_Allgather(Free comm)");
MPITEST_message( MPITEST_INFO1, info_buf);

ierr = 0;
ierr = MPI_Allgather(input, MPITEST_nump, MPI_INT, output, MPITEST_nump, MPI_INT, comm);
if (ierr == MPI_SUCCESS)
  {
    sprintf(info_buf, "MPI_Allgather(Free Comm) did not FAIL");
    MPITEST_message(MPITEST_NONFATAL, info_buf);
    fail++;
  }
else
  {
    MPI_Error_class(ierr, &errorclass);
    if (errorclass != MPI_ERR_COMM)
  {
	fail++;
	sprintf(info_buf, "MPI_Allgather(Free Comm) returned %d, expected MPI_ERR_COMM",
	    errorclass);
	MPITEST_message(MPITEST_NONFATAL, info_buf);
	MPI_Error_string(ierr, error_string, &size);
	MPITEST_message( MPITEST_NONFATAL, info_buf);
      }
    else
      {
	pass++;
	MPI_Error_class(ierr, &errorclass);
	sprintf(info_buf, "ierr = %d, errorclass = %d", ierr,
	     errorclass);
	MPITEST_message( MPITEST_INFO2, info_buf);
	MPI_Error_string(ierr, error_string, &size);
	MPITEST_message( MPITEST_INFO1, info_buf);
      }
  }


/* report overall results  */
MPITEST_report(pass,fail, verify, testname);
      
MPI_Finalize();
return fail;
} /* main() */
