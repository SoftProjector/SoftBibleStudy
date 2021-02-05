/***************************************************************************
//
//    softProjector - an open source media projection software
//    Copyright (C) 2017  Vladislav Kobzar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
***************************************************************************/

#include "generalsettingwidget.hpp"
#include "ui_generalsettingwidget.h"

GeneralSettingWidget::GeneralSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettingWidget)
{
    ui->setupUi(this);
}

GeneralSettingWidget::~GeneralSettingWidget()
{
    delete ui;
}

void GeneralSettingWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
     switch ( e->type() ) {
     case QEvent::LanguageChange:
         ui->retranslateUi(this);
         break;
     default:
         break;
     }
}

//void GeneralSettingWidget::setSettings(GeneralSettings settings)
//{
//    mySettings = settings;
//    loadSettings();
//}

void GeneralSettingWidget::loadSettings()
{


}


//GeneralSettings GeneralSettingWidget::getSettings()
//{
//    mySettings.displayIsOnTop = ui->checkBoxDisplayOnTop->isChecked();
//    mySettings.displayOnStartUp = ui->checkBoxDisplayOnStartUp->isChecked();

//    int tmx = ui->comboBoxTheme->currentIndex();
//    if(tmx != -1)
//        mySettings.currentThemeId = themeIdList.at(tmx);
//    else
//        mySettings.currentThemeId = 0;

//    mySettings.displayScreen = ui->comboBoxDisplayScreen->currentIndex();
//    mySettings.displayScreen2 = monitors.indexOf(ui->comboBoxDisplayScreen_2->currentText());

//    mySettings.displayControls.buttonSize = ui->comboBoxIconSize->currentIndex();
//    mySettings.displayControls.alignmentV = ui->comboBoxControlsAlignV->currentIndex();
//    mySettings.displayControls.alignmentH = ui->comboBoxControlsAlignH->currentIndex();
//    qreal r = ui->horizontalSliderOpacity->value();
//    r = r/100;
//    mySettings.displayControls.opacity = r;

//    return mySettings;
//}

void GeneralSettingWidget::on_pushButtonDefault_clicked()
{
//    GeneralSettings g;
//    mySettings = g;
    loadSettings();
}

//void GeneralSettingWidget::updateSecondaryDisplayScreen()
//{
//    QString ds1 = ui->comboBoxDisplayScreen->currentText();
//    QString ds2 = ui->comboBoxDisplayScreen_2->currentText();
//    QStringList monitors2 = monitors;
//    monitors2.removeOne(ds1);

//    ui->comboBoxDisplayScreen_2->clear();
//    ui->comboBoxDisplayScreen_2->addItem(tr("None"));
//    ui->comboBoxDisplayScreen_2->addItems(monitors2);

//    int i = ui->comboBoxDisplayScreen_2->findText(ds2);
//    if(i != -1)
//        ui->comboBoxDisplayScreen_2->setCurrentIndex(i);
//    else
//    {
//        ui->comboBoxDisplayScreen_2->setCurrentIndex(0);
//        emit setDisp2Use(false);
//    }
//}



