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

#include <QtSql>
#include "settingsdialog.hpp"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    generalSettingswidget = new GeneralSettingWidget;

    ui->scrollAreaGeneralSettings->setWidget(generalSettingswidget);


    btnOk = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnApply = new QPushButton(tr("Apply"));

    ui->buttonBox->addButton(btnOk,QDialogButtonBox::AcceptRole);
    ui->buttonBox->addButton(btnCancel,QDialogButtonBox::RejectRole);
    ui->buttonBox->addButton(btnApply,QDialogButtonBox::ApplyRole);

    // Connect display screen slot
    connect(generalSettingswidget,SIGNAL(setDisp2Use(bool)),this,SLOT(setUseDispScreen2(bool)));
    connect(generalSettingswidget,SIGNAL(themeChanged(int)),this,SLOT(changeTheme(int)));


}

void SettingsDialog::loadSettings()
{




}

SettingsDialog::~SettingsDialog()
{
    delete ui;

    delete generalSettingswidget;

    delete btnOk;
    delete btnCancel;
    delete btnApply;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch ( e->type() ) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button == btnOk)
    {
        applySettings();
        close();
    }
    else if(button == btnCancel)
        close();
    else if(button == btnApply)
        applySettings();
}

void SettingsDialog::applySettings()
{

}

void SettingsDialog::applyToAllActive(int t, QString backName, QPixmap background)
{
    switch (t)
    {
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    default:
        break;
    }
}
