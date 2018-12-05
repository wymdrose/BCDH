
#include "hsmeasure.h"
#include "bcdh_gap.h"
#include "bcdh_step.h"
#include "dog.h"

QString mPath;
BCDH_gap* mpBCDH_gap;
BCDH_step* mpBCDH_step;

HSMeasure::HSMeasure(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	//
	this->setWindowTitle(QStringLiteral("大族激光IT装备DH测量软件 v1.0.0"));
	init();
	initValue();
	load_ini();
	//
	mPath = QCoreApplication::applicationDirPath();

	initDmcCard();
	login();
	hsmeasure_para();

	//	
	pDogTimer = new QTimer(this);
	connect(pDogTimer, SIGNAL(timeout()), this, SLOT(onDogTimer()));
	pDogTimer->start(1000 * 60 * 5);
	
	//
	pIoTimer = new QTimer(this);
	connect(pIoTimer, SIGNAL(timeout()), this, SLOT(onIoTimer()));
	pIoTimer->start(300);

	pValShowTimer = new QTimer(this); //show wave to view
	connect(pValShowTimer, SIGNAL(timeout()), this, SLOT(onToValShowTimer()));
	
	//
	mpBCDH_gap = new BCDH_gap(this);
	mpBCDH_step = new BCDH_step(this);

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

	m_pLoadDllfunc = CLoadLJDllFunc::GetInstance();

	connect(this, SIGNAL(colorSignal(QPushButton*, const QString)), this, SLOT(colorSlot(QPushButton*, const QString)));
	connect(this, SIGNAL(colorSignal(QCheckBox*, const QString)), this, SLOT(colorSlot(QCheckBox*, const QString)));
	connect(this, SIGNAL(ableSignal(QPushButton*, bool)), this, SLOT(ableSlot(QPushButton*, bool)));
	connect(this, SIGNAL(showMsgSignal(const QString&)), this, SLOT(show_msg(const QString &)));
	connect(this, SIGNAL(showDialogSignal(const QString&)), this, SLOT(show_dialog(const QString &)), Qt::BlockingQueuedConnection);

}

HSMeasure::~HSMeasure()
{
	delete mpBCDH_gap;
	delete mpBCDH_step;

	delete mpDMC5000Lib;
}

void HSMeasure::init()
{
//	this->setFixedSize(985, 650);
//	ui.tabWidget->resize(1085, 640);
	ui.tabWidget->setTabText(0, QStringLiteral("主界面"));
	ui.tabWidget->setTabText(1, QStringLiteral("系统参数"));
	ui.tabWidget->setTabText(2, QStringLiteral("I/O主板"));
	ui.tabWidget->setTabText(3, QStringLiteral("I/O扩展"));
	ui.tabWidget->setCurrentIndex(0);
	ui.tabWidget->setStyleSheet("QTabWidget:pane{border-top:0px solid #e8f3f9;background:#e8f3f9;}");
	ui.tabWidget->setStyleSheet("QTabBar:tab{height:36px };");

	ui.tabWidget->setTabEnabled(0, true);
//	ui.tabWidget->setTabEnabled(1, false);
//	ui.tabWidget->setTabEnabled(2, false);
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabWidgetChanged(int)));

	//默认启用4个槽位
	ui.checkBox_cave1->setCheckState(Qt::Checked);
	ui.checkBox_cave2->setCheckState(Qt::Checked);
	ui.checkBox_cave3->setCheckState(Qt::Checked);
	ui.checkBox_cave4->setCheckState(Qt::Checked);

	//默认产品国别 
	ui.comboBoxProduct->addItem(QStringLiteral("美国"));
	ui.comboBoxProduct->addItem(QStringLiteral("中国"));
	ui.comboBoxProduct->addItem(QStringLiteral("英国"));

	//产线编号  1....20
	ui.comboBoxLine->addItem(QStringLiteral("Line_1"));
	ui.comboBoxLine->addItem(QStringLiteral("Line_2"));
	ui.comboBoxLine->addItem(QStringLiteral("Line_3"));


	//
	//默认权限 操作员
	ui.comboBoxPower->addItem(QStringLiteral("操作员"));
	ui.comboBoxPower->addItem(QStringLiteral("技术员"));
	ui.comboBoxPower->addItem(QStringLiteral("工程师"));
	
	
	//table 1  此规格OK产品对应的上、下限值
	QStandardItemModel *model_product = new QStandardItemModel(this);
	model_product->setColumnCount(3);
	model_product->setHeaderData(0, Qt::Horizontal, QStringLiteral("检测项目"));
	model_product->setHeaderData(1, Qt::Horizontal, QStringLiteral("下限值"));
	model_product->setHeaderData(2, Qt::Horizontal, QStringLiteral("上限值"));
	ui.productTableView->setModel(model_product);
	ui.productTableView->setColumnWidth(0, 119);
	ui.productTableView->setColumnWidth(1, 120);
	ui.productTableView->setColumnWidth(2, 120);
	model_product->setItem(0, 0, new QStandardItem(QStringLiteral("缝隙（Gap）")));
	model_product->setItem(1, 0, new QStandardItem(QStringLiteral("段差（Step）")));
	model_product->setItem(0, 1, new QStandardItem(QStringLiteral("0.000mm")));
	model_product->setItem(1, 1, new QStandardItem(QStringLiteral("-0.050mm")));
	model_product->setItem(0, 2, new QStandardItem(QStringLiteral("0.080mm")));
	model_product->setItem(1, 2, new QStandardItem(QStringLiteral("0.110mm")));
	ui.productTableView->verticalHeader()->hide();

	//table 2  检测值
	model_result = new QStandardItemModel(this);
	model_result->setColumnCount(25);
	model_result->setHeaderData(0, Qt::Horizontal, QStringLiteral("Gap_A"));
	model_result->setHeaderData(1, Qt::Horizontal, QStringLiteral("Gap_B"));
	model_result->setHeaderData(2, Qt::Horizontal, QStringLiteral("Gap_C"));
	model_result->setHeaderData(3, Qt::Horizontal, QStringLiteral("Gap_D"));
	model_result->setHeaderData(4, Qt::Horizontal, QStringLiteral("Gap_E"));
	model_result->setHeaderData(5, Qt::Horizontal, QStringLiteral("Gap_F"));
	model_result->setHeaderData(6, Qt::Horizontal, QStringLiteral("Gap_G"));
	model_result->setHeaderData(7, Qt::Horizontal, QStringLiteral("Gap_H"));
	model_result->setHeaderData(8, Qt::Horizontal, QStringLiteral("Gap_I"));
	model_result->setHeaderData(9, Qt::Horizontal, QStringLiteral("Gap_J"));

	model_result->setHeaderData(10, Qt::Horizontal, QStringLiteral("Step_A"));
	model_result->setHeaderData(11, Qt::Horizontal, QStringLiteral("Step_B"));
	model_result->setHeaderData(12, Qt::Horizontal, QStringLiteral("Step_C"));
	model_result->setHeaderData(13, Qt::Horizontal, QStringLiteral("Step_D"));
	model_result->setHeaderData(14, Qt::Horizontal, QStringLiteral("Step_E"));
	model_result->setHeaderData(15, Qt::Horizontal, QStringLiteral("Step_F"));
	model_result->setHeaderData(16, Qt::Horizontal, QStringLiteral("Step_G"));
	model_result->setHeaderData(17, Qt::Horizontal, QStringLiteral("Step_H"));
	model_result->setHeaderData(18, Qt::Horizontal, QStringLiteral("Step_I"));
	model_result->setHeaderData(19, Qt::Horizontal, QStringLiteral("Step_J"));
	model_result->setHeaderData(20, Qt::Horizontal, QStringLiteral("Result"));
	model_result->setHeaderData(21, Qt::Horizontal, QStringLiteral("Cave_NO"));
	model_result->setHeaderData(22, Qt::Horizontal, QStringLiteral("Line_NO"));
	model_result->setHeaderData(23, Qt::Horizontal, QStringLiteral("Nation"));
	model_result->setHeaderData(24, Qt::Horizontal, QStringLiteral("DateTime"));
	ui.tableViewResult->setModel(model_result);
	ui.tableViewResult->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tableViewResult->verticalHeader()->setDefaultSectionSize(18);
	ui.tableViewResult->setAlternatingRowColors(true); 
	ui.tableViewResult->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
	
	/*
	QString para = "0.0823";
	model_result->setItem(0, 1, new QStandardItem(para));
	model_result->setItem(1, 1, new QStandardItem(para));
	model_result->setItem(2, 1, new QStandardItem(para));
	model_result->setItem(3, 1, new QStandardItem(para));
	model_result->setItem(4, 1, new QStandardItem(para));
	model_result->setItem(5, 1, new QStandardItem(para));
	model_result->setItem(6, 1, new QStandardItem(para));
	model_result->setItem(7, 1, new QStandardItem(para));
	*/

	//io
	initUiIo();
	
	//
	connect(ui.pushButtonSaveLog, SIGNAL(clicked()), this, SLOT(save_log()));
	connect(ui.pushButtonStart, SIGNAL(clicked()), this, SLOT(hs_start()));
	connect(ui.pushButtonPause, SIGNAL(clicked()), this, SLOT(hs_pause()));
	connect(ui.pushButtonZero, SIGNAL(clicked()), this, SLOT(hs_zero()));
	connect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(hs_stop()));
	connect(ui.pushButtonRestAlm, SIGNAL(clicked()), this, SLOT(hs_RestAlm()));
	connect(ui.pushButtonMute, SIGNAL(clicked()), this, SLOT(hs_Mute()));

	show_msg("the program is running!");

	//
	for (size_t i = 0; i <= SlotposNo4; i++)
	{
		flagSlot[i] = true;
	}

	connect(ui.checkBox_cave1, &QCheckBox::clicked, [this]()
	{
		if (true == ui.checkBox_cave1->isChecked())
		{
			flagSlot[0] = true;
		}
		else
		{
			flagSlot[0] = false;
		}
	});
	
	connect(ui.checkBox_cave2, &QCheckBox::clicked, [this]()
	{
		if (true == ui.checkBox_cave2->isChecked())
		{
			flagSlot[1] = true;
		}
		else
		{
			flagSlot[1] = false;
		}
	});

	connect(ui.checkBox_cave3, &QCheckBox::clicked, [this]()
	{
		if (true == ui.checkBox_cave3->isChecked())
		{
			flagSlot[2] = true;
		}
		else
		{
			flagSlot[2] = false;
		}
	});

	connect(ui.checkBox_cave4, &QCheckBox::clicked, [this]()
	{
		if (true == ui.checkBox_cave4->isChecked())
		{
			flagSlot[3] = true;
		}
		else
		{
			flagSlot[3] = false;
		}
	});

	//
	for (int i = 0; i < 6; i++)
	{
		m_laserEditor[i] = new laserEdit(this);
		m_laserEditor[i]->set_x_label(QString("laser%1").arg(i + 1));
	}

	m_mainLayout = new QVBoxLayout(this);
	m_topLayout = new QHBoxLayout(this);
	m_midLayout = new QHBoxLayout(this);
	m_bottomMainLayout = new QVBoxLayout(this);
	m_midBarLayout = new QHBoxLayout(this);
	m_midBar2Layout = new QHBoxLayout(this);
	m_midTopLayout = new QHBoxLayout(this);
	m_midBotLayout = new QHBoxLayout(this);

	m_mainLayout->addLayout(m_topLayout);
	m_mainLayout->addLayout(m_midLayout);
	m_mainLayout->addLayout(m_bottomMainLayout);
	m_mainLayout->setSpacing(5);
	m_mainLayout->setContentsMargins(5, 5, 5, 5);

	m_topLayout->addWidget(m_laserEditor[0]);
	m_topLayout->addWidget(m_laserEditor[1]);
	m_topLayout->addWidget(m_laserEditor[2]);
	m_midLayout->addWidget(m_laserEditor[5]);
	m_midLayout->addWidget(m_laserEditor[4]);
	m_midLayout->addWidget(m_laserEditor[3]);

	m_bottomMainLayout->addLayout(m_midBarLayout);
	m_bottomMainLayout->addLayout(m_midBar2Layout);
	m_bottomMainLayout->addLayout(m_midTopLayout);
	m_bottomMainLayout->addLayout(m_midBotLayout);
	m_bottomMainLayout->setContentsMargins(5, 10, 10, 55);

	m_laserName[0] = new QLabel(QStringLiteral("#1检测位："), this);
	m_laserName[1] = new QLabel(QStringLiteral(" #2检测位："), this);
	m_laserName[2] = new QLabel(QStringLiteral(" #3检测位："), this);
	m_laserName[3] = new QLabel(QStringLiteral(" #4检测位："), this);
	m_laserName[4] = new QLabel(QStringLiteral(" #5检测位："), this);
	m_laserName[5] = new QLabel(QStringLiteral("#6检测位："), this);

	m_laserTrig[0] = new QCheckBox(QStringLiteral("Laser实时显示"), this);
	/*m_laserTrig[1] = new QCheckBox(QStringLiteral("Laser2实时显示"), this);
	m_laserTrig[2] = new QCheckBox(QStringLiteral("Laser3实时显示"), this);
	m_laserTrig[3] = new QCheckBox(QStringLiteral("Laser4实时显示"), this);
	m_laserTrig[4] = new QCheckBox(QStringLiteral("Laser5实时显示"), this);
	m_laserTrig[5] = new QCheckBox(QStringLiteral("Laser6实时显示"), this);*/
	connect(m_laserTrig[0], &QCheckBox::clicked, [this]()
	{
		if (true == m_laserTrig[0]->isChecked()){ pValShowTimer->start(500); }
		else{ pValShowTimer->stop(); }
	});

	m_laseWave[0][0] = new QLineEdit(this);//#1
	m_laseWave[0][1] = new QLineEdit(this);
	m_laseWave[0][2] = new QLineEdit(this);
	m_laseWave[0][3] = new QLineEdit(this);

	m_laseWave[1][0] = new QLineEdit(this);//#2
	m_laseWave[1][1] = new QLineEdit(this);
	m_laseWave[1][2] = new QLineEdit(this);
	m_laseWave[1][3] = new QLineEdit(this);

	m_laseWave[2][0] = new QLineEdit(this);//#3
	m_laseWave[2][1] = new QLineEdit(this);
	m_laseWave[2][2] = new QLineEdit(this);
	m_laseWave[2][3] = new QLineEdit(this);

	m_laseWave[3][0] = new QLineEdit(this);//#4
	m_laseWave[3][1] = new QLineEdit(this);
	m_laseWave[3][2] = new QLineEdit(this);
	m_laseWave[3][3] = new QLineEdit(this);

	m_laseWave[4][0] = new QLineEdit(this);//#5
	m_laseWave[4][1] = new QLineEdit(this);
	m_laseWave[4][2] = new QLineEdit(this);
	m_laseWave[4][3] = new QLineEdit(this);

	m_laseWave[5][0] = new QLineEdit(this);//#6
	m_laseWave[5][1] = new QLineEdit(this);
	m_laseWave[5][2] = new QLineEdit(this);
	m_laseWave[5][3] = new QLineEdit(this);

	/*for (int i = 0; i < 6; i++)
	{
		m_laserTrig[i]->setObjectName("checkBox");
		for (int j = 0; j < 4; j++)
			m_laseWave[i][j]->setObjectName("lineEdit");
	}*/

	m_midBarLayout->addWidget(m_laserTrig[0]);
	//m_midBarLayout->addWidget(m_laserTrig[1]);
	//m_midBarLayout->addWidget(m_laserTrig[2]);
	//m_midBar2Layout->addWidget(m_laserTrig[5]);
	//m_midBar2Layout->addWidget(m_laserTrig[4]);
	//m_midBar2Layout->addWidget(m_laserTrig[3]);

	//laser #1 lineEdit
	m_midTopLayout->addWidget(m_laserName[0]);
	m_midTopLayout->addWidget(m_laseWave[0][0]);
	m_midTopLayout->addWidget(m_laseWave[0][1]);
	m_midTopLayout->addWidget(new QLabel(QStringLiteral("^"), this));
	m_midTopLayout->addWidget(m_laseWave[0][2]);
	m_midTopLayout->addWidget(m_laseWave[0][3]);

	//laser #2 lineEdit
	m_midTopLayout->addWidget(m_laserName[1]);
	m_midTopLayout->addWidget(m_laseWave[1][0]);
	m_midTopLayout->addWidget(m_laseWave[1][1]);
	m_midTopLayout->addWidget(new QLabel(QStringLiteral("^"), this));
	m_midTopLayout->addWidget(m_laseWave[1][2]);
	m_midTopLayout->addWidget(m_laseWave[1][3]);

	//laser #3 lineEdit
	m_midTopLayout->addWidget(m_laserName[2]);
	m_midTopLayout->addWidget(m_laseWave[2][0]);
	m_midTopLayout->addWidget(m_laseWave[2][1]);
	m_midTopLayout->addWidget(new QLabel(QStringLiteral("^"), this));
	m_midTopLayout->addWidget(m_laseWave[2][2]);
	m_midTopLayout->addWidget(m_laseWave[2][3]);

	// //laser #6 lineEdit
	m_midBotLayout->addWidget(m_laserName[5]);
	m_midBotLayout->addWidget(m_laseWave[5][0]);
	m_midBotLayout->addWidget(m_laseWave[5][1]);
	m_midBotLayout->addWidget(new QLabel(QStringLiteral("^"), this));
	m_midBotLayout->addWidget(m_laseWave[5][2]);
	m_midBotLayout->addWidget(m_laseWave[5][3]);

	//laser #5 lineEdit
	m_midBotLayout->addWidget(m_laserName[4]);
	m_midBotLayout->addWidget(m_laseWave[4][0]);
	m_midBotLayout->addWidget(m_laseWave[4][1]);
	m_midBotLayout->addWidget(new QLabel(QStringLiteral("^"), this));
	m_midBotLayout->addWidget(m_laseWave[4][2]);
	m_midBotLayout->addWidget(m_laseWave[4][3]);

	//laser #4 lineEdit
	m_midBotLayout->addWidget(m_laserName[3]);
	m_midBotLayout->addWidget(m_laseWave[3][0]);
	m_midBotLayout->addWidget(m_laseWave[3][1]);
	m_midBotLayout->addWidget(new QLabel(QStringLiteral("^"), this));
	m_midBotLayout->addWidget(m_laseWave[3][2]);
	m_midBotLayout->addWidget(m_laseWave[3][3]);

	QWidget* laserWidget = new QWidget(this);
	laserWidget->setObjectName("laserWidget");
	laserWidget->setLayout(m_mainLayout);

	ui.tabWidget->insertTab(2, laserWidget, QStringLiteral("激光参数"));  //激光参数tab页

	pCsvWavePosition = new _FILECLASS::CSV_FILE(QCoreApplication::applicationDirPath() + "/cfg/laserWave.csv");
	pCsvWavePosition->get(vWavePosition);
	
	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			m_laseWave[i][j]->setText(vWavePosition[i + 1][j + 1]);

			connect(m_laseWave[i][j], &QLineEdit::editingFinished, [this, i, j]()
			{
				vWavePosition[i + 1][j + 1] = m_laseWave[i][j]->text();
			});
		}
	}

	
