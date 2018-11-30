#ifndef BCDH_STEP_H
#define BCDH_STEP_H

#include <QObject>
#include <array>
#include "hsmeasure.h"
#include <qstringlist.h>
#include "lalg.h"

#define CHECKNUM 5
#define GET_LASER_RESULT
class BCDH_step : public QObject
{
	Q_OBJECT

public:
	BCDH_step(HSMeasure* pHSMeasure);
	~BCDH_step();

	const int LSCOUNT = 800;
	QStringList POS_STEP;
	struct STEP_MAP
	{
		LASER_AXIS_POS laserPos;
		LASER_NO laserNo;
	};

	LJIF_OPENPARAM_ETHERNET laser1_config;
	LJIF_OPENPARAM_ETHERNET laser2_config;
	LJIF_OPENPARAM_ETHERNET laser3_config;

	void BCDH_step::GetOneHeadData(LJIF_PROFILETARGET head, LJIF_OPENPARAM_ETHERNET config, float* buf, uint num);
	void BCDH_step::GetTwoHeadData(LJIF_OPENPARAM_ETHERNET config, float* buf1, float* buf2, uint num);
	void BCDH_step::ShowStepValueToView();
	void get_laser_result(LJIF_PROFILETARGET head, LJIF_OPENPARAM_ETHERNET config, float* result, int program, int nStep);
	static const STEP_MAP arrayStepMap[(SlotposNo4 + 1) * (StepJ + 1)];

	bool getPosFromCfg();
	bool getStepValue(QString, float[]);
	bool getStepValue();
signals:
	void showMsgSignal(const QString&);
	void laserGetSignal(int, float*);

private:
	CLoadLJDllFunc * m_pLoadDllfunc;

	float posLaserAxis[laserPos11 + 1];
	float posPlat;

	int mCurStep = 0;
	HSMeasure* mpHSMeasure;

};


#endif // BCDH_STEP_H
