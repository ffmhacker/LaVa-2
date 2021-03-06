
/*  LaVa 2, a inventory managment tool
    Copyright (C) 2015 - Robert Ewert - robert.ewert@gmail.com - www.robert.ewert.de.vu
    created with QtCreator(Qt 4.8)

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

#include "cdlgbalancelist.h"
#include "ui_cdlgbalancelist.h"

CDlgBalanceList::CDlgBalanceList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgBalanceList)
{
    ui->setupUi(this);
    m_pThread=NULL;
    m_iArticleId=-1;
    QDate cur_dt=QDate().currentDate();
    ui->dateEditFrom->setDate(cur_dt);
    ui->dateEditTo->setDate(cur_dt);
    ui->dateEditFrom->setMaximumDate(cur_dt);
    ui->dateEditTo->setMaximumDate(cur_dt);
    ui->listWidgetArticle->addItem(QString::fromUtf8("Bitte einen Artikel auswählen..."));

    //disable auto default buttons
    ui->pushButtonBrowseArticle->setAutoDefault(false);
    ui->pushButtonExport->setAutoDefault(false);
    ui->pushButtonPrint->setAutoDefault(false);

    for(int i=0;i<ui->tableWidgetList->columnCount();i++)
        ui->tableWidgetList->setColumnWidth(i,150);

    setMaximumSize(width(),height());
    setMinimumSize(width(),height());
    ui->pushButtonBrowseArticle->setFocus();

    settings(false);//load & set settings

    //connects
    connect(ui->pushButtonBrowseArticle,SIGNAL(clicked()),this,SLOT(browse_article_dlg()));
    connect(ui->dateEditFrom,SIGNAL(dateChanged(QDate)),this,SLOT(dates_changed()));
    connect(ui->dateEditTo,SIGNAL(dateChanged(QDate)),this,SLOT(dates_changed()));
    connect(ui->pushButtonPrint,SIGNAL(clicked()),this,SLOT(print_button_press()));
    connect(ui->pushButtonExport,SIGNAL(clicked()),this,SLOT(export_button_press()));
}

CDlgBalanceList::~CDlgBalanceList()
{
    settings(true);//save settings if update
    delete ui;
}

bool CDlgBalanceList::settings(bool bUpdate)
{
    bool b;
    CSettings settings;
    QList<QString> lsSType,lsSValue,lsSUpdateType,lsSUpdateValue;
    lsSType.push_back(QString::fromUtf8("DLG_BALANCE_LIST_TABLE_COLUMNS_WIDTHS"));
    //-
    b=settings.load(lsSType,lsSValue);//load all settings
    if(b)
    {
        if(!bUpdate)//set
        {
            //table width
            if(lsSValue.count()>0)
                settings.set_table_columns_width(ui->tableWidgetList,lsSValue[0],150);
        }
        if(bUpdate)//write
        {
            //table width
            if(lsSValue.count()>0)
            {
                if(settings.get_table_columns_width(ui->tableWidgetList,lsSValue[0]))//setting change?
                {
                    lsSUpdateType.push_back(lsSType[0]);
                    lsSUpdateValue.push_back(lsSValue[0]);
                }
            }

            //write updates
            if(lsSUpdateType.count()>0 && lsSUpdateType.count()==lsSUpdateValue.count())
                b=settings.write(lsSUpdateType,lsSUpdateValue);
        }
    }
    //-
    lsSValue.clear();
    lsSType.clear();
    lsSUpdateValue.clear();
    lsSUpdateType.clear();
    return b;
}

bool CDlgBalanceList::check_user_input(void)
{
    bool b=true;
    if(ui->tableWidgetList->rowCount()<=0)//nothing in table
        b=false;
    ui->pushButtonPrint->setEnabled(b);
    ui->pushButtonExport->setEnabled(b);
    return true;
}

bool CDlgBalanceList::dates_changed(void)
{
    QDate dtFrom=ui->dateEditFrom->date();
    QDate dtTo=ui->dateEditTo->date();
    //-
    if(dtTo<dtFrom)
        ui->dateEditTo->setDate(dtFrom);
    update_balance_list();
    return true;
}

bool CDlgBalanceList::browse_article_dlg(void)
{
    CInputDialogBrowseArticle dlg;
    dlg.setWindowTitle(QString::fromUtf8("Artikelauswahl"));
    dlg.set_thread(m_pThread);
    dlg.set_data(m_iArticleId);
    if(dlg.exec())
    {
        dlg.get_data(m_iArticleId);
        update_article_info();
        update_balance_list();
    }
    return true;
}

bool CDlgBalanceList::update_article_info(void)
{
    bool b=false;
    QString s,s2;
    CArticle ar;
    if(m_pThread!=NULL)
    {
        if(m_pThread->m_pDbInterface!=NULL)
        {
            if(m_pThread->m_pDbInterface->article_get(m_iArticleId,ar))
            {
                b=true;
                ui->listWidgetArticle->clear();

                s=QString::fromUtf8("Bezeichnung: %1").arg(ar.get_name());
                ui->listWidgetArticle->addItem(s);

                s2=ar.get_articlenumber();
                if(s2.length()>0)
                {
                    s=QString::fromUtf8("1.Artikelnummer: %1").arg(s2);
                    ui->listWidgetArticle->addItem(s);
                }

                s2=ar.get_articlenumber2();
                if(s2.length()>0)
                {
                    s=QString::fromUtf8("2.Artikelnummer: %1").arg(s2);
                    ui->listWidgetArticle->addItem(s);
                }

                s2=m_pThread->m_pDbInterface->maker_get_name(ar.get_maker_id());
                if(s2.length()>0)
                {
                    s=QString::fromUtf8("Hersteller: %1").arg(s2);
                    ui->listWidgetArticle->addItem(s);
                }

                s2=m_pThread->m_pDbInterface->waregroup_get_path(ar.get_waregroup_id());
                if(s2.length()>0)
                {
                    s=QString::fromUtf8("Warengruppe: %1").arg(s2);
                    ui->listWidgetArticle->addItem(s);
                }
            }
        }
    }
    return b;
}

bool CDlgBalanceList::update_balance_list(void)
{
    if(m_pThread==NULL || m_iArticleId<0)
        return false;
    CPointerMemory memory;
    QDate dtFrom=ui->dateEditFrom->date();
    QDate dtTo=ui->dateEditTo->date();
    memory.set_tablewidget(ui->tableWidgetList);
    memory.set_date(&dtFrom);
    memory.set_date2(&dtTo);
    memory.set_int(&m_iArticleId);
    bool b=m_pThread->set_work(WORK_BALANCELIST_UPDATE_TABLEWIDGET,&memory);
    check_user_input();
    return b;
}

bool CDlgBalanceList::print_button_press(void)
{
    bool b,b2;
    CSettings settings;
    QList<int> lsInt;
    qreal margin_left=15,margin_right=10,margin_top=10,margin_bottom=10;
    bool bAllColumns=true;
    QString sMargins,sOrientation,sOriUpdate,sMarUpdate,sPrinter,sPriUpdate;
    QString sTitle=QString::fromUtf8("Artikelsaldenliste vom %1 bis %2:").arg(ui->dateEditFrom->date().toString(QString::fromUtf8("dd.MM.yyyy")),ui->dateEditTo->date().toString(QString::fromUtf8("dd.MM.yyyy")));

    //load settings
    b2=settings.load("PRINT_DIALOG_MARGINS",sMargins);
    if(b2)
    {
        b2=settings.cast_string_to_int_list(sMargins,lsInt);
        if(lsInt.count()>=4)
        {
            margin_left=lsInt[0];
            margin_top=lsInt[1];
            margin_right=lsInt[2];
            margin_bottom=lsInt[3];
        }
    }
    if(!settings.load("PRINT_DIALOG_ORIENTATION",sOrientation))
        sOrientation=QString::fromUtf8("0");//error set default

    settings.load("PRINT_DIALOG_PRINTER",sPrinter);

    //init print job
    m_print_job.m_memory.clear();
    m_print_job.set_type(PRINT_JOB_TABLE_AND_LIST);
    m_print_job.m_memory.set_tablewidget(ui->tableWidgetList);
    m_print_job.m_memory.set_bool(&bAllColumns);
    m_print_job.m_memory.set_listwidget(ui->listWidgetArticle);
    m_print_job.m_sFirstRow=sTitle;

    //init preview dlg
    QPrinter printer(QPrinter::PrinterResolution);
    b=true;
    QMessageBox msg(QMessageBox::Critical,"","");
    msg.setWindowTitle(QString::fromUtf8("Fehler"));
    msg.setText(QString::fromUtf8("Dem Betriebssystem ist kein Drucker bekannt."));
    if(!printer.isValid())
    {
        b=false;
        msg.exec();
    }
    else
    {
        printer.setDocName(QString("LaVa 2 - Artikelsaldenliste"));
        QPrintPreviewDialog previewDlg(&printer, this);
        previewDlg.setWindowTitle(QString::fromUtf8("Druckvorschau"));
        previewDlg.setWindowFlags ( Qt::Window );
        connect(&previewDlg, SIGNAL(paintRequested(QPrinter* )), SLOT(print(QPrinter* )));
        previewDlg.setMinimumSize(800,500);

        //orientation
        if(sOrientation==QString::fromUtf8("0"))
            printer.setOrientation(QPrinter::Portrait);
        else
            printer.setOrientation(QPrinter::Landscape);

        //margins
        printer.setPageMargins(margin_left ,margin_top ,margin_right ,margin_bottom ,QPrinter::Millimeter);

        //printer
        if(sPrinter.length()>0)
        {
            printer.setPrinterName(sPrinter);
            if(!printer.isValid())
            {//error, last selected printer is not online
                msg.setText(QString("Der zuletzt benutzte Drucker ist nicht erreichbar, es wird der Standarddrucker gesetzt."));
                msg.exec();
                printer.setPrinterName(QString(""));//set default printer
            }
        }

        //-
        previewDlg.showMaximized();
        previewDlg.exec();

        //get settings
        printer.getPageMargins(&margin_left ,&margin_top ,&margin_right ,&margin_bottom ,QPrinter::Millimeter); //margins
        sMarUpdate=QString("%1,").arg(margin_left);
        sMarUpdate+=QString("%1,").arg(margin_top);
        sMarUpdate+=QString("%1,").arg(margin_right);
        sMarUpdate+=QString("%1").arg(margin_bottom);

        if(printer.orientation()==0)//orientation
            sOriUpdate=QString("0");
        else
            sOriUpdate=QString("1");

        sPriUpdate=printer.printerName();

        //update settings
        if(sOrientation!=sOriUpdate && sOriUpdate.length()>0)
            settings.write(QString("PRINT_DIALOG_ORIENTATION"),sOriUpdate);
        if(sMargins!=sMarUpdate && sMarUpdate.length()>0)
            settings.write(QString("PRINT_DIALOG_MARGINS"),sMarUpdate);
        if(sPrinter!=sPriUpdate && sPriUpdate.length()>0)
            settings.write(QString("PRINT_DIALOG_PRINTER"),sPriUpdate);
    }
    lsInt.clear();
    return b;
}

bool CDlgBalanceList::export_button_press(void)
{
    QString s=QString::fromUtf8("Artikelsaldenliste_%1-%2").arg(ui->dateEditFrom->date().toString(QString::fromUtf8("dd.MM.yyyy")),ui->dateEditTo->date().toString(QString::fromUtf8("dd.MM.yyyy")));
    CExportCSV exportCSV;
    return exportCSV.write_data_list_table(this,ui->listWidgetArticle,ui->tableWidgetList,s,QString::fromUtf8(""),true);
}

bool CDlgBalanceList::print(QPrinter * pPrinter)
{
    bool b=false;
    if(pPrinter!=NULL)
    {
        m_print_job.m_pPrinter=pPrinter;
        b=m_print_job.print_table_and_list();
    }
    return b;
}
