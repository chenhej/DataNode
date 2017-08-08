#pragma warning(disable:4996)
#pragma warning(disable:4244)
#include "DataEcho.h"
#include "NodeServer.h"
#include "../Infrastructure/DateTime.h"


///< 字符串转小写
std::string& Str2Lower( std::string& sStr )
{
	std::transform( sStr.begin(), sStr.end(), sStr.begin(), ::tolower );

	return sStr;
}

bool SplitString( char** pArgv, int& nArgc, const char* pszStr )
{
	static std::string		vctArgv[16];
	int						nRawArgc = nArgc;
	bool					bLastCharIsNotSeperator = true;
	int						nCmdStrLen = ::strlen( pszStr );

	///< 清空缓存
	nArgc = 0;
	for( int j = 0; j < 16; j++ )
	{
		vctArgv[j] = "";
	}

	///< split命令字符串
	for( int n = 0; n < nCmdStrLen; n++ )
	{
		char	ch = pszStr[n];

		if( ch > '!' && ch < '~' )
		{
			vctArgv[nArgc] += ch;
			bLastCharIsNotSeperator = true;
		}
		else				///< 过滤掉分隔符
		{
			if( true == bLastCharIsNotSeperator && vctArgv[nArgc] != "" )
			{
				if( nArgc >= (nRawArgc-1) )
				{
					return false;
				}

				nArgc++;	///< 新启一行字符子项
			}

			bLastCharIsNotSeperator = false;
		}
	}

	nArgc++;

	///< 构造新的命令方法的参数
	for( int i = 0; i < nArgc; i++ )
	{
		pArgv[i] = (char*)vctArgv[i].c_str();
	}

	return true;
}


const unsigned int	IDataEcho::s_nMaxEchoBufLen = 1024*1024*10;
char*				IDataEcho::s_pEchoDataBuf = new char[s_nMaxEchoBufLen];


IDataEcho::IDataEcho( std::string sMarketName )
 : m_sMarketName( sMarketName )
{
}

const std::string& IDataEcho::GetMarketName()
{
	return m_sMarketName;
}

bool IDataEcho::operator()( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	if( NULL == s_pEchoDataBuf )
	{
		::sprintf( szResult, "IDataEcho::operator() : [ERR] invalid buffer pointer." );
		return true;
	}

	::memset( s_pEchoDataBuf, 0, s_nMaxEchoBufLen );

	return ExcuteCommand( pArgv, nArgc, szResult, uiSize );
}

ModuleControl::ModuleControl()
 : IDataEcho( "模块控制器" )
{
}

ModuleControl& ModuleControl::GetSingleton()
{
	static	ModuleControl obj;

	return obj;
}

bool ModuleControl::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "help" )
	{
		::sprintf( szResult, "%s", "命令字说明：\nhelp	帮助命令。\nreload	重新初始化。" );
		return true;
	}
	else if( sCmd == "push" )
	{
		::sprintf( szResult, "数据已补发! [%u]", DateTime::Now().TimeToLong() );
		return true;
	}

	return false;
}


CTP_DL_Echo::CTP_DL_Echo()
 : IDataEcho( "CTP_大连" )
{
}

CTP_DL_Echo& CTP_DL_Echo::GetSingleton()
{
	static CTP_DL_Echo	obj;

	return obj;
}

