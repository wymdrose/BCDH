#include "bcdh_gap.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>


extern QString mPath;

BCDH_gap::BCDH_gap(HSMeasure* pHSMeasure)
	: mpHSMeasure(pHSMeasure)
{
	connect(this, SIGNAL(showMsgSignal(const QString&)), mpHSMeasure, SLOT(show_msg(const QString &)));
	connect(this, SIGNAL(showDialogSignal(const QString&)), mpHSMeasure, SLOT(show_dialog(const QString &)), Qt::BlockingQueuedConnection);
		
	mpHSMeasure->mpSerialportCcd[0]->open();
	mpHSMeasure->mpSerialportCcd[1]->open();
	
	
	mpTcpClientCcd[0] = mpHSMeasure->mpTcpClientCcd[0];
	mpTcpClientCcd[1] = mpHSMeasure->mpTcpClientCcd[1];
	
	/*
	mpTcpClientCcd[0]->connect();
	mpTcpClientCcd[1]->connect();
	*/

	getCcdPos();
}

BCDH_gap::~BCDH_gap()
{
	mpHSMeasure->mpSerialportCcd[0]->close();
	mpHSMeasure->mpSerialportCcd[1]->close();

	qDebug() << "~BCDH_gap()";
}


bool BCDH_gap::getPosFromCfg()
{
	QString fileName = mPath + "/cfg/gapPos.csv";

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

		float posCcd = oneLine[1].toFloat();
		float posPlat = oneLine[2].toFloat();

		arrayGapPos[i - 1] = { posCcd, posPlat };
	}

	return true;
}

bool BCDH_gap::getGapValue(const SLOTPOS_NO slotposNo, const GAP_NO gapNo, float& gapValue)
{
	if (false == getPosFromCfg())
	{
		return false;
	}

	qDebug() << "arrayGapPos[gapNo].ccdPos:" << arrayGapPos[gapNo].ccdPos;

	if (false == mpHSMeasure->mpMOTIONLib->mpDmcAxis[mpHSMeasure->mCardNo][mpHSMeasure->ccdAxisNo]->moveAndCheckdone(arrayGapPos[gapNo].ccdPos, DEFAUL_HOME_TIME_OUT))
	{
		return false;
	}

	if (false == mpHSMeasure->mpMOTIONLib->mpDmcAxis[mpHSMeasure->mCardNo][mpHSMeasure->platformAxisNo]->moveAndCheckdone(arrayGapPos[gapNo].platPos, DEFAUL_HOME_TIME_OUT))
	{
		return false;
	}
	
	

	gapValue = 5.0;

	return true;
}

bool BCDH_gap::getGapValue()
{

	for (size_t j = 0; j < GapJ + 1; j++)
	{

		if (GapF == j)
		{
			showDialogSignal("rotate...");

			/*
			if (false == mpHSMeasure->mpMOTIONLib->mpDmcAxis[mpHSMeasure->mCardNo][mpHSMeasure->rotateAxisNo]->moveAndCheckdone(0, DEFAUL_HOME_TIME_OUT))
			{
			return false;
			}
			*/
		}

		float tValue[SlotposNo4 + 1] = { -1 };

		if (false == getGapValue((GAP_NO)j, tValue))
		{

			return false;
		}

		mpHSMeasure->gapValues[SlotposNo1][j] = tValue[SlotposNo1];
		mpHSMeasure->gapValues[SlotposNo2][j] = tValue[SlotposNo2];
		mpHSMeasure->gapValues[SlotposNo3][j] = tValue[SlotposNo3];
		mpHSMeasure->gapValues[SlotposNo4][j] = tValue[SlotposNo4];
	}

	return true;
}




//
bool BCDH_gap::getCcdPos()
{
	QString fileName = mPath + "/cfg/ccdPos.csv";

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

		float posCcd = oneLine[1].toFloat();
		float posPlat = oneLine[2].toFloat();

		arrayGapPos[i - 1] = { posCcd, posPlat };
	}

	return true;
}

bool BCDH_gap::gapMove(const GAP_NO gapNo)
{
	long nPulseX;
	long nPulseY;

	if (gapNo <= GapE)
	{
		nPulseX = mpHSMeasure->posionFromIni.basePositionX1 + arrayGapPos[gapNo].ccdPos * 500;
		nPulseY = mpHSMeasure->posionFromIni.basePositionY1 + arrayGapPos[gapNo].platPos * 500;
	}
	else
	{
		nPulseX = mpHSMeasure->posionFromIni.basePositionX2 + arrayGapPos[gapNo].ccdPos * 500;
		nPulseY = mpHSMeasure->posionFromIni.basePositionY2 + arrayGapPos[gapNo].platPos * 500;
	}

	if (false == mpHSMeasure->mpMOTIONLib->mpDmcAxis[mpHSMeasure->mCardNo][mpHSMeasure->ccdAxisNo]->moveAndCheck(nPulseX))
	{
		return false;
	}

	if (false == mpHSMeasure->mpMOTIONLib->mpDmcAxis[mpHSMeasure->mCardNo][mpHSMeasure->platformAxisNo]->moveAndCheck(nPulseY))
	{
		return false;
	}

	if (gapNo == GapC || gapNo == GapH)
	{
		Sleep(50);
	}

	return true;

}

