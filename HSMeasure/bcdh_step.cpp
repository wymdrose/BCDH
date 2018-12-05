
#include "bcdh_step.h"

extern QString mPath;
//const QStringList POS_STEP;

const BCDH_step::STEP_MAP BCDH_step::arrayStepMap[(SlotposNo4 + 1) * (StepJ + 1)] = {
	{ laserPos1, laserNo1 }, { laserPos1, laserNo6 },
	{ laserPos2, laserNo2 }, { laserPos2, laserNo5 },
	{ laserPos3, laserNo1 }, { laserPos3, laserNo6 }, { laserPos3, laserNo2 }, { laserPos3, laserNo5 },
	{ laserPos4, laserNo2 }, { laserPos4, laserNo5 },{ laserPos4, laserNo3 }, { laserPos4, laserNo4 },
	{ laserPos5, laserNo1 }, { laserPos5, laserNo6 }, { laserPos5, laserNo2 }, { laserPos5, laserNo5 }, { laserPos5, laserNo3 }, { laserPos5, laserNo4 },
	{ laserPos6, laserNo2 }, { laserPos6, laserNo5 }, { laserPos6, laserNo3 }, { laserPos6, laserNo4 },
	{ laserPos7, laserNo1 }, { laserPos7, laserNo6 }, { laserPos7, laserNo2 }, { laserPos7, laserNo5 }, { laserPos7, laserNo3 }, { laserPos7, laserNo4 },
	{ laserPos8, laserNo2 }, { laserPos8, laserNo5 }, { laserPos8, laserNo3 }, { laserPos8, laserNo4 },
	{ laserPos9, laserNo2 }, { laserPos9, laserNo5 }, { laserPos9, laserNo3 }, { laserPos9, laserNo4 },
	{ laserPos10, laserNo3 }, { laserPos10, laserNo4 },
	{ laserPos11, laserNo3 }, { laserPos11, laserNo4 },

};

BCDH_step::BCDH_step(HSMeasure* pHSMeasure)
	: mpHSMeasure(pHSMeasure)
{
	connect(this, SIGNAL(showMsgSignal(const QString&)), mpHSMeasure, SLOT(show_msg(const QString &)));
	connect(this, SIGNAL(laserGetSignal(int, float*)), mpHSMeasure, SLOT(get_laser_result(int, float*)), Qt::BlockingQueuedConnection);


	getPosFromCfg();

	auto a = arrayStepMap[(SlotposNo3 + 1) * (StepD + 1)];
	
	m_pLoadDllfunc = CLoadLJDllFunc::GetInstance();

	laser1_config.IPAddress.S_un.S_un_b.s_b1 = 192;
	laser1_config.IPAddress.S_un.S_un_b.s_b2 = 168;
	laser1_config.IPAddress.S_un.S_un_b.s_b3 = 10;
	laser1_config.IPAddress.S_un.S_un_b.s_b4 = 100;

	laser2_config.IPAddress.S_un.S_un_b.s_b1 = 192;
	laser2_config.IPAddress.S_un.S_un_b.s_b2 = 168;
	laser2_config.IPAddress.S_un.S_un_b.s_b3 = 20;
	laser2_config.IPAddress.S_un.S_un_b.s_b4 = 100;

	laser3_config.IPAddress.S_un.S_un_b.s_b1 = 192;
	laser3_config.IPAddress.S_un.S_un_b.s_b2 = 168;
	laser3_config.IPAddress.S_un.S_un_b.s_b3 = 30;
	laser3_config.IPAddress.S_un.S_un_b.s_b4 = 100;

	//0   2   4   8   12   18   22   28   32   36   38 
	POS_STEP<< "StepC_1" << "StepH_1"
		<< "StepD_1" << "StepG_1"
		<< "StepC_2" << "StepH_2" << "StepB_1" << "StepI_1"
		<< "StepD_2" << "StepG_2" << "StepE_1" << "StepF_1"
		<< "StepC_3" << "StepH_3" << "StepB_2" << "StepI_2" << "StepA_1" << "StepJ_1"
		<< "StepD_3" << "StepG_3" << "StepE_2" << "StepF_2"
		<< "StepC_4" << "StepH_4" << "StepB_3" << "StepI_3" << "StepA_2" << "StepJ_2"
		<<"StepD_4" << "StepG_4" << "StepE_3" << "StepF_3"
		<<"StepB_4" << "StepI_4" << "StepA_3" << "StepJ_3"
		<<"StepE_4" << "StepF_4" 
		<<"StepA_4" << "StepJ_4";

}

