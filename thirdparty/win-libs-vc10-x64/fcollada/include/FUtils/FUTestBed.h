/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_TESTBED_H_
#define _FU_TESTBED_H_

#ifndef _FU_LOG_FILE_H_
#include "FUtils/FULogFile.h"
#endif // _FU_LOG_FILE_H_
#ifndef _FU_ASSERT_H_
#include "FUtils/FUAssert.h"
#endif // _FU_ASSERT_H_

class FUTestSuite;

/**
	A test suite runner.
	This class is used to run test suites and compile the results.
	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUTestBed
{
private:
	size_t testPassed, testFailed;
	FULogFile fileOut;
	fstring filename;
	bool isVerbose;

public:
	/** Constructor.
		@param filename The filename of the logfile to open.
		@param isVerbose Whether to enable more logging.
			Currently, this means writing one line to the log when
			starting any test suite starts. */
	FUTestBed(const fchar* filename, bool isVerbose);

	/** Retrieves whether verbose logging is enabled.
		@return Whether verbose logging is enabled. */
	inline bool IsVerbose() const { return isVerbose; }
	
	/** [INTERNAL] Retrieves the log file.
		Used by test suites to write out custom output.
		@return The log file structure. */
	FULogFile& GetLogFile() { return fileOut; }

	/** Runs a test suite and returns the results.
		@param headTestSuite The top-level test suite to start.
		@return Whether all the tests have passed. */
	bool RunTestbed(FUTestSuite* headTestSuite);
	
	/** [INTERNAL] Runs a test suite and compiles the results.
		This is used recursively to run a full hierarchy of test suites.
		@param testSuite A test suite to run. */
	void RunTestSuite(FUTestSuite* testSuite);
};

extern FCOLLADA_EXPORT bool FUTestBed_skipAsserts;

/**
	A test suite.
	This class is derived by all the test suites and provides the interface
	for running test suites and setting up results.
	@ingroup FUtils
*/
class FUTestSuite
{
public:
	/** Destructor. */
	virtual ~FUTestSuite() {}
		
	/** [INTERNAL] Runs one test.
		Called by the FUTestBed.
		@param testBed The test bed compiling the results.
		@param fileOut The log file of the test bed.
		@param __testSuiteDone Out parameter: whether the test suite has no more tests after this one.
		@param testIndex An increasing index used to run all the tests.
		@return Whether the current test has passed. */	
	virtual bool RunTest(FUTestBed& testBed, FULogFile& fileOut, bool& __testSuiteDone, size_t testIndex) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Helpful Macros
//
#if defined(_FU_ASSERT_H_) && defined(_DEBUG)
#define FailIf(a) \
	if ((a)) { \
		fileOut.WriteLine(__FILE__, __LINE__, " Test('%s') failed: %s.", szTestName, #a); \
		if (!FUTestBed_skipAsserts) { \
			FUBreak; \
			FUTestBed_skipAsserts = ignoreAssert; } \
		return false; }

#else // _FU_ASSERT_H_ && _DEBUG

#define FailIf(a) \
	if ((a)) { \
		fileOut.WriteLine(__FILE__, __LINE__, " Test('%s') failed: %s.", szTestName, #a); \
		return false; }

#endif // _FU_ASSERT_H_

#define PassIf(a) FailIf(!(a))

#define Fail { bool b = true; FailIf(b); }

///////////////////////////////////////////////////////////////////////////////
// TestSuite Generation Macros.
// Do use the following macros, instead of writing your own.
//
#ifdef ENABLE_TEST
#define TESTSUITE_START(suiteName) \
FUTestSuite* _test##suiteName; \
static class FUTestSuite##suiteName : public FUTestSuite \
{ \
public: \
	FUTestSuite##suiteName() : FUTestSuite() { _test##suiteName = this; } \
	virtual ~FUTestSuite##suiteName() {} \
	virtual bool RunTest(FUTestBed& testBed, FULogFile& fileOut, bool& __testSuiteDone, size_t testIndex) \
	{ \
		switch (testIndex) { \
			case ~0 : { \
				if (testBed.IsVerbose()) { \
					fileOut.WriteLine("Running %s...", #suiteName); \
				}

#define TESTSUITE_TEST(testIndex, testName) \
			} return true; \
		case testIndex: { \
			static const char* szTestName = #testName;

#define TESTSUITE_END \
			} return true; \
		default: { __testSuiteDone = true; return true; } \
		} \
		fileOut.WriteLine(__FILE__, __LINE__, " Test suite implementation error."); \
		return false; \
	} \
} __testSuite;

#define RUN_TESTSUITE(suiteName) \
	extern FUTestSuite* _test##suiteName; \
	testBed.RunTestSuite(_test##suiteName);

#define RUN_TESTBED(testBedObject, szTestSuiteHead, testPassed) { \
	extern FUTestSuite* _test##szTestSuiteHead; \
	testPassed = testBedObject.RunTestbed(_test##szTestSuiteHead); }

#else // ENABLE_TEST

// The code will still be compiled, but the linker should take it out.
#define TESTSUITE_START(suiteName) \
	extern FUTestSuite* _test##suiteName = NULL; \
	inline bool __testCode##suiteName(FULogFile& fileOut, const char* szTestName) { { fileOut; szTestName;
#define TESTSUITE_TEST(testIndex, testName) } {
#define TESTSUITE_END } return true; }
#define RUN_TESTSUITE(suiteName)
#define RUN_TESTBED(testBedObject, szTestSuiteHead, testPassed) testPassed = true;

#endif

#endif // _FU_TESTBED_H_
