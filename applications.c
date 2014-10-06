/* ----------------------------------------------------------------------------- 
 * applications.c
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
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <math.h>       /* fabs */
#include "parser.h"
#include "global.h" 
#include "dallocate.h"
#include "applications.h"
#include "apps.h"
#include "connections.h"
#include <unistd.h>

void invertion( gsl_matrix_complex * a,gsl_matrix_complex * b,int pivot ,int col){
	int i,j;
	int size = a->size1;
	
	gsl_complex factor;
	factor = gsl_matrix_complex_get (a, pivot, col);
	gsl_complex tmp;
	for(i=0;i<size;i++){
		tmp = gsl_matrix_complex_get (a, pivot, i);
		tmp = gsl_complex_div(tmp,factor);
		gsl_matrix_complex_set (a,  pivot, i, tmp);
		
		tmp = gsl_matrix_complex_get (b, pivot, i);
		tmp = gsl_complex_div(tmp,factor);
		gsl_matrix_complex_set (b,  pivot, i, tmp);
	}
	
	for(i=0;i<size;i++)
		if(i!=pivot){
			factor = gsl_matrix_complex_get (a, i, col);
			gsl_complex tmp1;
			gsl_complex tmp2;	
			for(j=0;j<size;j++){
				tmp1 = gsl_matrix_complex_get (a, i, j);
				tmp2 = gsl_matrix_complex_get (a, pivot, j);	
				tmp2 = gsl_complex_mul(tmp2,factor);
				tmp1 = gsl_complex_sub(tmp1,tmp2);
				gsl_matrix_complex_set (a, i, j, tmp1);
				
				tmp1 = gsl_matrix_complex_get (b, i, j);
				tmp2 = gsl_matrix_complex_get (b, pivot, j);	
				tmp2 = gsl_complex_mul(tmp2,factor);
				tmp1 = gsl_complex_sub(tmp1,tmp2);
				gsl_matrix_complex_set (b, i, j, tmp1);
			}
		}
}

// b = inverse(a)
void inverse(gsl_matrix_complex * a,gsl_matrix_complex * b){
	gsl_matrix_complex_set_zero(b);
	int i,j;
	
	int size = a->size1;
	for(i=0;i<size;i++)    // Append Unit Matrix
		for(j=0;j<size;j++)
			if(j==i)
				gsl_matrix_complex_set (b, i, j, GSL_COMPLEX_ONE);

	for(i=0;i<size;i++)
    	invertion(a,b,i,i);
}


void* APIs(){
	printf("In APIs()\n");
	
	int i,err;
	char *line;
	char *d1, *svptr;
	size_t len = 0;
	ssize_t result;
	
	FILE *fm;
	FILE *fp_csv;
	char *filePath = "applications.csv";
	fp_csv = fopen (filePath,"r");
			
	getdelim (&line, &len, ('\n'), fp_csv); // Remove header
	getdelim (&line, &len, ('\n'), fp_csv);	
	
	d1 = strtok_r(line,",",&svptr);
	num_api = atoi(d1);

/*	printf("numapi = %d\n",num_api);*/
	active_api_ids = (int*)malloc(num_api * sizeof(int));
	
	for(i=0; i<num_api; i++){
		getdelim (&line, &len, ('\n'), fp_csv); // Remove header
		getdelim (&line, &len, ('\n'), fp_csv);

		d1 = strtok_r(line,",",&svptr);
		int api_id = atoi(d1);
/*		printf("%d\n",api_id);*/
		
		active_api_ids[i] = api_id;
		
		d1 = strtok_r(NULL,",",&svptr);
		int priority = atoi(d1);
/*		printf("%d\n",priority);*/
		switch(api_id){
		case 1:	
			printf("API 1\n");
			int j;
			
			spdc_app1 = (struct SPDCApp1_Info*)malloc(sizeof(struct SPDCApp1_Info));
			spdc_app1->app_id = api_id;
			spdc_app1->data_priority = priority;
			
			d1 = strtok_r(NULL,",",&svptr);
			spdc_app1->num_modes = atoi(d1);
			
			d1 = strtok_r(NULL,",",&svptr);
			spdc_app1->num_lpdcs = atoi(d1);
			d1 = strtok_r(NULL,",",&svptr);
			d1 = strtok_r(NULL,",",&svptr);
			d1 = strtok_r(NULL,",",&svptr);
			app1_time_out=atoi(d1);
			d1 = strtok_r(NULL,",",&svptr);
			pkt_loss=atoi(d1);
			printf("app1_time_out = %ld\n",app1_time_out);
			
			spdc_app1->current_lpdcs = 0;
					
			spdc_app1->modes = (struct SPDCmode_info **)malloc(spdc_app1->num_modes * sizeof(struct SPDCmode_info *));
			
			APP1Rear=-1;
			APP1Front=-1;
			APP1TSB_Count_SPDC=0;
			old_df_rear_SAPP1=-1;
			old_df_front_SAPP1=-1;
			glob_fracsec=0;
			glob_sec=0;

			int fi;
			for(fi=0;fi<APP1MAXTSBS;fi++)
			{
				LT[fi].Min_pmutolpdc=0.0 ;
				LT[fi].first_arrival_lpdc=DBL_MAX;
			}
			/*			getdelim (&line, &len, ('\n'), fp_csv);*/
/*			d1 = strtok_r(line,",",&svptr);*/
					
			for(j=0; j<spdc_app1->num_modes; j++){
				spdc_app1->modes[j] = (struct SPDCmode_info *)malloc(sizeof(struct SPDCmode_info));
				spdc_app1->modes[j]->sum_theta_c = 0.0;
				
/*				d1 = strtok_r(NULL,",",&svptr);*/
/*				spdc_app1->modes[j]->threshold = atof(d1);*/
/*				printf("Threshold= %f\n",spdc_app1->modes[j]->threshold);*/
				
			}
			
			
			
			spdc_app1->cap_theta = (struct Captured_theta_c_i **)malloc( spdc_app1->num_lpdcs * sizeof(struct Captured_theta_c_i *));

			getdelim (&line, &len, ('\n'), fp_csv);
			d1 = strtok_r(line,",",&svptr);
						
			for(j=0; j<spdc_app1->num_lpdcs; j++){
		
				spdc_app1->cap_theta[j] = (struct Captured_theta_c_i *)malloc(sizeof(struct Captured_theta_c_i));
				d1 = strtok_r(NULL,",",&svptr);
				
				spdc_app1->cap_theta[j]->lpdc_id = atoi(d1);
				
				
				spdc_app1->cap_theta[j]->mw = (struct modewise **)malloc(spdc_app1->num_modes * sizeof(struct modewise*));
				
				char *l2;
				char *d2, *svptr2;
				size_t len2 = 0;
				
				char *l3;
				char *d3, *svptr3;
				size_t len3 = 0;
				
				int k;
				
				getdelim (&l2, &len2, ('\n'), fp_csv);
				d2 = strtok_r(l2,",",&svptr2);
				
				getdelim (&l3, &len3, ('\n'), fp_csv);
				d3 = strtok_r(l3,",",&svptr3);
			
				for(k=0; k<spdc_app1->num_modes; k++){
					spdc_app1->cap_theta[j]->mw[k] = (struct modewise *)malloc(sizeof(struct modewise));
					spdc_app1->cap_theta[j]->mw[k]->theta_c_i = 0.0;

					d2 = strtok_r(NULL,",",&svptr2);
					d3 = strtok_r(NULL,",",&svptr3);
					
					spdc_app1->cap_theta[j]->mw[k]->power_value = atof(d2);
					spdc_app1->cap_theta[j]->mw[k]->threshold = atof(d3);
					
					spdc_app1->modes[k]->power_sum += spdc_app1->cap_theta[j]->mw[k]->power_value;
					
				}
			}
			
/*			printf("spdc_app1->modes[0]->sum_theta_c = %f\n",spdc_app1->modes[0]->sum_theta_c);*/
/*			printf("spdc_app1->modes[0]->sum_theta_c = %f\n",spdc_app1->modes[1]->sum_theta_c);*/
/*	printf("spdc_app1->modes[0]->sum_theta_c = %f\n",spdc_app1->modes[2]->sum_theta_c);						*/
			
			break;
		case 2:
				printf("API 2 : DSE\n");
				pthread_mutex_init(&mutex_App2, NULL);
								
				spdc_app2 = (struct SPDCApp2_Info*)malloc(sizeof(struct SPDCApp2_Info));
				spdc_app2->app_id = api_id;
				spdc_app2->data_priority = priority;
				
				d1 = strtok_r(NULL,",",&svptr);
				spdc_app2->num_lpdcs = atoi(d1);
				spdc_app2->current_lpdcs = 0;
				
				int* pdcids = malloc(spdc_app2->num_lpdcs * sizeof(int));
				int* lpdc_numpmu= malloc(spdc_app2->num_lpdcs * sizeof(int));
				int* lpdc_Neighbour_numpmu= malloc(spdc_app2->num_lpdcs * sizeof(int));
				
				int l;
				for(l=0; l<spdc_app2->num_lpdcs; l++){
					d1 = strtok_r(NULL,",",&svptr); // PDCID
					pdcids[l] = atoi(d1);
				
					d1 = strtok_r(NULL,",",&svptr); // PDC#pmu
					lpdc_numpmu[l] = atoi(d1);
					
					d1 = strtok_r(NULL,",",&svptr); // PDC_Neighbour#pmu
					lpdc_Neighbour_numpmu[l] = atoi(d1);
				}
				
				d1 = strtok_r(NULL,",",&svptr); // Z index Mapping for input and output
				fm = fopen(d1,"r");
   				
   				char *infokey, *infoptr;
				size_t lthinfo = 0;
				
   				getdelim (&line, &lthinfo, ('\n'), fm);
   				strtok_r(line,",",&infoptr); 
   				infokey = strtok_r(NULL,",",&infoptr); 
   				
				int mapcount = atoi(infokey);
/*				printf("%d\n",mapcount);*/
				
				memset(&(spdc_app2->hashForZmatrixIndex), 0, sizeof(spdc_app2->hashForZmatrixIndex));
				if( hcreate_r(mapcount , &(spdc_app2->hashForZmatrixIndex)) == 0 ) {
					perror("hcreate_r");
				}
					
				for(l=0; l<mapcount; l++){
					getdelim(&infokey, &lthinfo, ('\n'), fm);
					int size = strlen(infokey);
					infokey[size-1] = '\0'; // Remove \n
/*					printf("%s\n",infokey);*/
					int * index = (int*)malloc(sizeof(int));
					*index = l;
					ENTRY item;
					ENTRY * ret;
		
					item.key = strdup(infokey);
					item.data = index;
					
					if( hsearch_r(item, ENTER, &ret, &(spdc_app2->hashForZmatrixIndex)) == 0 ) {
						perror("hsearch_r");
						exit(1);
					}
				}
   				fclose(fm);
   				
   				d1 = strtok_r(NULL,",",&svptr); // m (No of rows in M)
				int row = atoi(d1);
				d1 = strtok_r(NULL,",",&svptr); // n (No of columns in M)
				int column = atoi(d1);
				
				spdc_app2->M = gsl_matrix_complex_alloc (row,column);     // M
				d1 = strtok_r(NULL,",",&svptr);
				fm = fopen(d1,"r");
   				gsl_matrix_complex_fscanf(fm, spdc_app2->M);   
   				fclose(fm);
   				
   				spdc_app2->Z = gsl_matrix_complex_alloc (row,1);     // Z
   				spdc_app2->W = gsl_matrix_complex_alloc (row,row);     // W
   				spdc_app2->Vhat = gsl_matrix_complex_alloc (column,1);     // Vhat
   				
				memset(&(spdc_app2->hashForApp2Pdcs), 0, sizeof(spdc_app2->hashForApp2Pdcs));
				if( hcreate_r(spdc_app2->num_lpdcs , &(spdc_app2->hashForApp2Pdcs)) == 0 ) {
					perror("hcreate_r");
				}
				
				for(l=0; l<spdc_app2->num_lpdcs; l++){
					struct SPDCApp2_phasor_map* map = (struct SPDCApp2_phasor_map*) malloc(sizeof(struct SPDCApp2_phasor_map));
					
					char lpdc[10];
					sprintf(lpdc,"%d",pdcids[l]);
					map->num_phasor = lpdc_numpmu[l];
					map->num_Neighbour_phasor = lpdc_Neighbour_numpmu[l];
					
					getdelim (&line, &len, ('\n'), fp_csv);
					d1 = strtok_r(line,",",&svptr);
					
					int m;
					map->pmu_phasor = (unsigned char**)malloc(map->num_phasor*sizeof(unsigned char*));
					map->pmu_Neighbour_phasor = (unsigned char**)malloc(map->num_Neighbour_phasor*sizeof(unsigned char*));
					
					for(m=0; m<map->num_phasor; m++){
						d1 = strtok_r(NULL,",",&svptr);
						map->pmu_phasor[m] = strdup(d1);
						char * pch;
						pch = strstr(map->pmu_phasor[m],":");	
						*pch = ',';
					}

					for(m=0; m<map->num_Neighbour_phasor; m++){
						d1 = strtok_r(NULL,",",&svptr);
						map->pmu_Neighbour_phasor[m] = strdup(d1);
						char * pch;
						pch = strstr(map->pmu_Neighbour_phasor[m],":");	
						*pch = ',';
					}
					
					ENTRY item;
					ENTRY * ret;
		
					item.key = strdup(lpdc);
					item.data = map;
					
					if( hsearch_r(item, ENTER, &ret, &(spdc_app2->hashForApp2Pdcs)) == 0 ) {
						perror("hsearch_r");
						exit(1);
					}
				}
				
				free(lpdc_Neighbour_numpmu);
				free(lpdc_numpmu);
				free(pdcids);
			break;
		case 6:
			printf("API 6\n");
			while(LLfirst == NULL) {
				printf("I am here\n");
				usleep(1000*100);
			}
			usleep(1000*1000);
	
			d1 = strtok_r(NULL,",",&svptr);
			int num_queries = atoi(d1);
			
/*			size_t max_APIs = 2000;*/
			
			memset(&hashForAPIs, 0, sizeof(hashForAPIs));
			if( hcreate_r(num_queries, &hashForAPIs) == 0 ) {
		  		perror("hcreate_r");
			}
			
			getdelim (&line, &len, ('\n'), fp_csv); // Remove header
			// A new thread is created for each TCP connection in 'detached' mode. Thus allowing any number of threads to be created. 
			pthread_attr_t attr;
			pthread_attr_init(&attr);

			/* In  the detached state, the thread resources are immediately freed when it terminates, but
			   pthread_join(3) cannot be used to synchronize on the thread termination. */
			if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {
				perror(strerror(err));
				exit(1);
			}																																										     	  
			/* Shed policy = SCHED_FIFO (realtime, first-in first-out) */
			if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {
				perror(strerror(err));		     
				exit(1);
			}
				
			int k;
			for(k=0; k<num_queries; k++){
			
				struct Ph_API_info* temp_query;	
				temp_query = (struct Ph_API_info*) malloc(sizeof(struct Ph_API_info));
				
				temp_query->api_id = api_id; //api_id
				
				getdelim (&line, &len, ('\n'), fp_csv);			
					
				d1 = strtok_r(line,",",&svptr);
				temp_query->query_id = atoi(d1);  // query_id

				d1 = strtok_r(NULL,",",&svptr);
				temp_query->pdcid1 = atoi(d1);  // pdcid1
		
				d1 = strtok_r(NULL,",",&svptr);
				temp_query->pmuid1 = atoi(d1); // pmuid1
		
				d1 = strtok_r(NULL,",",&svptr);
				strcpy(temp_query->phname1,d1); // phname1
		
				d1 = strtok_r(NULL,",",&svptr);
				strcpy(temp_query->protocol1,d1); // protocol1
		
				d1 = strtok_r(NULL,",",&svptr);
				temp_query->pdcid2 = atoi(d1);  // pdcid2
		
				d1 = strtok_r(NULL,",",&svptr);
				temp_query->pmuid2 = atoi(d1); // pmuid2
		
				d1 = strtok_r(NULL,",",&svptr);
				strcpy(temp_query->phname2,d1); // phname2
		
				d1 = strtok_r(NULL,",",&svptr);
				strcpy(temp_query->protocol2,d1); // protocol2
		
				d1 = strtok_r(NULL,",",&svptr);
				temp_query->threshold = atof(d1);  // threshold
				
				temp_query->bounds_slope = 0; // Initially always voilation condition
				temp_query->displace_lb = 0;
				temp_query->displace_ub = 0;
		
				temp_query->model_status = 0; // 0 :means model created. 1 : means model creation in progress
		
				//temp_query->next_query = NULL;
				temp_query->lower_pdc1 = NULL;
				temp_query->lower_pdc2 = NULL;
		
				(temp_query->captured).soc = 0;
				(temp_query->captured).fracsec = 0;
		
				// Insert into hash table
				ENTRY item;
				ENTRY * ret;
		
				char key[100];
				sprintf(key,"%d,%d",temp_query->api_id,temp_query->query_id);
		
				item.key = strdup(key);
				item.data = temp_query;  
		
				if( hsearch_r(item, ENTER, &ret, &hashForAPIs) == 0 ) {
					perror("hsearch_r");
					exit(1);
				}
		
				/* Threads are created for send_query to fetch query from file and send it to LPDC */
				pthread_t t;
				if((err = pthread_create(&t,&attr,send_query,(void*)temp_query))) {
					perror(strerror(err));
					exit(1);	
				}
		
/*printf("temp_query->api_id = %d, temp_query->query_id = %d, temp_query->pdcid1 = %d, temp_query->pdcid2 = %d \n",temp_query->api_id, temp_query->query_id, temp_query->pdcid1, temp_query->pdcid2);	*/
				
			}
			break;
		}
	}
	printf("APIs(Exit)\n");

	fclose(fp_csv);
	pthread_exit(NULL);
}

