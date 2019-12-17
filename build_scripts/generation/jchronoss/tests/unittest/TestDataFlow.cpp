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
#include <DataFlow.h>

using namespace std;
using namespace svUnitTest;

class TestDataFlow : public svutTestCase {
	public:
		void testMethodsRegistration(void);
		virtual void setUp(void);
		virtual void tearDown(void);
		DataFlow getDataFlowObject() {return *mainDataFlow;}
	
	protected:
		void testSimplePush(void);
		void testMediumPush(void);
		void testComplexPush(void);
		void testCreateHash(void);
		void testMatchHash(void);
		
		DataFlow* mainDataFlow;
		FILE* shortFile;
		FILE* mediumFile;
		FILE* longFile;
};

void TestDataFlow::testMethodsRegistration(void){
	setTestCaseName("TestDataFlow");
	SVUT_REG_TEST_METHOD(testSimplePush);
	SVUT_REG_TEST_METHOD(testMediumPush);
	SVUT_REG_TEST_METHOD(testComplexPush);
	SVUT_REG_TEST_METHOD(testCreateHash);
	SVUT_REG_TEST_METHOD(testMatchHash);
}

void TestDataFlow::setUp(void)	{
	safeOpenRead(shortFile, "../../../tests/unittest/DataFlowFile.txt"); fseek(shortFile,3856 , SEEK_CUR);
	safeOpenRead(mediumFile, "../../../tests/unittest/DataFlowFile.txt");fseek(mediumFile,3060 , SEEK_CUR);
	safeOpenRead(longFile, "../../../tests/unittest/DataFlowFile.txt");
	mainDataFlow = new DataFlow(1024);
}

void TestDataFlow::tearDown(void){
	delete mainDataFlow;
	safeClose(shortFile);
	safeClose(mediumFile);
	safeClose(longFile);
}

void TestDataFlow::testSimplePush ( void ) {
	SVUT_ASSERT_EQUAL(mainDataFlow->fillFrom(shortFile), (size_t)478);
}

void TestDataFlow::testMediumPush( void ) {
	SVUT_ASSERT_EQUAL(mainDataFlow->fillFrom(mediumFile), (size_t)1274);
}

void TestDataFlow::testComplexPush( void ) {
	SVUT_ASSERT_EQUAL(mainDataFlow->fillFrom(longFile), (size_t)(2*(1024)));
}

void TestDataFlow::testCreateHash ( void ) {
	mainDataFlow->fillFrom(shortFile);
	size_t ref = hash_fn(mainDataFlow->getContent());
	SVUT_ASSERT_EQUAL(mainDataFlow->createHash(), ref);
}

void TestDataFlow::testMatchHash ( void ) {
	mainDataFlow->fillFrom(longFile);
	size_t ref = hash_fn(mainDataFlow->getContent());
	SVUT_ASSERT_EQUAL(mainDataFlow->matchHash(ref), true);
}

SVUT_REGISTER_STANDELONE(TestDataFlow)
