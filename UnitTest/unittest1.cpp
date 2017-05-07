#include "stdafx.h"
#include "CppUnitTest.h"
#include "CumBuffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

const uint64_t UINT64_ZERO = 0;

namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public :
		
		// 교수님이 주신 기본 테스트 메소드.
		TEST_METHOD(TestMethod1)
		{
			char data[100] = { 0, };
			char dataOut[100] = { 0, };

			CumBuffer buffering;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == buffering.Init()); //default buffer length
			Assert::AreEqual(buffering.GetCurHeadPos(), UINT64_ZERO);
			Assert::AreEqual(buffering.GetCurTailPos(), UINT64_ZERO);
			Assert::IsTrue(OP_RESULT::OP_RSLT_NO_DATA == buffering.GetData(3, dataOut));

			memset(data, 0x00, sizeof(data));
			memcpy(data, (void*)"aaa", 3);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == buffering.Append(3, data));
			Assert::AreEqual(buffering.GetCurTailPos(), (uint64_t)3);

			memset(data, 0x00, sizeof(data));
			memcpy(data, (void*)"bbb", 3);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == buffering.Append(3, data));
			Assert::AreEqual(buffering.GetCurTailPos(), (uint64_t)6);
			Assert::AreEqual(buffering.GetCumulatedLen(), (uint64_t)6);

			memset(dataOut, 0x00, sizeof(dataOut));
			Assert::IsTrue(OP_RESULT::OP_RSLT_INVALID_LEN == buffering.GetData(7, dataOut));
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == buffering.GetData(6, dataOut));
			Assert::AreEqual("aaabbb", dataOut);
			Assert::AreEqual(buffering.GetCurHeadPos(), (uint64_t)6);
			Assert::AreEqual(buffering.GetCurTailPos(), (uint64_t)6);
		}

		// 멤버 함수를 기본적으로 테스트 해봄.
		TEST_METHOD(Basic_Function_Test)
		{
			/* 초기 세팅. */
			CumBuffer testBuf;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.Init());
			
			// GetCumulateLen 테스트.
			Assert::AreEqual(UINT64_ZERO, testBuf.GetCumulatedLen());

			// GetCapacity 테스트. 
			Assert::AreEqual((uint64_t)DEFAULT_BUFFER_LEN, testBuf.GetCapacity());

			// GetTotalFreeSpace 테스트.
			Assert::AreEqual((uint64_t)DEFAULT_BUFFER_LEN, testBuf.GetTotalFreeSpace());

			// Append 후 테스트를 하기 위한 준비.
			char inputData[100] = { 0 };
			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)"test", 4);

			// 4바이트 집어넣음.
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.Append(4,inputData));

			// GetCumulateLen이 4를 뱉는지 테스트.
			Assert::AreEqual((uint64_t)4, testBuf.GetCumulatedLen());

			// GetTotalFreeSpace가 DEFAULT_BUFFER_LEN - 4를 뱉는지 테스트.
			Assert::AreEqual((uint64_t)(DEFAULT_BUFFER_LEN - 4), testBuf.GetTotalFreeSpace());

			// 현재 CurTailPos가 4만큼 왔는지 테스트.
			Assert::AreEqual((uint64_t)4, testBuf.GetCurTailPos());

			// GetLinearFreeSpace 테스트.
			Assert::AreEqual((uint64_t)(DEFAULT_BUFFER_LEN - 4), testBuf.GetLinearFreeSpace());

			// Data꺼내오는 테스트를 위한 준비.
			char outputData[100] = { 0 };
			memset(outputData, 0x00, sizeof(outputData));

			// GetData 테스트.
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.GetData(4, outputData));
			Assert::AreEqual(outputData, "test");
		}

		// Append 함수가 RimBuffer의 로테이션을 하지 않은 상태에서 제대로 성공하는지 테스트.
		TEST_METHOD(Append_Basic_Success)
		{
			/* 초기 세팅. */
			char inputData[30] = { 0 };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			/* 테스트용 작은 버퍼 사이즈로 생성. */
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 테스트 버퍼 사이즈 이내에서 첫 번째 Append 수행. */
			const char* firstInputData = "test";
			const uint64_t firstInputDataSize = strlen(firstInputData);
			const uint64_t firstFreeSpaceSize = rimBuffer.GetCapacity() - firstInputDataSize;
			memcpy(inputData, (void*)firstInputData, firstInputDataSize);

			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Append(firstInputDataSize, inputData));
			Assert::AreEqual((uint64_t)0, rimBuffer.GetCurHeadPos());
			Assert::AreEqual(firstInputDataSize, rimBuffer.GetCurTailPos());
			Assert::AreEqual(firstFreeSpaceSize, rimBuffer.GetLinearFreeSpace());
			Assert::AreEqual(firstFreeSpaceSize, rimBuffer.GetTotalFreeSpace());

			/* 두 번째 Append 수행. */
			const char* secondInputData = "Another";
			const uint64_t secondInputDataSize = strlen(secondInputData);
			const uint64_t secondFreeSpaceSize = firstFreeSpaceSize - secondInputDataSize;
			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)secondInputData, secondInputDataSize);

			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Append(secondInputDataSize, secondInputData));
			Assert::AreEqual((uint64_t)0, rimBuffer.GetCurHeadPos());
			Assert::AreEqual(firstInputDataSize + secondInputDataSize, rimBuffer.GetCurTailPos());
			Assert::AreEqual(secondFreeSpaceSize, rimBuffer.GetLinearFreeSpace());
			Assert::AreEqual(secondFreeSpaceSize, rimBuffer.GetTotalFreeSpace());
		}

		// Append 함수가 RimBuffer의 로테이션을 하지 않은 상태에서 제대로 실패를 뱉는지 테스트.
		TEST_METHOD(Append_Basic_Fail)
		{
			/* 초기 세팅. */
			char inputData[30] = { 0 };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 버퍼 총 길이보다 큰 데이터를 집어 넣는 경우. */
			const char* tooLargeData = "IsItTooLargeToAppend?";
			const uint64_t tooLargeDataSize = strlen(tooLargeData);
			memcpy(inputData, (void*)tooLargeData, tooLargeDataSize);

			Assert::IsTrue(rimBuffer.GetCapacity() < tooLargeDataSize);
			auto opRet = rimBuffer.Append(tooLargeDataSize, tooLargeData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_INVALID_LEN == opRet);

			/* 버퍼 총 길이 만큼 데이터 삽입. */
			const char* fitData = "ThisDataSize15!";
			const int fitDataSize = strlen(fitData);
			Assert::IsTrue(testBufferSize == fitDataSize);

			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)fitData, fitDataSize);
			opRet = rimBuffer.Append(fitDataSize, fitData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);

			/* 삽입시 데이터가 꽉 차있는 경우. */
			const char* overData = "BufferOver";
			const int overDataSize = strlen(overData);
			
			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)overData, overDataSize);
			opRet = rimBuffer.Append(overDataSize, overData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_BUFFER_FULL== opRet);
		}

		// GetData 함수가 원하는 데이터를 제대로 뱉어내는지 테스트.
		TEST_METHOD(GetData_Basic_Success)
		{
			/* 초기 세팅. */
			char inputData[30] = { 0 };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 빼내올 데이터 삽입. */
			const char* testData = "InputForTest";
			const int testDataSize = strlen(testData);
			auto opRet = rimBuffer.Append(testDataSize, testData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);

			/* 삽입한 데이터 중 "Input"을 GetData */
			const char* charInput = "Input";
			char outputData[30] = { 0 };
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.GetData(strlen(charInput), outputData));
			Assert::AreEqual(charInput, outputData);
			
			/* 삽입한 데이터 중 "For"를 GetData */
			const char* charFor = "For";
			memset(outputData, 0x00, sizeof(outputData));
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.GetData(strlen(charFor), outputData));
			Assert::AreEqual(charFor, outputData);

		}

		// GetData 함수가 제대로 실패 코드를 뱉어내는지 테스트.
		TEST_METHOD(GetData_Basic_Fail)
		{
			/* 초기 세팅. */
			char outputData[30] = { 0 };
			const int testBufferSize = 15;
			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 버퍼에 데이터가 없을 경우 실패 테스트.*/
			auto opRet = rimBuffer.GetData(1, outputData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_NO_DATA == opRet);

			/* 테스트를 위한 데이터 삽입. */
			const char* testData = "test";
			opRet = rimBuffer.Append(strlen(testData), testData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);

			/* GetData 인자 두 개가 모두 True인 경우 실패 테스트. */
			opRet = rimBuffer.GetData(1, outputData, true, true);
			Assert::IsTrue(OP_RESULT::OP_RSLT_INVALID_USAGE == opRet);

			/* 현재 있는 데이터보다 많은 데이터를 요구할 경우 실패 테스트. */
			opRet = rimBuffer.GetData(strlen(testData) + 1, outputData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_INVALID_LEN == opRet);
		}

		// GetData시 옵션을 Peek을 주었을 때 제대로 작동하는지 테스트.
		TEST_METHOD(GetData_TestPeek)
		{
			/* 초기 세팅. */
			char outputData[30] = { 0 };
			const int testBufferSize = 15;
			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 테스트를 위한 데이터 삽입. */
			const char* testData = "test";
			auto opRet = rimBuffer.Append(strlen(testData), testData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);

			/* 첫 번째 Peek 테스트. */
			opRet = rimBuffer.GetData(strlen(testData), outputData, true, false);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);
			Assert::AreEqual(outputData, testData);

			/* 두 번째 Peek 테스트. (첫 번째와 같아야 함.) */
			memset(outputData, 0x00, sizeof(outputData));
			opRet = rimBuffer.GetData(strlen(testData), outputData, true, false);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);
			Assert::AreEqual(outputData, testData);
		}

		// GetData시 옵션을 MoveHeadOnly를 주었을 때 제대로 작동하는지 테스트.
		TEST_METHOD(GetData_TestHeadMoveOnly)
		{
			/* 초기 세팅. */
			char emptyData[30] = { 0 };
			char outputData[30] = { 0 };
			const int testBufferSize = 15;
			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 테스트를 위한 데이터 삽입. */
			const char* testData = "testdata";
			auto opRet = rimBuffer.Append(strlen(testData), testData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);

			/* MoveHeadOnly 테스트. ("test"만 빼어보기.) */
			opRet = rimBuffer.GetData(strlen("test"), outputData, false, true);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);
			Assert::AreEqual(outputData, emptyData);

			/* 이후 Head가 제대로 이동했는지 테스트. ("Data"가 나와야함. ) */
			opRet = rimBuffer.GetData(strlen("Data"), outputData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);
			Assert::AreEqual(outputData, "Data");
		}

		// Append 함수가 로테이션한 뒤 상태에서 제대로 성공하는지 테스트.
		TEST_METHOD(Append_Beyond_Rotate_Success)
		{
			char inputData[30] = { 0, };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 로테이션 하도록 데이터를 넣고 뺴기. */
		}

		// Append 함수가 로테이션한 뒤 상태에서 제대로 실패를 뱉어내는 테스트.
		TEST_METHOD(Append_Beyond_Rotate_Fail)
		{

		}

		// GetData 함수가 로테이션 이후 제대로 성공하는지 테스트.
		TEST_METHOD(GetData_Beyond_Rotate_Success)
		{

		}

		// GetData 함수가 로테이션 이후 제대로 실패 코드를 뱉는지 테스트.
		TEST_METHOD(GetData_Beyond_Rotate_Fail)
		{

		}


	};
}