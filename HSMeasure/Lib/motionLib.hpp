
#pragma once

#include <QMessageBox>
#include <qDebug>
#include <QSettings>
#include <QThread>
#include <QTimer>

#include "DMCLib/LTDMC.h"
#include "APSLib/APS168.h"
#include "APSLib/APS_Define.h"

#ifdef _M_X64
#pragma comment(lib, "Lib/DMCLib/LTDMCx64.lib")
#pragma comment(lib, "Lib/APSLib/APS168x64.lib")
#endif

#ifdef _M_IX86  
#pragma comment(lib, "DMCLib/LTDMCx86.lib")
#pragma comment(lib, "APSLib/APS168x86.lib") 
#endif // _M_IX86


namespace _MOTIONCLASS
{
	#define startVel	5000
	#define maxVel		10000
	#define accT		0.1
	#define decT		0.1
	#define stopVel		500
	#define modeS		0
	#define paraS		1


	enum MOTION_CARD_TYPE
	{
		DMC5000,
		PCI7856
	};

	enum MOVE_TYPE
	{
		SingleAxisPositionMove,
		SingleAxisSpeedMove

	};

	struct DMC5000InitialPara
	{
		WORD  CardIdList[8];		//定义卡号数组
		DWORD CardTypeList[8];		//定义各卡类型
	};

	struct HomePara
	{
		
		WORD outmode = 0;

		double Min_Vel = 0;		double Max_Vel = 0;
		double Tacc = 0;		double Tdec = 0;
		double stop_vel = 200;

		WORD home_dir = 1;
		double vel_mode = 0;
		WORD mode = 0;
		WORD EZ_count = 0;
	};

	struct MovePara
	{
		double Min_Vel = 500; double Max_Vel = 10000; double Tacc = 0.1; double Tdec = 0.1; double stop_vel = 0;
		WORD s_mode; double s_para;

		WORD posiMode = 1;		//Absolute motion
		bool bLogic = true;	//direction
		MOVE_TYPE moveType = SingleAxisPositionMove;
		const int moveTimeOut = 100000;
	};

	struct PCI7856InitialPara
	{
		I32 boardID_InBits = 0; 
		I32 mode = 0;
		I32 cardName = 0; 
	};

class MOTIONLib
{
public:
	MOTIONLib(){};
	~MOTIONLib(){};

	virtual bool initial() = 0;
	virtual bool move(int, int, long) = 0;
	/*
	
	virtual bool home() = 0;
	virtual void emgStop() = 0;
	
	virtual long curPosition() = 0;
	virtual void sevonOnOff(WORD) = 0;
	*/
public:
	

private:
	
};



class DMC5000Lib : public MOTIONLib
{
	#define ABSOLUTE_MOTION 1
	#define RELATIVE_MOTION 0

	class DmcAxis
	{
	public:

		DmcAxis(int cardNo, int axisNo) : mCardNo(cardNo), mAxisNo(axisNo)
		{}
		//
		void setHomePara(
			WORD home_dir = 0,
			double Max_Vel = maxVel,	
			double Min_Vel = startVel, 
			double Tacc = accT, double Tdec = decT,

			double vel_mode = 0,
			WORD mode = 1,

			double stop_vel = 500, WORD EZ_count = 0)
		{
			mHomePara.Max_Vel = Max_Vel;
			mHomePara.home_dir = home_dir;
			mHomePara.Min_Vel = Min_Vel;
			mHomePara.Tacc = Tacc;		 mHomePara.Tdec = Tdec;

//			dmc_set_pulse_outmode(mCardNo, mAxisNo, 3);  //设置脉冲输出模式
			dmc_set_profile(mCardNo, mAxisNo, mHomePara.Min_Vel, mHomePara.Max_Vel, mHomePara.Tacc, mHomePara.Tdec, 200);//设置速度曲线
			dmc_set_homemode(mCardNo, mAxisNo, mHomePara.home_dir, mHomePara.vel_mode, mHomePara.mode, mHomePara.EZ_count = 1);//设置回零方式
		}

		bool home()
		{
			dmc_home_move(mCardNo, mAxisNo);
		
			return true;
		}

		bool checkHome()
		{
			DWORD inAxisS;

			if (0 == dmc_check_done(mCardNo, mAxisNo))
			{
				return false;
			}

			/*
			if ((dmc_axis_io_status(mCardNo, mAxisNo) >> 2) & 0x01)
			{
				dmc_home_move(mCardNo, mAxisNo);
				return false;
			}
			*/

			inAxisS = dmc_axis_io_status(mCardNo, mAxisNo);

			if (0 == (inAxisS & ORG))
			{
				dmc_home_move(mCardNo, mAxisNo);
				return false;
			}

			dmc_write_erc_pin(mCardNo, mAxisNo, 1);

			dmc_set_position(mCardNo, mAxisNo, 0);
			
			return true;
		}

