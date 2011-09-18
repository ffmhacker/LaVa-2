/*  LaVa 2, a inventory managment tool
    Copyright (C) 2011 - Robert Ewert - robert.ewert@gmail.com - www.robert.ewert.de.vu
    created with QtCreator(Qt 4.7.0)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CINPUTDIALOGARTICLE_H
#define CINPUTDIALOGARTICLE_H

#include <QtGui/QDialog>
#include "cworkthread.h"
#include "cinputdialogbrowsewaregroup.h"

namespace Ui {
    class CInputDialogArticle;
}

class CInputDialogArticle : public QDialog
{
    Q_OBJECT

public:
    CWorkThread * m_pThread;
    CInputdialogBrowseWaregroup m_dlgWaregroup;
    int m_iNewMakerId;
    int m_iNewWaregroupId;
    QString m_sMarkArticleName;//for edit

    explicit CInputDialogArticle(QWidget *parent = 0);
    ~CInputDialogArticle();
    bool init(int iSelectedWaregroupId=-1);
    bool set_thread(CWorkThread * pThread);
    bool get_data(CArticle & ar);
    bool set_data(CArticle & ar);

private:
    Ui::CInputDialogArticle *ui;

protected:
    void changeEvent(QEvent *e);

public slots:
    bool browse_waregroups(void);
    bool pressed_ok(void);
    bool edit_maker_combobox(QString s);
    bool checkbox_warning(void);
    bool check_user_input(void);
};

#endif // CINPUTDIALOGARTICLE_H