//	deal_raw_data();
	
}

void HSMeasure::onTabWidgetChanged(int tabIndex)
{
	int b = tabIndex;

	if (1 == tabIndex)
	{
		ui.lineEditCurPositionCcd->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition()));
		ui.lineEditCurPositionLaser->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition()));
		ui.lineEditCurPositionRotate->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition()));
		
		ui.lineEditCurPositionPlatform->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition()));	
		ui.lineEditCurPositionUnload->setText(QString("%1").arg(mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition()));
	}
}

void HSMeasure::onDogTimer()
{
	dogCheck();
}

void HSMeasure::initDmcCard()
{

	if (false == mpMOTIONLib->initial())
	{
		qDebug() << QStringLiteral("没有找到运动控制卡，或者控制卡异常 \n");
		QMessageBox::warning(this, "", QStringLiteral("运动控制卡初始化失败，请检查后重启!"));
		return;
	}
	else
	{
		qDebug() << QStringLiteral("找到%1张运动控制卡 \n").arg(mpMOTIONLib->mCardNum);
	}

	mCardNo = mpMOTIONLib->mInitialPara.CardIdList[0];
	
	for(int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_write_sevon_pin(mCardNo, Axis, DMC_SERV_ENABLE);
	}

	dmc_download_configfile(mCardNo, "D:\\paraDmc.ini");

	/*
	QString tPath = mPath + "/cfg/paraDmc.ini";
	if (dmc_download_configfile(mCardNo, tPath.toStdString().c_str()))
	{
		show_msg("Error: dmc_download_configfile()");
	}
	*/

	/*
	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_set_pulse_outmode(mCardNo, Axis, 3);
	}


	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_set_el_mode(mCardNo, Axis,1, 0, 0);
	}

	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_set_axis_io_map(mCardNo, Axis, 3, 3, 0, 1);
		dmc_set_emg_mode(mCardNo, Axis, 1, 0);
	}
		
	int Org_logic = 1;   
	int Filter = 0;       
	int Home_dir = 1;   
	int Vel_mode = 0;	
	int Mode = 2;        
	int EZ_count = 0; 

	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_set_home_pin_logic(mCardNo, Axis, Org_logic, Filter);
		dmc_set_homemode(mCardNo, Axis, Home_dir, Vel_mode, Mode, EZ_count);
	}
	

	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_set_s_profile(mCardNo, Axis, 0, 0.01);
	}
	
	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		mpDMC5000Lib->mpDmcAxis[0][Axis]->setMovePara();
	}
	*/

	for (size_t i = 0; i < IO_EX_NUM; i++)
	{
		if (dmc_set_can_state(mCardNo, i + 1, CAN_OPEN, 0))
		{
			show_msg(QString("I/O #%1 connnect failed.").arg(i+1));
		}
	}

	

//	dmc_set_can_state(mCardNo, 1, CAN_CLOSE, 0);
	//WORD status;
	//WORD node = 1;
	//int c = dmc_get_can_state(mCardNo, &node, &status);

}

