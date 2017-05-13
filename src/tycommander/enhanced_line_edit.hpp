/* TyTools - public domain
   Niels Martignène <niels.martignene@gmail.com>
   https://neodd.com/tytools

   This software is in the public domain. Where that dedication is not
   recognized, you are granted a perpetual, irrevocable license to copy,
   distribute, and modify this file as you see fit.

   See the LICENSE file for more details. */

#ifndef ENHANCED_LINE_EDIT_HH
#define ENHANCED_LINE_EDIT_HH

#include <QStringList>
#include <QComboBox>

class EnhancedLineEdit: public QComboBox {
    Q_OBJECT

    int wheel_delta_ = 0;

public:
    EnhancedLineEdit(QWidget *parent = nullptr);

public slots:
    void appendHistory(const QString &text);
    void commit();

signals:
    void textCommitted(const QString &text);

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

private:
    void moveInHistory(int movement);
};

#endif