void gapValuesProcess(float tValues[], int numIn)
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

bool BCDH_gap::getGapValue(const GAP_NO gapNo, float gapValue[])
{
	if (false == getCcdPos())
	{
		return false;
	}
	
	if (false == gapMove(gapNo))
	{
		return false;
	}
	
	QString tRecv;
	
	mpTcpClientCcd[0]->communicate("EXW," + QString("%1").arg(gapNo) + "\r", tRecv);
	mpTcpClientCcd[1]->communicate("EXW," + QString("%1").arg(gapNo) + "\r", tRecv);
	
	
	mpTcpClientCcd[0]->communicate("T1\r", tRecv);
	if (tRecv.left(1) != "T")	{ gapValue[SlotposNo1] = -1; }	else { gapValue[SlotposNo1] = tRecv.mid(3).toFloat(); }

	mpTcpClientCcd[1]->communicate("T1\r", tRecv);
	if (tRecv.left(1) != "T")	{ gapValue[SlotposNo3] = -1; }	else { gapValue[SlotposNo3] = tRecv.mid(3).toFloat(); }
	//
	mpTcpClientCcd[0]->communicate("T2\r", tRecv);
	if (tRecv.left(1) != "T")	{ gapValue[SlotposNo2] = -1; }	else { gapValue[SlotposNo2] = tRecv.mid(3).toFloat();}
	
	mpTcpClientCcd[1]->communicate("T2\r", tRecv);
	if (tRecv.left(1) != "T")	{ gapValue[SlotposNo4] = -1; }  else  { gapValue[SlotposNo4] = tRecv.mid(3).toFloat();}
	
	/*
	const int valueNum = 5;
	float tValues[SlotposNo4 + 1][valueNum];

	for (size_t i = 0; i < valueNum; i++)
	{
		mpTcpClientCcd[0]->communicate("T1\r", tRecv);
		tValues[SlotposNo1][i] = tRecv.mid(3).toFloat();
		mpTcpClientCcd[1]->communicate("T1\r", tRecv);
		tValues[SlotposNo3][i] = tRecv.mid(3).toFloat();
		//
		mpTcpClientCcd[0]->communicate("T2\r", tRecv);
		tValues[SlotposNo2][i] = tRecv.mid(3).toFloat();
		mpTcpClientCcd[1]->communicate("T2\r", tRecv);
		tValues[SlotposNo4][i] = tRecv.mid(3).toFloat();
	}

	for (size_t i = 0; i < SlotposNo4 + 1; i++)
	{
		gapValuesProcess(tValues[i], valueNum);
		gapValue[i] = tValues[i][0];
	}
	*/

	showMsgSignal(QString("..................NO.%1  X:%2  Y:%3").arg(gapNo).arg(arrayGapPos[gapNo].ccdPos).arg(arrayGapPos[gapNo].platPos));
	showMsgSignal(QString("GAP Value: %1, %2, %3, %4").arg(gapValue[SlotposNo1]).arg(gapValue[SlotposNo2]).arg(gapValue[SlotposNo3]).arg(gapValue[SlotposNo4]));

	return true;
}

bool BCDH_gap::getGapValueFront()
{
	for (; mCurStep < GapE + 1; mCurStep++)
	{

		float tValue[SlotposNo4 + 1] = { -1 };

		if (false == getGapValue((GAP_NO)mCurStep, tValue))
		{

			return false;
		}

		mpHSMeasure->gapValues[SlotposNo1][mCurStep] = tValue[SlotposNo1];
		mpHSMeasure->gapValues[SlotposNo2][mCurStep] = tValue[SlotposNo2];
		mpHSMeasure->gapValues[SlotposNo3][mCurStep] = tValue[SlotposNo3];
		mpHSMeasure->gapValues[SlotposNo4][mCurStep] = tValue[SlotposNo4];
	}

	return true;
}

bool BCDH_gap::getGapValueSide()
{
	for (; mCurStep < GapJ + 1; mCurStep++)
	{

		float tValue[SlotposNo4 + 1] = { -1 };

		if (false == getGapValue((GAP_NO)mCurStep, tValue))
		{
			return false;
		}

		mpHSMeasure->gapValues[SlotposNo1][mCurStep] = tValue[SlotposNo1];
		mpHSMeasure->gapValues[SlotposNo2][mCurStep] = tValue[SlotposNo2];
		mpHSMeasure->gapValues[SlotposNo3][mCurStep] = tValue[SlotposNo3];
		mpHSMeasure->gapValues[SlotposNo4][mCurStep] = tValue[SlotposNo4];
	}

	mCurStep = 0;
	return true;
}


