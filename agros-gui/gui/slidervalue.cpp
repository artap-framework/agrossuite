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

#include "slidervalue.h"

SliderValue::SliderValue(int precision, QWidget *parent)
{
    info = new QLabel(this);
    slider = new DoubleSlider(precision, this);
    slider->setTickInterval(precision);
    QObject::connect(slider, &QSlider::valueChanged, this, &SliderValue::sliderChanged);

    auto *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->setContentsMargins(1, 1, 1, 1);
    layoutHorizontal->addWidget(slider);
    layoutHorizontal->addWidget(info);

    setLayout(layoutHorizontal);
}

