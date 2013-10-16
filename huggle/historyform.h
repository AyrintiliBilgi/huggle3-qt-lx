//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef HISTORYFORM_H
#define HISTORYFORM_H

#include <QDockWidget>
#include "wikiedit.h"

namespace Ui {
class HistoryForm;
}

namespace Huggle
{
    class HistoryForm : public QDockWidget
    {
        Q_OBJECT

    public:
        explicit HistoryForm(QWidget *parent = 0);
        ~HistoryForm();
        void Update(WikiEdit* edit);

    private slots:
        void on_pushButton_clicked();

    private:
        Ui::HistoryForm *ui;
        WikiEdit* CurrentEdit;
    };
}

#endif // HISTORYFORM_H