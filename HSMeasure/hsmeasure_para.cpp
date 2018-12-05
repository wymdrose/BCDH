#include "hsmeasure.h"
#include "bcdh_step.h"
#include "bcdh_gap.h"

extern BCDH_step* mpBCDH_step;
extern BCDH_gap* mpBCDH_gap;

void HSMeasure::hsmeasure_para()
{
	QString fileName;
	fileName = QCoreApplication::applicationDirPath();
	fileName += "/cfg/config.ini";

	QSettings settings(fileName, QSettings::IniFormat);
	QString str_path = settings.value("path_name/path").toString();

	ui.lineEditCcdIp1->setText(settings.value("CCD_CFG/IP1").toString());
	ui.lineEditCcdIp2->setText(settings.value("CCD_CFG/IP2").toString());

	ui.lineEditCcdUart1->setText(settings.value("CCD_CFG/UART1").toString());
	ui.lineEditCcdUart2->setText(settings.value("CCD_CFG/UART2").toString());
	
	ui.lineEditLaserIp1->setText(settings.value("LASER_CFG/IP1").toString());
	ui.lineEditLaserIp2->setText(settings.value("LASER_CFG/IP2").toString());
	ui.lineEditLaserIp3->setText(settings.value("LASER_CFG/IP3").toString());
	
	ui.lineEdit_laserBasePosition->setText(settings.value("POSITION/basePositionLaser").toString());
	ui.lineEdit_ccdBasePositionX1->setText(settings.value("POSITION/basePositionX1").toString());
	ui.lineEdit_ccdBasePositionY1->setText(settings.value("POSITION/basePositionY1").toString());
	ui.lineEdit_ccdBasePositionX2->setText(settings.value("POSITION/basePositionX2").toString());
	ui.lineEdit_ccdBasePositionY2->setText(settings.value("POSITION/basePositionY2").toString());

	//
	mpTcpClientCcd[0] = std::make_shared<_COMMUNICATECLASS::TCP_CLIENT>(ui.lineEditCcdIp1->text(), settings.value("CCD_CFG/PORT1").toInt());
	mpTcpClientCcd[1] = std::make_shared<_COMMUNICATECLASS::TCP_CLIENT>(ui.lineEditCcdIp2->text(), settings.value("CCD_CFG/PORT2").toInt());

	mpTcpClientLaser[0] = std::make_shared<_COMMUNICATECLASS::TCP_CLIENT>(ui.lineEditLaserIp1->text(), settings.value("LASER_CFG/PORT1").toInt());
	mpTcpClientLaser[1] = std::make_shared<_COMMUNICATECLASS::TCP_CLIENT>(ui.lineEditLaserIp2->text(), settings.value("LASER_CFG/PORT2").toInt());
	mpTcpClientLaser[2] = std::make_shared<_COMMUNICATECLASS::TCP_CLIENT>(ui.lineEditLaserIp3->text(), settings.value("LASER_CFG/PORT3").toInt());

	//
	mpSerialportCcd[0] = std::make_shared<_COMMUNICATECLASS::COM_PORT_ONE>(ui.lineEditCcdUart1->text().toInt(), settings.value("CCD_CFG/baudRate").toInt());
	mpSerialportCcd[1] = std::make_shared<_COMMUNICATECLASS::COM_PORT_ONE>(ui.lineEditCcdUart2->text().toInt(), settings.value("CCD_CFG/baudRate").toInt());

	//
	connect(ui.pushButtonCcdSendIp1, &QPushButton::clicked, [this](){
		//
		mpTcpClientCcd[0]->send(ui.textEditCcdSend->toPlainText() + "\r");
		mpTcpClientCcd[0]->getRec();
	
	});

	connect(ui.pushButtonCcdSendIp2, &QPushButton::clicked, [this](){
	
		mpTcpClientCcd[1]->send(ui.textEditCcdSend->toPlainText() + "\r");
		mpTcpClientCcd[1]->getRec();


	});
	
	connect(ui.pushButtonCcdSendUart1, &QPushButton::clicked, [this](){
		
		if (false == mpSerialportCcd[0]->open())
		{
			return;
		}
		mpSerialportCcd[0]->send(ui.textEditCcdSend->toPlainText() + "\r");
		mpSerialportCcd[0]->recData();
	});

	connect(ui.pushButtonCcdSendUart2, &QPushButton::clicked, [this](){
		
		mpSerialportCcd[1]->open();

		mpSerialportCcd[1]->send(ui.textEditCcdSend->toPlainText() + "\r");
		mpSerialportCcd[1]->recData();


	});
	
	//	
	connect(ui.pushButtonLaserSendIp1, &QPushButton::clicked, [this](){
		
		float buf1[LASER_VALUE_NUM];
		float buf2[LASER_VALUE_NUM];
		
		double raw_data1 [LASER_VALUE_NUM];
		double raw_data6[LASER_VALUE_NUM];
		double result1[10];
		double result6[10];

		uint sf1_start;
		uint sf1_end;
		uint sf2_start;
		uint sf2_end;
		float fResult;
		#ifndef GET_LASER_RESULT
		mpBCDH_step->GetTwoHeadData(mpBCDH_step->laser1_config, buf1, buf2, CHECKNUM);
		sf1_start = vWavePosition[1][1].toInt();
		sf1_end =	vWavePosition[1][2].toInt();
		sf2_start = vWavePosition[1][3].toInt();
		sf2_end =	vWavePosition[1][4].toInt();

		LALG mLALG;

		for (size_t i = 0; i < LASER_VALUE_NUM; i++)
		{
			raw_data1[i] = buf1[i];
		}

		mLALG.calculate_result(raw_data1, result1, sf1_start, sf1_end, sf2_start, sf2_end);


		sf1_start = vWavePosition[6][1].toInt();
		sf1_end =	vWavePosition[6][2].toInt();
		sf2_start = vWavePosition[6][3].toInt();
		sf2_end =	vWavePosition[6][4].toInt();

		for (size_t i = 0; i < LASER_VALUE_NUM; i++)
		{
			raw_data6[i] = buf2[i];
		}

		mLALG.calculate_result(raw_data6, result6, sf1_start, sf1_end, sf2_start, sf2_end);
		#else

		mpBCDH_step->get_laser_result(LJIF_PROFILETARGET_HEAD_A, mpBCDH_step->laser1_config, &fResult, 0, 0);
		result1[0] = fResult;
		mpBCDH_step->get_laser_result(LJIF_PROFILETARGET_HEAD_B, mpBCDH_step->laser1_config, &fResult, 0, 0);
		result6[0] = fResult;
		#endif
		qDebug() << "laser1: " << result1[0] << "    laser6: " << result6[0];
	});

	connect(ui.pushButtonLaserSendIp2, &QPushButton::clicked, [this](){
	
		float buf1[LASER_VALUE_NUM];
		float buf2[LASER_VALUE_NUM];

		double raw_data2[LASER_VALUE_NUM];
		double raw_data5[LASER_VALUE_NUM];
		double result2[10];
		double result5[10];
		
		uint sf1_start;
		uint sf1_end;
		uint sf2_start;
		uint sf2_end;
		float fResult;
		#ifndef GET_LASER_RESULT
		mpBCDH_step->GetTwoHeadData(mpBCDH_step->laser2_config, buf1, buf2, CHECKNUM);

		sf1_start = vWavePosition[2][1].toInt();
		sf1_end = vWavePosition[2][2].toInt();
		sf2_start = vWavePosition[2][3].toInt();
		sf2_end = vWavePosition[2][4].toInt();

		LALG mLALG;

		for (size_t i = 0; i < LASER_VALUE_NUM; i++)
		{
			raw_data2[i] = buf1[i];
		}

		mLALG.calculate_result(raw_data2, result2, sf1_start, sf1_end, sf2_start, sf2_end);
		
		
		sf1_start = vWavePosition[5][1].toInt();
		sf1_end = vWavePosition[5][2].toInt();
		sf2_start = vWavePosition[5][3].toInt();
		sf2_end = vWavePosition[5][4].toInt();

		for (size_t i = 0; i < LASER_VALUE_NUM; i++)
		{
			raw_data5[i] = buf2[i];
		}

		mLALG.calculate_result(raw_data5, result5, sf1_start, sf1_end, sf2_start, sf2_end);
	#else

	mpBCDH_step->get_laser_result(LJIF_PROFILETARGET_HEAD_A, mpBCDH_step->laser2_config, &fResult, 0, 0);
	result2[0] = fResult;
	mpBCDH_step->get_laser_result(LJIF_PROFILETARGET_HEAD_B, mpBCDH_step->laser2_config, &fResult, 0, 0);
	result5[0] = fResult;
	#endif
		qDebug() << "laser2: " << result2[0] << "    laser5: " << result5[0];

	});

	connect(ui.pushButtonLaserSendIp3, &QPushButton::clicked, [this](){
		
		float buf1[LASER_VALUE_NUM];
		float buf2[LASER_VALUE_NUM];

		double raw_data3[LASER_VALUE_NUM];
		double raw_data4[LASER_VALUE_NUM];
		double result3[10];
		double result4[10];

		uint sf1_start;
		uint sf1_end;
		uint sf2_start;
		uint sf2_end;
		float fResult;
		#ifndef GET_LASER_RESULT
		mpBCDH_step->GetTwoHeadData(mpBCDH_step->laser3_config, buf1, buf2, CHECKNUM);

		sf1_start = vWavePosition[3][1].toInt();
		sf1_end =	vWavePosition[3][2].toInt();
		sf2_start = vWavePosition[3][3].toInt();
		sf2_end =	vWavePosition[3][4].toInt();


		for (size_t i = 0; i < LASER_VALUE_NUM; i++)
		{
			raw_data3[i] = buf1[i];
		}

		LALG mLALG;

		mLALG.calculate_result(raw_data3, result3, sf1_start, sf1_end, sf2_start, sf2_end);


		sf1_start = vWavePosition[4][1].toInt();
		sf1_end =	vWavePosition[4][2].toInt();
		sf2_start = vWavePosition[4][3].toInt();
		sf2_end =	vWavePosition[4][4].toInt();

		for (size_t i = 0; i < LASER_VALUE_NUM; i++)
		{
			raw_data4[i] = buf2[i];
		}

		mLALG.calculate_result(raw_data4, result4, sf1_start, sf1_end, sf2_start, sf2_end);
#else

		mpBCDH_step->get_laser_result(LJIF_PROFILETARGET_HEAD_A, mpBCDH_step->laser3_config, &fResult, 0, 0);
		result3[0] = fResult;
		mpBCDH_step->get_laser_result(LJIF_PROFILETARGET_HEAD_B, mpBCDH_step->laser3_config, &fResult, 0, 0);
		result4[0] = fResult;
#endif
		qDebug() << "laser3: " << result3[0] << "    laser4: " << result4[0];

	});
	//
	connect(ui.pushButtonPjogCcd, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition();

		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setMovePara(speedFromIni.ccdJogSpeed, ABSOLUTE_MOTION);

		mpMOTIONLib->move(mCardNo, ccdAxisNo, curPosition + speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));

	});

	connect(ui.pushButtonNjogCcd, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition();

		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setMovePara(speedFromIni.ccdJogSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, ccdAxisNo, curPosition - speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));

	});

	connect(ui.pushButtonMoveCcd, &QPushButton::clicked, [this](){

		auto nPulse = ui.lineEditTargetPositionCcd->text().toInt();
		
		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setMovePara(speedFromIni.ccdAutoSpeed, ABSOLUTE_MOTION);
		
		mpMOTIONLib->move(mCardNo, ccdAxisNo, nPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));
		}	

	});
	
	connect(ui.pushButtonHomeCcd, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setHomePara(1, speedFromIni.ccdHomeSpeed);
		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->home();

		DWORD tSystemTime = GetTickCount();

		ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));

		if (false == mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->checkHomeDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			QMessageBox::warning(this, "", "Failed");
			return;
		}

		QMessageBox::information(this, "", "OK");
		
	});

	connect(ui.pushButtonPoweronCcd, &QPushButton::clicked, [this](){
	
		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	});
	
	connect(ui.pushButtonPoweroffCcd, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, ccdAxisNo, 0);
		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	});

	connect(ui.pushButtonStopCcd, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, ccdAxisNo, 0);
	});
	

	//LASER
	connect(ui.pushButtonPjogLaser, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition();

		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(speedFromIni.laserJogSpeed, ABSOLUTE_MOTION);

		mpMOTIONLib->move(mCardNo, laserAxisNo, curPosition + speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));

	});

	connect(ui.pushButtonNjogLaser, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition();

		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(speedFromIni.laserJogSpeed, ABSOLUTE_MOTION);
		
		mpMOTIONLib->move(mCardNo, laserAxisNo, curPosition - speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));

	});

	connect(ui.pushButtonMoveLaser, &QPushButton::clicked, [this](){

		if (IO_ON == cylinderCheck(CLAMP2_I))
		{
			QMessageBox::warning(this,"","STEP CLAM");
			return;
		}

		if (mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition() < 0)
		{
			QMessageBox::warning(this, "", "Y1 SAFE?");
			return;
		}

		if (mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition() > posionFromIni.ngPositionY2)
		{
			QMessageBox::warning(this, "", "Y2 SAFE?");
			return;
		}

		auto nPulse = ui.lineEditTargetPositionLaser->text().toInt();

		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(speedFromIni.laserAutoSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, laserAxisNo, nPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));
		}
	});

	connect(ui.pushButtonHomeLaser, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setHomePara(1,speedFromIni.laserHomeSpeed);
		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->home();

		DWORD tSystemTime = GetTickCount();

		ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));

		if (false == mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->checkHomeDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			QMessageBox::warning(this, "", "Failed");
			return;
		}

		QMessageBox::information(this, "", "OK");
	});

	connect(ui.pushButtonPoweronLaser, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	});

	connect(ui.pushButtonPoweroffLaser, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, laserAxisNo, 0);
		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	});

	connect(ui.pushButtonStopLaser, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, laserAxisNo, 0);
	});

	//platform Y1
	connect(ui.pushButtonPjogPlatform, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition();
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setMovePara(speedFromIni.platJogSpeed, ABSOLUTE_MOTION);

		mpMOTIONLib->move(mCardNo, platformAxisNo, curPosition + speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));

	});

	connect(ui.pushButtonNjogPlatform, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition();
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setMovePara(speedFromIni.platJogSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, platformAxisNo, curPosition - speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));

	});

	connect(ui.pushButtonMovePlatform, &QPushButton::clicked, [this](){

		auto nPulse = ui.lineEditTargetPositionPlatform->text().toInt();

		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setMovePara(speedFromIni.platAutoSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, platformAxisNo, nPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));
		}
	});

	connect(ui.pushButtonHomePlatform, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setHomePara(0, speedFromIni.platHomeSpeed);
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->home();
		DWORD tSystemTime = GetTickCount();

		ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));

		if (false == mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->checkHomeDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			QMessageBox::warning(this, "", "Failed");
			return;
		}
		
		QMessageBox::information(this, "", "OK");

	});

	connect(ui.pushButtonPoweronPlatform, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	});

	connect(ui.pushButtonPoweroffPlatform, &QPushButton::clicked, [this](){
	
		dmc_stop(mCardNo, platformAxisNo, 0);
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	});

	connect(ui.pushButtonStopY1, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, platformAxisNo, 0);
	});

	//ROTATE NG Æ¤´øÏß
	connect(ui.pushButtonPjogRotate, &QPushButton::pressed, [this](){
		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition();
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setMovePara(speedFromIni.rotateJogSpeed, ABSOLUTE_MOTION);

		mpMOTIONLib->move(mCardNo, rotateAxisNo, curPosition + speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));

	});

	connect(ui.pushButtonNjogRotate, &QPushButton::pressed, [this](){

		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition();
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setMovePara(speedFromIni.rotateJogSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, rotateAxisNo, curPosition - speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));

	});

	connect(ui.pushButtonMoveRotate, &QPushButton::clicked, [this](){

		auto nPulse = ui.lineEditTargetPositionRotate->text().toInt();

		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setMovePara(speedFromIni.rotateJogSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, rotateAxisNo, nPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));
		}
	});

	connect(ui.pushButtonHomeRotate, &QPushButton::clicked, [this](){

	//	dmc_set_el_home(mCardNo, rotateAxisNo, 0);
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setHomePara(0, speedFromIni.rotateHomeSpeed);
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->home();
		DWORD tSystemTime = GetTickCount();

		if (false == mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->checkHomeDone(tSystemTime, DEFAUL_HOME_TIME_OUT))
		{
			ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));
		}

		QMessageBox::information(this, "", "OK");
	});

	connect(ui.pushButtonPoweronRotate, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	});

	connect(ui.pushButtonPoweroffRotate, &QPushButton::clicked, [this](){
		
		dmc_stop(mCardNo, rotateAxisNo, 0);
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	});

	connect(ui.pushButtonStopX3, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, rotateAxisNo, 0);
	});

	//UNLOAD Y2
	connect(ui.pushButtonPjogUnload, &QPushButton::pressed, [this](){
		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition();
		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setMovePara(speedFromIni.unloadJogSpeed, ABSOLUTE_MOTION);

		mpMOTIONLib->move(mCardNo, unloadAxisNo, curPosition + speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));

	});

	connect(ui.pushButtonNjogUnload, &QPushButton::pressed, [this](){
		long curPosition = mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition();

		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setMovePara(speedFromIni.unloadJogSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, unloadAxisNo, curPosition - speedFromIni.jogPulse);

		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));
		}

		ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));

	});

	connect(ui.pushButtonMoveUnload, &QPushButton::clicked, [this](){

		auto nPulse = ui.lineEditTargetPositionUnload->text().toInt();

		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setMovePara(speedFromIni.unloadAutoSpeed, ABSOLUTE_MOTION);
		mpMOTIONLib->move(mCardNo, unloadAxisNo, nPulse);
		DWORD tSystemTime = GetTickCount();

		while (1 == mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->checkDone(tSystemTime, DEFAUL_MOVE_TIME_OUT))
		{
			ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));
		}
	});

	connect(ui.pushButtonHomeUnload, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setHomePara(1, speedFromIni.unloadHomeSpeed);
		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->home();
		
		DWORD tSystemTime = GetTickCount();

		if (false == mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->checkHomeDone(tSystemTime, DEFAUL_HOME_TIME_OUT))
		{
			ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));
		}

		QMessageBox::information(this, "", "OK");
	});

	connect(ui.pushButtonPoweronUnload, &QPushButton::clicked, [this](){

		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	});

	connect(ui.pushButtonPoweroffUnload, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, unloadAxisNo, 0);
		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	});

	connect(ui.pushButtonStopY2, &QPushButton::clicked, [this](){

		dmc_stop(mCardNo, unloadAxisNo, 0);
	});


	//
	//
	connect(ui.pushButton_LightOn, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 3, 1, flag);

		if (flag == 0)
			ui.pushButton_LightOn->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButton_LightOn->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButton_vacuum1_2, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_outbit(mCardNo, 11, flag);
		dmc_write_can_outbit(mCardNo, 1, 0, flag);
	});

	connect(ui.pushButton_vacuum3_4, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 1, flag);
		dmc_write_can_outbit(mCardNo, 1, 2, flag);
	});

	connect(ui.pushButton_vacuum5, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 7, flag);
		
	});

	connect(ui.pushButton_vacuum6, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 8, flag);

	});

	connect(ui.pushButton_vacuum7, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 9, flag);

	});

	connect(ui.pushButton_vacuum8, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 10, flag);

	});

	connect(ui.pushButton_vacuum9, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 5, flag);

	});

	connect(ui.pushButton_vacuum10, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 6, flag);

	});

	connect(ui.pushButton_vacuum11, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 7, flag);

	});

	connect(ui.pushButton_vacuum12, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 8, flag);

	});

	
	connect(ui.pushButton_gapClamp, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 3, flag);

		if (flag == 0)
			ui.pushButton_gapClamp->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButton_gapClamp->setStyleSheet("QPushButton{background:}");

	});

	connect(ui.pushButton_gapRotate, &QPushButton::clicked, [this](){
		
		if (IO_OFF == cylinderCheck(CLAMP1_o))
		{
			QMessageBox::warning(this, "", "GAP CLAM");
			return;
		}

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 15, flag);
		dmc_write_can_outbit(mCardNo, 3, 0, !flag);

	});

	connect(ui.pushButton_stepClamp, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 11, flag);

		if (flag == 1)
			ui.pushButton_stepClamp->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButton_stepClamp->setStyleSheet("QPushButton{background:}");
	});


	connect(ui.pushButtonCylinderUp1_2, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_outbit(mCardNo, 9, !flag);
		dmc_write_outbit(mCardNo, 10, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp1_2->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp1_2->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp3_4, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 3, 2, !flag);
		dmc_write_can_outbit(mCardNo, 3, 3, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp3_4->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp3_4->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp5_6, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 5, !flag);
		dmc_write_can_outbit(mCardNo, 1, 6, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp5_6->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp5_6->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp7_8, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 3, 4, !flag);
		dmc_write_can_outbit(mCardNo, 3, 5, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp7_8->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp7_8->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp9, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 13, !flag);
		dmc_write_can_outbit(mCardNo, 1, 14, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp9->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp9->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp10, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 1, 15, !flag);
		dmc_write_can_outbit(mCardNo, 2, 0, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp10->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp10->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp11, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 1, !flag);
		dmc_write_can_outbit(mCardNo, 2, 2, flag);

		if (flag == 0)
			ui.pushButtonCylinderUp11->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp11->setStyleSheet("QPushButton{background:}");
	});

	connect(ui.pushButtonCylinderUp12, &QPushButton::clicked, [this](){

		static BOOL flag = TRUE;	
		flag = !flag;

		dmc_write_can_outbit(mCardNo, 2, 3, !flag);
		dmc_write_can_outbit(mCardNo, 2, 4, flag);


		if (flag == 0)
			ui.pushButtonCylinderUp12->setStyleSheet("QPushButton{background:lightgreen}");
		else
			ui.pushButtonCylinderUp12->setStyleSheet("QPushButton{background:}");
	});
	

	connect(ui.pushButton_laserSafePosition, &QPushButton::clicked, [this](){

		mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheck(posionFromIni.LaserSafePosition);

	});

	connect(ui.pushButton_loadPositionY1, &QPushButton::clicked, [this](){

		if (abs(mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()) > abs(posionFromIni.LaserSafePosition))
		{
			QMessageBox::warning(this, "", "Laser Safe ?");
			return;
		}

		mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->moveAndCheck(posionFromIni.loadPositionY1);

	});

	connect(ui.pushButton_unloadPositionY1, &QPushButton::clicked, [this](){

		if (abs(mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()) > abs(posionFromIni.LaserSafePosition))
		{
			QMessageBox::warning(this, "", "Laser Safe ?");
			return;
		}

		mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->moveAndCheck(posionFromIni.unloadPositionY1);
	});
	
	connect(ui.pushButton_loadPositionY2, &QPushButton::clicked, [this](){

		if (abs(mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()) > abs(posionFromIni.LaserSafePosition))
		{
			QMessageBox::warning(this, "", "Laser Safe ?");
			return;
		}

		mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.loadPositionY2);
	});
	
	connect(ui.pushButton_ngPositionY2, &QPushButton::clicked, [this](){
		
		if (abs(mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()) > abs(posionFromIni.LaserSafePosition))
		{
			QMessageBox::warning(this, "", "Laser Safe ?");
			return;
		}

		mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.ngPositionY2);

	});

	connect(ui.pushButton_okPositionY2, &QPushButton::clicked, [this](){
		
		if (abs(mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()) > abs(posionFromIni.LaserSafePosition))
		{
			QMessageBox::warning(this, "", "Laser Safe ?");
			return;
		}

		mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.okPositionY2);

	});
	
	ui.comboBox_laserPos->addItem("1");
	ui.comboBox_laserPos->addItem("2");
	ui.comboBox_laserPos->addItem("3");
	ui.comboBox_laserPos->addItem("4");
	ui.comboBox_laserPos->addItem("5");
	ui.comboBox_laserPos->addItem("6");
	ui.comboBox_laserPos->addItem("7");
	ui.comboBox_laserPos->addItem("8");
	ui.comboBox_laserPos->addItem("9");
	ui.comboBox_laserPos->addItem("10");
	ui.comboBox_laserPos->addItem("11");

	ui.comboBox_ccdPos->addItem("A");
	ui.comboBox_ccdPos->addItem("B");
	ui.comboBox_ccdPos->addItem("C");
	ui.comboBox_ccdPos->addItem("D");
	ui.comboBox_ccdPos->addItem("E");
	ui.comboBox_ccdPos->addItem("F");
	ui.comboBox_ccdPos->addItem("G");
	ui.comboBox_ccdPos->addItem("H");
	ui.comboBox_ccdPos->addItem("I");
	ui.comboBox_ccdPos->addItem("J");


	/*
	, laserPos, platPos
		pos1, -529000, 0     		pos1, -504000, 0
		pos2, -469000, 1			pos2, -444000, 0
		pos3, -444000, 2			pos3, -384000, 0
		pos4, -384000, 3			pos4, -359000, 0
		pos5, -359000, 4			pos5, -299000, 0
		pos6, -299000, 5			pos6, -274000, 0
		pos7, -274000, 6			pos7, -214000, 0
		pos8, -214000, 7			pos8, -189000, 0
		pos9, -189000, 0			pos9, -129000, 0
		pos10, -129000, 0			pos10, -104000, 0
		pos11, -104000, 0			pos11, -79000, 0
		*/
	/*
		ui.lineEdit_laserBasePosition->setText("-529000");
	*/
	connect(ui.pushButton_laserPosMove, &QPushButton::clicked, [this](){
		
		if (IO_ON == cylinderCheck(CLAMP2_I))
		{
			QMessageBox::warning(this, "", "STEP CLAM");
			return;
		}
		
		int basePosition = ui.lineEdit_laserBasePosition->text().toInt();
	
		int index = ui.comboBox_laserPos->currentIndex();
		
		int spacePosition = (index + 1) / 2 * 60000 + (index) / 2 * 25000 ;
		
		mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheck(basePosition + spacePosition);
		
	});
	
	connect(ui.pushButton_ccdPosMove, &QPushButton::clicked, [this](){

		ui.pushButton_ccdPosMove->setEnabled(false);

		int index = ui.comboBox_ccdPos->currentIndex();

		while (true)
		{
			QApplication::processEvents();

			if (true == mpBCDH_gap->gapMove(static_cast<GAP_NO>(index)))
			{
				break;
			}
			
		}				

		ui.pushButton_ccdPosMove->setEnabled(true);

	});

	connect(ui.pushButton_stepGet, &QPushButton::clicked, [this](){

		if (IO_ON == cylinderCheck(CLAMP2_I))
		{
			QMessageBox::warning(this, "", "STEP CLAM");
			return;
		}

		ui.pushButton_stepGet->setEnabled(false);
	
		while (true)
		{
			QApplication::processEvents();

			if (true == mpBCDH_step->getStepValue())
			{
				break;
			}
			
			ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));	
		}

		saveStepToLog(stepValues);

		mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheck(0);

		ui.pushButton_stepGet->setEnabled(true);
	});
	
	connect(ui.pushButton_gapGet, &QPushButton::clicked, [this](){

		ui.pushButton_gapGet->setEnabled(false);

		while (true)
		{
			ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));
			ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));

			QApplication::processEvents();

			if (false == mpBCDH_gap->getGapValueFront())
			{
				continue;
			}
		
			break;
		}

		dmc_write_can_outbit(mCardNo, 2, 15, 0);
		dmc_write_can_outbit(mCardNo, 3, 0, 1);

		Sleep(500);

		while (true)
		{
			ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));
			ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));

			QApplication::processEvents();

			if (false == mpBCDH_gap->getGapValueSide())
			{
				continue;
			}

			break;
		}

		dmc_write_can_outbit(mCardNo, 2, 15, 1);
		dmc_write_can_outbit(mCardNo, 3, 0, 0);

		saveGapToLog(gapValues);

		ui.pushButton_gapGet->setEnabled(true);

	});

	ui.comboBox_speed->addItem("1");
	ui.comboBox_speed->addItem("0.5");
	ui.comboBox_speed->addItem("2");


	connect(ui.pushButton_speedSet, &QPushButton::clicked, [this](){

		float SpeedPercen = ui.comboBox_speed->currentText().toFloat();

		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setMovePara(SpeedPercent * speedFromIni.ccdAutoSpeed, ABSOLUTE_MOTION);		//X1
		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(SpeedPercent * speedFromIni.laserAutoSpeed, ABSOLUTE_MOTION);	//X2
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setMovePara(SpeedPercent * speedFromIni.rotateAutoSpeed, ABSOLUTE_MOTION);	//X3
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setMovePara(SpeedPercent * speedFromIni.platAutoSpeed, ABSOLUTE_MOTION);	//Y1
		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setMovePara(SpeedPercent * speedFromIni.unloadAutoSpeed, ABSOLUTE_MOTION);	//Y2


	});


	connect(ui.pushButtonOut, &QPushButton::clicked, [this](){
		
		ui.pushButtonOut->setEnabled(false);

		cylinderMove(UPDOWN_B_o);
		cylinderMove(UPDOWN_C_1o);
		cylinderMove(UPDOWN_C_2o);
		cylinderMove(UPDOWN_C_3o);
		cylinderMove(UPDOWN_C_4o);

		if (QMessageBox::question(this, "", "cylinder") == QMessageBox::No)
		{
			goto END;
		}

		if (!mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheckdone(posionFromIni.LaserSafePosition, DEFAUL_MOVE_TIME_OUT))
		{
			goto END;
		}
	
		if (QMessageBox::question(this, "", "LaserSafePosition") == QMessageBox::No)
		{
			goto END;
		}

		if (!mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheckdone(posionFromIni.loadPositionY2, DEFAUL_MOVE_TIME_OUT))
		{
			goto END;
		}
			
		if (QMessageBox::question(this, "", "loadPositionY2") == QMessageBox::No)
		{
			goto END;
		}

		stepFiberFlag[0] = sensorIn(Fiber5);
		stepFiberFlag[1] = sensorIn(Fiber6);
		stepFiberFlag[2] = sensorIn(Fiber7);
		stepFiberFlag[3] = sensorIn(Fiber8);

		if (IO_ON == stepFiberFlag[SlotposNo1])	cylinderMove(UPDOWN_C_1I);
		if (IO_ON == stepFiberFlag[SlotposNo2])	cylinderMove(UPDOWN_C_2I);
		if (IO_ON == stepFiberFlag[SlotposNo3])	cylinderMove(UPDOWN_C_3I);
		if (IO_ON == stepFiberFlag[SlotposNo4])	cylinderMove(UPDOWN_C_4I);

		Sleep(1000);

		if (IO_ON == stepFiberFlag[SlotposNo1])	{ sensorOut(Vacuum9, IO_ON);	}
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ sensorOut(Vacuum10, IO_ON);	}
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ sensorOut(Vacuum11, IO_ON);	}
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ sensorOut(Vacuum12, IO_ON);	}
	
		Sleep(1000);

		cylinderMove(CLAMP2_o);

		Sleep(1000);


		cylinderMove(UPDOWN_C_1o);
		cylinderMove(UPDOWN_C_2o);
		cylinderMove(UPDOWN_C_3o);
		cylinderMove(UPDOWN_C_4o);
		
		Sleep(1000);

		if (!mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheckdone(posionFromIni.ngPositionY2, DEFAUL_MOVE_TIME_OUT))
		{
			goto END;
		}

		if (QMessageBox::question(this, "", "ngPositionY2") == QMessageBox::No)
		{
			goto END;
		}

		if (IO_ON == stepFiberFlag[SlotposNo1])	{ cylinderMove(UPDOWN_C_1I); }
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ cylinderMove(UPDOWN_C_2I); }
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ cylinderMove(UPDOWN_C_3I); }
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ cylinderMove(UPDOWN_C_4I); }

		Sleep(1000);

		if (IO_ON == stepFiberFlag[SlotposNo1])	{ sensorOut(Vacuum9, IO_OFF); }
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ sensorOut(Vacuum10, IO_OFF);}
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ sensorOut(Vacuum11, IO_OFF);}
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ sensorOut(Vacuum12, IO_OFF);}

		Sleep(1000);

		cylinderMove(UPDOWN_C_1o);
		cylinderMove(UPDOWN_C_2o);
		cylinderMove(UPDOWN_C_3o);
		cylinderMove(UPDOWN_C_4o);	

		if (!mpMOTIONLib->mpDmcAxis[mCardNo][rotateAxisNo]->moveAndCheckdone(35000, DEFAUL_MOVE_TIME_OUT))
		{
			goto END;
		}

		if (QMessageBox::question(this, "", "rotateAxisNo") == QMessageBox::No)
		{
			goto END;
		}

		if (!mpMOTIONLib->mpDmcAxis[mCardNo][rotateAxisNo]->moveAndCheckdone(posionFromIni.ngBreakPositionHome, DEFAUL_MOVE_TIME_OUT))
		{
			goto END;
		}
		

		QMessageBox::information(this, "", "OK");

	END:

		QMessageBox::information(this,"","Failed");

		ui.pushButtonOut->setEnabled(true);
	});
	



}



