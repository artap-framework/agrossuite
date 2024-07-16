// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SLIDERVALUE_H
#define SLIDERVALUE_H

#include "util/util.h"
#include "gui/other.h"

class DoubleSlider : public QSlider {
    Q_OBJECT

public:
    DoubleSlider(int precision = 1, QWidget *parent = 0) : QSlider(Qt::Horizontal, parent), m_precision(precision)
    {
    }

    void setPrecision(int precision) { m_precision = precision; }
    int precision() { return m_precision; }

    void setDoubleValue(double val) { setValue(val * m_precision); }
    double doubleValue() { return value() / ((double) m_precision); }

private:
    int m_precision;
};

class SliderValue : public QWidget
{
    Q_OBJECT
public:
    explicit SliderValue(int precision = 1, QWidget *parent = 0);

    void setMinimum(double value) { slider->setMinimum(value * slider->precision()); }
    void setMaximum(double value) { slider->setMaximum(value * slider->precision()); }
    void setTickInterval(int interval) { slider->setTickInterval(interval); }
    void setValue(double value) { slider->setDoubleValue(value); }
    double value() { return slider->doubleValue(); }

protected slots:
    void sliderChanged(double value) { info->setText(QString("%1").arg(value / ((double) slider->precision()), 0, 'f', log10(slider->precision()))); }

protected:
    DoubleSlider *slider;
    QLabel *info;
};

#endif //SLIDERVALUE_H