		bool checkHomeDone(const DWORD tSystemTime, const DWORD outTime)
		{
			while (0 == dmc_check_done(mCardNo, mAxisNo))
			{
				

				if ((GetTickCount() - tSystemTime) > outTime)
				{
					dmc_stop(mCardNo, mAxisNo, STOP_MODE_EMG);
					qDebug() << "outTime";
					return false;
				}

				if ((dmc_axis_io_status(mCardNo, mAxisNo) >> 2) & 0x01)
				{
					qDebug() << "dmc_axis_io_status";
					return false;
				}

			//	mCurPosition = dmc_get_position(mCardNo, mAxisNo);
				QApplication::processEvents();
			}

			dmc_write_erc_pin(mCardNo, mAxisNo, 1);

			dmc_set_position(mCardNo, mAxisNo, 0);
		
			return true;
		}

		void stop(WORD mode)
		{
			dmc_stop(mCardNo, mAxisNo, mode);
		}

		bool setMovePara(double Max_Vel = maxVel,
			WORD posiMode = ABSOLUTE_MOTION,		//Absolute motion
			double Min_Vel = startVel,  double Tacc = accT, double Tdec = decT, double stop_vel = stopVel,
			WORD s_mode = modeS, double s_para = paraS,
			bool bLogic = true,		//positive
			MOVE_TYPE moveType = SingleAxisPositionMove)
		{
			mMovePara.Max_Vel = Max_Vel;
			mMovePara.Min_Vel = Min_Vel; 
			mMovePara.Tacc = Tacc;		mMovePara.Tdec = Tdec; mMovePara.stop_vel = stop_vel;
			mMovePara.s_mode = s_mode;	mMovePara.s_para = s_para;
			mMovePara.bLogic = bLogic;

			short reValue = 0;

			if (dmc_check_done(mCardNo, mAxisNo) == 0) //已经在运动中
				return false;

			//设定脉冲模式及逻辑方向（此处脉冲模式固定为P+D方向：脉冲+方向）	
		//	reValue = dmc_set_pulse_outmode(mCardNo, mAxisNo, 0);

			reValue = dmc_set_profile(mCardNo, mAxisNo, mMovePara.Min_Vel, mMovePara.Max_Vel, mMovePara.Tacc, mMovePara.Tdec, 0);

			//设定S段时间
		//	reValue = dmc_set_s_profile(mCardNo, mAxisNo, 0, mMovePara.s_para);

			return true;
		}

		bool move(long nPulse)
		{
			short reValue = 0;

			if (SingleAxisPositionMove == mMovePara.moveType)
			{
				//点动(位置模式)
				reValue = dmc_pmove(mCardNo, mAxisNo, nPulse*(mMovePara.bLogic ? 1 : -1), 1);
			}
			else if (SingleAxisSpeedMove == mMovePara.moveType)
			{
				//持续驱动(速度模式)
				reValue = dmc_vmove(mCardNo, mAxisNo, mMovePara.bLogic ? 1 : 0);
			}

			return true;
		}

		int checkDone(const DWORD tSystemTime, const DWORD outTime)
		{
			while (true)
			{
				

				if ((GetTickCount() - tSystemTime) > outTime)
				{
					dmc_stop(mCardNo, mAxisNo, 1);
					return 2;
				}


				if (dmc_check_done(mCardNo, mAxisNo) == 0)
				{
					mCurPosition = dmc_get_position(mCardNo, mAxisNo);
					QApplication::processEvents();

					return 1;
				}

				return 0;	//done
			}	
		}

		bool moveAndCheck(long nPulse)
		{
			if (mAxisNo == 4)
			{
				mMutex.lock();
			}

			move(nPulse);

			if (mAxisNo == 4)
			{
				mMutex.unlock();
			}

			if (0 == dmc_check_done(mCardNo, mAxisNo))
			{
				return false;
			}

			return true;
		}

		bool moveAndCheckdone(long nPulse, const DWORD outTime)
		{		
			DWORD tSystemTime = GetTickCount();

			move(nPulse);

			while (0 == dmc_check_done(mCardNo, mAxisNo))
			{				
				if ((GetTickCount() - tSystemTime) > outTime)
				{
					dmc_stop(mCardNo, mAxisNo, STOP_MODE_EMG);

					return false;
				}

				mCurPosition = dmc_get_position(mCardNo, mAxisNo);
				Sleep(100);
				QApplication::processEvents();
			}

			return true;
		}