void HSMeasure::initValue()
{
	for (size_t i = 0; i <= SlotposNo4; i++)
	{
		fixValue.plug[i].ok_ng = MsSlotNull;

		for (size_t j = 0; j <= GapJ; j++)
		{
			fixValue.plug[i].gap[j] = -999.999;
		}

		for (size_t j = 0; j <= StepJ; j++)
		{
			fixValue.plug[i].step[j] = -999.999;
		}
	}

	homeStep = -1;
	curStateFlow1 = -1;
	curStateFlow2 = -1;
	curStateFlow3 = -1;
	curStateFlow4 = -1;
}

void HSMeasure::hs_pause()  //暂停button clicked
{
	static bool flag = false;

	flag = !flag;

	if (flag)
	{
		mRunFlag.mbPause = true;

		ui.pushButtonPause->setStyleSheet("QPushButton{background:yellow}");
		ui.pushButtonPause->setText(QStringLiteral("继续"));
	}
	else
	{
		mRunFlag.mbPause = false;

		ui.pushButtonPause->setStyleSheet("QPushButton{background:}");
		ui.pushButtonPause->setText(QStringLiteral("暂停"));
	}
	
}

void HSMeasure::hs_stop()
{
	mRunFlag.mbHome = false;
	mRunFlag.mbStop = true;
	mpDMC5000Lib->emgStop();

	mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->sevonOnOff(DMC_SERV_DISABLE);

	colorSignal(ui.pushButtonStart, "QPushButton{background:}");
	colorSignal(ui.pushButtonZero, "QPushButton{background:}");
	ableSignal(ui.pushButtonStart, true);
	ableSignal(ui.pushButtonZero, true);
	ableSignal(ui.pushButtonOut, true);
}

void HSMeasure::hs_RestAlm()
{
	sensorOut(RedLight, IO_OFF);
	sensorOut(YellowLight, IO_ON);
}

void HSMeasure::hs_Mute()
{
	sensorOut(Buzzer, IO_OFF);
}

void HSMeasure::caseHome()  //回原流程
{
	
	static DWORD curSystemTime;

	switch (homeStep)
	{
	case 0:
		sensorOut(FixtureVacuum, IO_OFF);
		for (size_t i = Vacuum1; i <= Vacuum12; i++)
		{
			sensorOut(static_cast<IO_SENSOR>(i), IO_OFF);
		}
		homeStep++;
		break;
	case 1:
		if (cylinderCheck(ROTATE_o) != IO_ON)
		{
			cylinderMove(ROTATE_o);
		}

		curSystemTime = GetTickCount();
		homeStep++;
		break;
	case 2:
		if ((GetTickCount() - curSystemTime) < CYLINDER_DELAY)
		{
			break;
		}
		homeStep++;
	case 3:
		for (int i = FEED_o; i <= UPDOWN_NG3_I; i+=2)
		{
			cylinderMove(static_cast<IO_CYLINDER>(i));
		}
		homeStep++;
		break;

	case 4:
		if (IO_ON == cylinderCheck(UPDOWN_A_o) && IO_ON == cylinderCheck(UPDOWN_B_o)
			&& IO_ON == cylinderCheck(UPDOWN_C_1o) && IO_ON == cylinderCheck(UPDOWN_C_2o) && IO_ON == cylinderCheck(UPDOWN_C_3o) && IO_ON == cylinderCheck(UPDOWN_C_4o))
		{
			homeStep++;
		}
		break;

	case 5:
		cylinderMove(FEED_I);		//送料气缸伸出
		homeStep++;
		break;
	
	case 6:
		cylinderMove(CLAMP2_I);
		homeStep++;
		break;
	case 7:
		if (IO_OFF == cylinderCheck(CLAMP2_I) || modeRunNullFlag)	//判断STEP 夹紧信号
			homeStep++;
		break;
	case 8:	
		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->home();	
		homeStep++;
		break;
	case 9:
		if (mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->checkHome())
			homeStep++;
		break;
	case 10:
		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->home();
		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->home();
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->home();
			homeStep++;
		break;
	case 11:
		if (mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->checkHome())
			homeStep++;
		break;
	case 12:
		if (mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->checkHome())
			homeStep++;
		break;
	case 13:
		if (mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->checkHome())
			homeStep++;
		break;
	case 14:
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->home();
		homeStep++;
		break;
	case 15:
		if (mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->checkHome())
			homeStep++;
		break;
		
	case 16:	
		mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setMovePara(SpeedPercent * speedFromIni.ccdAutoSpeed, ABSOLUTE_MOTION);		//X1
		mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(SpeedPercent * speedFromIni.laserAutoSpeed, ABSOLUTE_MOTION);	//X2
		mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setMovePara(SpeedPercent * speedFromIni.rotateAutoSpeed, ABSOLUTE_MOTION);	//X3
		mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setMovePara(SpeedPercent * speedFromIni.platAutoSpeed, ABSOLUTE_MOTION);	//Y1
		mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setMovePara(SpeedPercent * speedFromIni.unloadAutoSpeed, ABSOLUTE_MOTION);	//Y2
		homeStep++;
		break;
	
	case 17:
		if (sensorIn(Fiber5) == IO_ON || sensorIn(Fiber6) == IO_ON || sensorIn(Fiber7) == IO_ON || sensorIn(Fiber8) == IO_ON)
		{
			showMsgSignal(QStringLiteral("请先手动排料, 回原失败"));
			
			colorSignal(ui.pushButtonZero, "QPushButton{background:red}");
			mRunFlag.mbHome = false;
			homeStep = -1;
		}
		else
		{
			homeStep++;
		}
		break;
	default:
		break;
	}

	if (homeStep > 17)
	{
		colorSignal(ui.pushButtonZero, "QPushButton{background:lightgreen}");
		mRunFlag.mbHome = true;
		homeStep = -1;
	}

	

}

void HSMeasure::hs_zero()  //回原button clicked
{
	dmc_download_configfile(mCardNo, "D:\\DROSE\\HSMeasure\\x64\\Debug\\cfg\\2018.11.17.ini");

	
	mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->setHomePara(1, speedFromIni.ccdHomeSpeed);
	mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->setHomePara(1, speedFromIni.laserHomeSpeed);
	mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->setHomePara(0, speedFromIni.rotateHomeSpeed);
	mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->setHomePara(0, speedFromIni.platHomeSpeed);
	mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->setHomePara(1, speedFromIni.unloadHomeSpeed);
	
	
	/*
	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		mpDMC5000Lib->stop(mCardNo, Axis, _MOTIONCLASS::DMC5000Lib::STOP_MODE_EMG);
	}

	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_set_position(mCardNo, Axis, 0);
	}

	for (int Axis = 0; Axis < AXIS_NUM; Axis++)
	{
		dmc_write_sevon_pin(mCardNo, Axis, DMC_SERV_ENABLE);
	}
	*/

	ui.pushButtonZero->setEnabled(false);
	ui.pushButtonZero->setStyleSheet("QPushButton{background:yellow}");
	homeStep = 0;
}

void HSMeasure::ok_ngResult()
{
	if (gapQueue.size() == 0 || stepQueue.size() == 0)
	{
		show_msg("gapQueue.size() == 0 || stepQueue.size() == 0");
		return;
	}

	std::array<std::array<float, GapJ + 1>, SlotposNo4 + 1> tGap = gapQueue.dequeue();
	std::array<std::array<float, StepJ + 1>, SlotposNo4 + 1> tStep = stepQueue.dequeue();
	
	for (size_t i = 0; i < SlotposNo4 + 1; i++)
	{
		int result = 0;

		for (size_t j = 0; j < GapJ + 1; j++)
		{		
			if (tGap[i][j] < offsetGap_L || tGap[i][j] > offsetGap_H)
			{
				result |= 0x01;
				fixValue.plug[i].ok_ng = MsResultNgGap;
			}
			else if (tStep[i][j] < offsetStep_L || tStep[i][j] > offsetStep_H)
			{
				result |= 0x10;
				fixValue.plug[i].ok_ng = MsResultNgStep;
			}
			
			fixValue.plug[i].gap[j] = tGap[i][j];
			fixValue.plug[i].step[j] = tStep[i][j];
		}

		if (result == 0)
		{
			fixValue.plug[i].ok_ng = MsResultOk;
		}
		else if (result == 0x01)
		{
			fixValue.plug[i].ok_ng = MsResultNgGap;
		}
		else if (result == 0x10)
		{
			fixValue.plug[i].ok_ng = MsResultNgStep;
		}
		else if (result == 0x11)
		{
			fixValue.plug[i].ok_ng = MsResultNgBoth;
		}
		else
		{
			show_msg("Error ok_ngResult()");
		}

	}
}

bool HSMeasure::loadReady()
{
	//光纤信号
	for (size_t i = 0; i < 4; i++)
	{
		if ((true == flagSlot[i] && IO_OFF == sensorIn((IO_SENSOR)((int)Fiber1 + i)))
			|| (false == flagSlot[i] && IO_ON == sensorIn((IO_SENSOR)((int)Fiber1 + i))))
		{
			return false;
		}
	}

	return true;
}

bool HSMeasure::ngUnloadReady()
{
	if (fixValue.plug[0].ok_ng == 0 && fixValue.plug[1].ok_ng == 0 && fixValue.plug[2].ok_ng == 0 && fixValue.plug[3].ok_ng == 0)
	{
		return true;  //全部ok
	}

	if (curStateOut != -1)
	{
		return false;
	}

	//Y2运动至ng位
	if (false == mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.ngPositionY2))
	{
		return false;
	}

	if (curStateNgBreak!=-1)
	{
		return false;
	}

	

	return true;
}

