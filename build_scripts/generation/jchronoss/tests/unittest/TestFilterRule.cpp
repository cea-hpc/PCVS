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

#include <svUnitTest/svUnitTest.h>
#include <utils.h>
#include <FilterRule.h>

using namespace std;
using namespace svUnitTest;

class TestFilterRule : public svutTestCase {
	public:
		void testMethodsRegistration(void);
		virtual void setUp(void);
		virtual void tearDown(void);
	protected:
		void testGetName(void);
		void testSetName(void);
		void testOperatorRead(void);
		
		FilterRule* mainFilterRule;
		FilterRule* secondFilterRule;
};

void TestFilterRule::testMethodsRegistration(void){
	setTestCaseName("TestFilterRule");
	SVUT_REG_TEST_METHOD(testGetName);
	SVUT_REG_TEST_METHOD(testSetName);
	SVUT_REG_TEST_METHOD(testOperatorRead);
}

void TestFilterRule::setUp(void){
	mainFilterRule = new FilterRule("SuperFilter");
	secondFilterRule = new FilterRule;
}

void TestFilterRule::tearDown(void){
	delete mainFilterRule;
	delete secondFilterRule;
}

void TestFilterRule::testGetName(void){
	SVUT_ASSERT_EQUAL(mainFilterRule->getName(), "SuperFilter");
}

void TestFilterRule::testSetName(void){
	mainFilterRule->setName("AnAnotherSuperFilter");
	SVUT_ASSERT_EQUAL(mainFilterRule->getName(), "AnAnotherSuperFilter");
}

void TestFilterRule::testOperatorRead ( void ) {
	ifstream file("../../../tests/unittest/FilterRes.txt");
	SVUT_ASSERT_TRUE(file);
	file >> (*mainFilterRule) >> (*secondFilterRule);
	
	SVUT_ASSERT_EQUAL(mainFilterRule->getName(), "a.b.c");
	SVUT_ASSERT_EQUAL(secondFilterRule->getName(), "d1.d2.testname3");
	file.close();
}


SVUT_REGISTER_STANDELONE(TestFilterRule)
