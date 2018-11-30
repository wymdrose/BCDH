#ifndef LASEREDIT_H
#define LASEREDIT_H

#include "Lib/Laser/laseredit_global.h"
#include "3rdparty/qcustomplot.h"

class LASEREDIT_EXPORT laserEdit : public QWidget
{
	Q_OBJECT

public:
	explicit laserEdit(QWidget * parent = NULL, bool flag = false);
	~laserEdit();
	void set_x_label(const QString &label);
	void set_graphy_size(uint x, uint y);
	void ready_show_line();
	void show_line(const QVector<double> &x, const QVector<double> &y);
private:
	QCustomPlot* m_plot;
	
private slots:
	
	
};

#endif // LASEREDIT_H