bool HSMeasure::ngUnloading(const DWORD curSystemTime)
{
	std::vector<IO_SENSOR> tVector;

	if (fixValue.plug[0].ok_ng == 0 && fixValue.plug[1].ok_ng == 0 && fixValue.plug[2].ok_ng == 0 && fixValue.plug[3].ok_ng == 0)
	{
		return true;  //全部ok
	}

	if (fixValue.plug[0].ok_ng != MsResultOk)	{ cylinderMove(UPDOWN_C_1I); }
	if (fixValue.plug[1].ok_ng != MsResultOk)	{ cylinderMove(UPDOWN_C_2I); }
	if (fixValue.plug[2].ok_ng != MsResultOk)	{ cylinderMove(UPDOWN_C_3I); }
	if (fixValue.plug[3].ok_ng != MsResultOk)	{ cylinderMove(UPDOWN_C_4I); }


	if ((GetTickCount() - curSystemTime) < CYLINDER_DELAY)
	{
		return false;
	}

	if (fixValue.plug[0].ok_ng != MsResultOk)	{ sensorOut(Vacuum9, IO_OFF); tVector.push_back(Vacuum9); }
	if (fixValue.plug[1].ok_ng != MsResultOk)	{ sensorOut(Vacuum10, IO_OFF); tVector.push_back(Vacuum10); }
	if (fixValue.plug[2].ok_ng != MsResultOk)	{ sensorOut(Vacuum11, IO_OFF); tVector.push_back(Vacuum11); }
	if (fixValue.plug[3].ok_ng != MsResultOk)	{ sensorOut(Vacuum12, IO_OFF); tVector.push_back(Vacuum12); }

	if (false == vacuumCheckDown(tVector))
	{
		return false;
	}
	
	if (ngTypeQueue.size()!=0)
	{
		return false;
	}

	for (int i = SlotposNo4; i >= SlotposNo1; i--)
	{
		std::array<int, 2> tType;
		tType[0] = i;
		tType[1] = fixValue.plug[i].ok_ng;
		
		if (tType[1] != 0)
		{
			ngTypeQueue.enqueue(tType);
		}		
	}

	return true;
}

bool HSMeasure::okUnloadReady()
{
	if (fixValue.plug[0].ok_ng != MsResultOk && fixValue.plug[1].ok_ng != MsResultOk && fixValue.plug[2].ok_ng != MsResultOk && fixValue.plug[3].ok_ng != MsResultOk)
	{
		return true;  //全部ng
	}

	if (curStateOut != -1)
	{
		return false;
	}

	//Y2运动至OK位
	if (false == mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.okPositionY2))
	{
		return false;
	}

	if (1 == sensorIn(Opposite1))	//进料对射
	{
		if (1 == sensorIn(Opposite3))
		{
			sensorOut(RedLight, IO_ON);
			sensorOut(Buzzer, IO_ON);
		}

		return false;
	}

	

	return true;
}

bool HSMeasure::okUnloading(const DWORD curSystemTime)
{
	std::vector<IO_SENSOR> tVector;

	if (fixValue.plug[0].ok_ng != 0 && fixValue.plug[1].ok_ng != 0 && fixValue.plug[2].ok_ng != 0 && fixValue.plug[3].ok_ng != 0)
	{
		return true;  //全部ng
	}

	if (MsResultOk == fixValue.plug[0].ok_ng)	cylinderMove(UPDOWN_C_1I);	
	if (MsResultOk == fixValue.plug[1].ok_ng)	cylinderMove(UPDOWN_C_2I);
	if (MsResultOk == fixValue.plug[2].ok_ng)	cylinderMove(UPDOWN_C_3I);
	if (MsResultOk == fixValue.plug[3].ok_ng)	cylinderMove(UPDOWN_C_4I);


	if ((GetTickCount() - curSystemTime) < CYLINDER_DELAY)
	{
		return false;
	}
	
	if (MsResultOk == fixValue.plug[0].ok_ng)	{ sensorOut(Vacuum9, IO_OFF); tVector.push_back(Vacuum9); }
	if (MsResultOk == fixValue.plug[1].ok_ng)	{ sensorOut(Vacuum10, IO_OFF); tVector.push_back(Vacuum10); }
	if (MsResultOk == fixValue.plug[2].ok_ng)	{ sensorOut(Vacuum11, IO_OFF); tVector.push_back(Vacuum11); }
	if (MsResultOk == fixValue.plug[3].ok_ng)	{ sensorOut(Vacuum12, IO_OFF); tVector.push_back(Vacuum12); }


	if (false == vacuumCheckDown(tVector))
	{
		return false;
	}

	return true;
}

bool HSMeasure::unloadDone()
{
	cylinderMove(UPDOWN_C_1o);
	cylinderMove(UPDOWN_C_2o);
	cylinderMove(UPDOWN_C_3o);
	cylinderMove(UPDOWN_C_4o);

	if (cylinderCheck(UPDOWN_C_1o) != IO_ON || cylinderCheck(UPDOWN_C_2o) != IO_ON || cylinderCheck(UPDOWN_C_3o) != IO_ON || cylinderCheck(UPDOWN_C_4o))
	{
		return false;
	}

	return true;
}

void HSMeasure::caseOpposite()
{
	static DWORD curSystemTime;

	if (1 == sensorIn(Opposite3) || mRunFlag.mbStop)
	{
		sensorOut(LineUnloadOkRun, IO_OFF);
		return;
	}
		
	if (1 == sensorIn(Opposite1))
	{
		mRunFlag.okBelt = true;
		curSystemTime = GetTickCount();
	}

	if (mRunFlag.okBelt)
	{
		if (GetTickCount() - curSystemTime < 200)
		{
			sensorOut(LineUnloadOkRun, IO_ON);
			return;
		}	
	}

	if (sensorIn(Opposite2) != 1)
	{
		return;
	}

	mRunFlag.okBelt = false;
	
	sensorOut(LineUnloadOkRun, IO_OFF);
}

bool HSMeasure::vacuumCheckDown(std::vector<IO_SENSOR> indexVacuum)
{
	//关闭真空
	for (auto it = indexVacuum.begin(); it != indexVacuum.end(); ++it)
	{
		if (sensorIn(*it) != 0)
		{
			return false;
		}
	}
	
	return true;
}

bool HSMeasure::vacuumCheckUp(std::vector<IO_SENSOR> indexVacuum)
{
	//打开真空吸
	for (auto it = indexVacuum.begin(); it != indexVacuum.end(); ++it)
	{
		if (sensorIn(*it) != 1)
		{
			return false;
		}
	}

	return true;
}

bool HSMeasure::resultMonitor()
{
	if (lastResults.isEmpty())
	{
		return true;
	}

	for (size_t i = 0; i < lastResults.size(); i++)
	{
		int ngNum = 0;

		for (size_t j = 0; j < 4; j++)
		{
			if (lastResults[i].plug[j].ok_ng != 0)
			{
				ngNum++;
			}
		}
		
		if (ngNum >= 3)
		{
			showDialogSignal("Error: resultMonitor() ngNum >= 3");
			return false;
		}
	}

	if (lastResults.size() < 3)
	{
		return true;
	}

	while (lastResults.size()!=3)
	{
		lastResults.removeFirst();
	}

	for (size_t i = 0; i < 4; i++)
	{
		if (lastResults[0].plug[i].ok_ng != 0 && lastResults[1].plug[i].ok_ng != 0 && lastResults[2].plug[i].ok_ng != 0)
		{
			showDialogSignal(QString("Error: resultMonitor() slotNo %1").arg(i+1));
			return false;
		}
	}

	return true;
}

void HSMeasure::stateMonitor()
{
	const int stateNum = 6;
	const int monitorNum = 5;

	static DWORD curSystemTime;

	if ((curStateFlow1 == -1 || curStateFlow1 == 0) && curStateFlow2 == -1 && curStateFlow3 == -1 && 
		curStateFlow4 == -1 && curStateNgBreak ==  -1 && curStateOut == -1)
	{
		return;
	}

	if ((GetTickCount() - curSystemTime) < 1000 * 2)
	{
		return;
	}

	curSystemTime = GetTickCount();
	
	tState[0].push_back(curStateFlow1);
	tState[1].push_back(curStateFlow2);
	tState[2].push_back(curStateFlow3);
	tState[3].push_back(curStateFlow4);
	tState[4].push_back(curStateNgBreak);
	tState[5].push_back(curStateOut);
	
	if (tState[0].size() <= monitorNum)
	{
		return;
	}

	bool bPass = false;

	for (size_t i = 0; i < stateNum; i++)
	{
		for (size_t j = 0; j < monitorNum - 1; j++)
		{
			if (tState[i][j] != tState[i][j + 1])
			{
				bPass = true;
			}
		}
	}
	
	for (size_t i = 0; i < stateNum; i++)
	{
		tState[i].clear();
	}

	if (bPass == true)
	{
		return;
	}

	sensorOut(RedLight, IO_ON);
	sensorOut(Buzzer, IO_ON);
	showDialogSignal(curErrorMsg);	
}

inline bool HSMeasure::scanAlarmSafe()
{

	for (size_t i = 0; i < AXIS_NUM; i++)
	{
		DWORD inAxisS = dmc_axis_io_status(mCardNo, i);

		if (inAxisS & ALM)
		{
			sensorOut(RedLight, IO_ON);
			sensorOut(Buzzer, IO_ON);

			showDialogSignal(QString("ALARM AXIS:%1").arg(i));
			return false;
		}
	}
	
	//轴干涉
	long tCurPosX1 = mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->curPosition();
	long tCurPosX2 = mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->curPosition();
	long tCurPosX3 = mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->curPosition();
	long tCurPosY1 = mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition();
	long tCurPosY2 = mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->curPosition();

	if (tCurPosX2 < 0 && tCurPosY1 < 0)
	{
		show_msg("X2 # Y1");
		return false;
	}

	if (tCurPosX2 < 0 && abs(tCurPosY2) > abs(posionFromIni.ngPositionY2))
	{
		show_msg("X2 # Y2");
		return false;
	}

	return true;
}

inline void HSMeasure::caseFlow1()
{
	static DWORD curSystemTime;
	std::vector<IO_SENSOR> tVector;

	switch (curStateFlow1)
	{
	case 0:
		if (IO_ON == sensorIn(CONFIRM))
			curStateFlow1++;
		break;
	case 1:
		sensorOut(FixtureVacuum, IO_ON);
		curStateFlow1++;
		showMsgSignal(QStringLiteral("治具真空检测..."));
		break;
	case 2:
		if (1 == sensorIn(FixtureVacuum) || modeRunNullFlag)
			curStateFlow1++;
		break;
	case 3:
		if (true == loadReady() || modeRunNullFlag)
		{
			curStateFlow1++;
		}
		else
		{
			showMsgSignal(QStringLiteral("上料位检测失败！"));
		}
			
		break;
	case 4:
		cylinderMove(UPDOWN_A_o);		
		curStateFlow1++;
		showMsgSignal(QStringLiteral("1~4气缸上升..."));
		break;
	case 5:
		if (IO_ON == cylinderCheck(UPDOWN_A_o) || modeRunNullFlag)
			curStateFlow1++;
		break;		
	case 6:
		cylinderMove(FEED_o);		
		curStateFlow1++;
		showMsgSignal(QStringLiteral("送料气缸缩回..."));
		break;
	case 7:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
			curStateFlow1++;
		break;
	case 8:
		if (IO_ON == cylinderCheck(FEED_o) || modeRunNullFlag)
			curStateFlow1++;
		break;	
	case 9:
		cylinderMove(UPDOWN_A_I);	//1~4升降气缸伸出
		curStateFlow1++;
		curSystemTime = GetTickCount();
		break;
	case 10:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
		{ 
			curStateFlow1++;
			showMsgSignal(QStringLiteral("1~4路真空吸料..."));
		}
		break;
	case 11:
		if (true == flagSlot[0]) { sensorOut(Vacuum1, IO_ON);	tVector.push_back(Vacuum1); }
		if (true == flagSlot[1]) { sensorOut(Vacuum2, IO_ON);	tVector.push_back(Vacuum2); }
		if (true == flagSlot[2]) { sensorOut(Vacuum3, IO_ON);	tVector.push_back(Vacuum3); }
		if (true == flagSlot[3]) { sensorOut(Vacuum4, IO_ON);	tVector.push_back(Vacuum4); }

		if (true == vacuumCheckUp(tVector) || modeRunNullFlag)
		{
			curStateFlow1++;
		}
		break;
	case 12:
		curStateFlow1++;
		break;
	case 13:
		curStateFlow1++;
		break;	
	case 14:
		curStateFlow1++;
		break;
	case 15:
		curStateFlow1++;
		break;
	case 16:
		curStateFlow1++;
		break;	
	case 17:
		cylinderMove(UPDOWN_A_o);
		curStateFlow1++;
		showMsgSignal(QStringLiteral("1~4气缸上升..."));
		break;
	case 18:
		if (IO_ON == cylinderCheck(UPDOWN_A_o))
			curStateFlow1++;
		break;
	case 19:
		if (IO_ON == cylinderCheck(UPDOWN_A_o))
		{
			curStateFlow1++;
			showMsgSignal(QStringLiteral("光幕信号..."));
		}
		break;
	case 20:
		if (sensorIn(LightCurtain) != IO_ON)	
			curStateFlow1++;
		break;
	case 21:
		cylinderMove(FEED_I);		
		curStateFlow1++;
		showMsgSignal(QStringLiteral("送料气缸伸出..."));
		break;
	case 22:
		if (1 == cylinderCheck(FEED_I))
			curStateFlow1++;
		break;	
	case 23:	
		if (-1 == curStateFlow2)
		{
			curStateFlow2 = 0;
		}

	default:
		break;
	}
}

