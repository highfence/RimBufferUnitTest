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

		// TODO :: 테스트 함수_조건으로 테스트 메소드 이름을 만들고, 많이 분할하여 테스트한다.
		TEST_METHOD(TestMethod2)
		{
			// 내가 만드는 테스트 메소드.
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
	};
}