int CTP_DL_Echo::FormatStruct2OutputBuffer( char* pszEchoBuffer, unsigned int nMsgID, const char* pszInputBuffer )
{
	if( NULL == pszEchoBuffer || NULL == pszInputBuffer ) {
		return 0;
	}

	switch( nMsgID )
	{
	case 100:
		{
			tagDLFutureMarketInfo_LF100&	refMarketInfo = *((tagDLFutureMarketInfo_LF100*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "市场ID:%u, 商品总数:%u\n", refMarketInfo.MarketID, refMarketInfo.WareCount );
		}
	case 101:
		{
			tagDLFutureKindDetail_LF101&	refKind = *((tagDLFutureKindDetail_LF101*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "分类名称:%u, 分类商品数:%u\n", refKind.KindName, refKind.WareCount );
		}
	case 102:
		{
			tagDLFutureMarketStatus_HF102&	refMarketStatus = *((tagDLFutureMarketStatus_HF102*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "行情日期:%u, 时间:%u, 行情状态:%s \n", refMarketStatus.MarketDate, refMarketStatus.MarketTime, (0==refMarketStatus.MarketStatus)?"初始化":"行情中" );
		}
	case 103:
		{
			tagDLFutureReferenceData_LF103&	refRefData = *((tagDLFutureReferenceData_LF103*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 名称:%s, 合约乖数:%u, 手比率:%u, 交割日:%u\n", refRefData.Code, refRefData.Name, refRefData.ContractMult, refRefData.LotSize, refRefData.DeliveryDate );
		}
	case 104:
		{
			tagDLFutureSnapData_LF104&		refSnapDataLF = *((tagDLFutureSnapData_LF104*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s 昨结:%u, 今结:%u\n", refSnapDataLF.Code, refSnapDataLF.PreSettlePrice, refSnapDataLF.SettlePrice );
		}
	case 105:
		{
			tagDLFutureSnapData_HF105&		refSnapDataHF = *((tagDLFutureSnapData_HF105*)pszInputBuffer);
			return ::sprintf( pszEchoBuffer, "代码:%s, 最新:%u, 最高:%u, 最低:%u, 金额:%f, 成交量:%I64d\n", refSnapDataHF.Code, refSnapDataHF.Now, refSnapDataHF.High, refSnapDataHF.Low, refSnapDataHF.Amount, refSnapDataHF.Volume );
		}
	case 106:
		{
			unsigned int					nWritePos = 0;
			tagDLFutureSnapBuySell_HF106&	refBuySellDataHF = *((tagDLFutureSnapBuySell_HF106*)pszInputBuffer);

			nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "代码:%s, ", refBuySellDataHF.Code );

			for( int i = 0; i < 5; i++ )
			{
				nWritePos += ::sprintf( pszEchoBuffer+nWritePos, "买%d价:%u, 买%d量:%I64d\t卖%d价:%d,卖%i量:%I64d\n"
					, i+1, refBuySellDataHF.Buy[i].Price, i, refBuySellDataHF.Buy[i].Volume, i+1, refBuySellDataHF.Sell[i].Price, i, refBuySellDataHF.Sell[i].Volume );
			}

			return nWritePos;
		}
	}

	return 0;
}

bool CTP_DL_Echo::ExcuteCommand( char** pArgv, unsigned int nArgc, char* szResult, unsigned int uiSize )
{
	unsigned int	nWritePos = 0;
	std::string		sCmd = Str2Lower( std::string( pArgv[0] ) );

	if( sCmd == "marketinfo" )
	{
		tagDLFutureMarketInfo_LF100			tagMkInfo = { 0 };
		tagDLFutureMarketStatus_HF102		tagMkStatus = { 0 };

		::strcpy( tagMkInfo.Key, "mkinfo" );
		::strcpy( tagMkStatus.Key, "mkstatus" );

		if( DataNodeService::GetSerivceObj().OnQuery( 100, (char*)&tagMkInfo, sizeof(tagMkInfo) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 100, (char*)&tagMkInfo );
		if( DataNodeService::GetSerivceObj().OnQuery( 102, (char*)&tagMkStatus, sizeof(tagMkStatus) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 102, (char*)&tagMkStatus );
	}
	else if( sCmd == "nametable" )
	{
		unsigned int	nIndex = 0;
		std::string		sParam1 = Str2Lower( std::string( pArgv[1] ) );
		std::string		sParam2 = Str2Lower( std::string( pArgv[2] ) );
		unsigned int	nBeginPos = ::atol( sParam1.c_str() );
		unsigned int	nEndPos = nBeginPos + ::atol( sParam2.c_str() );
		int				nDataLen = DataNodeService::GetSerivceObj().OnQuery( 103, s_pEchoDataBuf, s_nMaxEchoBufLen );

		for( unsigned int nOffset = 0; nOffset < s_nMaxEchoBufLen && nOffset < nDataLen && nIndex < nEndPos; nOffset+=sizeof(tagDLFutureReferenceData_LF103), nIndex++ )
		{
			if( nIndex >= nBeginPos && nIndex < nEndPos )
			{
				nWritePos += ::sprintf( szResult+nWritePos, "%d. ", nIndex+1 );
				nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 103, s_pEchoDataBuf+nOffset );
			}
		}
	}
	else if( sCmd == "snaptable" )
	{
		std::string						sParam1 = pArgv[1];
		tagDLFutureSnapData_LF104		tagSnapLF = { 0 };
		tagDLFutureSnapData_HF105		tagSnapHF = { 0 };
		tagDLFutureSnapBuySell_HF106	tagBSHF = { 0 };

		::memcpy( tagSnapLF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagSnapHF.Code, sParam1.c_str(), sParam1.length() );
		::memcpy( tagBSHF.Code, sParam1.c_str(), sParam1.length() );
		if( DataNodeService::GetSerivceObj().OnQuery( 105, (char*)&tagSnapHF, sizeof(tagSnapHF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 105, (char*)&tagSnapHF );
		if( DataNodeService::GetSerivceObj().OnQuery( 104, (char*)&tagSnapLF, sizeof(tagSnapLF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 104, (char*)&tagSnapLF );
		if( DataNodeService::GetSerivceObj().OnQuery( 106, (char*)&tagBSHF, sizeof(tagBSHF) ) > 0 )
			nWritePos += CTP_DL_Echo::FormatStruct2OutputBuffer( szResult+nWritePos, 106, (char*)&tagBSHF );
	}

	return true;
}
