inline void HSMeasure::caseFlow2()
{
	static DWORD curSystemTime;
	std::vector<IO_SENSOR> tVector;

	switch (curStateFlow2)
	{
	case 0:
		cylinderMove(ROTATE_o);	//转动气缸缩回
		curStateFlow2++;
		showMsgSignal(QStringLiteral("GAP 夹爪松开..."));
		break;
	case 1:
		if (IO_OFF == cylinderCheck(CLAMP1_o) || modeRunNullFlag)
		{
			curStateFlow2++;
			showMsgSignal(QStringLiteral("Y1轴运动至上料位..."));
		}
		break;
	case 2:
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->moveAndCheck(posionFromIni.loadPositionY1))
		{
			curStateFlow2++;
		}
		
		break;
	case 3:
		if (mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition() < posionFromIni.loadPositionY1)
		{
			break;
		}
		curStateFlow2++;
	case 4:
		cylinderMove(UPDOWN_A_I);	//1~4升降气缸伸出	
		curStateFlow2++;
		curSystemTime = GetTickCount();
		break;
	case 5:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
		{
			curStateFlow2++;
			showMsgSignal(QStringLiteral("关闭1~4路真空..."));
		}
		break;
	case 6:
		sensorOut(Vacuum1, IO_OFF);  tVector.push_back(Vacuum1);
		sensorOut(Vacuum2, IO_OFF);  tVector.push_back(Vacuum2);
		sensorOut(Vacuum3, IO_OFF);  tVector.push_back(Vacuum3);
		sensorOut(Vacuum4, IO_OFF);  tVector.push_back(Vacuum4);
		
		if (vacuumCheckDown(tVector))
		{
			curStateFlow2++;
		}
	
		break;
		
	case 7:
		cylinderMove(UPDOWN_A_o);
		curStateFlow2++;
		showMsgSignal(QStringLiteral("1~4升降气缸缩回..."));
		break;
	case 8:
		if(IO_ON == cylinderCheck(UPDOWN_A_o))
			curStateFlow2++;
		break;
	case 9:
		cylinderMove(CLAMP1_I);	
		curSystemTime = GetTickCount();
		curStateFlow2++;
		break;
	case 10:
		if ((GetTickCount() - curSystemTime) > 100)
		{
			curStateFlow2++;
		}
		break;
	case 11:
		cylinderMove(CLAMP1_o);
		curSystemTime = GetTickCount();
		curStateFlow2++;
		break;
	case 12:
		if ((GetTickCount() - curSystemTime) > 100)
		{
			curStateFlow2++;
		}
		break;
	case 13:
		curStateFlow2++;
		break;
	case 14:
		curStateFlow2++;
		break;
	case 15:
		cylinderMove(CLAMP1_I);
		curStateFlow2++;
		showMsgSignal(QStringLiteral("GAP夹紧..."));
		break;
	case 16:
		if (IO_ON == cylinderCheck(CLAMP1_o) || modeRunNullFlag)
		{
			curStateFlow2++;
			showMsgSignal(QStringLiteral("GAP正面检测..."));
		}
		break;
	case 17:
		if (true == mpBCDH_gap->getGapValueFront())
		{
			curStateFlow2++;
		}			
		break;
	case 18:
		curStateFlow2++;
		break;
	case 19:
		curStateFlow2++;
		break;
	case 20:
		curStateFlow2++;
		break;
	case 21:
		cylinderMove(ROTATE_I);	
		curStateFlow2++;
		showMsgSignal(QStringLiteral("转动气缸..."));
		break;
	case 22:
		if(IO_ON == cylinderCheck(ROTATE_I))
		{ 
			curStateFlow2++;
			showMsgSignal(QStringLiteral("GAP侧面检测..."));
		}
		break;

	case 23:
		if (true == mpBCDH_gap->getGapValueSide())
		{
			curStateFlow2++;
		}
		break;

	case 24:
		gapQueue.enqueue(gapValues);
		curStateFlow2++;
		break;
	case 25:	
		curStateFlow2++;
		break;
	case 26:
		curStateFlow2++;
		break;
	case 27:
		cylinderMove(ROTATE_o);
		curStateFlow2++;
		showMsgSignal(QStringLiteral("回转气缸..."));
		break;
	case 28:
		if (IO_ON == cylinderCheck(ROTATE_o))
		{
			curStateFlow2++;
		}			
		break;


	case 29:
		if (-1 == curStateFlow3 && -1 == curStateFlow4)
		{
			curStateFlow2++;			
		}
		break;
	
	case 30:
		if (sensorIn(Vacuum9) != 1 && sensorIn(Vacuum10) != 1 && sensorIn(Vacuum11) != 1 && sensorIn(Vacuum12) != 1)
		{
			curStateFlow2++;
		}
		break;
	case 31:
		if (mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheck(posionFromIni.LaserSafePosition))
		{
			curStateFlow2++;
			showMsgSignal(QStringLiteral("X2轴安全位置..."));
		}
		break;

	case 32:
		if (IO_OFF == dmc_read_can_inbit(mCardNo, 3, 11))
		{
			curStateFlow2++;
		}
		break;

	case 33:
		if (curStateFlow4 == -1 && curStateFlow3 == -1 && curStateOut == -1)
		{
			curStateFlow2++;

			showMsgSignal(QStringLiteral("Y1轴运动至下料位..."));
			showMsgSignal(QStringLiteral("Y2轴运动到取料位..."));
			mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->move(posionFromIni.unloadPositionY1);
			mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->move(posionFromIni.loadPositionY2);
		}	
		break;

	case 34:
		if (mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->moveAndCheck(posionFromIni.unloadPositionY1))
		{
			curStateFlow1 = 0;
			curStateFlow2++;
		}		
		break;
	
	case 35:	
		if (mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.loadPositionY2))
			curStateFlow2++;	
		
		break;
	
	case 36:
		stepFiberFlag[0] = sensorIn(Fiber5);
		stepFiberFlag[1] = sensorIn(Fiber6);
		stepFiberFlag[2] = sensorIn(Fiber7);
		stepFiberFlag[3] = sensorIn(Fiber8);

		cylinderMove(UPDOWN_B_I);	//5~8升降气缸下降
		if (IO_ON == stepFiberFlag[SlotposNo1])	cylinderMove(UPDOWN_C_1I);	
		if (IO_ON == stepFiberFlag[SlotposNo2])	cylinderMove(UPDOWN_C_2I);
		if (IO_ON == stepFiberFlag[SlotposNo3])	cylinderMove(UPDOWN_C_3I);
		if (IO_ON == stepFiberFlag[SlotposNo4])	cylinderMove(UPDOWN_C_4I);	//9~12升降气缸下降
		curSystemTime = GetTickCount();
		curStateFlow2++;
		break;
	case 37:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
			curStateFlow2++;
		break;
	case 38:
		if (flagSlot[SlotposNo1])		{ sensorOut(Vacuum5, IO_ON);	tVector.push_back(Vacuum5); }	//开启5~8路真空
		if (flagSlot[SlotposNo2])		{ sensorOut(Vacuum6, IO_ON);	tVector.push_back(Vacuum6); }
		if (flagSlot[SlotposNo3])		{ sensorOut(Vacuum7, IO_ON);	tVector.push_back(Vacuum7); }
		if (flagSlot[SlotposNo4])		{ sensorOut(Vacuum8, IO_ON);	tVector.push_back(Vacuum8); }
		if (IO_ON == stepFiberFlag[SlotposNo1])	{ sensorOut(Vacuum9, IO_ON);	tVector.push_back(Vacuum9); }
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ sensorOut(Vacuum10, IO_ON);	tVector.push_back(Vacuum10); }
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ sensorOut(Vacuum11, IO_ON);	tVector.push_back(Vacuum11); }
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ sensorOut(Vacuum12, IO_ON);	tVector.push_back(Vacuum12); }
		
		if (true == vacuumCheckUp(tVector) || modeRunNullFlag)
		{
			curStateFlow2++;
		}		
		break;
	case 39:
		cylinderMove(CLAMP1_o);	//松开
		cylinderMove(CLAMP2_o);	
		curStateFlow2++;		
		break;
	case 40:
		curSystemTime = GetTickCount();
		curStateFlow2++;
		break;
	case 41:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
		{
			curStateFlow2++;
		}
		break;
	case 42:
		if (IO_OFF == cylinderCheck(CLAMP1_o))
		{
			curStateFlow2++;
		}	
		break;
	case 43:
		if (IO_ON == cylinderCheck(CLAMP2_o))
		{
			curStateFlow2++;
		}	
		break;

	
	case 44:
		cylinderMove(UPDOWN_B_o);	//5~8升降气缸上升
		cylinderMove(UPDOWN_C_1o);
		cylinderMove(UPDOWN_C_2o);
		cylinderMove(UPDOWN_C_3o);
		cylinderMove(UPDOWN_C_4o);	//9~12升降气缸上升		
		curStateFlow2++;

		showMsgSignal(QStringLiteral("气缸上升检查..."));

		break;
	case 45:
		if (IO_ON == cylinderCheck(UPDOWN_B_o))
		{
			curStateFlow2++;
		}
		break;

	case 46:
		if (IO_ON == cylinderCheck(UPDOWN_C_1o))
			curStateFlow2++;
		break;
	case 47:
		if (IO_ON == cylinderCheck(UPDOWN_C_2o))
			curStateFlow2++;
		break;
	case 48:
		if (IO_ON == cylinderCheck(UPDOWN_C_3o))
			curStateFlow2++;
		break;
	case 49:
		if (IO_ON == cylinderCheck(UPDOWN_C_4o))
			curStateFlow2++;
		break;

	case 50:
		if (flagSlot[SlotposNo1])		{ sensorOut(Vacuum5, IO_ON);	tVector.push_back(Vacuum5); }	//开启5~8路真空
		if (flagSlot[SlotposNo2])		{ sensorOut(Vacuum6, IO_ON);	tVector.push_back(Vacuum6); }
		if (flagSlot[SlotposNo3])		{ sensorOut(Vacuum7, IO_ON);	tVector.push_back(Vacuum7); }
		if (flagSlot[SlotposNo4])		{ sensorOut(Vacuum8, IO_ON);	tVector.push_back(Vacuum8); }
		if (IO_ON == stepFiberFlag[SlotposNo1])	{ sensorOut(Vacuum9, IO_ON);	tVector.push_back(Vacuum9); }
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ sensorOut(Vacuum10, IO_ON);	tVector.push_back(Vacuum10); }
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ sensorOut(Vacuum11, IO_ON);	tVector.push_back(Vacuum11); }
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ sensorOut(Vacuum12, IO_ON);	tVector.push_back(Vacuum12); }

		if (true == vacuumCheckUp(tVector) || modeRunNullFlag)
		{
			curStateFlow2++;
		}
		break;
	
	case 51:
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->moveAndCheck(0))
			curStateFlow2++;
		break;

	case 52:
		if (-1 == curStateFlow3)
		{
			curStateFlow2++;
			curStateFlow3 = 0;
			showMsgSignal(QStringLiteral("Y2轴运动至STEP放料位..."));
		}
		break;
	case 53:
		curStateFlow2 = -1;
		break;
	default:
		break;
	}
}