void DSE_aggregate(unsigned char data[]){
	unsigned char temp3[3], temp5[5],*d;
	unsigned int aid, pdcid;
	unsigned long int soc, fracsec;
	
	d =  data;
	
	d += 2; // Skip SYNC
	d += 2; // Skip Framesize
	
	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);
	
	//SEPARATE soc
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	soc = to_long_int_convertor(temp5);
	
	//SEPARATE fracsec
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	fracsec = to_long_int_convertor(temp5);
	
	d += 2; // Skip Cmdcode
	
	//SEPARATE apiid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	aid = to_intconvertor(temp3);
	
	char key[16];
	sprintf(key,"%d",pdcid);
	ENTRY entry;
	ENTRY *found;
	entry.key = key;
	hsearch_r( entry, FIND, &found, &(spdc_app2->hashForApp2Pdcs));
	
	if(found!=NULL){
		struct SPDCApp2_phasor_map* map = (struct SPDCApp2_phasor_map*)found->data;
		
		memset(temp5,'\0',5);
		copy_cbyc (temp5,d,4);
		float sigma_real = decode_ieee_single(temp5);
		d += 4;
		
		memset(temp5,'\0',5);
		copy_cbyc (temp5,d,4);
		float sigma_imag = decode_ieee_single(temp5);
		d += 4;
		
		gsl_complex Sigmahat2 = gsl_complex_rect(sigma_real,sigma_imag);
/*		gsl_complex invSigmahat = gsl_complex_inverse(gsl_complex_sqrt(Sigmahat2));*/
		gsl_complex invSigmahat = gsl_complex_inverse(Sigmahat2);
		
/*		printf("%f %f\n",sigma_real,sigma_imag);*/
		float Zreal,Zimag,Zhatreal,Zhatimag,VarZhatreal,VarZhatimag;
		
		char Mapkey[50];
		ENTRY e;
		ENTRY *f;
/*printf("here1 %s\n",key);	*/

		int i;
		for(i=0; i<map->num_phasor; i++){
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			Zreal = decode_ieee_single(temp5);
			d += 4;
			
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			Zimag = decode_ieee_single(temp5);
			d += 4;
			
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			Zhatreal = decode_ieee_single(temp5);
			d += 4;

			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			Zhatimag = decode_ieee_single(temp5);
			d += 4;
			
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			VarZhatreal = decode_ieee_single(temp5);
			d += 4;
			
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			VarZhatimag = decode_ieee_single(temp5);
			d += 4;
			
			sprintf(Mapkey,"%s,0,%d",map->pmu_phasor[i],pdcid);
			e.key = Mapkey;
			hsearch_r( e, FIND, &f, &(spdc_app2->hashForZmatrixIndex));
			if(f!= NULL){
				int *ind = (int *)f->data;
				gsl_complex value = gsl_complex_rect(Zreal,Zimag);
				gsl_matrix_complex_set (spdc_app2->Z, *ind, 0, value );
				gsl_matrix_complex_set (spdc_app2->W, *ind, *ind, invSigmahat );
/*				printf("%s %d\n",Mapkey,*ind);*/
			}
			
			sprintf(Mapkey,"%s,1,%d",map->pmu_phasor[i],pdcid);
			e.key = Mapkey;
			hsearch_r( e, FIND, &f, &(spdc_app2->hashForZmatrixIndex));
			if(f!= NULL){
				int *ind = (int *)f->data;
				gsl_complex value = gsl_complex_rect(Zhatreal,Zhatimag);
				gsl_matrix_complex_set (spdc_app2->Z, *ind, 0, value );
				
				gsl_complex var = gsl_complex_rect(VarZhatreal,VarZhatimag);
/*				gsl_complex w = gsl_complex_inverse(gsl_complex_sqrt(var));*/
				gsl_complex w = gsl_complex_inverse(var);
				gsl_matrix_complex_set (spdc_app2->W, *ind, *ind, w );
				
/*				printf("%s %d\n",Mapkey,*ind);*/
			}
			
/*			printf("%s;%f,%f,%f,%f,%f,%f\n",map->pmu_phasor[i],Zreal,Zimag,Zhatreal,Zhatimag,VarZhatreal,VarZhatimag);*/
		}

/*printf("here2 %s\n",key);	*/
		
		float Vhatreal,Vhatimag,VarVhatreal,VarVhatimag;
		for(i=0; i<map->num_Neighbour_phasor; i++){
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			Vhatreal = decode_ieee_single(temp5);
			d += 4;

			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			Vhatimag = decode_ieee_single(temp5);
			d += 4;
			
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			VarVhatreal = decode_ieee_single(temp5);
			d += 4;
			
			memset(temp5,'\0',5);
			copy_cbyc (temp5,d,4);
			VarVhatimag = decode_ieee_single(temp5);
			d += 4;
			
			sprintf(Mapkey,"%s,1,%d",map->pmu_Neighbour_phasor[i],pdcid);
			e.key = Mapkey;
			hsearch_r( e, FIND, &f, &(spdc_app2->hashForZmatrixIndex));
			if(f!= NULL){
				int *ind = (int *)f->data;
				gsl_complex value = gsl_complex_rect(Vhatreal,Vhatimag);
				gsl_matrix_complex_set (spdc_app2->Z, *ind, 0, value );
				
				gsl_complex var = gsl_complex_rect(VarVhatreal,VarVhatimag);
/*				gsl_complex w = gsl_complex_inverse(gsl_complex_sqrt(var));*/
				gsl_complex w = gsl_complex_inverse(var);
				gsl_matrix_complex_set (spdc_app2->W, *ind, *ind, w );
				
/*				printf("%s %d\n",Mapkey,*ind);*/
			}
			
/*			printf("%s;%f,%f,%f,%f\n",map->pmu_Neighbour_phasor[i],Vhatreal,Vhatimag,VarVhatreal,VarVhatimag);*/
		}
		
/*		gsl_matrix_complex_fprintf (stdout, spdc_app2->W, "%g");   */
/*printf("here3 %s\n",key);	*/

		pthread_mutex_lock(&mutex_App2);
		spdc_app2->current_lpdcs++;
		
		if(spdc_app2->current_lpdcs == spdc_app2->num_lpdcs){
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			int err;
			if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {
				perror(strerror(err));
				exit(1);
			}
			if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {
				perror(strerror(err));		     
				exit(1);
			}  
			pthread_t t;
			if((err = pthread_create(&t,&attr,SPDC_DSE_Result,NULL))) {
				perror(strerror(err));		     
				exit(1);
			}
			
			spdc_app2->current_lpdcs = 0;
		}
		pthread_mutex_unlock(&mutex_App2);
	
	}
}

