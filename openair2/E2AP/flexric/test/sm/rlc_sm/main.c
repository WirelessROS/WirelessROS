/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "../../rnd/fill_rnd_data_rlc.h"
#include "../../../src/sm/rlc_sm/rlc_sm_agent.h"
#include "../../../src/sm/rlc_sm/rlc_sm_ric.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static
rlc_ind_data_t cp;

/////
// AGENT
////

static
void read_rlc_ind(void* read)
{
  assert(read != NULL);

  rlc_ind_data_t* rlc = (rlc_ind_data_t*)read;

  fill_rlc_ind_data(rlc);
  cp.hdr = cp_rlc_ind_hdr(&rlc->hdr);
  cp.msg = cp_rlc_ind_msg(&rlc->msg);
}

/*
static 
sm_ag_if_ans_t write_RAN(sm_ag_if_wr_t const* data)
{
  assert(data != NULL);
  assert(0!=0 && "Not implemented");
  sm_ag_if_ans_t ans = {0};
  return ans;
}
*/

/////////////////////////////
// Check Functions
// //////////////////////////

static
void check_eq_ran_function(sm_agent_t const* ag, sm_ric_t const* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);
  assert(ag->ran_func_id == ric->ran_func_id);
}

// RIC -> E2
static
void check_subscription(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  char sub[] = "2_ms";
  sm_subs_data_t data = ric->proc.on_subscription(ric, &sub);
 
  subscribe_timer_t t = ag->proc.on_subscription(ag, &data); 
  assert(t.ms == 2); 

  free_sm_subs_data(&data);
}

// E2 -> RIC
static
void check_indication(sm_agent_t* ag, sm_ric_t* ric)
{
  assert(ag != NULL);
  assert(ric != NULL);

  sm_ind_data_t sm_data = ag->proc.on_indication(ag, NULL);
  if(sm_data.call_process_id != NULL){
    assert(sm_data.len_cpid != 0);
  }
  if(sm_data.ind_hdr != NULL){
    assert(sm_data.len_hdr != 0);
  }
  if(sm_data.ind_msg != NULL){
    assert(sm_data.len_msg != 0);
  }

 sm_ag_if_rd_ind_t msg = ric->proc.on_indication(ric, &sm_data);

  assert(msg.type == RLC_STATS_V0);

  rlc_ind_data_t* data = &msg.rlc;

 if(msg.rlc.msg.rb != NULL){
      assert(msg.rlc.msg.len != 0);
 } 

  assert(eq_rlc_ind_hdr(&data->hdr, &cp.hdr) == true);
  assert(eq_rlc_ind_msg(&data->msg, &cp.msg) == true);
  assert(eq_rlc_call_proc_id(data->proc_id, cp.proc_id) == true);

  free_rlc_ind_hdr(&data->hdr);
  free_rlc_ind_msg(&data->msg);

  free_sm_ind_data(&sm_data); 
}

int main()
{
  sm_io_ag_ran_t io_ag = {0};

  io_ag.read_ind_tbl[RLC_SUBS_V0] = read_rlc_ind; 

  sm_agent_t* sm_ag = make_rlc_sm_agent(io_ag);

  sm_ric_t* sm_ric = make_rlc_sm_ric();

  check_eq_ran_function(sm_ag, sm_ric);
  check_subscription(sm_ag, sm_ric);
  check_indication(sm_ag, sm_ric);

  sm_ag->free_sm(sm_ag);
  sm_ric->free_sm(sm_ric);

  printf("Success\n");
  return EXIT_SUCCESS;
}