inline void HSMeasure::caseFlow3()
{
	static DWORD curSystemTime;
	std::vector<IO_SENSOR> tVector;

	switch (curStateFlow3)
	{
	case 0:
		if (curStateFlow4 == -1 &&  curStateOut == -1)
			curStateFlow3++;
		break;

	case 1:
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.stepPositionY2))
			curStateFlow3++;	
		break;
	case 2:
		showMsgSignal(QStringLiteral("STEP放料..."));
		cylinderMove(CLAMP2_o);	
		curStateFlow3++;
		break;
	
	case 3:
		if (IO_ON == sensorIn(Fiber5) ||
			IO_ON == sensorIn(Fiber6) ||
			IO_ON == sensorIn(Fiber7) ||
			IO_ON == sensorIn(Fiber8))
		{
			break;
		}
		else
			curStateFlow3++;

	case 4:
		if (0 == cylinderCheck(CLAMP2_o))
			curStateFlow3++;
		break;
	
	case 5:
		showMsgSignal(QStringLiteral("5~8升降伸出..."));
		cylinderMove(UPDOWN_B_I);	
		curSystemTime = GetTickCount();
		curStateFlow3++;
		break;
	case 6:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
			curStateFlow3++;
		break;
	case 7:
		showMsgSignal(QStringLiteral("关闭5~8路真空..."));
		sensorOut(Vacuum5, IO_OFF);	 tVector.push_back(Vacuum5);
		sensorOut(Vacuum6, IO_OFF);  tVector.push_back(Vacuum6);
		sensorOut(Vacuum7, IO_OFF);  tVector.push_back(Vacuum7);
		sensorOut(Vacuum8, IO_OFF);  tVector.push_back(Vacuum8);
		
		if (true == vacuumCheckDown(tVector))
		{
			curStateFlow3++;
		}	
		break;
	case 8:
		curStateFlow3++;
		break;
	case 9:
		showMsgSignal(QStringLiteral("5~8升降气缸上升..."));
		cylinderMove(UPDOWN_B_o);	
		curStateFlow3++;
		break;
	case 10:
		if (IO_ON == cylinderCheck(UPDOWN_B_o))
			curStateFlow3++;
		break;
	
	case 11:
		if (curStateFlow4 != -1 || curStateOut != -1)
		{
			break;
		}

		mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->move(posionFromIni.ngPositionY2);
		curStateFlow3++;
		
	case 12:
		showMsgSignal(QStringLiteral("STEP 夹紧..."));
		cylinderMove(CLAMP2_I);
		curStateFlow3++;
		curSystemTime = GetTickCount();
		break;
	case 13:
		if ((GetTickCount() - curSystemTime) > 100)
		{
			curStateFlow3++;
		}
		break;
	case 14:
		showMsgSignal(QStringLiteral("STEP 松开..."));
		cylinderMove(CLAMP2_o);
		curSystemTime = GetTickCount();
		curStateFlow3++;
		break;
	case 15:
		if ((GetTickCount() - curSystemTime) > 300)
		{
			curStateFlow3++;
		}
		break;
	case 16:
		showMsgSignal(QStringLiteral("STEP气缸夹紧..."));
		cylinderMove(CLAMP2_I);	
		curStateFlow3++;
		curSystemTime = GetTickCount();
		break;
	case 17:
		if ((GetTickCount() - curSystemTime) > 100)
		{
			curStateFlow3++;
		}
		break;

	case 18:
		curStateFlow3++;
		break;
	case 19:
		curStateFlow3++;
		break;
	case 20:
		curStateFlow3++;
		break;

	case 21:
		showMsgSignal(QStringLiteral("Y1 SAFE..."));
		showMsgSignal(QStringLiteral("Y2轴在安全位置..."));
		curStateFlow3++;
		break;

	case 22:		
		if (mpMOTIONLib->mpDmcAxis[mCardNo][platformAxisNo]->curPosition() >= 0
			&& true == mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.ngPositionY2))
			curStateFlow3++;
		break;
	case 23:
		if (-1 == curStateFlow4 && stepQueue.size() > 0)
		{
			ok_ngResult();
			curStateFlow4 = 0;
		}
		else
		{
			curStateFlow3++;
		}	
		break;
	case 24:
		showMsgSignal(QStringLiteral("判断STEP 夹紧信号..."));
		if (1 == cylinderCheck(CLAMP2_I) || modeRunNullFlag)	
			curStateFlow3++;
		break;

	case 25:		
		curStateFlow3++;
		showMsgSignal(QStringLiteral("激光测量..."));
		mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(100000 + SpeedPercent * speedFromIni.laserAutoSpeed, ABSOLUTE_MOTION);	//X2
		break;

	case 26:
		if (false == mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheck(posionFromIni.basePositionLaser))
		{           
			break;
		}
		
		mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->setMovePara(SpeedPercent * speedFromIni.laserAutoSpeed, ABSOLUTE_MOTION);	//X2
		curStateFlow3++;

	case 27:
		if (true == mpBCDH_step->getStepValue())
		{
			curStateFlow3++;
		}

		break;

	case 28:	
		stepQueue.enqueue(stepValues);
	
		curStateFlow3++;
		break;

	case 29:	
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][laserAxisNo]->moveAndCheck(posionFromIni.LaserSafePosition))
		{
			curStateFlow3++;
			showMsgSignal(QStringLiteral("X2 SAFE..."));	
		}
		break;
	case 30:
		if (IO_OFF == dmc_read_can_inbit(mCardNo, 3, 11))
		{
			curStateFlow3++;		
		}
		break;

	case 31:
		if (curStateFlow4 != -1)
		{
			break;
		}
		curStateFlow3++;
		break;
		
	case 32:
		if (-1 == curStateFlow2 && stepQueue.size()==1)
		{
			ok_ngResult();
			curStateOut = 0;
			showMsgSignal(QStringLiteral("排料..."));
		}
	
		curStateFlow3++;
		break;
		
	case 33:	
		if (curStateOut != -1 || curStateFlow4 != -1)
		{
			break;
		}
		curStateFlow3++;
		break;

	case 34:
		curStateFlow3 = -1;
		break;
	}

}

void HSMeasure::caseOut()
{
	static DWORD curSystemTime;
	std::vector<IO_SENSOR> tVector;

	switch (curStateOut)
	{
	case 0:
		if (mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.loadPositionY2))
			curStateOut++;
		break;
	case 1:
		stepFiberFlag[0] = sensorIn(Fiber5);
		stepFiberFlag[1] = sensorIn(Fiber6);
		stepFiberFlag[2] = sensorIn(Fiber7);
		stepFiberFlag[3] = sensorIn(Fiber8);

		if (IO_ON == stepFiberFlag[SlotposNo1])	cylinderMove(UPDOWN_C_1I);
		if (IO_ON == stepFiberFlag[SlotposNo2])	cylinderMove(UPDOWN_C_2I);
		if (IO_ON == stepFiberFlag[SlotposNo3])	cylinderMove(UPDOWN_C_3I);
		if (IO_ON == stepFiberFlag[SlotposNo4])	cylinderMove(UPDOWN_C_4I);	//9~12升降气缸下降
		curSystemTime = GetTickCount();
		curStateOut++;
		break;
	case 2:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
			curStateOut++;
		break;
	case 3:
		if (IO_ON == stepFiberFlag[SlotposNo1])	{ sensorOut(Vacuum9, IO_ON);	tVector.push_back(Vacuum9); }
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ sensorOut(Vacuum10, IO_ON);	tVector.push_back(Vacuum10); }
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ sensorOut(Vacuum11, IO_ON);	tVector.push_back(Vacuum11); }
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ sensorOut(Vacuum12, IO_ON);	tVector.push_back(Vacuum12); }

		if (true == vacuumCheckUp(tVector) || modeRunNullFlag)
		{
			curStateOut++;
		}
		break;
	case 4:
		cylinderMove(CLAMP2_o);
		curStateOut++;
		break;
	case 5:
		curStateOut++;
		break;
	case 6:
		if (IO_ON == cylinderCheck(CLAMP2_o))
		{
			curStateOut++;
		}
		break;

	case 7:
		curSystemTime = GetTickCount();
		curStateOut++;
		break;
	case 8:
		if ((GetTickCount() - curSystemTime) > CYLINDER_DELAY)
		{
			curStateOut++;
		}
		break;
	case 9:
		cylinderMove(UPDOWN_C_1o);
		cylinderMove(UPDOWN_C_2o);
		cylinderMove(UPDOWN_C_3o);
		cylinderMove(UPDOWN_C_4o);	//9~12升降气缸上升		
		curStateOut++;

		showMsgSignal(QStringLiteral("气缸上升检查..."));

		break;
	case 10:
		curStateOut++;
		break;

	case 11:
		if (IO_ON == cylinderCheck(UPDOWN_C_1o))
			curStateOut++;
		break;
	case 12:
		if (IO_ON == cylinderCheck(UPDOWN_C_2o))
			curStateOut++;
		break;
	case 13:
		if (IO_ON == cylinderCheck(UPDOWN_C_3o))
			curStateOut++;
		break;
	case 14:
		if (IO_ON == cylinderCheck(UPDOWN_C_4o))
			curStateOut++;
		break;
	case 15:
		if (IO_ON == stepFiberFlag[SlotposNo1])	{ sensorOut(Vacuum9, IO_ON);	tVector.push_back(Vacuum9); }
		if (IO_ON == stepFiberFlag[SlotposNo2])	{ sensorOut(Vacuum10, IO_ON);	tVector.push_back(Vacuum10); }
		if (IO_ON == stepFiberFlag[SlotposNo3])	{ sensorOut(Vacuum11, IO_ON);	tVector.push_back(Vacuum11); }
		if (IO_ON == stepFiberFlag[SlotposNo4])	{ sensorOut(Vacuum12, IO_ON);	tVector.push_back(Vacuum12); }

		if (true == vacuumCheckUp(tVector) || modeRunNullFlag)
		{
			curStateOut++;
		}
		break;
	case 16:
		if (mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.ngPositionY2))
			curStateOut++;
		break;
	case 17:
		curStateFlow4 = 0;
		curStateOut = -1;
		break;
	default:
		break;
	}
}