void * SPDC_DSE_Result(){
/*	printf("SPDC\n");*/
	
/*	gsl_matrix_complex_fprintf (stdout, spdc_app2->W, "%g");   */
	
	struct timeval tm;
/*	gettimeofday(&tm, NULL);*/
/*    long int s = tm.tv_sec, f = tm.tv_usec;*/
    
	gsl_matrix_complex * Const = gsl_matrix_complex_alloc (spdc_app2->M->size2,spdc_app2->W->size2); 
	gsl_blas_zgemm(CblasTrans,CblasNoTrans,GSL_COMPLEX_ONE, spdc_app2->M,spdc_app2->W,GSL_COMPLEX_ZERO,Const);
	
	gsl_matrix_complex * Const2 = gsl_matrix_complex_alloc (Const->size1,spdc_app2->M->size2); 
	gsl_blas_zgemm(CblasNoTrans,CblasNoTrans,GSL_COMPLEX_ONE, Const,spdc_app2->M,GSL_COMPLEX_ZERO,Const2);
	
	gsl_matrix_complex * Constinv = gsl_matrix_complex_alloc (Const2->size1,Const2->size2); 
	inverse(Const2,Constinv);

	gsl_matrix_complex * FinalConst = gsl_matrix_complex_alloc (Constinv->size1,Const->size2); 
	gsl_blas_zgemm(CblasNoTrans,CblasNoTrans,GSL_COMPLEX_ONE, Constinv,Const,GSL_COMPLEX_ZERO,FinalConst);

	gsl_blas_zgemm(CblasNoTrans,CblasNoTrans,GSL_COMPLEX_ONE, FinalConst,spdc_app2->Z,GSL_COMPLEX_ZERO,spdc_app2->Vhat);
	
	gsl_matrix_complex_free(Const);
	gsl_matrix_complex_free(Const2);
	gsl_matrix_complex_free(Constinv);
	gsl_matrix_complex_free(FinalConst);
	
	gettimeofday(&tm, NULL);

	app2_Analysing(23,0,0,0,tm.tv_sec,tm.tv_usec);
	
	pthread_exit(NULL);
}

void partial_computation_aggregate(unsigned char data[]){

	unsigned char temp3[3],temp4[4], temp5[5],*d,*ptr,id[2],soC[4],fracsec1[3];
	unsigned int aid, pdcid;
	unsigned int soc, fracsec;
	int APP1TSBIndex=-1;
	d =  data;

	d += 2; // Skip SYNC
	d += 2; // Skip Framesize

	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);

	//SEPARATE soc
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	soc = to_long_int_convertor(temp5);

	//TODO: get correct fracsec

	//SEPARATE fracsec
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	unsigned char *data_fsec;
	data_fsec=(unsigned char *)malloc(sizeof(unsigned char));
	data_fsec[0]= temp5[1];
	data_fsec[1]= temp5[2];
	data_fsec[2]= temp5[3];
	fracsec = to_long_int_convertor1(data_fsec);
	d += 4;


	d += 2; // Skip Cmdcode



			//printf("Newly calculated doc=%u fracsec=%u\n",soc,fracsec);
//printf("Newly calculated doc=%lu fracsec=%lu\n",soc,fracsec);
//		d += 2; // Skip Cmdcode
//
//		//SEPARATE apiid
//		memset(temp3,'\0',3);
//		copy_cbyc (temp3,d,2);
//		d += 2;
//		aid = to_intconvertor(temp3);
//


		int i,index;
		for(index=0; index<spdc_app1->num_lpdcs; index++){
			if(spdc_app1->cap_theta[index]->lpdc_id == pdcid)
				break;
		}

		//TODO : below function
		pthread_mutex_lock(&mutex_App1);



		if((old_df_front_SAPP1 != -1) &&  ((soc < ODFTAPP1S[old_df_front_SAPP1].soc) || ((soc == ODFTAPP1S[old_df_front_SAPP1].soc) && (fracsec < ODFTAPP1S[old_df_front_SAPP1].fracsec)))) {
			//drop_count++;
					//free_dataframe_object(df);
			//		myfree(tsb_fsec);
				//	myfree(fsec);
					//return;
			//printf("old frame arrived \n");
			pthread_mutex_unlock(&mutex_App1);
			return;
				}

		if((app1_time_out < 100000) &&  ((soc < glob_sec) || ((soc == glob_sec) && (fracsec <= glob_fracsec)))) {
			//printf("old frame arrived \n");
			//drop_count++;
			pthread_mutex_unlock(&mutex_App1);
						return;

		}


		//printf("Passing inside soc = %u fracsec=%u\n",soc,fracsec);

		int funret = matchDataFrameTimeToTSBTime_SPDC(soc,fracsec);
				APP1TSBIndex = funret;
			//printf("FUnret in side fun  = %d\n",funret);
				if( funret >= 99) // TSB index match is found
				{
					APP1TSBIndex = funret - 99;
				}
				else
				{
					//printf("funret = %d\n",funret);
					pthread_mutex_lock(&mutex_timeout);
					//TODO : below finction
					if(!create_and_assigntoAPP1TSB_SPDC(data,APP1TSBIndex))
						printf("Problem in function create_and_assigntoAPP1TSB()\n");
					pthread_mutex_unlock(&mutex_timeout);

				}


					//TODO: below function
					prosess_and_store_SPDC(data,i,APP1TSBIndex,index);

		pthread_mutex_unlock(&mutex_App1);		//}

}

void prosess_and_store_SPDC(unsigned char data[],int i,int TSBindex,int index)
{
	unsigned char temp3[3], temp5[5],*d;
	unsigned int aid, pdcid;

	d =  data;
	d += 2; // Skip SYNC
	d += 2; // Skip Framesize
	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);
	//Skip soc
	d += 4;
	//SKip fracsec
	d += 4;
	// Skip Cmdcode
	d += 2;
	//Skip apiid
	d += 2;

//	printf("processing APP1TSBS[TSBindex] %d\n",TSBindex);
//TODO: How to store in TSb
	for(i=0; i<spdc_app1->num_modes; i++){
		memset(temp5,'\0',5);
		copy_cbyc (temp5,d,4);
		APP1TSBS[TSBindex].cap_theta[index].mw[i].theta_c_i=decode_ieee_single(temp5);
		APP1TSBS[TSBindex].modes[i].theta_c_i += (APP1TSBS[TSBindex].cap_theta[index].mw[i].theta_c_i * (spdc_app1->cap_theta[index]->mw[i]->power_value));
		d += 4;
	}
	APP1TSBS[TSBindex].num++;// this count gives how many lpdc packets with this timestamp have been processed so far
	spdc_app1->current_lpdcs ++;

	if(APP1TSBS[TSBindex].num == spdc_app1->num_lpdcs )
	{
		pthread_cancel(APP1TSBS[TSBindex].threadID);
		//mayur change
		//printf("Killed Thread... TSBindex=%d \n",TSBindex);
		pthread_mutex_lock(&mutex_timeout);

		intermediate_dispatch_app1_SPDC(TSBindex);

		pthread_mutex_unlock(&mutex_timeout);

	}
	//TODO: write condition for all packets arrived

}

