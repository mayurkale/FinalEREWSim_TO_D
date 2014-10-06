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
#define APP1MAXTSBS 6
#define OLDFRAMECOUNTAPP1S 6

struct timeval tim;
struct hsearch_data hashForAPIs;
pthread_mutex_t mutex_PhAPI_info;

struct  oldDataFramesTimeStampListApp1S
{
	unsigned int soc;
	unsigned int fracsec;

}ODFTAPP1S[OLDFRAMECOUNTAPP1S];

int old_df_rear_SAPP1;
int old_df_front_SAPP1;


struct captured_info_for_model_creation{
	int number;
//	unsigned int pdcid;
	unsigned int pmuid;
	char phname[17];
	
	unsigned long int soc;
	unsigned long int fracsec;
	
	float freq;
	float phasor_angle;
};

struct Ph_API_info{
	unsigned int api_id;	
	unsigned int query_id;
	
	unsigned int pdcid1;
	unsigned int pmuid1;
	char phname1[17];
	char protocol1[5];
//	float freq1;
//	float phasor_angle1;
	
	unsigned int pdcid2;
	unsigned int pmuid2;
	char phname2[17];
	char protocol2[5];
//	float freq2;
//	float phasor_angle2;
	
	float threshold;
//	char * lower_bound;
//	char * upper_bound;
	
	float bounds_slope;
	float displace_lb;
	float displace_ub;
	
	int send_remain;
	int model_status;
	
	struct Lower_Layer_Details *lower_pdc1;
	struct Lower_Layer_Details *lower_pdc2;
	
	struct captured_info_for_model_creation captured;
	struct captured_info_for_model_creation expected;
	
	// For analysis only
	unsigned long int modelstart_fsec;
};	
	//struct Ph_API_info* next_query;
//}*FirstQuery;


struct query_send_info{
	int type;
	struct Ph_API_info* send_query;
	
};

struct App1TimeStampBuffer_SPDC {

	char *soc;
	char *fracsec;
	int num; // Total Number of PMUs
	int total_mode_lpdc;
	pthread_t threadID;
	struct modeApp1_SPDC *modes;
	int cap_theta_index;
	struct Captured_theta_c_i1 *cap_theta;

}APP1TSBS[APP1MAXTSBS];

struct Captured_theta_c_i1{
		int lpdc_id;
		struct modewise1 *mw;
	} ;

	struct modewise1{
				float theta_c_i;
				float power_value;
				float threshold;
			} ;
//struct modewise1{
//			float theta_c_i;
//			float power_value;
//			float threshold;
//		} ;

//TODO: Allocate memory for all the new variables
struct modeApp1_SPDC{
	float theta_c_i;
	float power_sum;
	int current_lpdcs;


		};

int APP1TSB_Count_SPDC;
void CGG_final_process(int);
int create_and_assigntoAPP1TSB_SPDC(unsigned char [],int);
void createStartTimerThread_SPDC(int);
void *startTimer_SPDC(void *);
void prosess_and_store_SPDC(unsigned char [],int,int,int);
int matchDataFrameTimeToTSBTime_SPDC_No_Update(unsigned int,unsigned int );
void getAPP1TSBFree_SPDC(int);
int pkt_loss;
void intermediate_dispatch_app1_SPDC(int);
int matchDataFrameTimeToTSBTime_SPDC(unsigned int,unsigned int);
void getAPP1TSBFree_SPDC(int);
void adjustOldFrameCountApp1_SPDC(int);

struct SPDCApp1_Info{
	int app_id;
	int data_priority;

	int num_modes;	
	int num_lpdcs;
	int current_lpdcs;
	
	struct SPDCmode_info{
		
		float sum_theta_c;
		float power_sum;
	} ** modes;
	
	struct Captured_theta_c_i{
		int lpdc_id;
		struct modewise{
			float theta_c_i;
			float power_value;
			float threshold;
		} ** mw;
	} ** cap_theta ;
	
}* spdc_app1;


struct SPDCApp2_phasor_map{
	int num_phasor;
	unsigned char** pmu_phasor;
	
	int num_Neighbour_phasor;
	unsigned char** pmu_Neighbour_phasor;
};
struct SPDCApp2_Info{
	int app_id;
	int data_priority;
	int num_lpdcs;
	int current_lpdcs;
	struct hsearch_data hashForApp2Pdcs;
	
	gsl_matrix_complex * M;
	gsl_matrix_complex * Z;
	struct hsearch_data hashForZmatrixIndex;			
	gsl_matrix_complex * W;
	gsl_matrix_complex * Vhat;
	
}* spdc_app2;


/* ---------------------------------------------------------------- */
/*                 	Function prototypes   		            */
/* ---------------------------------------------------------------- */

void* APIs();

void* send_query(void *q);

void* query_sender(void* q);

int create_cmd_frame(struct query_send_info* t,char cmdframe[]);

void f2c (float f, unsigned char temp_1[]);

void initiate_create_model(unsigned char data[]);


void model_creater(unsigned char data[]);

void partial_computation_aggregate(unsigned char data[]);

void DSE_aggregate(unsigned char data[]);
void * SPDC_DSE_Result();

//float decode_ieee_single(const void *v);

/**************************************** End of File *******************************************************/


