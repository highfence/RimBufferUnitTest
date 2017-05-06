#ifndef __CUMBUFFER_HPP__
#define __CUMBUFFER_HPP__
/****************************************************************************
 Copyright (c) 2016, ko jung hyun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
// https://github.com/jeremyko/CumBuffer

// NO THREAD SAFETY HERE
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <string.h>
#include <stdint.h>
#include <cstdlib>
#include <memory>

//#define CUMBUFFER_DEBUG

const int DEFAULT_BUFFER_LEN = 1024 * 4;
const int CACHE_LINE_SIZE = 64;

enum class OP_RESULT : int
{
    OP_RSLT_OK = 0,
    OP_RSLT_NO_DATA,
    OP_RSLT_BUFFER_FULL,
    OP_RSLT_ALLOC_FAILED,
    OP_RSLT_INVALID_LEN,
    OP_RSLT_INVALID_USAGE
};

#define CACHE_ALIGN __declspec(align(CACHE_LINE_SIZE))  

///////////////////////////////////////////////////////////////////////////////
class CACHE_ALIGN CumBuffer
{	
  public:
    CumBuffer() 
    {
        m_pBuffer = nullptr; 
		m_CumulatedLen = 0;
		m_CurHead = 0;
		m_CurTail = 0;
		m_BufferLen = 0;
    }

    virtual ~CumBuffer() 
	{ 
	};

	CumBuffer(const CumBuffer&) = delete;
	CumBuffer& operator = (const CumBuffer &) = delete;

    //------------------------------------------------------------------------
	OP_RESULT Init(int nMaxBufferLen = DEFAULT_BUFFER_LEN)
    {
        m_BufferLen = nMaxBufferLen;

        try
        {
			m_pBuffer = std::make_unique<char[]>(m_BufferLen);
        }
        catch (std::exception& e)
        {
            std::cerr <<"["<< __func__ <<"-"<<__LINE__ <<"] alloc failed :"<<e.what()  <<"\n"; 
            m_strErrMsg="alloc failed :";
            m_strErrMsg+= e.what();
            return OP_RESULT::OP_RSLT_ALLOC_FAILED;
        }

        return OP_RESULT::OP_RSLT_OK;
    }

    // Append계열 함수-------------------------------------------------------
    OP_RESULT Append(const size_t nLen, const char* pData)
    {
#ifdef CUMBUFFER_DEBUG
        std::cout <<"["<< __func__ <<"-"<<__LINE__ <<"] nLen="<<nLen<< "["<< pData<<"]\n";  
        DebugPos(__LINE__);
#endif
        OP_RESULT opRet = CheckAppendDataBufferLength(nLen);

		if (opRet != OP_RESULT::OP_RSLT_OK)
		{
			return opRet;
		}

        if(m_CurTail < m_CurHead)
        {
            // tail이 버퍼 끝을 지난 경우
			opRet = CheckAppendDataSpaceAfterRotation(pData, nLen);

			if (opRet == OP_RESULT::OP_RSLT_OK)
			{
				AppendDataAfterRotaion(pData, nLen);
			}

			return opRet;
        }
        else
        {
            if (m_BufferLen < m_CurTail + nLen)  //tail 이후, 남은 버퍼로 모자라는 경우
            {
				opRet = AppendDataWhenRotate(pData, nLen);

				if (opRet != OP_RESULT::OP_RSLT_OK) 
				{
					return opRet;
				}               
            }
            else
            {
                //most general case
                memcpy(m_pBuffer.get() + m_CurTail , pData, nLen); 
                m_CurTail += nLen;
                m_CumulatedLen += nLen;
#ifdef CUMBUFFER_DEBUG
                DebugPos(__LINE__);
#endif
                return OP_RESULT::OP_RSLT_OK;
            }
        }

        return OP_RESULT::OP_RSLT_OK;
    }

	// 추가할 데이터가 버퍼 총 길이보다 크거나, 현재 데이터가 꽉 찼는지 검사하는 함수.
	OP_RESULT CheckAppendDataBufferLength(size_t appendLength)
	{
		if (m_BufferLen < appendLength)
		{
			std::cerr << "[" << __func__ << "-" << __LINE__ << "] invalid len :" << appendLength << "\n";
			m_strErrMsg = "invalid length";
			return OP_RESULT::OP_RSLT_INVALID_LEN;
		}
		else if (m_BufferLen == m_CumulatedLen)
		{
			std::cerr << "[" << __func__ << "-" << __LINE__ << "] buffer full" << "\n";
			m_strErrMsg = "buffer full";
			return OP_RESULT::OP_RSLT_BUFFER_FULL;
		}

		return OP_RESULT::OP_RSLT_OK;
	}

	// 림 버퍼가 한바퀴 돌고 난 후, (헤드가 테일보다 뒤에 있을 경우) 추가할 데이터만큼 공간이 남았는지 검사하는 함수.
	OP_RESULT CheckAppendDataSpaceAfterRotation(const char* pAppendData, const size_t appendLength)
	{
		const auto space = m_CurHead - m_CurTail;

		if (space < appendLength)
		{
			std::cerr << "[" << __func__ << "-" << __LINE__ << "] buffer full" << "\n";
			m_strErrMsg = "buffer full";
			return OP_RESULT::OP_RSLT_BUFFER_FULL;
		}

#ifdef CUMBUFFER_DEBUG
			DebugPos(__LINE__);
#endif
		return OP_RESULT::OP_RSLT_OK;		
	}

	// 추가할 때, 로테이트해야할 경우를 계산해주는 함수.
	OP_RESULT AppendDataWhenRotate(const char* pAppendData, const size_t appendLength)
	{
		// 로테이션 한 뒤 필요한 공간 계산.
		const size_t spaceRequiredAfterRotation = appendLength - (m_BufferLen - m_CurTail);

		//2번 나누어서 들어갈 공간이 있는 경우
		if (m_CurTail > 0 && spaceRequiredAfterRotation <= m_CurHead)
		{

#ifdef CUMBUFFER_DEBUG
			DebugPos(__LINE__);
#endif
			int nFirstBlockLen = static_cast<int>(m_BufferLen - m_CurTail);
			int nSecondBlockLen = static_cast<int>(appendLength - nFirstBlockLen);

#ifdef CUMBUFFER_DEBUG
			std::cout << "[" << __func__ << "-" << __LINE__ << "] nFirstBlockLen =" << nFirstBlockLen
				<< "/nSecondBlockLen=" << nSecondBlockLen << "\n";
#endif
			AppendDataBeyondBufferEnd(pAppendData, appendLength, nFirstBlockLen, nSecondBlockLen);
		
#ifdef CUMBUFFER_DEBUG
			DebugPos(__LINE__);
#endif
			return OP_RESULT::OP_RSLT_OK;
		}
		
		std::cerr << "[" << __func__ << "-" << __LINE__ << "] buffer full" << "\n";
		m_strErrMsg = "buffer full";
		return OP_RESULT::OP_RSLT_BUFFER_FULL;
	}

    //------------------------------------------------------------------------
	// 데이터는 가져오지만 head를 이동 시키지 않는다
    OP_RESULT PeekData(size_t nLen, char* pDataOut)
    {
        return GetData(nLen, pDataOut, true, false);
    }

    //------------------------------------------------------------------------
	// 데이터는 가져오지 않고 head만 이동 시킨다
    OP_RESULT ConsumeData(size_t nLen)
    {
        //PeekData 사용해서 처리한 data length 만큼 버퍼내 nCurHead_ 를 이동.
        return GetData(nLen, NULL, false, true);
    }

    //------------------------------------------------------------------------
	OP_RESULT GetData(size_t  nLen,
		char*   pDataOut,
		bool    bPeek = false,
		bool    bMoveHeadOnly = false)
	{
		// 인자 두개가 둘다 True인 경우를 허용하지 않음.
		if (!IsGetDataOptionRight(bMoveHeadOnly, bPeek))
		{
			return OP_RESULT::OP_RSLT_INVALID_USAGE;
		}

#ifdef CUMBUFFER_DEBUG
		DebugPos(__LINE__);
#endif

		OP_RESULT nRslt = ValidateBuffer(nLen);
		if (OP_RESULT::OP_RSLT_OK != nRslt)
		{
			std::cerr << "[" << __func__ << "-" << __LINE__ << "] invalid buffer :" << m_strErrMsg << "\n";
			return nRslt;
		}

		// 림 버퍼가 로테이트 하지 않은 경우.
		if (m_CurTail > m_CurHead)
		{
			// 존재하는 데이터보다 많이 달라고하는 경우.
			if (m_CurTail < m_CurHead + nLen)
			{
				std::cerr << "[" << __func__ << "-" << __LINE__ << "] invalid length :" << nLen << "\n";
				m_strErrMsg = "invalid length";
				return OP_RESULT::OP_RSLT_INVALID_LEN;
			}
			// 정상적으로 처리하는 경우.
			else
			{
				GetDataOut(nLen, pDataOut, bMoveHeadOnly, bPeek);
			}
		}
		// 림 버퍼가 로테이트 한 상태의 경우.
		else
		{
			// 요구하는 데이터를 주려면 로테이트가 필요한 경우.
			if (m_BufferLen < m_CurHead + nLen)
			{
				// 현재 헤드에서 버퍼 끝까지 공간.
				size_t nFirstBlockLen = m_BufferLen - m_CurHead;
				// 버퍼 처음부터 필요한 공간.
				size_t nSecondBlockLen = nLen - nFirstBlockLen;
#ifdef CUMBUFFER_DEBUG
				std::cout << "[" << __func__ << "-" << __LINE__ << "] nFirstBlockLen=" << nFirstBlockLen
					<< "/nSecondBlockLen=" << nSecondBlockLen << "\n";
#endif

				if (m_CurTail > 0 &&
					m_CurTail >= nSecondBlockLen)
				{
					if (!bMoveHeadOnly)
					{
						memcpy(pDataOut, m_pBuffer.get() + m_CurHead, nFirstBlockLen);
						memcpy(pDataOut + nFirstBlockLen, m_pBuffer.get(), nSecondBlockLen);
					}

					if (!bPeek)
					{
						m_CurHead = nSecondBlockLen;
					}
				}
				// nSecondBlockLen이 현재 존재하는 데이터보다 많은 데이터를 요청한 경우. (에러)
				else
				{
					std::cerr << "[" << __func__ << "-" << __LINE__ << "] invalid length :" << nLen
						<< " / nFirstBlockLen =" << nFirstBlockLen << "/nSecondBlockLen=" << nSecondBlockLen << "\n";
					m_strErrMsg = "invalid length";
					return OP_RESULT::OP_RSLT_INVALID_LEN;
				}
			}
			// 로테이트가 필요하지 않고 데이터를 줄 수 있는 경우.
			else
			{
				GetDataOut(nLen, pDataOut, bMoveHeadOnly, bPeek);
			}
		}

		// Peek가 아니라면, 현재 저장된 데이터에서 나간 데이터만큼 빼준다.
		if (!bPeek)
		{
			m_CumulatedLen -= nLen;
		}

#ifdef CUMBUFFER_DEBUG
		std::cout << "[" << __func__ << "-" << __LINE__ << "] out data [" << pDataOut << "]\n";
		DebugPos(__LINE__);
#endif
		return OP_RESULT::OP_RSLT_OK;
	}

    //-----------------------------------------------------------------------
	// GetData를 할 때, 데이터가 없거나 요구하는 데이터가 지금 있는 데이터보다 큰지 확인해주는 함수.
    OP_RESULT ValidateBuffer(size_t nLen)
    {
        if( m_CumulatedLen == 0 )
        {
            m_strErrMsg="no data";
            return OP_RESULT::OP_RSLT_NO_DATA;
        }
        else if( m_CumulatedLen < nLen )
        {
            m_strErrMsg="invalid length";
            return OP_RESULT::OP_RSLT_INVALID_LEN;
        }

        return OP_RESULT::OP_RSLT_OK;
    }
        
    //------------------------------------------------------------------------
    size_t GetCumulatedLen()
    {
        return m_CumulatedLen ;
    }

    //------------------------------------------------------------------------
    size_t GetCapacity()
    {
        return m_BufferLen ;
    }

    //------------------------------------------------------------------------
    size_t GetTotalFreeSpace()
    {
        return m_BufferLen - m_CumulatedLen;
    }

    //------------------------------------------------------------------------
    uint64_t GetCurHeadPos()
    {
        return m_CurHead; 
    }

    //------------------------------------------------------------------------
    uint64_t GetCurTailPos()
    {
        return m_CurTail; 
    }

    //------------------------------------------------------------------------
	// 로테이트 된 경우를 고려하여 얼마만큼의 여유 공간이 남았는지 반환해주는 함수.
    uint64_t GetLinearFreeSpace() //for direct buffer write
    {
        //current maximun linear buffer size

        if(m_CurTail==m_BufferLen) //nCurTail_ is at last position
        {
            return m_BufferLen - m_CumulatedLen ; 
        }
        else if(m_CurHead < m_CurTail)
        {
            return m_BufferLen- m_CurTail; 
        }
        else if(m_CurHead > m_CurTail)
        {
            return m_CurHead-m_CurTail; 
        }
        else
        {
            return m_BufferLen- m_CurTail;
        }
    }

    //------------------------------------------------------------------------
    //char* GetLinearAppendPtr() //for direct buffer write
    //{
    //    if(m_CurTail == m_BufferLen) //nCurTail_ is at last position
    //    {
    //        if(m_BufferLen != m_CumulatedLen) //and buffer has free space
    //        {
    //            //-> append at 0  
    //            //nCurTail_ -> 버퍼 마지막 위치하고, 버퍼에 공간이 존재. -> 처음에 저장
    //            //XXX dangerous XXX 
    //            //this is not a simple get function, nCurTail_ changes !!
    //            m_CurTail = 0;
    //        }
    //    }

    //    return (m_pBuffer.get() + m_CurTail);
    //}

    //------------------------------------------------------------------------
    void IncreaseData(size_t nLen)
    {
        m_CurTail+= nLen;
        m_CumulatedLen +=nLen;
    }

    //------------------------------------------------------------------------
    void DebugPos(int nLine)
    {
        std::cout <<"line=" <<nLine<<"/ nCurHead_=" << m_CurHead  << "/ nCurTail_= "  << m_CurTail 
                  <<" / nBufferLen_=" << m_BufferLen
                  <<" / nCumulatedLen_=" << m_CumulatedLen
                  <<"\n";
    }

    //------------------------------------------------------------------------
    void ReSet()
    {
        m_CumulatedLen=0;
        m_CurHead=0;
        m_CurTail=0;
    }

	std::string GetErrMsg() { return m_strErrMsg; };

  private:
	void GetDataOut(size_t nLen, char* pDataOut, bool bMoveHeadOnly, bool bPeek)
	{
		if (!bMoveHeadOnly)
		{
			memcpy(pDataOut, m_pBuffer.get() + m_CurHead, nLen);
		}
		if (!bPeek)
		{
			m_CurHead += nLen;
		}
	}
	bool IsGetDataOptionRight(bool bMoveHeadOnly, bool bPeek)
	{
		if (bMoveHeadOnly && bPeek)
		{
			std::cerr << "[" << __func__ << "-" << __LINE__ << "] invalid usage" << "\n";
			m_strErrMsg = "invalid usage";
			return false;
		}
		return true;
	}
	void AppendDataAfterRotaion(const char* pAppendData, const size_t appendLength)
	{
		memcpy(m_pBuffer.get() + m_CurTail, pAppendData, appendLength);
		m_CurTail += appendLength;
		m_CumulatedLen += appendLength;
	}
	void AppendDataBeyondBufferEnd(
		const char* pAppendData,
		const size_t appendLength,
		const int firstBlockLength,
		const int secondBlockLength)
	{
		if (firstBlockLength > 0)
		{
			memcpy(m_pBuffer.get() + m_CurTail, pAppendData, firstBlockLength);
		}

		memcpy(m_pBuffer.get(), pAppendData + (firstBlockLength), secondBlockLength);

		m_CurTail = secondBlockLength;
		m_CumulatedLen += appendLength;
	}

    std::string m_strErrMsg;
    //char*       m_pBuffer;
	std::unique_ptr<char[]> m_pBuffer;
    size_t      m_BufferLen;
    size_t      m_CumulatedLen; // 저장된 데이터 길이

    uint64_t    m_CurHead; // 가장 최신 읽기의 마지막 위치
    uint64_t    m_CurTail; // 가장 최신 쓰기의 마지막 위치
};


#endif