void CGG_final_process(int index)
{	int i;
//printf("Final Process... \n");

adjustOldFrameCountApp1_SPDC(index);
drop_count += spdc_app1->num_lpdcs - APP1TSBS[index].num;
unsigned int soc,fracsec;

soc = to_long_int_convertor(APP1TSBS[index].soc);

unsigned char dffrac[3];
dffrac[0]=APP1TSBS[index].fracsec[1];
dffrac[1]=APP1TSBS[index].fracsec[2];
dffrac[2]=APP1TSBS[index].fracsec[3];

fracsec = to_long_int_convertor1(dffrac);

glob_fracsec=  fracsec;
glob_sec=soc;

//printf("Final Computation sending glob_sec = %u glob_fracsec = %u\n",glob_sec,glob_fracsec);

//printf("Exit\n");

for(i=0; i<spdc_app1->num_modes; i++){
	APP1TSBS[index].modes[i].theta_c_i /= spdc_app1->modes[i]->power_sum;

}

int j;
for(i=0; i<spdc_app1->num_lpdcs; i++){
	for(j=0; j<spdc_app1->num_modes; j++){
		float diff = fabs(spdc_app1->modes[j]->sum_theta_c - APP1TSBS[index].cap_theta[i].mw[j].theta_c_i);
		if( diff > spdc_app1->cap_theta[i]->mw[j]->threshold ){

			/*printf("Violation at LPDC [%d] Mode [%d] Diff [%f] Threshold [%f] theta_c [%f] Theta_c_i [%f] Soc [%ld] Fracsec [%ld]\n",spdc_app1->cap_theta[i]->lpdc_id,j,diff,spdc_app1->cap_theta[i]->mw[j]->threshold,spdc_app1->modes[j]->sum_theta_c , spdc_app1->cap_theta[i]->mw[j]->theta_c_i,soc,fracsec);*/

		}
		else{

			/*printf("LPDC [%d] Mode [%d] Diff [%f] Threshold [%f] Soc [%ld] Fracsec [%ld]\n",spdc_app1->cap_theta[i]->lpdc_id,j,diff,spdc_app1->cap_theta[i]->mw[j]->threshold,soc,fracsec);*/
			/*				printf("LPDC [%d] Mode [%d] Diff [%f] Theta_c [%f] Theta_c_i [%f]\n",spdc_app1->cap_theta[i]->lpdc_id,j,diff,spdc_app1->modes[j]->sum_theta_c , spdc_app1->cap_theta[i]->mw[j]->theta_c_i);*/
		}
		spdc_app1->cap_theta[i]->mw[j]->theta_c_i = 0.0;
	}
}

for(i=0; i<spdc_app1->num_modes; i++){
	spdc_app1->modes[i]->sum_theta_c = 0.0;
	APP1TSBS[index].modes[i].theta_c_i=0.0;
}
spdc_app1->current_lpdcs = 0;
//printf("Final Before TSB ...APP1TSB_Count_SPDC=%d\n",APP1TSB_Count_SPDC );
APP1TSB_Count_SPDC--;
//printf("Final After TSB ...APP1TSB_Count_SPDC=%d\n",APP1TSB_Count_SPDC );

struct timeval tm;
gettimeofday(&tm, NULL);
Analysing(12,0,tm.tv_sec,tm.tv_usec,index);

}
void intermediate_dispatch_app1_SPDC(int xx) {
	int i;
	//unsigned int yy = xx;

	if(APP1Front < APP1Rear) {

		if(xx == APP1Rear) {
			//TODO: Below functions
			CGG_final_process(xx);
			getAPP1TSBFree_SPDC(xx);

			APP1Rear= (APP1Rear - 1	)<0 ? APP1Rear-1+APP1MAXTSBS:APP1Rear -1; // one step backward
		} else if(xx == APP1Front) {
			//TODO: Below functions
			CGG_final_process(APP1Front);
			getAPP1TSBFree_SPDC(APP1Front);

			APP1Front = (APP1Front + 1) % APP1MAXTSBS;

		} else if(xx <APP1Rear)  {
			//TODO: Below functions
			CGG_final_process(xx);
			getAPP1TSBFree_SPDC(xx);
			//unsigned int tsb_soc1;
			while (xx < APP1Rear ) {

				copy_cbyc ((unsigned char *)APP1TSBS[xx].soc,(unsigned char *)APP1TSBS[xx + 1].soc,4);
				copy_cbyc ((unsigned char *)APP1TSBS[xx].fracsec,(unsigned char *)APP1TSBS[xx + 1].fracsec,4);

				APP1TSBS[xx].threadID = APP1TSBS[xx + 1].threadID;
				APP1TSBS[xx].num = APP1TSBS[xx + 1].num;
				APP1TSBS[xx].total_mode_lpdc = APP1TSBS[xx + 1].total_mode_lpdc;
				for(i=0;i<spdc_app1->num_modes;i++)
				{
					APP1TSBS[xx].modes[i].current_lpdcs=APP1TSBS[xx+1].modes[i].current_lpdcs;
					APP1TSBS[xx].modes[i].power_sum=APP1TSBS[xx+1].modes[i].power_sum;
					APP1TSBS[xx].modes[i].theta_c_i=APP1TSBS[xx+1].modes[i].theta_c_i;
				}
				APP1TSBS[xx].cap_theta_index = APP1TSBS[xx+1].cap_theta_index;

				//========
					int j;
					for(j=0; j<spdc_app1->num_lpdcs; j++){
						APP1TSBS[xx].cap_theta[j].lpdc_id = APP1TSBS[xx+1].cap_theta[j].lpdc_id;

						int k;
						for(k=0; k<spdc_app1->num_modes; k++){
							 APP1TSBS[xx].cap_theta[j].mw[k].theta_c_i = APP1TSBS[xx+1].cap_theta[j].mw[k].theta_c_i;
							 APP1TSBS[xx].cap_theta[j].mw[k].power_value = APP1TSBS[xx+1].cap_theta[j].mw[k].power_value;
							 APP1TSBS[xx].cap_theta[j].mw[k].threshold = APP1TSBS[xx+1].cap_theta[j].mw[k].threshold;

						}
					}
				//=======



				xx++;
			}

			APP1Rear= (APP1Rear - 1	)<0 ? APP1Rear-1+APP1MAXTSBS:APP1Rear -1; // one step backward
		}
	} else if (APP1Front > APP1Rear) {

		if(xx == APP1Rear) {
			//TODO: Below functions
			CGG_final_process(APP1Rear);
			getAPP1TSBFree_SPDC(APP1Rear);

			if(APP1Front - APP1Rear == APP1MAXTSBS)
			{
				APP1Rear = APP1Front;
			 } else {

				APP1Rear = APP1Rear - 1;
			 	if(APP1Rear <0)
			 		APP1Rear = APP1Rear + APP1MAXTSBS;
				}

			} else if(xx == APP1Front) {
				//TODO: Below functions
				CGG_final_process(APP1Front);
				getAPP1TSBFree_SPDC(APP1Front);

				APP1Front = (APP1Front + 1) % APP1MAXTSBS;

			} else if(xx > APP1Front) { // xx>APP1Front
				//TODO: Below functions
				CGG_final_process(xx);
				getAPP1TSBFree_SPDC(xx);

				int rr = xx;

				while (rr != APP1Rear) {

					int h = (rr + 1) % APP1MAXTSBS;
					copy_cbyc ((unsigned char *)APP1TSBS[rr].soc,(unsigned char *)APP1TSBS[h].soc,4);
					copy_cbyc ((unsigned char *)APP1TSBS[rr].fracsec,(unsigned char *)APP1TSBS[h].fracsec,4);

					APP1TSBS[rr].threadID = APP1TSBS[h].threadID;
					APP1TSBS[rr].num = APP1TSBS[h].num;
					APP1TSBS[rr].total_mode_lpdc = APP1TSBS[h].total_mode_lpdc;
					for(i=0;i<app1->num_modes;i++)
					{
						APP1TSBS[rr].modes[i].current_lpdcs=APP1TSBS[h].modes[i].current_lpdcs;
						APP1TSBS[rr].modes[i].power_sum=APP1TSBS[h].modes[i].power_sum;
						APP1TSBS[rr].modes[i].theta_c_i=APP1TSBS[h].modes[i].theta_c_i;
					}
					//=====
					APP1TSBS[rr].cap_theta_index = APP1TSBS[h].cap_theta_index;

					//========
					int j;
					for(j=0; j<spdc_app1->num_lpdcs; j++){
						APP1TSBS[rr].cap_theta[j].lpdc_id = APP1TSBS[h].cap_theta[j].lpdc_id;

						int k;
						for(k=0; k<spdc_app1->num_modes; k++){
							APP1TSBS[rr].cap_theta[j].mw[k].theta_c_i = APP1TSBS[h].cap_theta[j].mw[k].theta_c_i;
							APP1TSBS[rr].cap_theta[j].mw[k].power_value = APP1TSBS[h].cap_theta[j].mw[k].power_value;
							APP1TSBS[rr].cap_theta[j].mw[k].threshold = APP1TSBS[h].cap_theta[j].mw[k].threshold;

						}
					}

					//=====


					rr = (rr + 1) % APP1MAXTSBS;
				}

				APP1Rear= (APP1Rear - 1	)<0 ? APP1Rear-1+APP1MAXTSBS:APP1Rear -1; // one step backward

		} else if(xx < APP1Front) {
			//TODO: Below functions
			CGG_final_process(xx);
			getAPP1TSBFree_SPDC(xx);
			int rr = xx;
			while (rr != APP1Rear) {

					int h = (rr + 1) % APP1MAXTSBS;


					copy_cbyc ((unsigned char *)APP1TSBS[rr].soc,(unsigned char *)APP1TSBS[h].soc,4);
					copy_cbyc ((unsigned char *)APP1TSBS[rr].fracsec,(unsigned char *)APP1TSBS[h].fracsec,4);

					APP1TSBS[rr].threadID = APP1TSBS[h].threadID;
					APP1TSBS[rr].num = APP1TSBS[h].num;
					APP1TSBS[rr].total_mode_lpdc = APP1TSBS[h].total_mode_lpdc;
					for(i=0;i<app1->num_modes;i++)
					{
						APP1TSBS[rr].modes[i].current_lpdcs=APP1TSBS[h].modes[i].current_lpdcs;
						APP1TSBS[rr].modes[i].power_sum=APP1TSBS[h].modes[i].power_sum;
						APP1TSBS[rr].modes[i].theta_c_i=APP1TSBS[h].modes[i].theta_c_i;
					}

					//=====
					APP1TSBS[rr].cap_theta_index = APP1TSBS[h].cap_theta_index;

					//========
					int j;
					for(j=0; j<spdc_app1->num_lpdcs; j++){
						APP1TSBS[rr].cap_theta[j].lpdc_id = APP1TSBS[h].cap_theta[j].lpdc_id;

						int k;
						for(k=0; k<spdc_app1->num_modes; k++){
							APP1TSBS[rr].cap_theta[j].mw[k].theta_c_i = APP1TSBS[h].cap_theta[j].mw[k].theta_c_i;
							APP1TSBS[rr].cap_theta[j].mw[k].power_value = APP1TSBS[h].cap_theta[j].mw[k].power_value;
							APP1TSBS[rr].cap_theta[j].mw[k].threshold = APP1TSBS[h].cap_theta[j].mw[k].threshold;

						}
					}

					//=====




					rr = (rr + 1) % APP1MAXTSBS;
				}

				APP1Rear= (APP1Rear - 1	)<0 ? APP1Rear-1+APP1MAXTSBS:APP1Rear -1; // one step backward
		}
	} else {
		//TODO: Below functions
		CGG_final_process(APP1Front);
		getAPP1TSBFree_SPDC(APP1Front);
	}
}

void adjustOldFrameCountApp1_SPDC(int index) {

	//printf("Inside adjustOldFrameCountApp1()\n");
	int flag = 0;
	int i;

	unsigned char *tsb_fsec;
	unsigned int tsb_soc,tsb_fracsec;

	tsb_fsec = malloc(3*sizeof(unsigned char));
	tsb_fsec[0] = APP1TSBS[index].fracsec[1];
	tsb_fsec[1] = APP1TSBS[index].fracsec[2];
	tsb_fsec[2] = APP1TSBS[index].fracsec[3];

	tsb_soc = to_long_int_convertor((unsigned char *)APP1TSBS[index].soc);
	tsb_fracsec = to_long_int_convertor1(tsb_fsec);

	for(i = 0; i< OLDFRAMECOUNTAPP1S; i++) {

		if((ODFTAPP1S[i].soc == tsb_soc) && (ODFTAPP1S[i].fracsec == tsb_fracsec)) {

			flag = 1;
			break;
		}
	}

	if(flag)
	{
		int xx = i;
		ODFTAPP1S[xx].soc = 0;
		ODFTAPP1S[xx].fracsec = 0;
		if(old_df_front_SAPP1 < old_df_rear_SAPP1) {

			if(xx == old_df_rear_SAPP1) {

				old_df_rear_SAPP1= (old_df_rear_SAPP1 - 1)%OLDFRAMECOUNTAPP1S;
				if(old_df_rear_SAPP1 <0)

					old_df_rear_SAPP1 = old_df_rear_SAPP1 + OLDFRAMECOUNTAPP1S;//

			} else if(xx == old_df_front_SAPP1) {

				old_df_front_SAPP1 = (old_df_front_SAPP1 + 1) % OLDFRAMECOUNTAPP1S;

			} else if(xx < old_df_rear_SAPP1)  {

				while (xx < old_df_rear_SAPP1) {

					ODFTAPP1S[xx].soc = ODFTAPP1S[xx + 1].soc;
					ODFTAPP1S[xx].fracsec = ODFTAPP1S[xx + 1].fracsec;
					xx++;
				}

				ODFTAPP1S[old_df_rear_SAPP1].soc = 0;
				ODFTAPP1S[old_df_rear_SAPP1].fracsec = 0;

				old_df_rear_SAPP1 = (old_df_rear_SAPP1 - 1) % OLDFRAMECOUNTAPP1S;
				if(old_df_rear_SAPP1 < 0)
					old_df_rear_SAPP1 = old_df_rear_SAPP1 + OLDFRAMECOUNTAPP1S;
			}
		} else if (old_df_front_SAPP1 > old_df_rear_SAPP1) {

			if(xx == old_df_rear_SAPP1) {

				if(old_df_front_SAPP1 - old_df_rear_SAPP1 == OLDFRAMECOUNTAPP1S)
				{
								//front = rear;
					old_df_rear_SAPP1 = old_df_front_SAPP1;

				 } else {

					old_df_rear_SAPP1 = old_df_rear_SAPP1 - 1;
						if(old_df_rear_SAPP1 <0)
							old_df_rear_SAPP1 = old_df_rear_SAPP1 + OLDFRAMECOUNTAPP1S;
				}

			} else if(xx == old_df_front_SAPP1) {

				old_df_front_SAPP1 = (old_df_front_SAPP1 + 1) % OLDFRAMECOUNTAPP1S;

			} else  { // xx>front

				int rr = xx;
				while (rr != old_df_rear_SAPP1) {

					int h = (rr + 1) % OLDFRAMECOUNTAPP1S;
					ODFTAPP1S[rr].soc = ODFTAPP1S[h].soc;
					ODFTAPP1S[rr].fracsec = ODFTAPP1S[h].fracsec;
					rr = (rr + 1) % OLDFRAMECOUNTAPP1S;
				}

				old_df_rear_SAPP1 = (old_df_rear_SAPP1 - 1) % OLDFRAMECOUNTAPP1S;
				if(old_df_rear_SAPP1 < 0)
					old_df_rear_SAPP1 = old_df_rear_SAPP1 + OLDFRAMECOUNTAPP1S;
			}

		} else {

			old_df_front_SAPP1 = (old_df_front_SAPP1 + 1)%OLDFRAMECOUNTAPP1S;
			old_df_rear_SAPP1 = old_df_front_SAPP1;
		}
	}else {

	//printf("No match with ODFT\n");
	}
}


void getAPP1TSBFree_SPDC(int index)
{

	if(APP1TSBS[index].soc)
	{
		//free(APP1TSBS[index].soc);
		APP1TSBS[index].soc=NULL;
	}

	if(APP1TSBS[index].fracsec)
	{
	//	free(APP1TSBS[index].fracsec);
		APP1TSBS[index].fracsec=NULL;
	}


	if(APP1TSBS[index].modes)
	{
		//free(APP1TSBS[index].modes);
		APP1TSBS[index].modes=NULL;
	}
//	if(APP1TSBS[index].cap_theta->mw)
//	{
//		free(APP1TSBS[index].cap_theta->mw);
//		APP1TSBS[index].cap_theta->mw=NULL;
//	}
//	int i;
//	for(i=0;i<spdc_app1->num_lpdcs;i++)
//	{
//		if(APP1TSBS[index].cap_theta[i])
//		{
//
		//	free(APP1TSBS[index].cap_theta);
//			APP1TSBS[index].cap_theta[i]=NULL;
//		}
//
//	}


}