BCDH_step::~BCDH_step()
{
	qDebug() << "~BCDH_step()";
}

void BCDH_step::GetOneHeadData(LJIF_PROFILETARGET head, LJIF_OPENPARAM_ETHERNET config, float* buf, uint num)
{
	int iLSRet;
	int a = 0;
	LJIF_PROFILE_INFO profileInfo;
	float temp[5][800];
//	showMsgSignal("GetOneHeadData ERROR....");
	QString strValue[2];
	LJIF_MEASUREDATA* pMeasureData = new LJIF_MEASUREDATA[2];
	
	iLSRet = m_pLoadDllfunc->LJIF_OpenDeviceETHER(&config);
	if (iLSRet)
	{
	   qDebug() << head << "LJIF_OpenDeviceETHER ERROR...."; 
	   showMsgSignal("LJIF_OpenDeviceETHER ERROR....");
	}
	for (int j = 0; j < num; j++)
	{
	iLSRet = m_pLoadDllfunc->LJIF_GetProfileData(head, &profileInfo, temp[j], LSCOUNT);
	if (num > 1)
	Sleep(15);
	}
	m_pLoadDllfunc->LJIF_CloseDevice();
	
	for (int j = 0; j < num; j++)
	{
		if (temp[j] == NULL)
		{
		qDebug() << "~get raw data failed. __GetOneHeadData()";
		return;
		}
		
		for (int i = 0; i < LSCOUNT; i++)
		{
			a = (_isnan((double)temp[j][i]));
			
			if (a)
			{
				temp[j][i] = -999.99;
			}
			
		}
	}

	for (int i = 0; i < LSCOUNT; i++)
	{
		switch (num)
		{
		case 1:
			buf[i] = temp[0][i];
			break;
		case 2:
			buf[i] = (temp[0][i] + temp[1][i]) / num;
			break;
		case 3:
			buf[i] = (temp[0][i] + temp[1][i] + temp[2][i]) / num;
			break;
		case 4:
			buf[i] = (temp[0][i] + temp[1][i] + temp[2][i] + temp[3][i]) / num;
			break;
		case 5:
			buf[i] = (temp[0][i] + temp[1][i] + temp[2][i] + temp[3][i] + temp[4][i]) / num; 
			break;
		}
	}
	Sleep(15); //delay for opening laser controler 
}

