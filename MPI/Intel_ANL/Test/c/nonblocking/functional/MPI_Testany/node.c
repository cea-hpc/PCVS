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
*                          Functional test for MPI_Testany
*
*  Testany references:
*
*    MPI Standard:  Section 3.7.5  Multiple Completions
*                   Section 3.7.4  Semantics of Nonblocking Communications
*                   Section 3.7.3  For MPI_Request_free
*  MPI_Request_free states that an ongoing communication associated with
*      the request will be allowed to complete, after which the request
*      will be deallocated.  After deallocation, the request becomes
*      equal to MPI_REQUEST_NULL
*
*  Section 3.7.5  of the MPI Standard notes that if one or more of the
*      communications completed by a call to MPI_Testany fail, the function
*      will return ther error code:  MPI_ERR_IN_STATUS, and will set the
*      error field of each status to a spceific error code, which will
*      be MPI_SUCCESS if that specific communication was a success
*
*  This test sends messages from node 0 to node 1, and uses MPI_Testany
*  to check for their proper reception.  After the send the program calls
*  MPI_Request_free for two of the messages to ensure they are sent before
*  the Request Objects are freed.  This test Does a Testany on messages
*  that have already been Testanyed on.  The code verifies that a successful
*  waited on a message sets the request oobject for that message to
*  MPI_REQUEST_NULL.  The MPI_Test_cancelled function is used to test the
*  status object for the send operation, as per the MPI Standard, section 3.7.3
*
*
* Test history:
*    1  05/10/96     jh   Created
*
******************************************************************************/

#define  PROG_NAME   "MPI_Testany"

#define  NUMMESG     20	/* Number of messages to Isend/Irecv                 */
#define  NUMELM      10	/* # of elements to send and receive                 */

#include "mpitest_cfg.h"
#include "mpitest.h"

