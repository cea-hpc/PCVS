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
/******************************************************************************
                     Error test for MPI_Keyval_free()

This test verifies that the correct error is returned if MPI_Keyval_free()
is called with invalid arguments.

MPI_Keyval_free error tests
-----------------------------------
1)  Call with MPI_KEYVAL_INVALID..............[MPI_ERR_ARG/MPI_ERR_OTHER]
    10/30/02 - MPI-1.2 allow MPI_ERR_KEYVAL

In all cases, expect to receive appropriate error.

Rank 0 will call MPI_Keyval_free with MPI_KEYVAL_INVALID.
The resulting error code will then be checked and the corresponding
error class will be verified to make sure it is MPI_ERR_ARG/MPI_ERR_OTHER.

All other rank(s) will simply do nothing.

MPI Calls dependencies for this test:
  MPI_Keyval_free(), MPI_Init(), MPI_Finalize()
  MPI_Error_string()
  [MPI_Allreduce(), MPI_Comm_size(), MPI_Comm_rank()]

Test history:
   1  08/05/96     brdavis      Original version
******************************************************************************/
#include <limits.h>

#include "mpitest_cfg.h"
#include "mpitest.h"

int main(int argc, char *argv[])
{
    int
        pass, fail,	/* counts total number # of failures                 */
        ierr, ierr2,    /* return value from MPI calls                       */
        errorclass,	/* error class of ierr                               */
        size,
	key;

    char
        testname[128],	/* the name of this test                             */
        info_buf[256];	/* for sprintf                                       */
    char error_string[MPI_MAX_ERROR_STRING];

    MPI_Comm newcomm;

    /*-----------------------------------------------------------------------*/

    /*   
    **  Initialize the MPI environment and test environment.
    */

    ierr = MPI_Init(&argc, &argv);
    if (ierr != MPI_SUCCESS) {
       sprintf(info_buf, "MPI_Init() returned %d", ierr);
       MPITEST_message(MPITEST_FATAL, info_buf);
    }

    sprintf(testname, "MPI_Keyval_free_err1: MPI_KEYVAL_INVALID");

    MPITEST_init(argc, argv);
    if (MPITEST_me == 0) {
       sprintf(info_buf, "Starting %s test", testname);
       MPITEST_message(MPITEST_INFO0, info_buf);
    }

    pass = 0;
    fail = 0;

    /* Set an errorhandler so we get control back. */
    ierr = MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
    if (ierr != MPI_SUCCESS) {
       fail++;
       sprintf(info_buf, "MPI_Errorhandler_set returned %d", ierr);
       MPITEST_message(MPITEST_NONFATAL, info_buf);
       MPI_Error_string(ierr, error_string, &size);
       MPITEST_message(MPITEST_FATAL, error_string);
    }

    if (MPITEST_me == 0) {
      /* Calling MPI_Keyval_free with MPI_KEYVAL_INVALID */
      sprintf(info_buf, "Calling MPI_Keyval_free with MPI_KEYVAL_INVALID");
      MPITEST_message(MPITEST_INFO1, info_buf);

      key = MPI_KEYVAL_INVALID;
      ierr2 = MPI_Keyval_free(&key);
      if (ierr2 == MPI_SUCCESS) {
	fail++;
	sprintf(info_buf, "MPI_Keyval_free() returned MPI_SUCCESS");
	MPITEST_message(MPITEST_NONFATAL, info_buf);
      }
      else {
	ierr = MPI_Error_class(ierr2, &errorclass);
	if (ierr != MPI_SUCCESS) {
	  fail++;
          sprintf(info_buf, "MPI_Error_class() returned %d", ierr);
          MPITEST_message(MPITEST_NONFATAL, info_buf);
          MPI_Error_string(ierr, error_string, &size);
          MPITEST_message(MPITEST_FATAL, error_string);
	}
#define CLASS_KEYVAL
#ifdef MPI_VERSION
#  if MPI_VERSION > 1 || MPI_SUBVERSION > 1
#       undef CLASS_KEYVAL
#       define CLASS_KEYVAL && (errorclass != MPI_ERR_KEYVAL)
#  endif
#endif
	else if ((errorclass != MPI_ERR_ARG) && 
		 (errorclass != MPI_ERR_OTHER) CLASS_KEYVAL) {
	  fail++;
          sprintf(info_buf, "MPI_Keyval_free() with MPI_KEYVAL_INVALID returned error class %d, expected MPI_ERR_ARG or MPI_ERR_OTHER", errorclass);
          MPITEST_message(MPITEST_NONFATAL, info_buf);
          MPI_Error_string(ierr2, error_string, &size);
          MPITEST_message(MPITEST_NONFATAL, error_string);
        }
        else {
          pass++;
          sprintf(info_buf, "ierr = %d, errorclass = %d", ierr2,
                  errorclass);
          MPITEST_message(MPITEST_INFO2, info_buf);
          MPI_Error_string(ierr2, error_string, &size);
          MPITEST_message(MPITEST_INFO1, error_string);
        }
      }
    }

    /* report overall results  */
    MPITEST_report(pass, fail, 0, testname);

    ierr = MPI_Finalize();

    return fail;
}/* main() */
