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

		// TODO :: �׽�Ʈ �Լ�_�������� �׽�Ʈ �޼ҵ� �̸��� �����, ���� �����Ͽ� �׽�Ʈ�Ѵ�.
		TEST_METHOD(TestMethod2)
		{
			// ���� ����� �׽�Ʈ �޼ҵ�.
			CumBuffer testBuf;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.Init());
			
			// GetCumulateLen �׽�Ʈ.
			Assert::AreEqual(UINT64_ZERO, testBuf.GetCumulatedLen());

			// GetCapacity �׽�Ʈ. 
			Assert::AreEqual((uint64_t)DEFAULT_BUFFER_LEN, testBuf.GetCapacity());

			// GetTotalFreeSpace �׽�Ʈ.
			Assert::AreEqual((uint64_t)DEFAULT_BUFFER_LEN, testBuf.GetTotalFreeSpace());

			// Append �� �׽�Ʈ�� �ϱ� ���� �غ�.
			char inputData[100] = { 0, };
			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)"test", 4);

			// 4����Ʈ �������.
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.Append(4,inputData));

			// GetCumulateLen�� 4�� ����� �׽�Ʈ.
			Assert::AreEqual((uint64_t)4, testBuf.GetCumulatedLen());

			// GetTotalFreeSpace�� DEFAULT_BUFFER_LEN - 4�� ����� �׽�Ʈ.
			Assert::AreEqual((uint64_t)(DEFAULT_BUFFER_LEN - 4), testBuf.GetTotalFreeSpace());

			// ���� CurTailPos�� 4��ŭ �Դ��� �׽�Ʈ.
			Assert::AreEqual((uint64_t)4, testBuf.GetCurTailPos());

			// GetLinearFreeSpace �׽�Ʈ.
			Assert::AreEqual((uint64_t)(DEFAULT_BUFFER_LEN - 4), testBuf.GetLinearFreeSpace());

			// Data�������� �׽�Ʈ�� ���� �غ�.
			char outputData[100] = { 0, };
			memset(outputData, 0x00, sizeof(outputData));

			// GetData �׽�Ʈ.
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == testBuf.GetData(4, outputData));
			Assert::AreEqual(outputData, "test");
		}
	};
}