/*****************************************************************************
 *
 *  NAME:     async/node.c
 *
 *  SYNOPSIS:  This test stresses asynchronous node-to-node
 *             communication.
 *
 *  USAGE: node.nx [-d] [-h] [-l maxloopcnt] [-m maxlen ] [-r] [-s seed]
 *                 [-t minutes] [-v] [-V]
 *
 *         -d   Print Defaults
 *         -h   Print Usage Message
 *         -l   maxloopcnt: number of outer loops, -1 = run forever
 *                          default = DFL_NUM_MSG
 *         -m   maxlen: largest message length,
 *                      default (scales to the size of the partition) =
 *                      DFL_NUM_MSGS*sizeof(long)*(num_nodes - 1)
 *         -r   Enable random message lengths by setting useRandom = 1,
 *              default = 0
 *         -s   seed: seed for the random number generator
 *                    default = (long)(1000*MPI_Wtime())
 *         -t   minutes: number of minutes the test should run, sets
 *                       maxloopcnt to -1 and calls alarm()
 *
 *  DESCRIPTION:
 *              Inner Loop: (repeated maxcnt = maxlen/sizeof(int) times)
 *              1)  Each node isend()s a message of length x to node y
 *                  where:
 *                  x = starts at 4 bytes and increases by 4 bytes for
 *                      each message sent OR (determined by the value of
 *                      useRandom) x is a random number between 0 and maxlen
 *                  y = starts at (mynode() + 1) and increases by 1
 *                      node (mod numnodes()) for each message sent.
 *                      A node does not isend() a message to itself.
 *              2)  Each node irecv()s a message and increments a count of
 *                  the number of messages received form each node.
 *
 *              msgdone()s for the isend()s and irecv()s are not done
 *              one-for-one at the beginning of a loop, but they must
 *              sync up (be equal) at the end of each inner loop.
 *
 *              Outer Loop: (repeated maxloopcnt times)
 *              1)  Do the inner loop
 *              2)  Compare the message counts from the inner loop to the
 *                  calculated values.
 *
 *              Both SIGALRM and SIGINT are caught so that PASS/FAIL
 *              information can be printed before the program terminates.
 *              SIGALRM is generated by alarm(minutes) and SIGINT is generated
 *              by ^C. longjmp is called by the signal handler
 *              to skip to the end of the test and print results.
 *
 *              To reduce the quantity of output with large numbers of
 *              nodes, only node 0 prints status information. All nodes
 *              print error information.
 *
 *              Specifying the seed value allows a random sequence to be
 *              repeated.
 *
 *  HISTORY:
 *
 *  12/05/89   tes   Created
 *  05/13/91   tes   Re-created for delta.
 *  10/28/93   egb   Ported to Paragon with improvements
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

#include "mpitest_cfg.h"
#include "mpitest.h"

#define DFL_NUM_MSGS 50
#define SEED_TYPE    1000

/* Globals */
MPI_Status mpi_status;
MPI_Request send_request, recv_request;
int mpi_count, mpi_flag;

int failed, loopcnt;
int maxcnt, maxloopcnt;
int useRandom, minutes;
unsigned int seed;

long maxlen;

char timebuf[20];

/* Globals for getopt() */
int opterr;
char *optarg;

/* Environment variables for set/longjmp */
int sig_caught;
jmp_buf to_vec;

/* Function Prototypes */
void get_cmd_opt(int, char **);
long get_length(long);
void init_expect(int *expect);
char *tod(void);

char info_buf[512];	/* buffer for passing messages to
					 * MPITEST */
char error_string[MPI_MAX_ERROR_STRING];

/*****************************************************************************
 *
 *   main()
 *
 *****************************************************************************/