void HSMeasure::caseFlow4()	//下料过程
{
	static DWORD curSystemTime;

	switch (curStateFlow4)
	{
	case 0:
		showMsgSignal(QStringLiteral("下料..."));
		if (!modeRepeatFlag)
		{
			lastResults.append(fixValue);
			saveValueToLog(fixValue);
			showValueToUi(fixValue);
			showMsgSignal(QStringLiteral("saveValueToLog..."));
		}
		curStateFlow4++;
		break;
	case 1:
		if (curStateOut == -1)
			curStateFlow4++;
		break;
	case 2:
		if (ngUnloadReady())
			curStateFlow4++;
		break;
	case 3:
		curSystemTime = GetTickCount();
		curStateFlow4++;
		break;
	case 4:
		if (ngUnloading(curSystemTime))
			curStateFlow4++;
		break;
	case 5:
		if (unloadDone())
		{
			curStateNgBreak = 0;
			curStateFlow4++;
		}
			
		break;
	case 6:	
		if (okUnloadReady())
			curStateFlow4++;
		break;	
	case 7:
		curSystemTime = GetTickCount();
		curStateFlow4++;
		break;
	case 8:
		if (okUnloading(curSystemTime))
			curStateFlow4++;
		break;
		
	case 9:
		if (unloadDone())
			curStateFlow4++;
		break;
	case 10:
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][unloadAxisNo]->moveAndCheck(posionFromIni.ngPositionY2))
		{
			curStateFlow4++;
		}
		break;
	case 11:
		curStateFlow4++;
		break;
	case 12:
		curStateFlow4++;
		break;
	case 13:
		curStateFlow4 = -1;
		break;
	default:
		break;
	}
}

void HSMeasure::caseNgBreak()
{
	static DWORD curSystemTime;

	switch (curStateNgBreak)
	{

		static std::array<int, 2> curNgType;
	case 0:
		//X3运动初始位
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][rotateAxisNo]->moveAndCheck(posionFromIni.ngBreakPositionHome))
			curStateNgBreak++;
		break;
	case 1:
		if (ngTypeQueue.isEmpty())
		{
			curStateNgBreak = -1;
			break;
		}

		curNgType = ngTypeQueue.dequeue();

		curNgType[0] = 3 - curNgType[0];

		curStateNgBreak++;
		break;
	case 2:
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][rotateAxisNo]->moveAndCheck(posionFromIni.ngBreakPositionBegin + posionFromIni.ngBreakPositionSpace * curNgType[0]))
			curStateNgBreak++;
		break;
		
	case 3:
		if (BBREAK == 1)
		{
			if (MsResultNgGap == curNgType[1])	{ cylinderMove(UPDOWN_NG1_I); }
			if (MsResultNgStep == curNgType[1])	{ cylinderMove(UPDOWN_NG1_I); cylinderMove(UPDOWN_NG2_I); }
			if (MsResultNgBoth == curNgType[1])	{ cylinderMove(UPDOWN_NG1_I); cylinderMove(UPDOWN_NG2_I); cylinderMove(UPDOWN_NG3_I); }
		}
			
		curSystemTime = GetTickCount();
		curStateNgBreak++;
		break;
	case 4:
		if (GetTickCount() - curSystemTime > CYLINDER_DELAY)
			curStateNgBreak++;
		break;
	case 5:
		cylinderMove(UPDOWN_NG1_o);
		cylinderMove(UPDOWN_NG2_o);
		cylinderMove(UPDOWN_NG3_o);
		curStateNgBreak++;
		break;
	case 6:
		if (ngTypeQueue.size()!=0)
		{
			curStateNgBreak = 1;
		}
		else
		{
			if (true == mpMOTIONLib->mpDmcAxis[mCardNo][rotateAxisNo]->moveAndCheck(40000))
				curStateNgBreak++;
			break;			
		}
		break;
	
	case 7:
		if (true == mpMOTIONLib->mpDmcAxis[mCardNo][rotateAxisNo]->moveAndCheck(posionFromIni.ngBreakPositionHome))
			curStateNgBreak++;
		break;
	case 8:
		curStateNgBreak = -1;
	default:
		break;
	}
}

void MY_THREAD::run()  //运动流程
{	
	while (true)
	{
		short ioStart = mpHSMeasure->sensorIn(START);
		short ioConfirm = mpHSMeasure->sensorIn(CONFIRM);
		short ioStop = mpHSMeasure->sensorIn(STOP);
		short ioReset = mpHSMeasure->sensorIn(RESET);
		short ioEmgStop = mpHSMeasure->sensorIn(EMGSTOP);
		mpHSMeasure->modeRepeatFlag = mpHSMeasure->sensorIn(SWITCH_MODE);

		if (1 == ioEmgStop)
		{		
			mpHSMeasure->hs_stop();
			break;
		}

		if (IO_ON == ioStop)
		{
			mpHSMeasure->mRunFlag.mbPause = true;
		}

		if (IO_ON == ioReset)
		{
			mpHSMeasure->sensorOut(RedLight, IO_OFF);
			mpHSMeasure->sensorOut(Buzzer, IO_OFF);
		}
		

		if (true == mpHSMeasure->mRunFlag.mbStop)
		{
			mpHSMeasure->mpDMC5000Lib->emgStop();
			break;
		}

		if (true == mpHSMeasure->mRunFlag.mbPause)
		{
			continue;
		}
		
		mpHSMeasure->caseHome();

		if (mpHSMeasure->mRunFlag.mbHome != true)
		{
			continue;
		}

		mpHSMeasure->stateMonitor();

		if (false == mpHSMeasure->scanAlarmSafe())
		{
			continue;
		}
			
		mpHSMeasure->caseFlow1();
		mpHSMeasure->caseFlow2();
		mpHSMeasure->caseFlow3();
		mpHSMeasure->caseFlow4();
		mpHSMeasure->caseNgBreak();
		mpHSMeasure->caseOut();
		mpHSMeasure->caseOpposite();

		QSettings settings(mPath +"/cfg/config.ini", QSettings::IniFormat);
		int bStart = settings.value("DEBUG/BSTART").toInt();

		if ((IO_ON == ioStart) || 1 == bStart)
		{
			settings.setValue("DEBUG/BSTART","0");
			
			if (-1 == mpHSMeasure->curStateFlow1)
			{
				mpHSMeasure->curStateFlow1 = 0;
			}		
		}
			
		QApplication::processEvents();
		QThread::msleep(5);
	}

}

inline bool HSMeasure::netPing()
{
	if (mpTcpClientCcd[0]->connect() == false)
	{
		show_msg("Ccd1 connect error.");
		return false;
	}
	mpTcpClientCcd[0]->disConnect();

	if (mpTcpClientCcd[1]->connect() == false)
	{
		show_msg("Ccd2 connect error.");
		return false;
	}
	mpTcpClientCcd[1]->disConnect();
	
	//
	if (m_pLoadDllfunc->LJIF_OpenDeviceETHER(&laser1_config) != 0)
	{
		show_msg("laser1_config connect error.");
		return false;
	}
	m_pLoadDllfunc->LJIF_CloseDevice();

	if (m_pLoadDllfunc->LJIF_OpenDeviceETHER(&laser2_config) != 0)
	{
		show_msg("laser2_config connect error.");
		return false;
	}
	m_pLoadDllfunc->LJIF_CloseDevice();

	if (m_pLoadDllfunc->LJIF_OpenDeviceETHER(&laser3_config) != 0)
	{
		show_msg("laser3_config connect error.");
		return false;
	}
	m_pLoadDllfunc->LJIF_CloseDevice();


	/*
	if (mpTcpClientLaser[0]->connect() == false)
	{
		show_msg("Laser1 connect error.");
		return;
	}
	mpTcpClientLaser[0]->disConnect();

	if (mpTcpClientLaser[1]->connect() == false)
	{
		show_msg("Laser2 connect error.");
		return;
	}
	mpTcpClientLaser[1]->disConnect();

	if (mpTcpClientLaser[2]->connect() == false)
	{
		show_msg("Laser3 connect error.");
		return;
	}
	mpTcpClientLaser[2]->disConnect();
	*/

	return true;
}

bool HSMeasure::dogCheck()
{
	ErrorPrinter errorPrinter;
	dogStatus status;

	//
	/*
	CDog dog1(CDogFeature::defaultFeature());

	status = dog1.login(vendorCode);

	if (!DOG_SUCCEEDED(status))
	{
		QMessageBox::warning(this, "", QStringLiteral("请插入超级狗"));
		return false;
	}
	*/

	//
	CDogFeature feature25 = CDogFeature::fromFeature(25);
	const char* defaultScope = "<dogscope />";
	CDog dog25(feature25);
	status = dog25.login(vendorCode, defaultScope);

	if (!DOG_SUCCEEDED(status))
	{
		QMessageBox::warning(this, "", QStringLiteral("超级狗feature错误"));
		return false;
	}

	std::string info;
	status = dog25.getSessionInfo(CDog::keyInfo(), info);

	if (!DOG_SUCCEEDED(status) || info.size() < 200)
	{
		QMessageBox::warning(this, "", "Error: getSessionInfo()");
		return false;
	}

	auto id = info.substr(147, 9);

	if (id != "586805081" && id != "205610107" && id != "125401969")
	{
		QMessageBox::warning(this, "", QStringLiteral("超级狗ID错误"));
		return false;
	}


	dog25.logout();


	/*
	std::string infoTemplate =
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
	"<dogformat root=\"dogscope\">"
	"<dog>"
	"<attribute name=\"id\" />"
	"</dog>"
	"</dogformat>";

	std::string resultingScope;
	status = dog1.getSessionInfo(infoTemplate, resultingScope);
	*/

	return true;
}

