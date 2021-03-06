/****************************************************************************/
/*                                                                          */
/*                         Copyright or (C) or Copr.                        */
/*       Commissariat a l'Energie Atomique et aux Energies Alternatives     */
/*                                                                          */
/* Version : 1.2                                                            */
/* Date    : Tue Jul 22 13:28:10 CEST 2014                                  */
/* Ref ID  : IDDN.FR.001.160040.000.S.P.2015.000.10800                      */
/* Author  : Julien Adam <julien.adam@cea.fr>                               */
/*           Marc Perache <marc.perache@cea.fr>                             */
/*                                                                          */
/* This file is part of JCHRONOSS software.                                 */
/*                                                                          */
/* This software is governed by the CeCILL-C license under French law and   */
/* abiding by the rules of distribution of free software.  You can  use,    */
/* modify and/or redistribute the software under the terms of the CeCILL-C  */
/* license as circulated by CEA, CNRS and INRIA at the following URL        */
/* "http://www.cecill.info".                                                */
/*                                                                          */
/* As a counterpart to the access to the source code and  rights to copy,   */
/* modify and redistribute granted by the license, users are provided only  */
/* with a limited warranty  and the software's author,  the holder of the   */
/* economic rights,  and the successive licensors  have only  limited       */
/* liability.                                                               */
/*                                                                          */
/* In this respect, the user's attention is drawn to the risks associated   */
/* with loading,  using,  modifying and/or developing or reproducing the    */
/* software by the user in light of its specific status of free software,   */
/* that may mean  that it is complicated to manipulate,  and  that  also    */
/* therefore means  that it is reserved for developers  and  experienced    */
/* professionals having in-depth computer knowledge. Users are therefore    */
/* encouraged to load and test the software's suitability as regards their  */
/* requirements in conditions enabling the security of their systems and/or */
/* data to be ensured and,  more generally, to use and operate it in the    */
/* same conditions as regards security.                                     */
/*                                                                          */
/* The fact that you are presently reading this means that you have had     */
/* knowledge of the CeCILL-C license and that you accept its terms.         */
/*                                                                          */
/****************************************************************************/


#include "Runner.h"

Runner::Runner() {

}
Runner::Runner ( JobManager* job, Configuration* config ) {
	
	jobMan = job;
	this->config = config;
	switch(config->system().getPolicy()){
		case POLICY_DEFAULT:
			policy = new DefaultPolicy(job, config);
			break;
		case POLICY_RES:
			policy = new SchedResPolicy(job, config);
			break;
		case POLICY_TIME:
			policy = new SchedTimePolicy(job, config);
			break;
		case POLICY_CUSTOM:
			policy = new CustomPolicy(job, config);
			break;
	}
}

Runner::~Runner() {
	safeFree(policy);
}

void Runner::fillWorker ( Worker * cur, size_t nbResources ) {
	assert(cur != NULL);
	assert(nbResources > 0);
	UNUSED(cur);
	UNUSED(nbResources);
}

void Runner::launchWorker ( Worker* cur ) {
	assert(cur != NULL);
	UNUSED(cur);
}

void Runner::postActions(Worker* cur) {
	assert(cur != NULL);
	UNUSED(cur);
}

void Runner::preActions() {
}

Worker* Runner::waitNextWorker() {
	return NULL;
}

void Runner::pullJobsFromFiles()
{
	jobMan->pullJobsFromFiles();
}

void Runner::pushJobsIntoFiles()
{

}

void Runner::startCompilation()
{

}