int create_and_assigntoAPP1TSB_SPDC(unsigned char data[],int index)
{
	int i=0;
	APP1TSBS[index].soc = malloc(5);
	if(APP1TSBS[index].soc == NULL)
	{	printf("Memory Allocation Error\n");
		return 0;
	}

	APP1TSBS[index].fracsec = malloc(5);
	if(APP1TSBS[index].fracsec == NULL)
	{	printf("Memory Allocation Error\n");
	return 0;
	}

	memset(APP1TSBS[index].soc,'\0',5);
	memset(APP1TSBS[index].fracsec,'\0',5);

	unsigned char temp3[3], temp5[5],*d,soc_temp[5],fracsec_temp[5],*ptr,fracsec1[3],soC[4],id[2];
	unsigned int aid, pdcid;
	unsigned int soc, fracsec;
	d =  data;

	d += 2; // Skip SYNC
	d += 2; // Skip Framesize

	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);

	//SEPARATE soc
	memset(soc_temp,'\0',5);
	copy_cbyc (soc_temp,d,4);
	soc = to_long_int_convertor(soc_temp);
	d += 4;


	//SEPARATE fracsec
	memset(fracsec_temp,'\0',5);
	copy_cbyc (fracsec_temp,d,4);
	unsigned char *data_fsec;
	data_fsec=(unsigned char *)malloc(sizeof(unsigned char));
	data_fsec[0]= fracsec_temp[1];
	data_fsec[1]= fracsec_temp[2];
	data_fsec[2]= fracsec_temp[3];
	fracsec = to_long_int_convertor1(data_fsec);

	d += 4;


	d += 2; // Skip Cmdcode



	//TODO: copy iinto below dfs
	copy_cbyc((unsigned char *)APP1TSBS[index].soc,soc_temp,4);
	copy_cbyc((unsigned char *)APP1TSBS[index].fracsec,fracsec_temp,4);



	if(old_df_front_SAPP1 == -1) {

		old_df_front_SAPP1 = 0;
		old_df_rear_SAPP1 = 0;

		ODFTAPP1S[old_df_rear_SAPP1].soc = soc;
		ODFTAPP1S[old_df_rear_SAPP1].fracsec = fracsec;
		//printf("Assigning first ODFTAPP1S[old_df_rear_SAPP1].fracsec=%u\n",ODFTAPP1S[old_df_rear_SAPP1].fracsec);

	} else {
		if(old_df_rear_SAPP1 != old_df_front_SAPP1) //This is additional step to the algorithm
		old_df_rear_SAPP1 = (old_df_rear_SAPP1 + 1)%OLDFRAMECOUNTAPP1S;
		ODFTAPP1S[old_df_rear_SAPP1].soc = soc;
		ODFTAPP1S[old_df_rear_SAPP1].fracsec = fracsec;
		//printf("Assigning Next ODFTAPP1S[old_df_rear_SAPP1].fracsec=%u\n",ODFTAPP1S[old_df_rear_SAPP1].fracsec);
	}





	APP1TSBS[index].num = 0;

	//TODO: count_total_modes_PMU variable
	APP1TSBS[index].total_mode_lpdc = count_total_modes_PMU;
	APP1TSBS[index].cap_theta_index=0;
	APP1TSBS[index].modes = (struct modeApp1_SPDC *)malloc(spdc_app1->num_modes * sizeof(struct modeApp1_SPDC));

	if(APP1TSBS[index].modes == NULL)
	{	printf("Memory Allocation Error\n");
		return 0;
	}

	for(i=0;i<spdc_app1->num_modes;i++)
	{
		APP1TSBS[index].modes[i].current_lpdcs=0;
		APP1TSBS[index].modes[i].power_sum=0.0;
		APP1TSBS[index].modes[i].theta_c_i=0.0;
	}


	int j;
	APP1TSBS[index].cap_theta = (struct Captured_theta_c_i1 *)malloc(sizeof(struct Captured_theta_c_i1));
	for(j=0; j<spdc_app1->num_lpdcs; j++){

		APP1TSBS[index].cap_theta[j].lpdc_id = (int)pdcid;//This line does not make any sense

		APP1TSBS[index].cap_theta[j].mw = (struct modewise1 *)malloc(spdc_app1->num_modes * sizeof(struct modewise1));
		int k;
		for(k=0; k<spdc_app1->num_modes; k++){

			APP1TSBS[index].cap_theta[j].mw[k].theta_c_i = 0.0;
			APP1TSBS[index].cap_theta[j].mw[k].power_value = 0.0;
			APP1TSBS[index].cap_theta[j].mw[k].threshold = 0.0;

		}
	}



	//printf("Created TSB ...Index = %d  APP1TSB_Count_SPDC=%d\n",index,APP1TSB_Count_SPDC );
	//Launching timer thread, since this will be the first packet.
	APP1TSB_Count_SPDC++;
	//printf("incremented TSB ...APP1TSB_Count_SPDC=%d\n",APP1TSB_Count_SPDC );
	//TODO: Below function
	createStartTimerThread_SPDC(index);


	return 1;
	}

void *startTimer_SPDC(void *ind) {

	long index;
	index = (long)ind;
	//printf("Sleeping Thread...TSBindex=%ld \n",index);
	APP1TSBS[index].threadID = pthread_self();
	usleep(app1_time_out);
	//printf("Awaken Thread...TSBindex=%ld \n",index);
	pthread_mutex_lock(&mutex_timeout);
	//TODO: below function
	intermediate_dispatch_app1_SPDC(index);
	pthread_mutex_unlock(&mutex_timeout);
	return NULL;

}

/* ----------------------------------------------------------------------------	*/
/* void createStartTimerThread(int index)										*/
/* ----------------------------------------------------------------------------	*/

void createStartTimerThread_SPDC(int index) {

	int err;
	pthread_t t;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

		perror(strerror(err));
		exit(1);
	}

	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

		perror(strerror(err));
		exit(1);
	}

	long ind = index;
	if((err = pthread_create(&t,&attr,startTimer_SPDC,(void *)ind))) {

			perror(strerror(err));
			exit(1);
	}
}

int matchDataFrameTimeToTSBTime_SPDC(unsigned int df_soc,unsigned int df_fracsec) {

	pthread_mutex_lock(&mutex_timeout);
	//printf("fun Prev : APP1Front=%d APP1Rear=%d APP1TSBS_Count=%d",APP1Front,APP1Rear,APP1TSBS_Count);
	int flag = 0,i;
	unsigned int tsb_soc,tsb_fracsec;
	unsigned char *tsb_fsec;
			tsb_fsec = malloc(3*sizeof(unsigned char));	for(i = 0; i<APP1MAXTSBS; i++) {
		if(APP1TSBS[i].soc != NULL) {
			tsb_soc = to_long_int_convertor((unsigned char *)APP1TSBS[i].soc);
			tsb_fsec[0] = APP1TSBS[i].fracsec[1];
			tsb_fsec[1] = APP1TSBS[i].fracsec[2];
			tsb_fsec[2] = APP1TSBS[i].fracsec[3];
			tsb_fracsec = to_long_int_convertor1(tsb_fsec);
			//printf("Update %d:  soc:%u frame %u   fracsec=%u frame %u\n",i,tsb_soc,df_soc,tsb_fracsec,df_fracsec);
			if(df_soc == tsb_soc)
			{
				if(tsb_fracsec == df_fracsec) {

					flag = 1;
					break;
				}
			}
		}
	}

	//printf("flag = %d application soc:%u %u  fracsec=%u %u\n",flag,tsb_soc,df_soc,tsb_fracsec,df_fracsec);
//	free(tsb_fsec);
	pthread_mutex_unlock(&mutex_timeout);

	if(flag)
		return (99+i); // If match, return index of TSB also.
	else if(APP1Front == -1)
	{
		APP1Front=0;
		APP1Rear=0;
		//printf("Later : APP1Front=%d APP1Rear=%d APP1TSBS_Count=%d",APP1Front,APP1Rear,APP1TSBS_Count);
		return 0;
	}

	else
	{
		int xxx;
		if(APP1TSB_Count_SPDC == APP1MAXTSBS)
		{	xxx = (APP1Rear + 1) % APP1MAXTSBS;
			APP1Rear = (APP1Rear + 1) % APP1MAXTSBS;
		}

		 else
		 {
			if (APP1Front == APP1Rear)
			{
				if (APP1TSB_Count_SPDC == 0)
					xxx = APP1Rear;
				else
				{
					xxx = (APP1Rear + 1) % APP1MAXTSBS;
					APP1Rear = (APP1Rear + 1) % APP1MAXTSBS;
				}

			} else {

					xxx = (APP1Rear + 1) % APP1MAXTSBS;
					APP1Rear = (APP1Rear + 1) % APP1MAXTSBS;
			}
		}
		//printf("fun Later : APP1Front=%d APP1Rear=%d APP1TSB_Count_SPDC=%d",APP1Front,APP1Rear,APP1TSB_Count_SPDC);
		return xxx; // Find index of TSB where the frame would be assigned
	}
}


int matchDataFrameTimeToTSBTime_SPDC_No_Update(unsigned int df_soc,unsigned int df_fracsec) {

	pthread_mutex_lock(&mutex_timeout);
	//printf("fun Prev : APP1Front=%d APP1Rear=%d APP1TSB_Count_SPDC=%d\n",APP1Front,APP1Rear,APP1TSB_Count_SPDC);
	int flag = 0,i;
	unsigned int tsb_soc,tsb_fracsec;
	unsigned char *tsb_fsec;
			tsb_fsec = malloc(3*sizeof(unsigned char));
	for(i = 0; i<APP1MAXTSBS; i++) {
		if(APP1TSBS[i].soc != NULL) {
			tsb_soc = to_long_int_convertor((unsigned char *)APP1TSBS[i].soc);
			tsb_fsec[0] = APP1TSBS[i].fracsec[1];
			tsb_fsec[1] = APP1TSBS[i].fracsec[2];
			tsb_fsec[2] = APP1TSBS[i].fracsec[3];
			tsb_fracsec = to_long_int_convertor1(tsb_fsec);
			//printf("Flag : %d In the loop  %d:frame %u   fracsec=%u n",flag,i,to_long_int_convertor((unsigned char *)APP1TSBS[i].soc),to_long_int_convertor1(tsb_fsec));

			if(df_soc == tsb_soc)
			{
				if(tsb_fracsec == df_fracsec) {

					flag = 1;
					break;
				}
			}
		}
	}



	pthread_mutex_unlock(&mutex_timeout);
	//free(tsb_fsec);
	//printf("Flag : %d No Update %d:  soc:%u frame %u   fracsec=%u frame %u\n",flag,i,tsb_soc,df_soc,tsb_fracsec,df_fracsec);
	if(flag)
		return (99); // If match, return index of TSB also.
	else if(APP1Front == -1)
	{
		//printf("Later : APP1Front=%d APP1Rear=%d APP1TSB_Count_SPDC=%d\n",APP1Front,APP1Rear,APP1TSB_Count_SPDC);
		return 0;
	}

	else
	{
		int xxx;
		if(APP1TSB_Count_SPDC == APP1MAXTSBS)
		{	xxx = (APP1Rear + 1) % APP1MAXTSBS;

		}

		 else
		 {
			if (APP1Front == APP1Rear)
			{
				if (APP1TSB_Count_SPDC == 0)
					xxx = APP1Rear;
				else
				{
					xxx = (APP1Rear + 1) % APP1MAXTSBS;

				}

			} else {

					xxx = (APP1Rear + 1) % APP1MAXTSBS;

			}
		}
		//printf("fun Later : APP1Front=%d APP1Rear=%d APP1TSB_Count_SPDC=%d\n",APP1Front,APP1Rear,APP1TSB_Count_SPDC);
		return xxx; // Find index of TSB where the frame would be assigned
	}

	return -1;
}