void BCDH_step::GetTwoHeadData(LJIF_OPENPARAM_ETHERNET config, float* buf1, float* buf2, uint num)
{
	int iLSRet;
	int a = 0, b = 0;
	LJIF_PROFILE_INFO profileInfo;
	float temp[5][800], temp2[5][800];

	if (num == 0)
		num = 1;
	if (num > 5)
		num = 5;
	
	iLSRet = m_pLoadDllfunc->LJIF_OpenDeviceETHER(&config);
	if (iLSRet)
	{
		qDebug() << "gLJIF_OpenDeviceETHER ERROR....";
		showMsgSignal("LJIF_OpenDeviceETHER ERROR....");
	}
	for (int j = 0; j < num; j++)
	{
		iLSRet = m_pLoadDllfunc->LJIF_GetProfileData(LJIF_PROFILETARGET_HEAD_A, &profileInfo, temp[j], LSCOUNT);
		if (num > 1)
			Sleep(20);
	}
	
	m_pLoadDllfunc->LJIF_CloseDevice();
	
	/*
	if (config.IPAddress.S_un.S_un_b.s_b3 == laser1_config.IPAddress.S_un.S_un_b.s_b3)
	{
		Sleep(80);
	}
	*/

	Sleep(30);//delay for opening laser controler 

	iLSRet = m_pLoadDllfunc->LJIF_OpenDeviceETHER(&config);
	if (iLSRet)
	{
		qDebug() << "LJIF_OpenDeviceETHER ERROR....";
		showMsgSignal("LJIF_OpenDeviceETHER ERROR....");
	}
	for (int j = 0; j < num; j++)
	{
		iLSRet = m_pLoadDllfunc->LJIF_GetProfileData(LJIF_PROFILETARGET_HEAD_B, &profileInfo, temp2[j], LSCOUNT);
		if (num > 1)
			Sleep(20);
	}
	m_pLoadDllfunc->LJIF_CloseDevice();

	for (int j = 0; j < num; j++)
	{
		if (temp[j] == NULL || temp2[j] == NULL)
		{
			qDebug() << "~get raw data failed. __GetOneHeadData()";
			return;
		}


		for (int i = 0; i < LSCOUNT; i++)
		{
			a = (_isnan((double)temp[j][i]));
			b = (_isnan((double)temp2[j][i]));
			if (a)
			{
				temp[j][i] = -999.99;
			}

			if (b)
			{
				temp2[j][i] = -999.99;
			}
		}
	}


	for (int i = 0; i < LSCOUNT; i++)
	{
		switch (num)
		{
		case 1:
			  buf1[i] = temp[0][i]; 
			  buf2[i] = temp2[0][i];
			  break;
		case 2: 
			buf1[i] = (temp[0][i] + temp[1][i]) / num;
			buf2[i] = (temp2[0][i] + temp2[1][i]) / num;
			break;
		case 3: 
			buf1[i] = (temp[0][i] + temp[1][i] + temp[2][i]) / num; 
			buf2[i] = (temp2[0][i] + temp2[1][i] + temp2[2][i]) / num;
			break;
		case 4: 
			buf1[i] = (temp[0][i] + temp[1][i] + temp[2][i] + temp[3][i]) / num; 
			buf2[i] = (temp2[0][i] + temp2[1][i] + temp2[2][i] + temp2[3][i]) / num;
			break;
		case 5: 
			buf1[i] = (temp[0][i] + temp[1][i] + temp[2][i] + temp[3][i] + temp[4][i]) / num; 
			buf2[i] = (temp2[0][i] + temp2[1][i] + temp2[2][i] + temp2[3][i] + temp2[4][i]) / num; 
			break;
		}
	}
	
}

bool BCDH_step::getPosFromCfg()
{
	QString fileName = mPath + "/cfg/stepPos.csv";

	QFile csvFile(fileName);

	QStringList CSVList;

	if (!csvFile.open(QIODevice::ReadOnly))
	{
		return false;
	}

	QTextStream stream(&csvFile);

	while (!stream.atEnd())
	{
		CSVList.push_back(stream.readLine());
	}
	csvFile.close();

	for (size_t i = 1; i < CSVList.size(); i++)
	{
		QStringList oneLine = CSVList[i].split(",");

		float posLaser = oneLine[1].toFloat();
		posPlat = oneLine[2].toFloat();

		posLaserAxis[i - 1] = posLaser;
	}

	return true;
}

void valuesProcess(float tValues[], int numIn)
{
	int tNum = numIn;
	float tValueSum = 0;
	float tValueMax = 0;

	for (size_t i = 0; i < numIn; i++)
	{
		if (tValues[numIn] == 0)
		{
			tNum--;
		}

		if (tValues[numIn] > tValueMax)
		{
			tValueMax = tValues[numIn];
		}

		tValueSum += tValues[numIn];
	}

	if (tNum == 0)
	{
		tValues[0] = 0;
	}
	else if (tNum == 1)
	{
		tValues[0] = tValueMax;
	}
	else
	{
		tValues[0] = (tValueSum - tValueMax) / (tNum - 1);
	}
}