int main(int argc, char *argv[])
{
   int rcnt, scnt;
   int failloop, i, ierr, size;
   int *expect, *rec_tally;

   long *rbuf, *sbuf;
   long dest_node, len;
   long type;

   double lastprint;

   ierr = MPI_Init(&argc, &argv);
   if (ierr != MPI_SUCCESS) {
      sprintf(info_buf, "MPI_Init() returns (%d)\n", ierr);
      MPITEST_message(MPITEST_FATAL, info_buf);
   }

   MPITEST_init(argc, argv);

   /*
    * Initialize node info variables. Set default values for maxlen and
    * maxloopcnt based on partition size. Set maxlen so that each node sends
    * DFL_NUM_MSGS messages to all nodes. Set maxloopcnt to 100. Set minutes
    * to 0 so that maxloopcnt loops are performed.
    */

   maxlen = DFL_NUM_MSGS * (MPITEST_nump - 1);
   maxloopcnt = DFL_NUM_MSGS;
   useRandom = 0;
   minutes = 0;
   seed = (unsigned int) (1000 * MPI_Wtime());

   /*
    * Parse command line arguments, set maxcnt (the number of msgs passed per
    * loop)
    */
   get_cmd_opt(argc, argv);
   maxcnt = maxlen;

   /* Broadcast seed value and seed random number generator */
   ierr = MPI_Bcast(&seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
   if (ierr != MPI_SUCCESS) {
      sprintf(info_buf, "MPI_Bcast() returned %d", ierr);
      MPITEST_message(MPITEST_NONFATAL, info_buf);
      MPI_Error_string(ierr, error_string, &size);
      MPITEST_message(MPITEST_FATAL, error_string);
   }

   srand(seed * (MPITEST_me + 1));

   if (MPITEST_me == 0) {
      sprintf(info_buf, "(%d) %s Start ASYNC test\n", MPITEST_me, tod());
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\tmaxcnt     = %d\n", maxcnt);
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\tmaxlen     = %ld\n", maxlen);
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\tmaxloopcnt = %d\n", maxloopcnt);
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\tminutes    = %d\n", minutes);
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\tnum_nodes  = %d\n", MPITEST_nump);
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\trandom     = %d\n", useRandom);
      MPITEST_message(MPITEST_INFO1, info_buf);

      sprintf(info_buf, "\tseed       = %u\n", seed);
      MPITEST_message(MPITEST_INFO1, info_buf);
   }

   /*
    * Allocate user buffers
    */
   if ((rbuf = (long *) malloc(maxlen * sizeof(long))) == NULL) {
      sprintf(info_buf, "malloc *** FAILED *** for rbuf[]");
      MPITEST_message(MPITEST_FATAL, info_buf);
   }

   if ((sbuf = (long *) malloc(maxlen * sizeof(long))) == NULL) {
      sprintf(info_buf, "malloc *** FAILED *** for sbuf[]");
      MPITEST_message(MPITEST_FATAL, info_buf);
   }

   /* Initialize buffers */
   for (i = 0; i < maxcnt; i++) {
      rbuf[i] = i;
      sbuf[i] = i;
   }

   /*
    * Allocate tally arrays
    */
   if ((rec_tally = (int *) calloc(MPITEST_nump, sizeof(long))) == NULL) {
      sprintf(info_buf, "calloc() *** FAILED *** for rec_tally[]");
      MPITEST_message(MPITEST_FATAL, info_buf);
   }

   if ((expect = (int *) calloc(MPITEST_nump, sizeof(int))) == NULL) {
      sprintf(info_buf, "calloc() *** FAILED *** for expect[]");
      MPITEST_message(MPITEST_FATAL, info_buf);
   }

   /*
    * Initialize expect[n] with the number of expected messages from node n.
    * The number of messages depends on maxlen and the numnodes.
    */
   init_expect(expect);

   /*
    * Pass messages
    */
   failed = 0;
   loopcnt = 0;

   while (maxloopcnt) {
      loopcnt++;

      /* if maxloopcnt < 0 then loop forever, else decrement */
      if (maxloopcnt > 0) {
	 maxloopcnt--;
      }

      /* zero loop fail flag and rec_tally[] */
      failloop = 0;
      for (i = 0; i < MPITEST_nump; i++) {
	 rec_tally[i] = 0;
      }

      /*
       * Post receive and send for first message
       */
      len = get_length(0);
      type = loopcnt;
      dest_node = (MPITEST_me + 1) % MPITEST_nump;
      scnt = 0;
      rcnt = 0;

      ierr = MPI_Irecv(rbuf, maxlen, MPI_LONG, MPI_ANY_SOURCE, type, MPI_COMM_WORLD, &recv_request);
      if (ierr != MPI_SUCCESS) {
	 sprintf(info_buf, "MPI_Irecv() returned %d", ierr);
	 MPITEST_message(MPITEST_NONFATAL, info_buf);
	 MPI_Error_string(ierr, error_string, &size);
	 MPITEST_message(MPITEST_FATAL, error_string);
      }

      ierr = MPI_Isend(sbuf, len, MPI_LONG, dest_node, type, MPI_COMM_WORLD, &send_request);
      if (ierr != MPI_SUCCESS) {
	 sprintf(info_buf, "MPI_Isend() returned %d", ierr);
	 MPITEST_message(MPITEST_NONFATAL, info_buf);
	 MPI_Error_string(ierr, error_string, &size);
	 MPITEST_message(MPITEST_FATAL, error_string);
      }

      /* Loop for message lengths 4, 8, 12, ..., maxlen */
      lastprint = MPI_Wtime();
      while ((scnt < maxcnt) || (rcnt < maxcnt)) {

	 /*
	  * Check if isend has completed, post another isend if it has
	  * completed.
	  */
	 if (scnt < maxcnt) {

	    ierr = MPI_Test(&send_request, &mpi_flag, &mpi_status);
	    if (ierr != MPI_SUCCESS) {
	       sprintf(info_buf, "MPI_Test() returned %d", ierr);
	       MPITEST_message(MPITEST_NONFATAL, info_buf);
	       MPI_Error_string(ierr, error_string, &size);
	       MPITEST_message(MPITEST_FATAL, error_string);
	    }

	    if (mpi_flag) {

	       sprintf(info_buf, "(%d) loop %d - MPI_Isend() complete: scnt = %d, dest_node = %ld, len = %ld\n",
		      MPITEST_me, loopcnt, scnt, dest_node, len);
	       MPITEST_message(MPITEST_INFO1, info_buf);

	       scnt++;
	       dest_node = (dest_node + 1) % MPITEST_nump;
	       if (dest_node == MPITEST_me) {
		  dest_node = (dest_node + 1) % MPITEST_nump;
	       }

	       /* Send the next message */
	       if (scnt < maxcnt) {
		  len = get_length(len);
		  ierr = MPI_Isend(sbuf, len, MPI_LONG, dest_node, type, MPI_COMM_WORLD, &send_request);
		  if (ierr != MPI_SUCCESS) {
		     sprintf(info_buf, "MPI_Isend() returned %d", ierr);
		     MPITEST_message(MPITEST_NONFATAL, info_buf);
		     MPI_Error_string(ierr, error_string, &size);
		     MPITEST_message(MPITEST_FATAL, error_string);
		  }
	       }
	    }			/* MPI_Test */
	 }			/* scnt < maxcnt */

	 /*
	  * Check if irecv has completed, post another recv if it has
	  * completed.
	  */
	 if (rcnt < maxcnt) {
	    ierr = MPI_Test(&recv_request, &mpi_flag, &mpi_status);
	    if (ierr != MPI_SUCCESS) {
	       sprintf(info_buf, "MPI_test() returned %d", ierr);
	       MPITEST_message(MPITEST_NONFATAL, info_buf);
	       MPI_Error_string(ierr, error_string, &size);
	       MPITEST_message(MPITEST_FATAL, error_string);
	    }

	    if (mpi_flag) {

	       ierr = MPI_Get_count(&mpi_status, MPI_LONG, &mpi_count);
	       if (ierr != MPI_SUCCESS) {
		  sprintf(info_buf, "MPI_Get_count() returned %d", ierr);
		  MPITEST_message(MPITEST_NONFATAL, info_buf);
		  MPI_Error_string(ierr, error_string, &size);
		  MPITEST_message(MPITEST_FATAL, error_string);
	       }

	       sprintf(info_buf, "(%d) loop %d - MPI_Irecv() complete: rcnt = %d, source rank = %d, message size = %d\n",
		       MPITEST_me, loopcnt, rcnt,
		       mpi_status.MPI_SOURCE, mpi_count);
	       MPITEST_message(MPITEST_INFO1, info_buf);

	       rec_tally[mpi_status.MPI_SOURCE]++;
	       rcnt++;

	       /* Post receive for another message */
	       if (rcnt < maxcnt) {
		  ierr = MPI_Irecv(rbuf, maxlen, MPI_LONG, MPI_ANY_SOURCE, type, MPI_COMM_WORLD, &recv_request);
		  if (ierr != MPI_SUCCESS) {
		     sprintf(info_buf, "MPI_Irecv() returned %d", ierr);
		     MPITEST_message(MPITEST_NONFATAL, info_buf);
		     MPI_Error_string(ierr, error_string, &size);
		     MPITEST_message(MPITEST_FATAL, error_string);
		  }
	       }
	    }			/* msgdone */
	 }			/* rcnt < maxcnt */

	 if (MPI_Wtime() - lastprint > 60.0) {
	    if (MPITEST_me == 0) {
	       sprintf(info_buf, "(%d) %s Loop %d: scnt = %d, rcnt = %d\n",
		       MPITEST_me, tod(), loopcnt, scnt, rcnt);
	       MPITEST_message(MPITEST_INFO1, info_buf);
	    }
	    lastprint = MPI_Wtime();
	 }
      }				/* while */

      /*
       * Check that we received the expected number of messages from each
       * node.
       */
      for (i = 0; i < MPITEST_nump; i++) {
	 if (rec_tally[i] != expect[i]) {
	    failloop++;
	    sprintf(info_buf, "(%d) loop %d: rec_tally[%d] = %d, expected %d, FAILED\n", MPITEST_me, loopcnt, i, rec_tally[i], expect[i]);
	    MPITEST_message(MPITEST_NONFATAL, info_buf);
	 }
      }

      /* print status message */
      if (MPITEST_me == 0) {
	 if (failloop == 0) {
	    sprintf(info_buf, "(%d) %s Loop %d: Completed\n",
		    MPITEST_me, tod(), loopcnt);
	    MPITEST_message(MPITEST_INFO0, info_buf);
	 }
	 else {
	    failed++;
	    sprintf(info_buf, "(%d) %s Loop %d: *** FAILED *** (%d lost msgs)\n",
		    MPITEST_me, tod(), loopcnt, failloop);
	    MPITEST_message(MPITEST_INFO0, info_buf);
	 }
      }

   }				/* while(maxloopcnt) */

   /* Report results */

   MPITEST_report(loopcnt - failed, failed, 0, "ASYNC");

   MPI_Finalize();
   return 0;
}				/* main */


/*****************************************************************************
 *
 *   get_cmd_opt():
 *
 *****************************************************************************/
void 
get_cmd_opt(int argc, char *argv[])
{
   int i;

   i = 1;
   while (i < argc) {
      if (!strcmp(argv[i], "-d")) {
	 if (MPITEST_me == 0) {
	    sprintf(info_buf, "Defaults for this partition:\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "\tmaxlen     = %ld\n", maxlen);
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "\tmaxloopcnt = %d\n", maxloopcnt);
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "\tminutes    = %d\n", minutes);
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "\tnum_nodes  = %d\n", MPITEST_nump);
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "\trandom     = %d\n", useRandom);
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "\tseed       = %u\n", seed);
	    MPITEST_message(MPITEST_INFO1, info_buf);
	 }
	 exit(1);
      }
      else if (!strcmp(argv[i], "-l")) {
	 i++;
	 if (i >= argc) {
	    sprintf(info_buf, "Option -l requires argument");
	    MPITEST_message(MPITEST_FATAL, info_buf);
	 }
	 else {
	    maxloopcnt = atoi(argv[i]);
	    i++;
	 }
      }
      else if (!strcmp(argv[i], "-m")) {
	 i++;
	 if (i >= argc) {
	    sprintf(info_buf, "Option -m requires argument");
	    MPITEST_message(MPITEST_FATAL, info_buf);
	 }
	 else {
	    maxlen = atol(argv[i]);
	    i++;
	 }
      }
      else if (!strcmp(argv[i], "-r")) {
	 i++;
	 if (i >= argc) {
	    sprintf(info_buf, "Option -r requires argument");
	    MPITEST_message(MPITEST_FATAL, info_buf);
	 }
	 else {
	    useRandom = 1;
	    i++;
	 }
      }
      else if (!strcmp(argv[i], "-s")) {
	 i++;
	 if (i >= argc) {
	    sprintf(info_buf, "Option -s requires argument");
	    MPITEST_message(MPITEST_FATAL, info_buf);
	 }
	 else {
	    seed = atol(argv[i]);
	    i++;
	 }
      }
      else if (!strcmp(argv[i], "-t")) {
	 i++;
	 if (i >= argc) {
	    sprintf(info_buf, "Option -t requires argument");
	    MPITEST_message(MPITEST_FATAL, info_buf);
	 }
	 else {
	    minutes = atoi(argv[i]);
	    maxloopcnt = -1;
	    i++;
	 }
      }
      else if ((!strcmp(argv[i], "-h")) || (!strcmp(argv[i], "-?"))) {
	 if (MPITEST_me == 0) {
	    sprintf(info_buf, "Usage: %s\t[-d] [-h] [-l maxloopcnt] [-m maxlen ] [-r] [-s seed]\n\t\t[-t minutes] [-v] [-V]\n\n", argv[0]);
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-d   Print Defaults\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-h   Print Usage Message\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-l   Specify the number of loops, -1 = run forever\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-m   Messages will be ramped to this value each loop\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-r   Enable random message lengths\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-s   Specify the seed for the random number generator\n");
	    MPITEST_message(MPITEST_INFO1, info_buf);

	    sprintf(info_buf, "-t   Specify the number of minutes the test should run, overides maxloopcnt\n");
	 }
	 exit(1);
      }
      else {
	 /* Simply ignore option that is not recognised */
	 i++;
      }
   }
}				/* get_cmd_opt */