void partial_computation_aggregate1(unsigned char data[]){

	unsigned char temp3[3], temp5[5],*d;
	unsigned int aid, pdcid;
	unsigned long int soc, fracsec;
	
	d =  data;
	
	d += 2; // Skip SYNC
	d += 2; // Skip Framesize
	
	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);
	
	//SEPARATE soc
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	soc = to_long_int_convertor(temp5);
	
	//SEPARATE fracsec
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	fracsec = to_long_int_convertor(temp5);
	
	d += 2; // Skip Cmdcode
	
	//SEPARATE apiid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	aid = to_intconvertor(temp3);

	int i,index;
	for(index=0; index<spdc_app1->num_lpdcs; index++){
		if(spdc_app1->cap_theta[index]->lpdc_id == pdcid)
			break;
	}
/*	printf("Index = %d, pdcid = %d\n",index,pdcid);*/
	
	pthread_mutex_lock(&mutex_App1);
	
	for(i=0; i<spdc_app1->num_modes; i++){
		memset(temp5,'\0',5);
		copy_cbyc (temp5,d,4);
		
		spdc_app1->cap_theta[index]->mw[i]->theta_c_i = decode_ieee_single(temp5);
		
		spdc_app1->modes[i]->sum_theta_c += (spdc_app1->cap_theta[index]->mw[i]->theta_c_i) * (spdc_app1->cap_theta[index]->mw[i]->power_value);
		d += 4;
/*		printf("Mode = %d, Index = %d, decodedTheta = %f, Theta_c_i = %f, power = %f\n",i,index, decode_ieee_single(temp5),(spdc_app1->cap_theta[index]->mw[i]->theta_c_i) ,(spdc_app1->cap_theta[index]->mw[i]->power_value));*/
	}
	
	
	spdc_app1->current_lpdcs ++;
		
	if( spdc_app1->num_lpdcs == spdc_app1->current_lpdcs ){

		for(i=0; i<spdc_app1->num_modes; i++){
			spdc_app1->modes[i]->sum_theta_c /= spdc_app1->modes[i]->power_sum;
/*			printf("Mode = %d, Theta_c = %f, powersum = %f\n",i,spdc_app1->modes[i]->sum_theta_c,spdc_app1->modes[i]->power_sum);*/
		}
		
		int j;
		for(i=0; i<spdc_app1->num_lpdcs; i++){
			for(j=0; j<spdc_app1->num_modes; j++){
				float diff = fabs(spdc_app1->modes[j]->sum_theta_c - spdc_app1->cap_theta[i]->mw[j]->theta_c_i);
/*				printf("LPDCIndex = %d, Mode = %d, Diff = %f\n",i,j,diff);*/
				if( diff > spdc_app1->cap_theta[i]->mw[j]->threshold ){

/*printf("Violation at LPDC [%d] Mode [%d] Diff [%f] Threshold [%f] theta_c [%f] Theta_c_i [%f] Soc [%ld] Fracsec [%ld]\n",spdc_app1->cap_theta[i]->lpdc_id,j,diff,spdc_app1->cap_theta[i]->mw[j]->threshold,spdc_app1->modes[j]->sum_theta_c , spdc_app1->cap_theta[i]->mw[j]->theta_c_i,soc,fracsec);*/
					
				}
				else{

/*printf("LPDC [%d] Mode [%d] Diff [%f] Threshold [%f] Soc [%ld] Fracsec [%ld]\n",spdc_app1->cap_theta[i]->lpdc_id,j,diff,spdc_app1->cap_theta[i]->mw[j]->threshold,soc,fracsec);*/
/*				printf("LPDC [%d] Mode [%d] Diff [%f] Theta_c [%f] Theta_c_i [%f]\n",spdc_app1->cap_theta[i]->lpdc_id,j,diff,spdc_app1->modes[j]->sum_theta_c , spdc_app1->cap_theta[i]->mw[j]->theta_c_i);*/
				}
				spdc_app1->cap_theta[i]->mw[j]->theta_c_i = 0.0;
			}
		}
		
		for(i=0; i<spdc_app1->num_modes; i++){
			spdc_app1->modes[i]->sum_theta_c = 0.0;
		}
		spdc_app1->current_lpdcs = 0;	
		
		struct timeval tm;
		gettimeofday(&tm, NULL);
		Analysing(12,pdcid,tm.tv_sec,tm.tv_usec,0);
	}
	
	pthread_mutex_unlock(&mutex_App1);
	
}






/* ----------------------------------------------------------------------------	*/
/* FUNCTION  send_query():                             	     			*/
/* Sends Query to all LPDCs		 			 		*/
/* ----------------------------------------------------------------------------	*/
void* send_query(void *q){
/*	printf("In send_query()\n");*/
	
	struct Ph_API_info* temp_query = (struct Ph_API_info*) q;
	int flag = 0, err, match = 0;
	
	// A new thread is created for each TCP connection in 'detached' mode. Thus allowing any number of threads to be created. 
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/* In  the detached state, the thread resources are immediately freed when it terminates, but
	   pthread_join(3) cannot be used to synchronize on the thread termination. */
	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

		perror(strerror(err));
		exit(1);
	}																																										     	  
	/* Shed policy = SCHED_FIFO (realtime, first-in first-out) */
	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

		perror(strerror(err));		     
		exit(1);
	}  
	
	pthread_mutex_lock(&mutex_Lower_Layer_Details);

	if(LLfirst == NULL) {

		printf("No PMU Present?\n");
		return NULL;

	} else {

		flag = 1;
	}

	pthread_mutex_unlock(&mutex_Lower_Layer_Details);

	if(flag) {
		struct Lower_Layer_Details *temp_pmu = LLfirst;	
		while(temp_pmu != NULL) {
									
			if((match<2) && ((temp_pmu->pmuid == temp_query->pdcid1) && (!strncasecmp(temp_pmu->protocol,temp_query->protocol1,3)))) {			
				temp_query->lower_pdc1 = temp_pmu;
				match++;
			} 
			if((match<2) && ((temp_pmu->pmuid == temp_query->pdcid2) && (!strncasecmp(temp_pmu->protocol,temp_query->protocol2,3)))) {	
				temp_query->lower_pdc2 = temp_pmu;
				match++;
			}
			
			if(match == 2){ //Alaways See This. This is the cause of many unexpected results
/*			printf("swadesh here 3\n");*/
				struct query_send_info* s1 = (struct query_send_info*)malloc(sizeof(struct query_send_info));
				struct query_send_info* s2 = (struct query_send_info*)malloc(sizeof(struct query_send_info));
				
				s1->type = 1;
				s1->send_query = temp_query;
				
				s2->type = 2;
				s2->send_query = temp_query;
				
				pthread_t t1,t2;
				if((err = pthread_create(&t1,&attr,query_sender,(void *)s1))) {
					perror(strerror(err));		     
					exit(1);
				}
				if((err = pthread_create(&t2,&attr,query_sender,(void *)s2))) {
					perror(strerror(err));		     
					exit(1);
				}
				break;
			}
			temp_pmu = temp_pmu->next; 						
		}
	} 
	pthread_exit(NULL);
}

