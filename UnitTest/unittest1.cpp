#include "stdafx.h"
#include "CppUnitTest.h"
#include "CumBuffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

const uint64_t UINT64_ZERO = 0;

namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
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
			CumBuffer testBuf;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.Init());
			
			// GetCumulateLen 테스트.
			Assert::AreEqual(UINT64_ZERO, testBuf.GetCumulatedLen());

			// GetCapacity 테스트. 
			Assert::AreEqual((uint64_t)DEFAULT_BUFFER_LEN, testBuf.GetCapacity());

			// GetTotalFreeSpace 테스트.
			Assert::AreEqual((uint64_t)DEFAULT_BUFFER_LEN, testBuf.GetTotalFreeSpace());

			// Append 후 테스트를 하기 위한 준비.
			char inputData[100] = { 0, };
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
			char outputData[100] = { 0, };
			memset(outputData, 0x00, sizeof(outputData));

			// GetData 테스트.
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.GetData(4, outputData));
			Assert::AreEqual(outputData, "test");
		}

		// Append 함수가 RimBuffer의 로테이션을 하지 않은 상태에서 제대로 성공하는지 테스트.
		TEST_METHOD(Append_Basic_Success)
		{
			char inputData[30] = { 0, };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			/* 테스트용 작은 버퍼 사이즈로 생성. */
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* 테스트 버퍼 사이즈 이내에서 Append 수행. */
			const uint64_t inputDataSize = 4;
			const uint64_t freeSpaceSize = rimBuffer.GetCapacity() - inputDataSize;
			memcpy(inputData, (void*)"test", inputDataSize);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Append(4, inputData));
			Assert::AreEqual((uint64_t)0, rimBuffer.GetCurHeadPos());
			Assert::AreEqual(inputDataSize, rimBuffer.GetCurTailPos());
			Assert::AreEqual(freeSpaceSize, rimBuffer.GetLinearFreeSpace());
			Assert::AreEqual(freeSpaceSize, rimBuffer.GetTotalFreeSpace());

		}

		// Append 함수가 RimBuffer의 로테이션을 하지 않은 상태에서 제대로 실패를 뱉는지 테스트.
		TEST_METHOD(Append_Basic_Fail)
		{
			
		}

		// Append 함수가 로테이션한 뒤 상태에서 제대로 성공하는지 테스트.
		TEST_METHOD(Append_Beyond_Rotate_Success)
		{

		}

		// Append 함수가 로테이션한 뒤 상태에서 제대로 실패를 뱉어내는 테스트.
		TEST_METHOD(Append_Beyond_Rotate_Fail)
		{

		}

		// GetData 함수가 원하는 데이터를 제대로 뱉어내는지 테스트.
		TEST_METHOD(GetData_Basic_Success)
		{

		}

		// GetData 함수가 제대로 실패 코드를 뱉어내는지 테스트.
		TEST_METHOD(GetData_Basic_Fail)
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