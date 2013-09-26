//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.


#include "reportuser.h"
#include "ui_reportuser.h"

ReportUser::ReportUser(QWidget *parent) : QDialog(parent), ui(new Ui::ReportUser)
{
    ui->setupUi(this);
    this->user = NULL;
    this->q = NULL;
    ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pushButton->setEnabled(false);
    ui->pushButton->setText("Retrieving history...");
    QStringList header;
    ui->tableWidget->setColumnCount(5);
    header << "Page" << "Time" << "Link" << "DiffID" << "Include in report";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Messaging = false;
    report = "";
    Loading = false;
#if QT_VERSION >= 0x050000
// Qt5 code
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
// Qt4 code
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    //ui->tableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(20,-1));
    ui->tableWidget->setShowGrid(false);
    this->timer = NULL;
}

bool ReportUser::SetUser(WikiUser *u)
{
    if (q != NULL)
    {
        delete q;
        q = NULL;
    }
    this->user = u;
    ui->label->setText(u->Username);
    q = new ApiQuery();
    q->Parameters = "list=recentchanges&rcuser=" + QUrl::toPercentEncoding(u->Username) +
            "&rcprop=user%7Ccomment%7Ctimestamp%7Ctitle%7Cids%7Csizes&rclimit=20&rctype=edit%7Cnew";
    q->SetAction(ActionQuery);
    q->Process();
    this->timer = new QTimer(this);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(Tick()));
    this->timer->start(200);
    return true;
}

ReportUser::~ReportUser()
{
    delete q;
    delete ui;
}

void ReportUser::Tick()
{
    if (this->q == NULL)
    {
        return;
    }

    if (Loading)
    {
        if (q->Processed())
        {
            QDomDocument d;
            d.setContent(q->Result->Data);
            QDomNodeList results = d.elementsByTagName("rev");
            if (results.count() == 0)
            {
                ui->pushButton->setText("Error unable to retrieve report page at " + Configuration::LocalConfig_ReportPath);
                this->timer->stop();
                return;
            }
            QDomElement e = results.at(0).toElement();
            _p = e.text();
            if (!this->CheckUser())
            {
                ui->pushButton->setText("This user is already reported");
                this->timer->stop();
                return;
            }
            this->InsertUser();
            Core::EditPage(Core::AIVP, _p, "Reporting " + user->Username);
            this->timer->stop();
            ui->pushButton->setText("Reported");
            return;
        }
        return;
    }

    if (q->Processed())
    {
        Core::DebugLog(q->Result->Data);
        QDomDocument d;
        d.setContent(q->Result->Data);
        QDomNodeList results = d.elementsByTagName("rc");
        int xx = 0;
        if (results.count() > 0)
        {
            while (results.count() > xx)
            {
                QDomElement edit = results.at(xx).toElement();
                if (!edit.attributes().contains("type"))
                {
                    continue;
                }
                QString page = "unknown page";
                if (edit.attributes().contains("title"))
                {
                    page = edit.attribute("title");
                }
                QString time = "unknown time";
                if (edit.attributes().contains("timestamp"))
                {
                    time = edit.attribute("timestamp");
                }
                QString diff = "";
                if (edit.attributes().contains("revid"))
                {
                    diff = edit.attribute("revid");
                }
                QString link = Core::GetProjectScriptURL() + "index.php?title=" + page + "&diff=" + diff;
                ui->tableWidget->insertRow(0);
                ui->tableWidget->setItem(0, 0, new QTableWidgetItem(page));
                ui->tableWidget->setItem(0, 1, new QTableWidgetItem(time));
                ui->tableWidget->setItem(0, 2, new QTableWidgetItem(link));
                ui->tableWidget->setItem(0, 3, new QTableWidgetItem(diff));
                QCheckBox *Item = new QCheckBox(this);
                this->CheckBoxes.insert(0, Item);
                ui->tableWidget->setCellWidget(0, 4, Item);
                xx++;
            }
        }
        this->timer->stop();
        ui->pushButton->setEnabled(true);
        ui->pushButton->setText("Report");
    }
}

void ReportUser::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    // we need to get a report info for all selected diffs
    QString reports = "";
    int xx = 0;
    int EvidenceID = 0;
    while (xx < ui->tableWidget->rowCount())
    {
        QString page = ui->tableWidget->item(xx, 0)->text();
        if (this->CheckBoxes.count() > xx)
        {
            if (this->CheckBoxes.at(xx)->isChecked())
            {
                EvidenceID++;
                reports += "[" + QString(Core::GetProjectScriptURL() + "index.php?title=" +
                                 QUrl::toPercentEncoding(ui->tableWidget->item(xx, 0)->text()) + "&diff="
                                 + ui->tableWidget->item(xx, 3)->text()).toUtf8() + " Evidence #" + QString::number(EvidenceID) + "] ";
            }
        }
        xx++;
    }
    if (reports == "")
    {
        QMessageBox::StandardButton mb;
        mb = QMessageBox::question(this, "Question", "You didn't provide any diffs as evidence, this will make it extremery hard for administrators to figure out if this "\
                   "user is vandal or not. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
        if (mb == QMessageBox::No)
        {
            ui->pushButton->setEnabled(true);
            return;
        }
    }
    // obtain current page
    Loading = true;
    ui->pushButton->setText("Retrieving current report page");
    if (this->q != NULL)
    {
        delete q;
    }

    q = new ApiQuery();
    q->SetAction(ActionQuery);
    q->Parameters = "prop=revisions&rvprop=" + QUrl::toPercentEncoding("timestamp|user|comment|content") + "&titles=" +
            QUrl::toPercentEncoding(Configuration::LocalConfig_ReportPath);
    q->Process();
    this->report = reports;
    this->timer->start(800);
    return;
}

void ReportUser::on_pushButton_2_clicked()
{
    QUrl u = QUrl::fromEncoded(QString(Core::GetProjectWikiURL() + QUrl::toPercentEncoding
                                   (this->user->GetTalk()) + "?action=render").toUtf8());
    ui->webView->load(u);
}

void ReportUser::on_tableWidget_clicked(const QModelIndex &index)
{
    QUrl u = QUrl::fromEncoded(QString(Core::GetProjectScriptURL() + "index.php?title=" + QUrl::toPercentEncoding(ui->tableWidget->itemAt(index.row(), 0)->text()) + "&diff="
              + ui->tableWidget->item(index.row(), 3)->text()).toUtf8());
    ui->webView->load(u);
}

bool ReportUser::CheckUser()
{
    if (_p.contains(this->user->Username))
    {
        return false;
    }
    return true;
}

void ReportUser::InsertUser()
{
    QString xx = Configuration::LocalConfig_IPVTemplateReport;
    if (!this->user->IP)
    {
        xx = Configuration::LocalConfig_RUTemplateReport;
    }
    xx = xx.replace("$1", this->user->Username);
    xx = xx.replace("$2", report);
    xx = xx.replace("$3", ui->lineEdit->text());
    _p = _p + "\n" + xx;
}