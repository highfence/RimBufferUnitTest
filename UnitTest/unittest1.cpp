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
		
		// �������� �ֽ� �⺻ �׽�Ʈ �޼ҵ�.
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

		// ��� �Լ��� �⺻������ �׽�Ʈ �غ�.
		TEST_METHOD(Basic_Function_Test)
		{
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

		// Append �Լ��� RimBuffer�� �����̼��� ���� ���� ���¿��� ����� �����ϴ��� �׽�Ʈ.
		TEST_METHOD(Append_Basic_Success)
		{
			char inputData[30] = { 0, };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			/* �׽�Ʈ�� ���� ���� ������� ����. */
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* �׽�Ʈ ���� ������ �̳����� ù ��° Append ����. */
			const char* firstInputData = "test";
			const uint64_t firstInputDataSize = strlen(firstInputData);
			const uint64_t firstFreeSpaceSize = rimBuffer.GetCapacity() - firstInputDataSize;
			memcpy(inputData, (void*)firstInputData, firstInputDataSize);

			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Append(firstInputDataSize, inputData));
			Assert::AreEqual((uint64_t)0, rimBuffer.GetCurHeadPos());
			Assert::AreEqual(firstInputDataSize, rimBuffer.GetCurTailPos());
			Assert::AreEqual(firstFreeSpaceSize, rimBuffer.GetLinearFreeSpace());
			Assert::AreEqual(firstFreeSpaceSize, rimBuffer.GetTotalFreeSpace());

			/* �� ��° Append ����. */
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

		// Append �Լ��� RimBuffer�� �����̼��� ���� ���� ���¿��� ����� ���и� ����� �׽�Ʈ.
		TEST_METHOD(Append_Basic_Fail)
		{
			char inputData[30] = { 0, };
			const int testBufferSize = 15;

			CumBuffer rimBuffer;
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == rimBuffer.Init(testBufferSize));

			/* ���� �� ���̺��� ū �����͸� ���� �ִ� ���. */
			const char* tooLargeData = "IsItTooLargeToAppend?";
			const uint64_t tooLargeDataSize = strlen(tooLargeData);
			memcpy(inputData, (void*)tooLargeData, tooLargeDataSize);

			Assert::IsTrue(rimBuffer.GetCapacity() < tooLargeDataSize);
			auto opRet = rimBuffer.Append(tooLargeDataSize, tooLargeData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_INVALID_LEN == opRet);

			/* ���� �� ���� ��ŭ ������ ����. */
			const char* fitData = "ThisDataSize15!";
			const int fitDataSize = strlen(fitData);
			Assert::IsTrue(testBufferSize == fitDataSize);

			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)fitData, fitDataSize);
			opRet = rimBuffer.Append(fitDataSize, fitData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_OK == opRet);

			/* ���Խ� �����Ͱ� �� ���ִ� ���. */
			const char* overData = "BufferOver";
			const int overDataSize = strlen(overData);
			
			memset(inputData, 0x00, sizeof(inputData));
			memcpy(inputData, (void*)overData, overDataSize);
			opRet = rimBuffer.Append(overDataSize, overData);
			Assert::IsTrue(OP_RESULT::OP_RSLT_BUFFER_FULL== opRet);
		}

		// Append �Լ��� �����̼��� �� ���¿��� ����� �����ϴ��� �׽�Ʈ.
		TEST_METHOD(Append_Beyond_Rotate_Success)
		{

		}

		// Append �Լ��� �����̼��� �� ���¿��� ����� ���и� ���� �׽�Ʈ.
		TEST_METHOD(Append_Beyond_Rotate_Fail)
		{

		}

		// GetData �Լ��� ���ϴ� �����͸� ����� ������ �׽�Ʈ.
		TEST_METHOD(GetData_Basic_Success)
		{

		}

		// GetData �Լ��� ����� ���� �ڵ带 ������ �׽�Ʈ.
		TEST_METHOD(GetData_Basic_Fail)
		{

		}

		// GetData �Լ��� �����̼� ���� ����� �����ϴ��� �׽�Ʈ.
		TEST_METHOD(GetData_Beyond_Rotate_Success)
		{

		}

		// GetData �Լ��� �����̼� ���� ����� ���� �ڵ带 ����� �׽�Ʈ.
		TEST_METHOD(GetData_Beyond_Rotate_Fail)
		{

		}
	};
}