		long curPosition()
		{
			return dmc_get_position(mCardNo, mAxisNo);
		}

		void sevonOnOff(WORD on_off)
		{
			dmc_write_sevon_pin(mCardNo, mAxisNo, on_off);
		}

		QMutex mMutex;
		QMutex*	 mpMutex;
		WORD mCardNo;	
		WORD mAxisNo;
		long mCurPosition;
	private:
		HomePara mHomePara;
		MovePara mMovePara;
		
	};
	
public:
	DMC5000Lib(int cardNum, int axisNum) :mCardNum(cardNum), mAxisNum(axisNum)
	{

		for (size_t i = 0; i < cardNum; i++)
		{
			for (size_t j = 0; j < axisNum; j++)
			{
				mpDmcAxis[i][j] = new DmcAxis(i, j);
			}
		}	
	}

	~DMC5000Lib()
	{
		for (size_t i = 0; i < mCardNum; i++)
		{
			for (size_t j = 0; j < mAxisNum; j++)
			{
				delete mpDmcAxis[i][j];
			}
		}		
	}

	bool initial()
	{
		auto a = dmc_board_init();

		if (a <= 0)   //控制卡的初始化操作
		{
			return false;
		}

		dmc_get_CardInfList(&mCardNum, mInitialPara.CardTypeList, mInitialPara.CardIdList);    //获取正在使用的卡号

		return true;
	}

	bool move(int cardNo, int axisNo, long nPulse)
	{
		mpDmcAxis[cardNo][axisNo]->move(nPulse);

		return true;
	}

	void stop(int cardNo, int axisNo, WORD mode)
	{
		mpDmcAxis[cardNo][axisNo]->stop(mode);	// 0:减速停止  1：紧急停止
	}

	void emgStop()
	{
		for (size_t i = 0; i < mCardNum; i++)
		{
			dmc_emg_stop(mInitialPara.CardIdList[i]);
		}
	}
	

public:
	static const WORD STOP_MODE_EMG = 1;
	static const WORD STOP_MODE_DEC = 0;

	WORD  mCardNum = 1;
	DMC5000InitialPara mInitialPara;
	
	int  mAxisNum = 8;

	static const int  maxCardNum = 4;				
	static const int  maxAxisNum = 16;
	DmcAxis* mpDmcAxis[maxCardNum][maxAxisNum];
	
};


class PCI7856Lib : public MOTIONLib
{
public:
	PCI7856Lib()
	{
	
	}
	~PCI7856Lib(){}

	bool initial()
	{
		I32 ret;				// Return code. [ㄧΑ肚]
		
		I32 cardId = 0;	// Device ID [北ID]
		I32 ver = 0; // Return APS library version. [眔APSセ絪腹]
		

		/*
		if (mIsInitial())
		{
			APS_close();

			// UI setting (诀ざ砞﹚)
			mSetState(STA_CLOSED);
			return false;
		}
		*/

		// Initial control card.
		ret = APS_initial(&mInitialPara.boardID_InBits, mInitialPara.mode);

//		if (ret != ERR_NoError)
		{
		//	MessageBox("APS initial failed.");
		//	mSetState(STA_CLOSED);
			return false;
		}

		APS_get_card_name(cardId, &mInitialPara.cardName);
		if (mInitialPara.cardName != 5) //5:PCI-7856
		{
		//	MessageBox("PCI-7856 not found.");
		//	mSetState(STA_CLOSED);
			return false;
		}

		// Check APS library version.
		ver = APS_version();
		if (ver < 100721) //If library version not match. 
		{ 
			// Interrupt function does not supported. 
		//	MessageBox("The device does not support DI interrupt functions.");
		//	MessageBox("Device closed.");

			APS_close();

			// UI setting (诀ざ砞﹚)
		//	mSetState(STA_CLOSED);
			return false;
		}

		//mCardId = cardId;

		// UI setting (诀ざ砞﹚)
		//mSetState(STA_INIT);

		return true;
	}

	static void home(I32 Axis_ID)
	{
		 APS_home_move(Axis_ID);
	}

	bool home()
	{
		return true;
	}

	bool move()
	{

		return true;
	}

	void emgStop()
	{

	}


	

private:
	PCI7856InitialPara mInitialPara;
//	static PCI7856Lib mHomePara;
//static PCI7856Lib mMovePara;
};


};//namespace

