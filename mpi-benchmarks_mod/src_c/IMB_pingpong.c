/*****************************************************************************
 *                                                                           *
 * Copyright Intel Corporation.                                              *
 *                                                                           *
 *****************************************************************************

This code is covered by the Community Source License (CPL), version
1.0 as published by IBM and reproduced in the file "license.txt" in the
"license" subdirectory. Redistribution in source and binary form, with
or without modification, is permitted ONLY within the regulations
contained in above mentioned license.

Use of the name and trademark "Intel(R) MPI Benchmarks" is allowed ONLY
within the regulations of the "License for Use of "Intel(R) MPI
Benchmarks" Name and Trademark" as reproduced in the file
"use-of-trademark-license.txt" in the "license" subdirectory. 

THE PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT
LIMITATION, ANY WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Each Recipient is
solely responsible for determining the appropriateness of using and
distributing the Program and assumes all risks associated with its
exercise of rights under this Agreement, including but not limited to
the risks and costs of program errors, compliance with applicable
laws, damage to or loss of data, programs or equipment, and
unavailability or interruption of operations.

EXCEPT AS EXPRESSLY SET FORTH IN THIS AGREEMENT, NEITHER RECIPIENT NOR
ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING
WITHOUT LIMITATION LOST PROFITS), HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OR
DISTRIBUTION OF THE PROGRAM OR THE EXERCISE OF ANY RIGHTS GRANTED
HEREUNDER, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 

EXPORT LAWS: THIS LICENSE ADDS NO RESTRICTIONS TO THE EXPORT LAWS OF
YOUR JURISDICTION. It is licensee's responsibility to comply with any
export regulations applicable in licensee's jurisdiction. Under
CURRENT U.S. export regulations this software is eligible for export
from the U.S. and can be downloaded by or otherwise exported or
reexported worldwide EXCEPT to U.S. embargoed destinations which
include Cuba, Iraq, Libya, North Korea, Iran, Syria, Sudan,
Afghanistan and any other country to which the U.S. has embargoed
goods and services.

 ***************************************************************************

For more documentation than found here, see

[1] doc/ReadMe_IMB.txt 

[2] Intel(R) MPI Benchmarks
    Users Guide and Methodology Description
    In 
    doc/IMB_Users_Guide.pdf
    
 File: IMB_pingpong.c 

 Implemented functions: 

 IMB_pingpong;

 ***************************************************************************/
#include <papi.h>
//#include "/u/dssc/s271550/fast/papi_library/include/papi.h"
#include "IMB_benchmark.h"
#include "IMB_declare.h"
#include "IMB_prototypes.h"
#include "mypapi.h"
#define PCHECK(e)                                                    \
    if (e != PAPI_OK) {                                              \
        printf("PROBLEMI CON IL PAPI,LINEA %d\n", __LINE__);         \
        fprintf(stderr, "PAPI error %d: %s\n", e, PAPI_strerror(e)); \
        return e;                                                    \
    }
#define NEVENTS 3

/*************************************************************************/

/* ===================================================================== */
/* 
IMB 3.1 changes
July 2007
Hans-Joachim Plum, Intel GmbH

- replace "int n_sample" by iteration scheduling object "ITERATIONS"
  (see => IMB_benchmark.h)

- proceed with offsets in send / recv buffers to eventually provide
  out-of-cache data
*/
/* ===================================================================== */

