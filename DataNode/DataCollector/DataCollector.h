#ifndef __DATA_COLLECTOR_H__
#define	__DATA_COLLECTOR_H__


#include <string>
#include "../Interface.h"
#include "../Infrastructure/Dll.h"
#include "../Infrastructure/Lock.h"


/**
 * @class				CollectorStatus
 * @brief				当前行情会话的状态
 * @detail				服务框架需要通过这个判断（组合初始化策略实例）来判断是否需要重新初始化等动作
 * @author				barry
 */
class CollectorStatus
{
public:
	CollectorStatus();

public:
	enum E_SS_Status		Get() const;

	bool					Set( enum E_SS_Status eNewStatus );

private:
	mutable CriticalObject	m_oCSLock;
	enum E_SS_Status		m_eStatus;			///< 当前行情逻辑状态，用于判断当前该做什么操作了
};


/**
 * @class					DataCollector
 * @brief					数据采集模块控制注册接口
 * @note					采集模块只提供三种形式的回调通知( I_DataHandle: 初始化映像数据， 实时行情数据， 初始化完成标识 ) + 重新初始化方法函数
 * @date					2017/5/3
 * @author					barry
 */
class DataCollector
{
public:
	DataCollector();

	/**
	 * @brief				数据采集模块初始化
	 * @param[in]			pIDataCallBack				行情回调接口
	 * @return				==0							成功
							!=0							错误
	 */
	int						Initialize( I_DataHandle* pIDataCallBack );

	/**
	 * @breif				数据采集模块释放退出
	 */
	void					Release();

public:///< 数据采集模块事件定义
	/**
 	 * @brief				初始化/重新初始化回调
	 * @note				同步函数，即函数返回后，即初始化操作已经做完，可以判断执行结果是否为“成功”
	 * @return				==0							成功
							!=0							错误
	 */
	int						ReInitializeDataCollector();

	/**
	 * @biref				取得当前数据采集模块状态
	 */
	const CollectorStatus&	InquireDataCollectorStatus();

private:
	Dll						m_oDllPlugin;					///< 插件加载类
};







#endif