bool BCDH_step::getStepValue(QString posStep, float stepValue[])
{
	auto a = POS_STEP.indexOf(posStep);

	
	if (mCurStep == 0 || mCurStep == 2 || mCurStep == 4 || mCurStep == 8 || mCurStep == 12 || mCurStep == 18 || mCurStep == 22 || mCurStep == 28 || mCurStep == 32 || mCurStep == 36 || mCurStep == 38)
	{	
		if (GetTickCount() - mCurSystemTime < 120)
		{
			return false;
		}
	
	}
	
	if (mCurStep == 0 || mCurStep == 1)
	{
		Sleep(80);
	}

	int tLaserNo = arrayStepMap[a].laserNo;
	#ifndef GET_LASER_RESULT
	float buf1[800] = { 999.999 };
	float buf2[800] = { 999.999 };
	#else 
	
	#endif
	switch (tLaserNo + 1)
	{
	case 1:
	case 6:
		#ifndef GET_LASER_RESULT
		GetTwoHeadData(laser1_config, buf1, buf2, CHECKNUM);
		#else
		mpHSMeasure->get_laser_result(1, stepValue);
		#endif
		break;
	
		
	case 2:
	case 5:
		#ifndef GET_LASER_RESULT
		GetTwoHeadData(laser2_config, buf1, buf2, CHECKNUM);
		#else
		mpHSMeasure->get_laser_result(2, stepValue);
		#endif
		break;
	
	case 3:
	case 4:
		#ifndef GET_LASER_RESULT
		GetTwoHeadData(laser3_config, buf1, buf2, CHECKNUM);
		#else
		mpHSMeasure->get_laser_result(3, stepValue);
		#endif
		break;
	
	default:
		break;
	}

#ifndef GET_LASER_RESULT
	double raw_data[2][LASER_VALUE_NUM];
	double result[2][10];

	uint sf1_start[2];
	uint sf1_end[2];
	uint sf2_start[2];
	uint sf2_end[2];

	sf1_start[0] = mpHSMeasure->vWavePosition[tLaserNo + 1][1].toInt();
	sf1_end[0] = mpHSMeasure->vWavePosition[tLaserNo + 1][2].toInt();
	sf2_start[0] = mpHSMeasure->vWavePosition[tLaserNo + 1][3].toInt();
	sf2_end[0] = mpHSMeasure->vWavePosition[tLaserNo + 1][4].toInt();
	
	sf1_start[1] = mpHSMeasure->vWavePosition[6 - tLaserNo][1].toInt();
	sf1_end[1] = mpHSMeasure->vWavePosition[6 - tLaserNo][2].toInt();
	sf2_start[1] = mpHSMeasure->vWavePosition[6 - tLaserNo][3].toInt();
	sf2_end[1] = mpHSMeasure->vWavePosition[6 - tLaserNo][4].toInt();


	if (tLaserNo == laserNo6 || tLaserNo == laserNo5 || tLaserNo == laserNo4 || tLaserNo == laserNo3 || tLaserNo == laserNo2)
	{
//		std::reverse(&buf1[0], &buf1[LASER_VALUE_NUM - 1]);
	}
		
	for (size_t i = 0; i < LASER_VALUE_NUM; i++)
	{
		raw_data[0][i] = buf1[i];
		raw_data[1][i] = buf2[i];
	}

	LALG mLALG;

	mLALG.calculate_result(raw_data[0], result[0], sf1_start[0], sf1_end[0], sf2_start[0], sf2_end[0]);
	mLALG.calculate_result(raw_data[1], result[1], sf1_start[1], sf1_end[1], sf2_start[1], sf2_end[1]);

	stepValue[0] = result[0][0];
	stepValue[1] = result[1][0];
	#else
	
	#endif
	showMsgSignal(QString("laserControl:%1   value:%2    value:%3").arg(tLaserNo + 1).arg(stepValue[0]).arg(stepValue[1]));

	return true;
}

bool BCDH_step::getStepValue()
{
	float tValue[2] = {-1};

	for (; mCurStep < POS_STEP.size(); mCurStep+=2)
	{
		auto a = POS_STEP.indexOf(POS_STEP[mCurStep]);

		if (false == mpHSMeasure->mpMOTIONLib->mpDmcAxis[mpHSMeasure->mCardNo][mpHSMeasure->laserAxisNo]->moveAndCheck(posLaserAxis[arrayStepMap[a].laserPos]))
		{
			mCurSystemTime = GetTickCount();
			return false;
		}
		
		if (false == getStepValue(POS_STEP[mCurStep], tValue))
		{
			return false;
		}

		int slotNo1 = POS_STEP[mCurStep].right(1).toInt() - 1;
		QByteArray tByteArray1 = POS_STEP[mCurStep].mid(4, 1).toLocal8Bit();
		int stepNo1 = tByteArray1[0] - 'A';
		//
		int slotNo2 = POS_STEP[mCurStep + 1].right(1).toInt() - 1;
		QByteArray tByteArray2 = POS_STEP[mCurStep + 1].mid(4, 1).toLocal8Bit();
		int stepNo2 = tByteArray2[0] - 'A';

		mpHSMeasure->stepValues[slotNo1][stepNo1] = tValue[0];
		mpHSMeasure->stepValues[slotNo2][stepNo2] = tValue[1];
	}
	
	mCurStep = 0;

	return true;
}

