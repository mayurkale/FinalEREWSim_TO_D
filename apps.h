/* ----------------------------------------------------------------------------- 
 * applications.h
 * 
 * iPDC - Phasor Data Concentrator
 *
 * Copyright (C) 2011-2012 Nitesh Pandit
 * Copyright (C) 2011-2012 Kedar V. Khandeparkar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Authors: 
 *		Kedar V. Khandeparkar <kedar.khandeparkar@gmail.com>			
 *
 * ----------------------------------------------------------------------------- */
#define _GNU_SOURCE

#include <search.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include  <pthread.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_linalg.h>
#include <sys/time.h>

/* ---------------------------------------------------------------- */
/*                         global variable                          */
/* ---------------------------------------------------------------- */

#define APP1MAXTSB 6 // this has to be set for SPDC too
#define OLDFRAMECOUNTAPP1 6 // this has to be set for SPDC too
/* ---------------------------------------------------------------- */
/*                         global Data Structure                    */
/* ---------------------------------------------------------------- */


struct LatencyTracker {

	double first_arrival_lpdc;
	double Min_pmutolpdc;

}LT[APP1MAXTSB];

pthread_mutex_t mutex_query;
struct hsearch_data hashForPMUQuery;
struct PMUQueryInfo{
	unsigned int pmuid;
	unsigned int api_id;
	unsigned int query_id;
	unsigned char phasorname[17];
	unsigned int phasorNumber;
	float bounds_slope;
	float displace_lb;
	float displace_ub;
	unsigned long int invoke_soc;
	unsigned long int invoke_fracsec;
	struct Upper_Layer_Details *spdc;
	struct PMUQueryInfo * nextQuery;
};

int* active_api_ids; // It will decide which api should run
int num_api;

int total_buses;
int listed_buses;
float* buses_power;
pthread_mutex_t mutex_App1;
pthread_mutex_t mutex_timeout;

struct App1_Info{
	int app_id;
	int num_modes;
	int current_modes;
	int data_priority;
	int pkt_loss;
	struct hsearch_data hashForApp1Pmus;
	struct mode_info{
		int mode_freq;
		int monitor_groupno;
		int num_pmus;
		int phasor_index;
		struct hsearch_data pmuids;
		float theta_c_i;
		int current_pmus;
		float power_sum;
	} ** modes;
}* app1;

int **cggModes;

//Below variables are used to set if timeout time is less than 20ms, because in that case ODFTAPP1 won't have any packet
//to compare with new packet timestamp which will result in processing time as -INFI
unsigned int glob_sec;
unsigned int glob_fracsec;

struct  oldDataFramesTimeStampListApp1
{
	unsigned int soc;
	unsigned int fracsec;

}ODFTAPP1[OLDFRAMECOUNTAPP1];

int old_df_rear_APP1;
int old_df_front_APP1;
void adjustOldFrameCountApp1(int);
int unq_pmu_app1;

int checkIfCGG(char *);
int count_total_modes_PMU;
int total_modes_PMU;

struct storedDF
{
	struct data_frame *df;
	struct storedDF *next;
}*storedDF_var,*storedDF_head,*storedDF_tail;
int IsDelayed;
int app1_delay;
volatile int drop_count;
unsigned long g_esoc,g_fsoc;
long int to_esoc,to_fsoc;
void app1_pkt_send(int);
int my_timer();
void set_Timer();
int app1_first_cycle;
int first_pkt;
long int app1_time_out;
unsigned int current_TS_fsec;
unsigned int current_TS_esec;
int Isthreadrunning;
int app1_flag;
int app1_count;
pthread_t t_app1;
int err_app1;
pthread_attr_t attr_app1;

pthread_mutex_t mutex_App2;
void *app1_thread_fun();
//=================================================
//TODO: Initilization of variables

int APP1TSB_Count;
volatile int APP1Front;
volatile int APP1Rear;

struct App1TimeStampBuffer {

	char *soc;
	char *fracsec;
	int tot_pmu; // this represents total number of unique PMUs in all the three modes
	int num; // Total Number of PMUs
	int total_mode_pmus;
	pthread_t threadID;
	struct modeApp1 *modes;
}APP1TSB[APP1MAXTSB];

struct modeApp1{
			float theta_c_i;
			float power_sum;
			int current_pmus;
		};


//int matchDataFrameTimeToTSBTime(unsigned int,unsigned int);
void getAPP1TSBFree(int);
void intermediate_dispatch_app1(int);
void prosess_and_store(struct data_frame *,int, char[],int);
void createStartTimerThread(int);
void *startTimer(void *);
int matchDataFrameTimeToTSBTime(unsigned int,unsigned int);
int matchDataFrameTimeToTSBTime_NoUpdate(unsigned int,unsigned int,unsigned int);
int create_and_assigntoAPP1TSB(struct data_frame *,int);
//=================================================


struct App2_phasor_map{
	int num_phasor;
	int* phasor_ids;
};
struct App2_Info{
	int app_id;
	int num_pmus;
	int current_pmus;	
	int data_priority;
	// Matrixes 
	gsl_matrix_complex * M;
	struct sparseMatrix *MsparseHead;
	struct sparseMatrix *MsparseTail;
	
	gsl_matrix_complex * Mpinv;
	struct sparseMatrix *MpinvsparseHead;
	struct sparseMatrix *MpinvsparseTail;
	
	gsl_matrix_complex * Pdiag;
	gsl_matrix_complex * MHMinvdiag;
	
	gsl_matrix_complex * Z;
	struct hsearch_data hashForZmatrixIndex;			
	
	gsl_matrix_complex * Vhat;
	gsl_matrix_complex * Zhat;
	gsl_matrix_complex * r;
	
	int numOutputKeys;
	char ** OutputKeys;			
	
	int numOutputVhatIndex;
	int * OutputVhatIndex;	
	
	struct hsearch_data hashForApp2Pmus;
}* app2;


struct sparseMatrix{
	int x;
	int y;
	gsl_complex value;
	struct sparseMatrix* next;
};
				

//struct query_thread_info *FirstTh = NULL;
/* ---------------------------------------------------------------- */
/*                 	Function prototypes   		            */
/* ---------------------------------------------------------------- */

void buildQueryListHashTables();

void updateQueryList(char * elmt_key,struct PMUQueryInfo *elmt_data, struct hsearch_data * table);

float c2f_ieee(const void *v);

void create_send(void *info,void *datafrm,int type);

void check(void *datafrm);

void dispatcher(unsigned char* dataframe, int size, void *s);

void* execute_query(void * q);

void* LPDC_APIs();

void application6(void *datafrm);

void application1(void *datafrm);

void application2(void *datafrm);

void * DSE_dispatcher(void *datafrm);

void funcKedar(struct data_frame *df,int i, char []);
/**************************************** My Functions *******************************************************/

/**************************************** End of File *******************************************************/




