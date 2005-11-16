/* Ini File used:
  tpcpp.ini */

/* Preprocessor macro definitions:
  */

/* Preprocessor options:
   */

/* Compiler flags:
  -g */

/*******************************************************************************
* $Id$
* $Descr: Studio-modifiable Settings (Parsed by Products.pl) $
*******************************************************************************/



#define ATL_YES      1
#define ATL_NO       0

#ifndef ATL_WITHOUT_STUDIO

/* WARNING:
     @PRODUCT@ lines are updated by Perl script ! */

/* Use TestRT Unit Testing */
#define USE_ATU ATL_NO
/* Use TestRT System Testing */
#define USE_ATS ATL_NO
/* Use TestRT Object Testing */
#define USE_ATO ATL_NO
/* Use TestRT Contract Check */
#define USE_ATK ATL_NO
/* Use TestRT Coverage */
#define USE_ATC ATL_YES
/* Use TestRT Trace */
#define USE_ATT ATL_NO
/* Use TestRT PurifyLT */
#define USE_ATP ATL_NO
/* Use TestRT QuantifyLT */
#define USE_ATQ ATL_NO

#else /* ATL_WITHOUT_STUDIO */

#ifndef USE_ATU
#define USE_ATU ATL_NO
#endif
#ifndef USE_ATS
#define USE_ATS ATL_NO
#endif
#ifndef USE_ATO
#define USE_ATO ATL_NO
#endif
#ifndef USE_ATK
#define USE_ATK ATL_NO
#endif
#ifndef USE_ATC
#define USE_ATC ATL_NO
#endif
#ifndef USE_ATT
#define USE_ATT ATL_NO
#endif
#ifndef USE_ATP
#define USE_ATP ATL_NO
#endif
#ifndef USE_ATQ
#define USE_ATQ ATL_NO
#endif

#endif /* ATL_WITHOUT_STUDIO */


/*******************************************************************************
* TestRT Unit Testing:
*******************************************************************************/

#if USE_ATU

#ifndef ATL_WITHOUT_STUDIO
#define ATU_BREAK_ON_TEST_FAILED 0
#else
#ifndef ATU_BREAK_ON_TEST_FAILED
/*! Calls a breakpoint function on an error if 1 */
#define ATU_BREAK_ON_TEST_FAILED ATL_NO
#endif
#endif

#endif


/*******************************************************************************
* TestRT System Testing:
*******************************************************************************/

#if USE_ATS

#ifndef ATL_WITHOUT_STUDIO
#define ATS_BUFFER_SIZE 8192
#else
#ifndef ATS_BUFFER_SIZE
/*! Size in bytes of buffer (used to store string vars and lines) */
#define ATS_BUFFER_SIZE 1024
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATS_TAGTBUF_SIZE 4096
#else
#ifndef ATS_TAGTBUF_SIZE
/*! Maximum size in bytes of lines raised by testers */
#define ATS_TAGTBUF_SIZE 4096
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATS_RT_BUFFER0
#else
#ifndef ATS_RT_BUFFER
/*! Use bufferized items */
#define ATS_RT_BUFFER ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATS_DYNFLW 0
#else
#ifndef ATS_DYNFLW
/*! Activates dynamic tracking */
#define ATS_DYNFLW ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATS_INTERRECV_TIMEOUT 
#else
#ifndef ATS_INTERRECV_TIMEOUT
/*! Timeout for INTERRECV instruction */
#define ATS_INTERRECV_TIMEOUT 300
#endif
#endif

#endif


/*******************************************************************************
* TestRT Object Testing:
*******************************************************************************/

#if USE_ATO

#ifndef ATL_WITHOUT_STUDIO
#define ATO_BREAK_ON_CHECK_FAILED ATL_NO
#else
#ifndef ATO_BREAK_ON_CHECK_FAILED
/*! Calls a breakpoint function on an error if 1 */
#define ATO_BREAK_ON_CHECK_FAILED ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATO_DUMP_SUCCESS ATL_NO
#else
#ifndef ATO_DUMP_SUCCESS
/*! Also dumps successful checks in traces */
#define ATO_DUMP_SUCCESS 1
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATO_MAX_INSTANCES 64
#else
#ifndef ATO_MAX_INSTANCES
/*! Size of instances stack */
#define ATO_MAX_INSTANCES 256
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATO_PRINT_BUFFER_SIZE 1024
#else
#ifndef ATO_PRINT_BUFFER_SIZE
/* Size of PRINT buffer (0 means no buffering: PRINT arguments are displayed
 * on separate notes) */