int main(int argc, char *argv[])
{
    int
        flag,   	/* flag return from  MPI_Test calls                  */
        i,              /* general index variables                           */
        cnt_len,	/* received length returned by MPI_Get_Count         */
        fail,   	/* counts total number # of failures                 */
        error,  	/* errors from one MPI call                          */
        ierr,   	/* return value from MPI calls                       */
        errorclass,	/* error class of ierr                               */
        size,   	/* length of error message                           */
        index,          /* index of completed operation from MPI_Testany     */
        send_loop_cnt,  /* count tries on send Testall loop                  */
        recv_loop_cnt,  /* count tries on receive Testall loop               */
        total_cnt;	/* total test count                                  */

    char
        testname[128],	/* the name of this test                             */
        info_buf[256];	/* for sprintf                                       */
    char error_string[MPI_MAX_ERROR_STRING];

    struct dataTemplate
        value;   	/* dataTemplate for initializing buffers             */

    MPI_Request
	send_request[4*NUMMESG],
	recv_request[4*NUMMESG]; /* MPI request structure                    */

    MPI_Status
        one_stat,
	send_stat[4*NUMMESG],	/* Send Source/Tag information               */
	recv_stat[4*NUMMESG];	/* Recv Source/Tag information               */

    int
        recv_buffer[4*NUMMESG][NUMELM],	/* messages to receive               */
        send_buffer[4*NUMMESG][NUMELM];	/* messages to send                  */

    char
        bsend_buff[NUMMESG * (8*NUMELM + MPI_BSEND_OVERHEAD + 100)];
    char *bb;
    /*-----------------------  MPI Initialization  --------------------------*/

    /*
     * *  Initialize the MPI environment and test environment.
     */

    ierr = MPI_Init(&argc, &argv);
    if (ierr != MPI_SUCCESS)
    {
	sprintf(info_buf, "Non-zero return code (%d) from MPI_Init()", ierr);
	MPITEST_message(MPITEST_FATAL, info_buf);
    }

    sprintf(testname, "%s", PROG_NAME);


    /*--------------------  MPITEST Initialization  -------------------------*/

    MPITEST_init(argc, argv);

    if (MPITEST_me == 0)
    {
	sprintf(info_buf, "Starting:  %s ", testname);
	MPITEST_message(MPITEST_INFO0, info_buf);

    }

    total_cnt = 0;
    fail = 0;

    /* For this simple wait test we need only two nodes  */

    if (MPITEST_nump < 2)
    {
	if (MPITEST_me == 0)
	{
	    sprintf(info_buf, "At least 2 ranks required to run this test");
	    MPITEST_message(MPITEST_FATAL, info_buf);
	}
    }


    /*-------------------------------  ISEND --------------------------------*/
    if (MPITEST_me == 0)
    {
	ierr = MPI_Buffer_attach(bsend_buff,
				 NUMMESG * (8*NUMELM + MPI_BSEND_OVERHEAD + 100));
	if (ierr != MPI_SUCCESS)
	{
	    sprintf(info_buf, "Non-zero return code (%d) from MPI_Buffer_attach"
, ierr);
	    MPITEST_message(MPITEST_NONFATAL, info_buf);
	    MPI_Error_string(ierr, error_string, &size);
	    MPITEST_message(MPITEST_FATAL, error_string);
	    fail++;
	}

	/* Initialize Send buffers */

	for (i = 0; i < 4*NUMMESG; i++)
	{
	    MPITEST_dataTemplate_init(&value, i);
	    MPITEST_init_buffer(MPITEST_int, NUMELM, value, &send_buffer[i]);
	}

	MPI_Barrier(MPI_COMM_WORLD);
  
	for (i = 0; i < NUMMESG; i++)
	{
	    ierr = MPI_Isend(send_buffer[4*i],
			     NUMELM,
			     MPI_INT,
			     1,
			     4*i,
			     MPI_COMM_WORLD,
			     &send_request[4*i]);

	    total_cnt++;
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code (%d) from MPI_Isend", ierr);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }   /* Isend  Error Test  */

	    ierr = MPI_Ibsend(send_buffer[4*i+1],
			     NUMELM,
			     MPI_INT,
			     1,
			     4*i+1,
			     MPI_COMM_WORLD,
			     &send_request[4*i+1]);

	    total_cnt++;
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code (%d) from MPI_Ibsend", ierr);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }   /* Ibsend  Error Test  */

	    ierr = MPI_Irsend(send_buffer[4*i+2],
			     NUMELM,
			     MPI_INT,
			     1,
			     4*i+2,
			     MPI_COMM_WORLD,
			     &send_request[4*i+2]);

	    total_cnt++;
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code (%d) from MPI_Irsend", ierr);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }   /* Irsend  Error Test  */

	    ierr = MPI_Issend(send_buffer[4*i+3],
			     NUMELM,
			     MPI_INT,
			     1,
			     4*i+3,
			     MPI_COMM_WORLD,
			     &send_request[4*i+3]);

	    total_cnt++;
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code (%d) from MPI_Issend", ierr);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }   /* Issend  Error Test  */

	}       /* Send Loop */


	/*----------------  Testany for all  messages  to send -------------*/

	for (i = 0; i < 4*NUMMESG; i++)
	{

	    flag = FALSE;
	    ierr = FALSE;
	    send_loop_cnt = 0;
	    while (!flag && !ierr) 
	    {
		ierr = MPI_Testany(4*NUMMESG, send_request, &index, &flag, &one_stat);
		send_loop_cnt++;
	    }
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code ierr/flag (%d/%d) from MPI_Testall on Isend, after %d calls to MPI_Testany", ierr, flag, send_loop_cnt);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }	/* End of Testany Error Test  */


	    sprintf(info_buf,"Testany on send:  Index %d   Error %d  #Testany calls %d ", index, ierr, send_loop_cnt );
	    MPITEST_message(MPITEST_INFO1, info_buf);
 
	    if( index >= 0  )
	    {
		send_stat[index] = one_stat;
	    }

	} /* End of Testany for loop */

/*  At this point at least one  message that was sent has been Testany ed on */


	total_cnt++;
	ierr = MPI_Test_cancelled(&send_stat[index], &flag);
	if (ierr != MPI_SUCCESS)
	{
	    sprintf(info_buf, "Non-zero return code (%d) from MPI_Test_cancelled", ierr);
	    MPITEST_message(MPITEST_NONFATAL, info_buf);
	    MPI_Error_string(ierr, error_string, &size);
	    MPITEST_message(MPITEST_NONFATAL, error_string);
	    fail++;
	}	/* MPI_Test_cancelled Error Test */

	total_cnt++;
	if (flag != FALSE)
	{
	    sprintf(info_buf, "MPI_Test_cancelled flag set,  record = %d,  flag = %d", i, flag);
	    MPITEST_message(MPITEST_NONFATAL, info_buf);
	    MPI_Error_string(ierr, error_string, &size);
	    MPITEST_message(MPITEST_NONFATAL, error_string);
	    fail++;
	}	/* MPI_Test_cancelled Error Test */

	ierr = MPI_Buffer_detach(&bb, &size);
	if (ierr != MPI_SUCCESS)
	{
	    sprintf(info_buf, "Non-zero return code (%d) from MPI_Buffer_detach"
, ierr);
	    MPITEST_message(MPITEST_NONFATAL, info_buf);
	    MPI_Error_string(ierr, error_string, &size);
	    MPITEST_message(MPITEST_FATAL, error_string);
	    fail++;
	}      

    }	/* Isend from node 0 */

    /*---------------------------------  Irecv  -----------------------------*/
    if (MPITEST_me == 1)
    {
	/* Initialize Receive  buffers */
	for (i = 0; i < 4*NUMMESG; i++)
	{
	    MPITEST_dataTemplate_init(&value, -1);
	    MPITEST_init_buffer(MPITEST_int, NUMELM, value, &recv_buffer[i]);
	}

	for (i = 0; i < 4*NUMMESG; i++)
	{
	    ierr = MPI_Irecv(recv_buffer[i],
			     NUMELM,
			     MPI_INT,
			     0,
			     i,
			     MPI_COMM_WORLD,
			     &recv_request[i]);

	    total_cnt++;
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code (%d) from MPI_Irecv", ierr);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }	/* Irecv Error Test  */

	}	/* Receive Loop  */

	MPI_Barrier(MPI_COMM_WORLD);

	/*-------  First Irecv  Testany, wait on one message  ---------*/

	for (i = 0; i < 4*NUMMESG; i++)
	{
	    flag = FALSE;
	    ierr = FALSE;
	    recv_loop_cnt = 0;

	    while (!flag && !ierr) 
	    {
		ierr = MPI_Testany(4*NUMMESG, recv_request, &index, &flag, &one_stat);
		recv_loop_cnt++;
	    }

	    sprintf(info_buf,"Testany on receive:  Index %d    Error %d ", index, ierr );
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    if( index >= 0  )
	    {
		recv_stat[index] = one_stat;
	    }

	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code ierr/flag (%d/%d) from MPI_Testany on Irecv, after %d calls to MPI_Testall", ierr, flag, recv_loop_cnt);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;

		/* If error is #17, MPI_ERR_IN_STATUS print out status  */

		MPI_Error_class(ierr, &errorclass);
		if (errorclass == MPI_ERR_IN_STATUS)
		{
		    sprintf(info_buf, "MPI_ERR_IN_STATUS on receive, printing non-zero statuses");
		    MPITEST_message(MPITEST_NONFATAL, info_buf);
		    total_cnt++;
		    ierr = recv_stat[index].MPI_ERROR;
		    sprintf(info_buf, "Error in received record %d, Source = %d,  Tag =  %d,  Error # =  %d", index, recv_stat[index].MPI_SOURCE, recv_stat[index].MPI_TAG, recv_stat[index].MPI_ERROR);
		    MPITEST_message(MPITEST_NONFATAL, info_buf);
		    MPI_Error_string(recv_stat[index].MPI_ERROR, error_string, &size);
		    MPITEST_message(MPITEST_NONFATAL, error_string);
		    fail++;

		}	/* End of MPI_ERR_IN_STATUS check */

	    }	/* End of Testany error test for receive */

	}  /* Testany for loop on all messages  */

/*  At this point all messages that were received have been Testany ed on  */
  
	for (i = 0; i < 4*NUMMESG; i++)
	{

	    /*
	     * Set up the dataTemplate for checking the recv'd buffer.  Note
	     * that the sending record number  will be sent.
	     */
	    MPITEST_dataTemplate_init(&value, i);

	    total_cnt++;

	    error = MPITEST_buffer_errors(MPITEST_int, NUMELM, value, recv_buffer[i]);
	    if (error)
	    {
		sprintf(info_buf, "Unexpected value in buffer %d, actual =  %d    expected = %d", i, recv_buffer[i][0], i);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		fail++;
	    }

	    /*
	     * Call the MPI_Get_Count function, and compare value with NUMELM
	     */
	    cnt_len = -1;

	    total_cnt++;
	    ierr = MPI_Get_count(&recv_stat[i], MPI_INT, &cnt_len);
	    if (ierr != MPI_SUCCESS)
	    {
		sprintf(info_buf, "Non-zero return code (%d) from MPI_Get_count", ierr);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		MPI_Error_string(ierr, error_string, &size);
		MPITEST_message(MPITEST_NONFATAL, error_string);
		fail++;
	    }

	    /*
	     * Print non-fatal error if Received length not equal to send
	     * length
	     */
	    total_cnt++;

	    error = NUMELM - cnt_len;

	    if (error)
	    {
		sprintf(info_buf, "send/receive lengths differ - Sender(length)=%d,  Receiver(index/length)=%d/%d", NUMELM, i, cnt_len);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		fail++;
	    }

	    /*
	     * Print non-fatal error if tag is not correct.
	     */
	    total_cnt++;
	    if (recv_stat[i].MPI_TAG != i)
	    {
		sprintf(info_buf, "Unexpected tag for #%d  value=%d, expected=%d", i, recv_stat[i].MPI_TAG, i);
		MPITEST_message(MPITEST_NONFATAL, info_buf);
		fail++;
	    }
	}	/* End of for loop: test received records */

    }	/* End of node 1 receives  */

    if (MPITEST_me >= 2)
    {	/* rank >= 2 need to match Barrier above */
	MPI_Barrier(MPI_COMM_WORLD);
    }

    /*
     * All nodes stop here so we can finish all sends and receives before we
     * finalize the run
     */

    MPI_Barrier(MPI_COMM_WORLD);


    /* report overall results  */

    MPITEST_report(total_cnt - fail, fail, 0, testname);

    MPI_Finalize();
    return fail;

} /* main() */