void* query_sender(void* q) {

/*	printf("In query_sender()\n");*/
	
	int err;
	struct query_send_info* t = (struct query_send_info*) q;
/*	char *filter_query = (char *)malloc(1000);*/
	
	// Query making * can be different for diffrent APIs
	
	struct Lower_Layer_Details *dest_pdc;
	
	switch(t->type){
	case 1: //create_query(filter_query,t);
		dest_pdc = t->send_query->lower_pdc1;
		break;
	case 2: //create_query(filter_query,t);
		dest_pdc = t->send_query->lower_pdc2;
		break;
	case 3: //create_query(filter_query,t);
/*		printf("Check-1\n");*/
		if((t->send_query->expected).number == 1){
			dest_pdc = t->send_query->lower_pdc1;
		}
		if((t->send_query->expected).number == 2){
			dest_pdc = t->send_query->lower_pdc2;
		}
		break;
	}
/*	*/
/*	printf("query = %s\n",filter_query);	*/
	// Query built End
	
/*	int q_length = strlen(filter_query);*/

/*	int additional = 4+q_length; // apiid+queryid+query*/

	int additional = 2+2+4+4+4+16+4+4; //apiid + queryid + bounds_slope + displace_lb + displace_ub + phasorname + InvokedSoc + InvokedFracsec;
	
	char *cmdframe = (char*)malloc(19+additional); // basic + (apiid+queryid+query)
	
	if(!strncasecmp(dest_pdc->protocol,"UDP",3)) { /* If Peer is UDP */

		int n;
		cmdframe[18+additional] = '\0';
		memset(cmdframe,'\0',19+additional);
/*if(t->type == 3) printf("Check-2\n");*/
		int size = 0;
		size = create_cmd_frame(t,cmdframe);

		if ((n = sendto(dest_pdc->sockfd,cmdframe, size, 0,(struct sockaddr *)&dest_pdc->llpmu_addr,sizeof(dest_pdc->llpmu_addr)) == -1)) {

			perror("sendto"); 

		} else {
/*			if(t->type == 3) printf("Check-3\n");*/
/*			if(t->type == 3){*/
/*				writeTimeToLog(3,(t->send_query->captured).pmuid,(t->send_query->captured).soc,(t->send_query->captured).fracsec);*/
/*				writeTimeToLog(4,(t->send_query->expected).pmuid,(t->send_query->expected).soc,(t->send_query->expected).fracsec);*/
/*			}*/
/*			else if(t->type == 1)*/
/*				writeTimeToLog(8,t->send_query->pmuid1,(t->send_query->captured).soc,(t->send_query->captured).fracsec);*/
/*			else if(t->type == 2)*/
/*				writeTimeToLog(8,t->send_query->pmuid2,(t->send_query->captured).soc,(t->send_query->captured).fracsec);*/
/*			printf("here [%s]\n",cmdframe);*/

/*printf("numbytes = %d\n",n);*/

/*	int j;*/
/*	for(j=0; j<58; j++){*/
/*		printf("%c",cmdframe[j]);*/
/*	}*/
/*	printf("\n\n");*/
	
			t->send_query->send_remain -= 1;
			free(cmdframe);	
			free(t);
			//dest_pdc->data_transmission_off = 0;	
		}

	} 
	else if(!strncasecmp(dest_pdc->protocol,"TCP",3)){ /* If Peer is TCP */

		int n;

		if(dest_pdc->up == 1) { /* If TCP Peer is UP */

			cmdframe[18+additional] = '\0';

			int size = create_cmd_frame(t,cmdframe);

			if ((n = send(dest_pdc->sockfd,cmdframe, size,0) == -1)) {

				printf("dest_pdc->sockfd %d \n",dest_pdc->sockfd);
				perror("send"); 

			} else {
/*				printf("CMD to filter \n");*/
				t->send_query->send_remain -= 1;
				free(cmdframe);
				free(t);
				//dest_pdc->data_transmission_off = 0;			
			}			
		}
	}

	pthread_exit(NULL);
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION  create_cmd_frame():                                	     			*/
/* ----------------------------------------------------------------------------	*/

int create_cmd_frame(struct query_send_info* t,char cmdframe[]) {
/*	printf("In create_cmd_frame()\n");*/
	int additional,f=0;
	
	long int sec,frac = 0;
	unsigned char fsize[2],pmuid[2],soc[4],fracsec[4];
	uint16_t chk;
	int pmu_id;
	
	unsigned char CMDFLTRSEND[3];
	CMDFLTRSEND[0] = 0x00;
	CMDFLTRSEND[1] = 0x06;
	CMDFLTRSEND[2] = '\0';

	unsigned char CMDRSLTREQSEND[3];
	CMDRSLTREQSEND[0] = 0x01;
	CMDRSLTREQSEND[1] = 0x06;
	CMDRSLTREQSEND[2] = '\0'; 

	unsigned char phasorname[17];
	unsigned char bounds_slope[4],displace_lb[4],displace_ub[4];
	unsigned char ExpectedSoc[4],ExpectedFracsec[4];
	unsigned char InvokedSoc[4],InvokedFracsec[4];
	
	f2c (t->send_query->bounds_slope, bounds_slope);
	f2c (t->send_query->displace_lb, displace_lb);
	f2c (t->send_query->displace_ub, displace_ub);
	
	switch(t->type){
	case 1:	pmu_id = t->send_query->pmuid1;
		strcpy(phasorname,t->send_query->phname1);
		break;
	case 2: pmu_id = t->send_query->pmuid2;
		strcpy(phasorname,t->send_query->phname2);
		break;
	case 3: pmu_id = (t->send_query->expected).pmuid;
		strcpy(phasorname,(t->send_query->expected).phname);
		break;	
	}
//	printf("checking : %ld %ld [%s] %d %f\n\n",t->send_query->expected.soc,t->send_query->expected.fracsec,phasorname,pmu_id,t->send_query->bounds_slope);
/*	*/
	unsigned char apiid[2];	
	unsigned char queryid[2];

	int_to_ascii_convertor(t->send_query->api_id,apiid);
	int_to_ascii_convertor(t->send_query->query_id,queryid);
	
	memset(fsize,'\0',2);

	int_to_ascii_convertor(f,fsize);
	int_to_ascii_convertor(pmu_id,pmuid);

   	gettimeofday(&tim,NULL);
   	sec = (long) tim.tv_sec;
   	long_int_to_ascii_convertor(sec,soc);
   
	frac = tim.tv_usec;
 	long_int_to_ascii_convertor(frac,fracsec);

//printf("fsize= %d, pmuid = %d, soc=%ld, fracsec=%ld, apiid= %d, qid= %d\n",f,pmu_id,sec,frac,t->send_query->api_id,t->send_query->query_id);  
//printf("t->type = %d\n",t->type);
	int index = 0;
	char filter_query[1000];
	int qlength;
	
	switch(t->type){
	case 1: 
	case 2:
		additional = 2+2+4+4+4+16+4+4; //apiid + queryid + bounds_slope + displace_lb + displace_ub + phasorname + InvokedSoc + InvokedFracsec;
		f = 18+additional;

		memset(fsize,'\0',2);
		
		int_to_ascii_convertor(f,fsize);
	
		byte_by_byte_copy((unsigned char *)cmdframe,CMDSYNC,index,2); // SEND CFG
		index += 2;
	
		byte_by_byte_copy((unsigned char *)cmdframe,fsize,index,2);
		index += 2;
		
		byte_by_byte_copy((unsigned char *)cmdframe,pmuid,index,2);
		index += 2;
		//printf("here [%x]\n",cmdframe[4]);
		//printf("here [%x]\n",cmdframe[5]);
	
		byte_by_byte_copy((unsigned char *)cmdframe,soc,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,fracsec,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,CMDFLTRSEND,index,2);
		index += 2;
	
		//printf("here [%x] [%c]\n",cmdframe[14],cmdframe[14]);
		//printf("here [%x] [%c] [%d]\n",cmdframe[15],cmdframe[15],(cmdframe[15]& 0x06)==0x06);
		
		
		byte_by_byte_copy((unsigned char *)cmdframe,apiid,index,2);
		index += 2;
		byte_by_byte_copy((unsigned char *)cmdframe,queryid,index,2);
		index += 2;
		
		long_int_to_ascii_convertor((t->send_query->captured).soc,InvokedSoc);
		long_int_to_ascii_convertor((t->send_query->captured).fracsec,InvokedFracsec);
		byte_by_byte_copy((unsigned char *)cmdframe,InvokedSoc,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,InvokedFracsec,index,4);
		index += 4;
		
		byte_by_byte_copy((unsigned char *)cmdframe,phasorname,index,16);
		index += 16;
		
		byte_by_byte_copy((unsigned char *)cmdframe,bounds_slope,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,displace_lb,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,displace_ub,index,4);
		index += 4;
			
		chk = compute_CRC((unsigned char *)cmdframe,index);
		cmdframe[index++] = (chk >> 8) & ~(~0<<8);  	// CHKSUM high byte; 
		cmdframe[index] = (chk ) & ~(~0<<8);     	// CHKSUM low byte;  
	
		break;
	
	case 3: 		
		additional = 2+2+4+4+16; // apiid + queryid + ExpectedSOC + ExpectedFracsec + phasorname
		f = 18+additional;
		memset(fsize,'\0',2);

		int_to_ascii_convertor(f,fsize);
		
		byte_by_byte_copy((unsigned char *)cmdframe,CMDSYNC,index,2); // SEND CFG
		index += 2;
	
		byte_by_byte_copy((unsigned char *)cmdframe,fsize,index,2);
		index += 2;

		byte_by_byte_copy((unsigned char *)cmdframe,pmuid,index,2);
		index += 2;
			
		byte_by_byte_copy((unsigned char *)cmdframe,soc,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,fracsec,index,4);
		index += 4;
		
		byte_by_byte_copy((unsigned char *)cmdframe,CMDRSLTREQSEND,index,2);
		index += 2;
	
		//printf("here [%x] [%c]\n",cmdframe[14],cmdframe[14]);
		//printf("here [%x] [%c] [%d]\n",cmdframe[15],cmdframe[15],(cmdframe[15]& 0x06)==0x06);
	
		byte_by_byte_copy((unsigned char *)cmdframe,apiid,index,2);
		index += 2;
		byte_by_byte_copy((unsigned char *)cmdframe,queryid,index,2);
		index += 2;
		
		
		//printf("here [%x]\n",cmdframe[4]);
		//printf("here [%x]\n",cmdframe[5]);
		
		long_int_to_ascii_convertor((t->send_query->expected).soc,ExpectedSoc);
		long_int_to_ascii_convertor((t->send_query->expected).fracsec,ExpectedFracsec);
		byte_by_byte_copy((unsigned char *)cmdframe,ExpectedSoc,index,4);
		index += 4;
		byte_by_byte_copy((unsigned char *)cmdframe,ExpectedFracsec,index,4);
		index += 4;
		
		byte_by_byte_copy((unsigned char *)cmdframe,phasorname,index,16);
		index += 16;
	
		//printf("here [%c]\n",cmdframe[20]);
	
		chk = compute_CRC((unsigned char *)cmdframe,index);
		cmdframe[index++] = (chk >> 8) & ~(~0<<8);  	// CHKSUM high byte; 
		cmdframe[index] = (chk ) & ~(~0<<8);     	// CHKSUM low byte;  
	
		//printf("here preview [%d] [%s]\n",strlen(cmdframe),cmdframe[2]);
		break;
	default:
		printf("Not a valid type\n");
		break;
	}
	

	
/*printf("index-f %d %d\n",index-f,t->type);*/
	return index+1;
/*	return 0;*/
}

/* ----------------------------------------------------------------------------	*/
/* FUNCTION li2c (long int t1, unsigned char temp_1[]):	     			*/
/* Function for float to unsigned Character Conversion		     			*/
/* ----------------------------------------------------------------------------	*/

void f2c (float f, unsigned char temp_1[])
{
	int i, j;
	float fv;
	unsigned char a1[sizeof fv];

	fv = f;
	memcpy(a1, &fv, sizeof fv);
	for (i=0, j=3; i<sizeof fv; i++, j--)
	{
		temp_1[j] = a1[i];
	}
}


void initiate_create_model(unsigned char data[]){
/*	printf("In initiate_create_model\n");*/
	// A new thread is created for each TCP connection in 'detached' mode. Thus allowing any number of threads to be created. 
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int err;
	/* In  the detached state, the thread resources are immediately freed when it terminates, but
	   pthread_join(3) cannot be used to synchronize on the thread termination. */
	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

		perror(strerror(err));
		exit(1);
	}																																										     	  
	/* Shed policy = SCHED_FIFO (realtime, first-in first-out) */
	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

		perror(strerror(err));		     
		exit(1);
	}  
	
	
	
	unsigned char temp3[3], temp5[5],*d;
	unsigned int aid, qid, pmuid, pdcid;
	unsigned long int soc, fracsec;
	
	d =  data;
	
	d += 2; // Skip SYNC
	d += 2; // Skip Framesize
	
	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);
	
	//SEPARATE soc
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	soc = to_long_int_convertor(temp5);
	
	//SEPARATE fracsec
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	fracsec = to_long_int_convertor(temp5);
	
	d += 2; // Skip Cmdcode
	
	//SEPARATE pmuid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pmuid = to_intconvertor(temp3);
	