#define ATO_PRINT_BUFFER_SIZE 512
#endif
#endif

#endif /* USE_ATO */

/*******************************************************************************
* TestRT Contract Check:
*******************************************************************************/

#if USE_ATK

#ifndef ATL_WITHOUT_STUDIO
#define ATK_BREAK_ON_ASSERT_FAILED ATL_NO
#else
#ifndef ATK_BREAK_ON_ASSERT_FAILED
/*! Calls a breakpoint function on an error if 1 */
#define ATK_BREAK_ON_ASSERT_FAILED 0
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATK_DUMP_SUCCESS ATL_NO
#else
#ifndef ATK_DUMP_SUCCESS
/*! Also dumps successful assertions in traces */
#define ATK_DUMP_SUCCESS 0
#endif
#endif

#endif /* USE_ATK */


/*******************************************************************************
* TestRT Coverage:
*******************************************************************************/

#if USE_ATC

/* 0: pass mode,
 * 1: compact mode,
 * 2: count mode */
#ifndef ATL_WITHOUT_STUDIO
#define ATC_INFORMATION 0
#else
#ifndef ATC_INFORMATION
/* pass mode */
#define ATC_INFORMATION 0
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATC_DUMP_DRIVER ATL_NO
#else
#ifndef ATC_DUMP_DRIVER
/* Dump Driver use */
#define ATC_DUMP_DRIVER ATL_NO
#endif
#endif

#endif


/*******************************************************************************
* TestRT Trace:
*******************************************************************************/

#if USE_ATT

#ifndef ATL_WITHOUT_STUDIO
#define ATT_ON_THE_FLY ATL_NO
#else
#ifndef ATT_ON_THE_FLY
/* Trace On The Fly (natives only) */
#define ATT_ON_THE_FLY ATL_YES
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATT_ITEMS_MAX 0
#else
#ifndef ATT_ITEMS_MAX
/* Maximum Number of Buffered Items */
#define ATT_ITEMS_MAX 0
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATT_DUMP_FROM_BEGINNING ATL_YES
#else
#ifndef ATT_DUMP_FROM_BEGINNING
/* No Partial Message Dump: if ATL_YES, message dump starting, stopping and
   toggling and stack dump are not available */
#define ATT_DUMP_FROM_BEGINNING ATL_YES
#endif
#endif

/* Action Associated to Signal Handling */
#define ATT_SIGNAL_TOGGLE_DUMP 1
#define ATT_SIGNAL_DUMP_CALL_STACK 2

#ifndef ATL_WITHOUT_STUDIO
#define ATT_SIGNAL 0
#else
#ifndef ATT_SIGNAL
/* Possible Values for ATT_SIGNAL:
   ATL_NO, ATT_SIGNAL_TOGGLE_DUMP or ATT_SIGNAL_DUMP_CALL_STACK */
#define ATT_SIGNAL ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATT_COMPUTE_MAX_CALLING_LEVEL ATL_YES
#else
#ifndef ATT_COMPUTE_MAX_CALLING_LEVEL
/* Computes Maximum Call Stack Size and Display it in a Note */
#define ATT_COMPUTE_MAX_CALLING_LEVEL ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATT_TIMESTAMP ATL_NO
#else
#ifndef ATT_TIMESTAMP
/* Time Stamping of UML/SD Messages: */
#define ATT_TIMESTAMP ATL_NO
#endif
#endif


/* multi-threaded applications only: */

#ifndef ATL_WITHOUT_STUDIO
#define ATT_THREAD_INFO ATL_YES
#else
#ifndef ATT_THREAD_INFO
/* thread info is computed or not */
#define ATT_THREAD_INFO ATL_NO
#endif
#endif

#endif


/*******************************************************************************
* TestRT PurifyLT:
*******************************************************************************/

#if USE_ATP