void  BCDH_step::ShowStepValueToView()
{
	float buf[6][800] = { 0 };

	QVector<double> x(800), y(800);
	QVector<double> y1(800);
	QVector<double> y2(800);
	QVector<double> y3(800);
	QVector<double> y4(800);
	QVector<double> y5(800);

	double t1,t2;
	t1 = GetTickCount();
//	m_pLoadDllfunc->LJIF_SetTrigger(LJIF_HEAD_BOTH);
	GetTwoHeadData(laser1_config, buf[0], buf[5], 1);
	GetTwoHeadData(laser2_config, buf[1], buf[4], 1);
	GetTwoHeadData(laser3_config, buf[2], buf[3], 1);
	t2 = GetTickCount() - t1;
//	qDebug() << "HEAD  time:"<< t2 << endl;

	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = 0; j < 800; j++)
		{
			if (i == 0)
			{
				x[j] = j;
				y[j] = buf[i][j];
			}				
			else if(i == 1)
				y1[j] = buf[i][j];
			else if(i == 2)
				y2[j] = buf[i][j];
			else if(i == 3)
				y3[j] = buf[i][j];
			else if(i == 4)
				y4[j] = buf[i][j];
			else if (i == 5)
			{
				y5[j] = buf[i][j];
			}
		}

	}
	
	/*std::reverse(&y1[0], &y1[799]);
	std::reverse(&y2[0], &y2[799]);
	std::reverse(&y3[0], &y3[799]);
	std::reverse(&y4[0], &y4[799]);
	std::reverse(&y5[0], &y5[799]);*/

	t1 = GetTickCount();
	mpHSMeasure->m_laserEditor[0]->show_line(x, y);
	mpHSMeasure->m_laserEditor[5]->show_line(x, y5);

	mpHSMeasure->m_laserEditor[1]->show_line(x, y1);
	mpHSMeasure->m_laserEditor[4]->show_line(x, y4);

	mpHSMeasure->m_laserEditor[2]->show_line(x, y2);
	mpHSMeasure->m_laserEditor[3]->show_line(x, y3);
	
	t2 = GetTickCount() - t1;
//	qDebug() << "show data  time:" << t2 << endl;


}

void  BCDH_step::get_laser_result(LJIF_PROFILETARGET head, LJIF_OPENPARAM_ETHERNET config, float* result,int program, int nStep)
{
	QString strValue[2];
	LJIF_MEASUREDATA* pMeasureData = new LJIF_MEASUREDATA[2];
	double t1, t2;
	t1 = GetTickCount();
	int iLSRet = m_pLoadDllfunc->LJIF_OpenDeviceETHER(&config);
	if (iLSRet)
	{
		qDebug() << "LJIF_OpenDeviceETHER ERROR....";
		showMsgSignal("LJIF_OpenDeviceETHER ERROR....");
	}

//	m_pLoadDllfunc->LJIF_ChangeProgram((LJIF_SETTINGTARGET)program);
	Sleep(80);
	int returnCode = m_pLoadDllfunc->LJIF_GetMeasureValue(pMeasureData, 2);

	for (int i = 0; i<2; i++)
	{
		if (_finite(pMeasureData[i].fValue) != 0)
		{
			strValue[i] = QString::number(pMeasureData[i].fValue);
		}
		else
		{
			strValue[i] = "- FFFFFF";
		}
	}

	m_pLoadDllfunc->LJIF_CloseDevice();
	t2 =GetTickCount()-t1;
//	qDebug() << "get_laser_result time:"<<t2<<endl;
	if (head == LJIF_PROFILETARGET_HEAD_A)
	   *result = strValue[0].toFloat();
	else if (head == LJIF_PROFILETARGET_HEAD_B)
	   *result = strValue[1].toFloat();
	delete[] pMeasureData;
	Sleep(20);
}