void IMB_pingpong(struct comm_info *c_info, int size, struct iter_schedule *ITERATIONS,
                  MODES RUN_MODE, double *time) {
    /*

                          MPI-1 benchmark kernel
                          2 process MPI_Send + MPI_Recv  pair

Input variables:

-c_info                   (type struct comm_info*)
                          Collection of all base data for MPI;
                          see [1] for more information


-size                     (type int)
                          Basic message size in bytes

-ITERATIONS               (type struct iter_schedule *)
                          Repetition scheduling

-RUN_MODE                 (type MODES)
                          (only MPI-2 case: see [1])

Output variables:

-time                     (type double*)
                          Timing result per sample

*/

    long long PAPI_val[NEVENTS] = {0};
    long long PAPI_accumulator[NEVENTS] = {0};
    int PAPI_EventSet = PAPI_NULL;
    int retval;
    int PAPI_myevents[NEVENTS] = {PAPI_TOT_CYC, PAPI_L1_DCM, PAPI_L2_DCM};
    int PAPI_acc[NEVENTS] = {0};
    retval = PAPI_library_init(PAPI_VER_CURRENT);

    if (retval != PAPI_VER_CURRENT)
        printf("wrong PAPI initialization: %d invece di %d\n", retval, PAPI_VER_CURRENT);

    //printf("Papi current version: %d\n",PAPI_VER_CURRENT);

    retval = PAPI_create_eventset(&PAPI_EventSet);
    PCHECK(retval);
    for (int i = 0; i < NEVENTS; i++) {
        retval = PAPI_query_event(PAPI_myevents[i]);
        PCHECK(retval); //check if the event can be shown on this machine
                        //retval = PAPI_add_event(PAPI_EventSet, PAPI_myevents[i]); PCHECK(retval);
    }
    retval = PAPI_add_events(PAPI_EventSet, PAPI_myevents, NEVENTS);
    PCHECK(retval);

    int i;
    Type_Size s_size, r_size;
    int s_num = 0, r_num = 0;
    int s_tag, r_tag;
    int dest, source;
    MPI_Status stat;

#ifdef CHECK
    int asize = (int)sizeof(assign_type);
    defect = 0;
#endif

    /*  GET SIZE OF DATA TYPE */
    MPI_Type_size(c_info->s_data_type, &s_size);
    MPI_Type_size(c_info->r_data_type, &r_size);

    if ((s_size != 0) && (r_size != 0)) {
        s_num = size / s_size;
        r_num = size / r_size;
    }

    s_tag = 1;
    r_tag = c_info->select_tag ? s_tag : MPI_ANY_TAG;

    size *= c_info->size_scale;

    *time = 0.;

    if (c_info->rank == c_info->pair0) {
        /*  CALCULATE SOURCE AND DESTINATION */
        dest = c_info->pair1;
        source = c_info->select_source ? dest : MPI_ANY_SOURCE;

        for (i = 0; i < N_BARR; i++)
            MPI_Barrier(c_info->communicator);

        *time -= MPI_Wtime();

        retval = PAPI_reset(PAPI_EventSet);
        PCHECK(retval);
        for (i = 0; i < ITERATIONS->n_sample; i++) {

            retval = PAPI_start(PAPI_EventSet);
            PCHECK(retval);
            MPI_Send((char *)c_info->s_buffer + i % ITERATIONS->s_cache_iter * ITERATIONS->s_offs,
                     s_num, c_info->s_data_type, dest,
                     s_tag, c_info->communicator);
            PCHECK(retval);
            //MPI_ERRHAND(MPI_Recv((char*)c_info->r_buffer + i%ITERATIONS->r_cache_iter*ITERATIONS->r_offs,
            //                     r_num, c_info->r_data_type, source,
            //                     r_tag, c_info->communicator, &stat));
            MPI_Recv((char *)c_info->r_buffer + i % ITERATIONS->r_cache_iter * ITERATIONS->r_offs,
                     r_num, c_info->r_data_type, source,
                     r_tag, c_info->communicator, &stat);
            retval = PAPI_stop(PAPI_EventSet, PAPI_val);
            for (int i = 0; i < NEVENTS; ++i)
                PAPI_accumulator[i] += PAPI_val[i];
            //retval = PAPI_stop(PAPI_EventSet,PAPI_val); PCHECK(retval);
            //for(int i=0;i<NEVENTS;++i)
            //  PAPI_acc[i]+=PAPI_val[i];

            CHK_DIFF("PingPong", c_info, (char *)c_info->r_buffer + i % ITERATIONS->r_cache_iter * ITERATIONS->r_offs, 0,
                     size, size, asize,
                     put, 0, ITERATIONS->n_sample, i,
                     dest, &defect);

        } /*for*/

        *time += MPI_Wtime();

        //fprintf(stderr,"iteration %d, message size %d,L1 missess %ld ,L2 missess %ld, L2 total %ld \n",ITERATIONS->n_sample,r_num,PAPI_val[1],PAPI_val[2],PAPI_val[3]);
        fprintf(stderr, "iteration %d, message size %d,L1 missess %ld ,L2 missess %ld\n", ITERATIONS->n_sample, r_num, PAPI_accumulator[1], PAPI_accumulator[2]);

    } else if (c_info->rank == c_info->pair1) {
        dest = c_info->pair0;
        source = c_info->select_source ? dest : MPI_ANY_SOURCE;

        for (i = 0; i < N_BARR; i++)
            MPI_Barrier(c_info->communicator);

        *time -= MPI_Wtime();
        for (i = 0; i < ITERATIONS->n_sample; i++) {
            MPI_ERRHAND(MPI_Recv((char *)c_info->r_buffer + i % ITERATIONS->r_cache_iter * ITERATIONS->r_offs,
                                 r_num, c_info->r_data_type, source,
                                 r_tag, c_info->communicator, &stat));

            MPI_ERRHAND(MPI_Send((char *)c_info->s_buffer + i % ITERATIONS->s_cache_iter * ITERATIONS->s_offs,
                                 s_num, c_info->s_data_type, dest,
                                 s_tag, c_info->communicator));

            CHK_DIFF("PingPong", c_info, (char *)c_info->r_buffer + i % ITERATIONS->r_cache_iter * ITERATIONS->r_offs, 0,
                     size, size, asize,
                     put, 0, ITERATIONS->n_sample, i,
                     dest, &defect);
        } /*for*/

        *time += MPI_Wtime();
    }

    *time /= ITERATIONS->n_sample;
}