/*	writeTimeToLog(1,pmuid,soc,fracsec);*/
	
	//SEPARATE apiid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	aid = to_intconvertor(temp3);
	
	//SEPARATE Queryid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	qid = to_intconvertor(temp3);
	
	ENTRY item;
	ENTRY *found;
		
	char key[100];
	sprintf(key,"%d,%d",aid,qid);
		
	item.key = strdup(key);
	hsearch_r( item, FIND, &found, &hashForAPIs);
	if(found != NULL) {
/*printf("here1\n");*/
		struct Ph_API_info* temp_query = (struct Ph_API_info*) found->data;
		int flag = 0;

		pthread_mutex_lock(&mutex_PhAPI_info);
		if(temp_query->model_status == 0){
/*printf("here2\n");*/
			temp_query->model_status = 1;
			flag = 1;
		}
		pthread_mutex_unlock(&mutex_PhAPI_info);
/*printf("here3\n");		*/
		if(flag==1){  // Model creation is not started and violation happen
			// Process according to API so that packet may vary
			
			struct timeval tm;
			pthread_mutex_lock(&mutex_App6Analysis);
			gettimeofday(&tm, NULL);
			temp_query->modelstart_fsec = tm.tv_usec;
			pthread_mutex_unlock(&mutex_App6Analysis);
			
			switch(aid){
			case 6:	(temp_query->captured).pmuid = pmuid;
				(temp_query->captured).soc = soc;
				(temp_query->captured).fracsec = fracsec;
				(temp_query->expected).soc = soc;
				(temp_query->expected).fracsec = fracsec;
/*printf("here4\n");			*/
				if(pmuid == temp_query->pmuid1){
					(temp_query->captured).number = 1;
					(temp_query->expected).number = 2;
					(temp_query->expected).pmuid = temp_query->pmuid2;
					strcpy((temp_query->expected).phname,temp_query->phname2);
				}
				else if(pmuid == temp_query->pmuid2){
					(temp_query->captured).number = 2;
					(temp_query->expected).number = 1;
					(temp_query->expected).pmuid = temp_query->pmuid1;
					strcpy((temp_query->expected).phname,temp_query->phname2);
				}
/*printf("here5\n");	*/
				//SEPARATE Phasor Angle
				memset(temp5,'\0',5);
				copy_cbyc (temp5,d,4);
				d += 4;
				(temp_query->captured).phasor_angle = decode_ieee_single(temp5);
				
				//SEPARATE Frequency
				memset(temp5,'\0',5);
				copy_cbyc (temp5,d,4);
				d += 4;
				(temp_query->captured).freq = decode_ieee_single(temp5);
				
				(temp_query->expected).freq = -1; // Requiered
				(temp_query->expected).phasor_angle = -1; // Required
				temp_query->bounds_slope = 0; // Need to calculate
				temp_query->displace_lb = 0; // Need to calculate
				temp_query->displace_ub = 0; // Need to calculate
/*printf("here6\n");*/
				struct query_send_info* s = (struct query_send_info*)malloc(sizeof(struct query_send_info));
				s->type = 3;
				
/*				struct Ph_API_info tempq3;*/
				struct Ph_API_info* info3;
/*				tempq3 = *temp_query;*/
/*				info3 = &tempq3;*/
				info3 = (struct Ph_API_info*)malloc(sizeof(struct Ph_API_info));
				memcpy((void*)info3,(void*)temp_query,sizeof(struct Ph_API_info));
				info3->api_id = temp_query->api_id;
				info3->query_id = temp_query->query_id;
				info3->pdcid1 = temp_query->pdcid1;
				info3->pmuid1 = temp_query->pmuid1;
				
				strcpy(info3->phname1,temp_query->phname1);
				strcpy(info3->protocol1,temp_query->protocol1);
				
				info3->pdcid2 = temp_query->pdcid2;
				info3->pmuid2 = temp_query->pmuid2;
				
				strcpy(info3->phname2,temp_query->phname2);
				strcpy(info3->protocol2,temp_query->protocol2);
				
				info3->threshold = temp_query->threshold;
				info3->bounds_slope = temp_query->bounds_slope;
				info3->displace_lb = temp_query->displace_lb;
				info3->displace_ub = temp_query->displace_ub;
				info3->model_status = temp_query->model_status;
				
				info3->lower_pdc1 = temp_query->lower_pdc1;
				info3->lower_pdc2 = temp_query->lower_pdc2;
				
				info3->captured.number = temp_query->captured.number;
				info3->captured.pmuid = temp_query->captured.pmuid;
				
				strcpy(info3->captured.phname,temp_query->captured.phname);
				
				info3->captured.soc = temp_query->captured.soc;
				info3->captured.fracsec = temp_query->captured.fracsec;
				info3->captured.freq = temp_query->captured.freq;
				info3->captured.phasor_angle = temp_query->captured.phasor_angle;
				
				info3->expected.number = temp_query->expected.number;
				info3->expected.pmuid = temp_query->expected.pmuid;
				
				strcpy(info3->expected.phname,temp_query->expected.phname);
				
				info3->expected.soc = temp_query->expected.soc;
				info3->expected.fracsec = temp_query->expected.fracsec;
				info3->expected.freq = temp_query->expected.freq;
				info3->expected.phasor_angle = temp_query->expected.phasor_angle;
				
/*				printf("info3->api_id = %d, query_id = %d, pdcid1 = %d, pmuid1 = %d, phname1 = [%s], protocol1 = [%s], pdcid2 = %d, pmuid2 = %d, phname2 = [%s], protocol2 = [%s], threshold = %f, bounds_slope = %f, displace_lb = %f, displace_ub = %f, model_status = %d\n\n",info3->api_id,info3->query_id,info3->pdcid1,info3->pmuid1,info3->phname1,info3->protocol1,info3->pdcid2,info3->pmuid2,info3->phname2,info3->protocol2,info3->threshold,info3->bounds_slope,info3->displace_lb,info3->displace_ub,info3->model_status);*/
/*				*/
/*				printf("info3->lower_pdc1 [%p] = temp_query->lower_pdc1 [%p], info3->lower_pdc2 [%p] = temp_query->lower_pdc2 [%p]",info3->lower_pdc1,temp_query->lower_pdc1,info3->lower_pdc2,temp_query->lower_pdc2);*/
/*				*/
/*				printf("info3->captured.number = %d, pmuid = %d, phname = [%s], soc = %ld, fracsec = %ld, freq = %f, angle = %f\n\n",info3->captured.number,info3->captured.pmuid,info3->captured.phname,info3->captured.soc,info3->captured.fracsec,info3->captured.freq,info3->captured.phasor_angle);*/
/*				printf("info3->expected.number = %d, pmuid = %d, phname = [%s], soc = %ld, fracsec = %ld, freq = %f, angle = %f\n\n",info3->expected.number,info3->expected.pmuid,info3->expected.phname,info3->expected.soc,info3->expected.fracsec,info3->expected.freq,info3->expected.phasor_angle);*/
/*				*/
				s->send_query  = info3;		//This is the change
/*				s->send_query  = temp_query;  //This is the change*/
				
/*printf("here7\n");				*/
				pthread_t t;
				
				if((err = pthread_create(&t,&attr,query_sender,(void *)s))) {
					perror(strerror(err));		     
					exit(1);
				}
/*printf("here8\n");					*/
				
/*				writeTimeToLog(3,(temp_query->captured).pmuid,(temp_query->captured).soc,(temp_query->captured).fracsec);*/
/*				writeTimeToLog(4,(temp_query->expected).pmuid,(temp_query->expected).soc,(temp_query->expected).fracsec);*/
/*				*/
				break;
			
			}
		}else{
			//printf("Model creation already in Progess...\n");
		}
		
	}else{
		printf("Wrong API\n");
	}
}


void model_creater(unsigned char data[]){
/*	printf("In model creater\n");*/
	// A new thread is created for each TCP connection in 'detached' mode. Thus allowing any number of threads to be created. 
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int err;
	/* In  the detached state, the thread resources are immediately freed when it terminates, but
	   pthread_join(3) cannot be used to synchronize on the thread termination. */
	if((err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))) {

		perror(strerror(err));
		exit(1);
	}																																										     	  
	/* Shed policy = SCHED_FIFO (realtime, first-in first-out) */
	if((err = pthread_attr_setschedpolicy(&attr,SCHED_FIFO))) {

		perror(strerror(err));		     
		exit(1);
	}  
	
	unsigned char temp3[3], temp5[5],*d;
	unsigned int aid, qid, pmuid, pdcid;
	unsigned long int soc, fracsec;
	
	d =  data;
	
	d += 2; // Skip SYNC
	d += 2; // Skip Framesize
	
	//SEPARATE pdcid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pdcid = to_intconvertor(temp3);
	
	//SEPARATE soc
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	soc = to_long_int_convertor(temp5);
	
	//SEPARATE fracsec
	memset(temp5,'\0',5);
	copy_cbyc (temp5,d,4);
	d += 4;
	fracsec = to_long_int_convertor(temp5);
	
	d += 2; // Skip Cmdcode
	
	//SEPARATE pmuid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	pmuid = to_intconvertor(temp3);
	
/*	writeTimeToLog(7,pmuid,soc,fracsec);*/
	
	//SEPARATE apiid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	aid = to_intconvertor(temp3);
	
	//SEPARATE Queryid
	memset(temp3,'\0',3);
	copy_cbyc (temp3,d,2);
	d += 2;
	qid = to_intconvertor(temp3);
	
	ENTRY item;
	ENTRY *found;
		
	char key[100];
	sprintf(key,"%d,%d",aid,qid);
		
	item.key = strdup(key);
	hsearch_r( item, FIND, &found, &hashForAPIs);
	if(found != NULL) {
		struct Ph_API_info* temp_query = (struct Ph_API_info*) found->data;
		int flag = 0;

		pthread_mutex_lock(&mutex_PhAPI_info);
		if(temp_query->model_status == 1 && (temp_query->expected).pmuid == pmuid && (temp_query->expected).soc == soc && (temp_query->expected).fracsec == fracsec){ // Model creation is running and this is the right response
			flag = 1;
		}
		pthread_mutex_unlock(&mutex_PhAPI_info);
		
		if(flag==1){  // Model creation is not started and violation happen
			// Process according to API so that packet may vary
			switch(aid){
			case 6:	
				//SEPARATE Phasor Angle
				memset(temp5,'\0',5);
				copy_cbyc (temp5,d,4);
				d += 4;
				(temp_query->expected).phasor_angle = decode_ieee_single(temp5);
				
				//SEPARATE Frequency
				memset(temp5,'\0',5);
				copy_cbyc (temp5,d,4);
				d += 4;
				(temp_query->expected).freq = decode_ieee_single(temp5);
				

				if( temp_query->threshold < fabs( (temp_query->expected).phasor_angle - (temp_query->captured).phasor_angle) ){
/*					printf("Global Voilation. at pmuid = %d, soc= %ld, fracsec= %ld\n",pmuid,soc,fracsec); // No need to create New model... Perform Preventive Action*/
					
/*					printf("phasor_angle1 = %f, phasor_angler2 = %f, threshold = %f, ang1-ang2 = %f\n",(temp_query->expected).phasor_angle, (temp_query->captured).phasor_angle,temp_query->threshold, fabs( (temp_query->expected).phasor_angle - (temp_query->captured).phasor_angle));*/
					pthread_mutex_lock(&mutex_App6Analysis);
					total_globle_violation++;
/*					printf("total_globle_violation = %ld\n",total_globle_violation);*/
					pthread_mutex_unlock(&mutex_App6Analysis);
					
/*					pthread_mutex_unlock(&mutex_on_thread);*/
				}
				else{
					temp_query->bounds_slope = (( temp_query->expected.freq + temp_query->captured.freq ) / 2 ) * 2 * 180;
					float theta1 = (temp_query->expected).phasor_angle;
					float theta2 = (temp_query->captured).phasor_angle;
					
					float max = theta1 > theta2 ? theta1 : theta2;
					float min = theta1 < theta2 ? theta1 : theta2;

					float alpha = (( temp_query->threshold - fabs( (temp_query->expected).phasor_angle - (temp_query->captured).phasor_angle ) )/2); 
					
					temp_query->displace_lb = min - alpha;
					temp_query->displace_ub = max + alpha;
/*printf("pmuid1 = %d, pmuid2 = %d, apiid = %d, queryid = %d\n",temp_query->pmuid1,temp_query->pmuid2,temp_query->api_id,temp_query->query_id);*/
					struct query_send_info* s1 = (struct query_send_info*)malloc(sizeof(struct query_send_info));
					struct query_send_info* s2 = (struct query_send_info*)malloc(sizeof(struct query_send_info));
					int err;
				
					struct Ph_API_info* info1;
					info1 = (struct Ph_API_info*)malloc(sizeof(struct Ph_API_info));
					memcpy((void*)info1,(void*)temp_query,sizeof(struct Ph_API_info));
					
					struct Ph_API_info* info2;
					info2 = (struct Ph_API_info*)malloc(sizeof(struct Ph_API_info));
					memcpy((void*)info2,(void*)temp_query,sizeof(struct Ph_API_info));
		
					s1->type = 1;
/*					s1->send_query = temp_query;*/
					s1->send_query = info1;
				
					s2->type = 2;
/*					s2->send_query = temp_query;*/
					s2->send_query = info2;
					
					temp_query->send_remain = 2;
				
					pthread_t t1,t2;
					if((err = pthread_create(&t1,&attr,query_sender,(void *)s1))) {
						perror(strerror(err));		     
						exit(1);
					}
					if((err = pthread_create(&t2,&attr,query_sender,(void *)s2))) {
						perror(strerror(err));		     
						exit(1);
					}
					//query_sender((void *)s1);
					//query_sender((void *)s2);
					
/*					writeTimeToLog(8,temp_query->pmuid1,(temp_query->captured).soc,(temp_query->captured).fracsec);*/
/*					writeTimeToLog(8,temp_query->pmuid2,(temp_query->captured).soc,(temp_query->captured).fracsec);*/
				
/*					pthread_mutex_unlock(&mutex_on_thread);*/
				
/*					while(temp_query->send_remain > 0);*/
				}
				
				pthread_mutex_lock(&mutex_PhAPI_info);
				temp_query->model_status = 0;
				pthread_mutex_unlock(&mutex_PhAPI_info);
/*			exit(0);*/
				break;
			}
			
			struct timeval tm;
			pthread_mutex_lock(&mutex_App6Analysis);
			gettimeofday(&tm, NULL);
			modelcreationtime_sum += tm.tv_usec - temp_query->modelstart_fsec;
			modelcreate_count++;
			if(flagapp6 == 0 && modelcreate_count > 1200){
				float avgtime = modelcreationtime_sum*1.0/modelcreate_count;
				float avglat = lpdctospdc_latencysum*1.0/lpdctospdc_pktcount;
				
/*				printf("modelcount = %d, processtime_sum = %ld, Avg Processing time = %f\n",modelcreate_count,modelcraetiontime_sum,avgtime);*/
/*				printf("pktcount = %d, latency_sum = %ld, Avg latency = %f\n",lpdctospdc_pktcount,lpdctospdc_latencysum,avglat);*/
/*			*/
/*				printf("total_globle_violation = %ld\n",total_globle_violation);*/
				printf("App6 : PT [%f], LT [%f], GV [%ld]\n",avgtime,avglat,total_globle_violation);
				flagapp6 = 1;
			}
			pthread_mutex_unlock(&mutex_App6Analysis);
			
			
		}else{
			//printf("Model creation already in Progess...\n");
/*			pthread_mutex_unlock(&mutex_on_thread);*/
		}
		
	}else{
		printf("Wrong API\n");
/*		pthread_mutex_unlock(&mutex_on_thread);*/
	}
}