void HSMeasure::hs_start()  //启动button clicked
{	
	//
	if (false == dogCheck())
	{
		return;
	}

	//

	if (false == netPing())
	{
		return;
	}

	ui.pushButtonStart->setStyleSheet("QPushButton{background:lightgreen}");
	ableSignal(ui.pushButtonStart, false);
	ableSignal(ui.pushButtonOut, false);

	mRunFlag.mbStop = false;

	initValue();

	mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->sevonOnOff(DMC_SERV_ENABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->sevonOnOff(DMC_SERV_ENABLE);

	//	
	std::shared_ptr <MY_THREAD> pWorkThread = std::make_shared<MY_THREAD>(this);
	connect(&pWorkThread->mThread, &QThread::started, &*pWorkThread, &MY_THREAD::run);
	this->pWorkThread = pWorkThread;

//	QTimer::singleShot(0, &*pWorkThread, SLOT(run()));
}

void HSMeasure::show_msg(const QString &msg)
{
	curErrorMsg = msg;

	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_time = current_date_time.toString("yyyy-MM-dd HH:mm:ss:zzz");
	ui.plainTextEditShowMsg->appendPlainText("["+ current_time +"] " + msg);
	qDebug() << msg;
}

void HSMeasure::show_dialog(const QString &msg)
{
	QMessageBox::information(this, "", msg);
}

void HSMeasure::load_ini()
{
	QString fileName;
	fileName = QCoreApplication::applicationDirPath();
	fileName += "/cfg/config.ini";

	QSettings settings(fileName, QSettings::IniFormat);
	QString str_path = settings.value("path_name/path").toString();

	ccdAxisNo = settings.value("AXIS_NO_CFG/ccdAxisNo").toInt();
	laserAxisNo = settings.value("AXIS_NO_CFG/laserAxisNo").toInt();
	platformAxisNo = settings.value("AXIS_NO_CFG/platformAxisNo").toInt();
	rotateAxisNo = settings.value("AXIS_NO_CFG/rotateAxisNo").toInt();
	unloadAxisNo = settings.value("AXIS_NO_CFG/unloadAxisNo").toInt();

	posionFromIni.LaserSafePosition = settings.value("POSITION/LaserSafePosition").toInt();
	posionFromIni.loadPositionY1 = settings.value("POSITION/loadPositionY1").toInt();
	posionFromIni.unloadPositionY1 = settings.value("POSITION/unloadPositionY1").toInt();
	posionFromIni.loadPositionY2 = settings.value("POSITION/loadPositionY2").toInt();
	posionFromIni.unloadPositionY2 = settings.value("POSITION/unloadPositionY2").toInt();
	posionFromIni.stepPositionY2 = settings.value("POSITION/stepPositionY2").toInt();
	posionFromIni.ngPositionY2 = settings.value("POSITION/ngPositionY2").toInt();
	posionFromIni.okPositionY2 = settings.value("POSITION/okPositionY2").toInt();
	posionFromIni.ngBreakPositionHome = settings.value("POSITION/ngBreakPositionHome").toInt();
	posionFromIni.ngBreakPositionBegin = settings.value("POSITION/ngBreakPositionBegin").toInt();
	posionFromIni.ngBreakPositionSpace = settings.value("POSITION/ngBreakPositionSpace").toInt();
	//
	posionFromIni.basePositionLaser = settings.value("POSITION/basePositionLaser").toInt();
	posionFromIni.basePositionX1 = settings.value("POSITION/basePositionX1").toInt();
	posionFromIni.basePositionY1 = settings.value("POSITION/basePositionY1").toInt();
	posionFromIni.basePositionX2 = settings.value("POSITION/basePositionX2").toInt();
	posionFromIni.basePositionY2 = settings.value("POSITION/basePositionY2").toInt();

	//
	speedFromIni.ccdHomeSpeed = settings.value("SPEED/ccdHomeSpeed").toDouble();
	speedFromIni.laserHomeSpeed = settings.value("SPEED/laserHomeSpeed").toDouble();
	speedFromIni.platHomeSpeed = settings.value("SPEED/platHomeSpeed").toDouble();
	speedFromIni.rotateHomeSpeed = settings.value("SPEED/rotateHomeSpeed").toDouble();
	speedFromIni.unloadHomeSpeed = settings.value("SPEED/unloadHomeSpeed").toDouble();
	
	speedFromIni.ccdJogSpeed = settings.value("SPEED/ccdJogSpeed").toDouble();
	speedFromIni.laserJogSpeed = settings.value("SPEED/laserJogSpeed").toDouble();
	speedFromIni.platJogSpeed = settings.value("SPEED/platJogSpeed").toDouble();
	speedFromIni.rotateJogSpeed = settings.value("SPEED/rotateJogSpeed").toDouble();
	speedFromIni.unloadJogSpeed = settings.value("SPEED/unloadJogSpeed").toDouble();
	speedFromIni.jogPulse = settings.value("SPEED/jogPulse").toInt();

	speedFromIni.ccdAutoSpeed = settings.value("SPEED/ccdAutoSpeed").toDouble();
	speedFromIni.laserAutoSpeed = settings.value("SPEED/laserAutoSpeed").toDouble();
	speedFromIni.platAutoSpeed = settings.value("SPEED/platAutoSpeed").toDouble();
	speedFromIni.rotateAutoSpeed = settings.value("SPEED/rotateAutoSpeed").toDouble();
	speedFromIni.unloadAutoSpeed = settings.value("SPEED/unloadAutoSpeed").toDouble();
	//
	offsetGap_L = settings.value("OFFSET/offsetGap_L").toFloat();
	offsetGap_H = settings.value("OFFSET/offsetGap_H").toFloat();
	offsetStep_L = settings.value("OFFSET/offsetStep_L").toFloat();
	offsetStep_H = settings.value("OFFSET/offsetStep_H").toFloat();

	//
	BBREAK = settings.value("DEBUG/BBREAK").toInt();
	modeRunNullFlag = settings.value("DEBUG/modeRunNullFlag").toInt();
	CYLINDER_DELAY = settings.value("DEBUG/CYLINDER_DELAY").toInt();
	SpeedPercent = settings.value("DEBUG/SpeedPercent").toFloat();
}

void HSMeasure::save_ini()
{
	QString fileName;
	fileName = QCoreApplication::applicationDirPath();
	fileName += "/cfg/config.ini";

	QSettings settings(fileName, QSettings::IniFormat);
	QString str_path = settings.value("path_name/path").toString();

	settings.setValue("CCD_CFG/IP1", ui.lineEditCcdIp1->text());
	settings.setValue("CCD_CFG/IP2", ui.lineEditCcdIp2->text());
	settings.setValue("CCD_CFG/UART1", ui.lineEditCcdUart1->text());
	settings.setValue("CCD_CFG/UART2", ui.lineEditCcdUart2->text());

	settings.setValue("LASER_CFG/IP1", ui.lineEditLaserIp1->text());
	settings.setValue("LASER_CFG/IP2", ui.lineEditLaserIp2->text());
	settings.setValue("LASER_CFG/IP3", ui.lineEditLaserIp3->text());

	settings.setValue("POSITION/basePositionLaser", ui.lineEdit_laserBasePosition->text());

	settings.setValue("POSITION/basePositionX1", ui.lineEdit_ccdBasePositionX1->text());
	settings.setValue("POSITION/basePositionY1", ui.lineEdit_ccdBasePositionY1->text());
	settings.setValue("POSITION/basePositionX2", ui.lineEdit_ccdBasePositionX2->text());
	settings.setValue("POSITION/basePositionY2", ui.lineEdit_ccdBasePositionY2->text());
}

void HSMeasure::save_log()
{
	QString path = create_file_path("D:/qdir/", "log.txt");  //file name = dir_ + nowtime + file_
	write_log_to_txt(path);
}

QString HSMeasure::create_file_path(const QString & dir_, const QString & file_)
{
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_time = current_date_time.toString("yyyyMMdd_HHmmss_");
	QString fileName;
	fileName = dir_;
	if (fileName.endsWith("/"))
	{ 
	}
	else
	{
		fileName = fileName.append("/");
	}

	QDir dir(fileName);
	if (dir.exists())
	{
	}
	else
	{
		dir.mkpath(fileName);
	}

	fileName += current_time + file_;
	return fileName;	
}

void HSMeasure::write_log_to_txt(const QString &file_name)
{
	QFile file(file_name);//文件命名  
	if (!file.open(QFile::WriteOnly | QFile::Text))     //检测文件是否打开  
	{
		QMessageBox::information(this, "Error Message", "Please close the Text File!");
		return;
	}
	QTextStream out(&file);   
	m_mutex.lock();
	out << ui.plainTextEditShowMsg->toPlainText();   
	ui.plainTextEditShowMsg->setPlainText("");  //清空msg
	m_mutex.unlock();
	show_msg(QStringLiteral("log保存路径：") + file_name);
	file.close();
}

void HSMeasure::closeEvent(QCloseEvent *event)
{
	save_ini();

	mpDMC5000Lib->emgStop();

	mpDMC5000Lib->mpDmcAxis[mCardNo][ccdAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][laserAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][rotateAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][platformAxisNo]->sevonOnOff(DMC_SERV_DISABLE);
	mpDMC5000Lib->mpDmcAxis[mCardNo][unloadAxisNo]->sevonOnOff(DMC_SERV_DISABLE);

	pCsvWavePosition->clear();

	for (size_t i = 0; i < 6 + 1; i++)
	{
		QStringList tList;

		for (size_t j = 0; j < 4 + 1; j++)
		{
			tList.append(vWavePosition[i][j]);
		}

		pCsvWavePosition->append(tList);
	}

	dmc_board_close();
}

void HSMeasure::deal_raw_data()
{
	int inum = 0;
	QVector<double> x(800), y(800);
	QString dbFileName = "C:\\data1.txt";
	QFile sptabFile(dbFileName);
	if (sptabFile.exists())
	{
		int j = 0;
	}
	else
	{
		int k = 0;
	}

	if (!sptabFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Can't open the file!" << endl;
	}
	while (!sptabFile.atEnd())
	{

		//QCoreApplication::processEvents();
		QString line = sptabFile.readLine();
		y[inum] = line.toFloat();
		x[inum] = inum;
		inum++;

	}  //end while		

	m_laserEditor[0]->show_line(x, y);
}

void HSMeasure::onToValShowTimer()
{
	mpBCDH_step->ShowStepValueToView();
}

void HSMeasure::get_laser_result(int controlNo, float* result)
{
	QString strValue[2];
	LJIF_MEASUREDATA* pMeasureData = new LJIF_MEASUREDATA[2];
	
	switch (controlNo)
	{
	case 1:
		m_pLoadDllfunc->LJIF_OpenDeviceETHER(&laser1_config);
		break;
	case 2:
		m_pLoadDllfunc->LJIF_OpenDeviceETHER(&laser2_config);
		break;
	case 3:
		m_pLoadDllfunc->LJIF_OpenDeviceETHER(&laser3_config);
		break;
	default:
		break;
	}
	
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

	delete[]pMeasureData;

	result[0] = strValue[0].toFloat();
	result[1] = strValue[1].toFloat();
}