/*****************************************************************************
 *
 *  get_length(): Return the new message length. If useRandom is on then return
 *                a random number < maxlen, else increment len by sizeof(long)
 *
 *****************************************************************************/
long 
get_length(long len)
{
   if (useRandom) {
      /* should yield a sufficiently random number in the range [0,maxlen) */
      len = (long) (maxlen * (rand() / (RAND_MAX + 1.0)));
   }
   else {
      /* len += sizeof(long); */
      len += 1;
   }

   if (len > maxlen) {
      printf("(%d) ASYN: Internal Failure: len > maxlen, %ld > %ld\n",
	     MPITEST_me, len, maxlen);
      exit(1);
   }

   return len;
}				/* get_length */


/*****************************************************************************
 *
 *  init_expect(): Initialize each element of expect[n] with the number of
 *                 expected messages from node n. The number of messages
 *                 depends on maxlen and MPITEST_nump.
 *
 *****************************************************************************/
void
init_expect(int *expect)
{
   int i, num_msgs, num_odds, odd_node;

   /* All nodes recv at least this many msgs */
   num_msgs = maxcnt / (MPITEST_nump - 1);

   for (i = 0; i < MPITEST_nump; i++) {
      expect[i] = num_msgs;
   }

   /* no messages from myself */
   expect[MPITEST_me] = 0;

   /*
    * Account for remaining messages: Expect one additonal message from
    * maxcnt%maxnode nodes < MPITEST_me
    */
   odd_node = MPITEST_me;
   num_odds = maxcnt % (MPITEST_nump - 1);
   for (i = 0; i < num_odds; i++) {
      if (odd_node == 0) {
	 odd_node = MPITEST_nump - 1;
      }
      else {
	 odd_node--;
      }
      expect[odd_node]++;
   }
}				/* init_expect */


/*****************************************************************************
 *
 *   tod(): returns a pointer to a formated string containing the current
 *          system date and time
 *
 *****************************************************************************/
char *
tod(void)
{
   time_t tp;
   struct tm *stm;

   tp = time((time_t *) NULL);
   stm = localtime(&tp);
   strftime(timebuf, 20, "%H:%M:%S", stm);
   return timebuf;
}				/* tod */