#ifndef ATL_WITHOUT_STUDIO
#define ATP_CALL_STACK_SIZE 6
#else
#ifndef ATP_CALL_STACK_SIZE
/* How Many Call Stack Items in each Memory Block Description */
#define ATP_CALL_STACK_SIZE 6
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_REPORTS_FIU ATL_YES
#else
#ifndef ATP_REPORTS_FIU
/* File In Use Reporting */
#define ATP_REPORTS_FIU ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_REPORTS_FFM_FMWL ATL_YES
#else
#ifndef ATP_REPORTS_FFM_FMWL
/* Late Detect Free Memory Write Reporting */
#define ATP_REPORTS_FFM_FMWL ATL_YES
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_MAX_FREEQ_LENGTH 100
#else
#ifndef ATP_MAX_FREEQ_LENGTH
/* Maximum Number of Memory Blocks Kept in the Free Queue */
#define ATP_MAX_FREEQ_LENGTH 100
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_MAX_FREEQ_SIZE 1048576
#else
#ifndef ATP_MAX_FREEQ_SIZE
/* Maximum Number of Bytes Kept Unfreed in the Free Queue */
#define ATP_MAX_FREEQ_SIZE 1000000
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_REPORTS_ABWL ATL_YES
#else
#ifndef ATP_REPORTS_ABWL
/* Late Detect Array Bounds Write Reporting */
#define ATP_REPORTS_ABWL ATL_YES
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_RED_ZONE_SIZE 16
#else
#ifndef ATP_RED_ZONE_SIZE
/* Size in Bytes of each of the two Red Zones
 * (one before and one after the Tracked Memory Block) */
#define ATP_RED_ZONE_SIZE 16
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_REPORTS_MIU ATL_NO
#else
#ifndef ATP_REPORTS_MIU
/* Memory In Use Reporting */
#define ATP_REPORTS_MIU ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATP_REPORTS_SIG ATL_YES
#else
#ifndef ATP_REPORTS_SIG
/* Signal Handled Reporting */
#define ATP_REPORTS_SIG ATL_NO
#endif
#endif

#endif


/*******************************************************************************
* TestRT QuantifyLT:
*******************************************************************************/

#if USE_ATQ

#ifndef ATL_WITHOUT_STUDIO
#define ATQ_COMPUTE_OVERHEAD ATL_NO
#else
#ifndef ATQ_COMPUTE_OVERHEAD
/* Computes Overhead on Measures Due to Its Measurement */
#define ATQ_COMPUTE_OVERHEAD ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATQ_DUMP_DRIVER ATL_NO
#else
#ifndef ATQ_DUMP_DRIVER
/* Dump Driver use */
#define ATQ_DUMP_DRIVER ATL_NO
#endif
#endif

#endif


/*******************************************************************************
* Several products settings:
*******************************************************************************/

#ifndef ATL_WITHOUT_STUDIO
#define ATL_MULTI_THREADS ATL_NO
#else
#ifndef ATL_MULTI_THREADS
/*! Multi-threaded Application */
#define ATL_MULTI_THREADS ATL_NO
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATL_THREADS_MAX 64
#else
#ifndef ATL_THREADS_MAX
/*! Threads Table Size */
#define ATL_THREADS_MAX 64
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATL_BUFFER_SIZE 16384
#else
#ifndef ATL_BUFFER_SIZE
/*! I/O Optimisation Buffer Size In Bytes */
#define ATL_BUFFER_SIZE 16384
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATL_NODE_NAME "gplot"
#else
#ifndef ATL_NODE_NAME
/*! Application Node Name */
#define ATL_NODE_NAME ".root."
#endif
#endif

#ifndef ATL_WITHOUT_STUDIO
#define ATL_TRACES_FILE "atlout.spt"
#else
#ifndef ATL_TRACES_FILE
/*! Traces File Name */
#define ATL_TRACES_FILE "atlout.spt"
#endif
#endif

#define ATL_OS_CLOCK 1
#define ATL_PROCESS_CLOCK 2
#ifndef ATL_WITHOUT_STUDIO
#define ATL_CLOCK_KIND ATL_OS_CLOCK
#else
#ifndef ATL_CLOCK_KIND
/*! Clock type : OS or process */
#define ATL_CLOCK_KIND ATL_OS_CLOCK
#endif
#endif

