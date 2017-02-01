#ifndef PASTELINEEDIT_H
#define PASTELINEEDIT_H

#include <QLineEdit>

#ifndef MOBILE_DEVICES
typedef QLineEdit PasteLineEdit;
#else
class QTimer;
class QMenu;
class QPoint;

class PasteLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit PasteLineEdit(QWidget *parent = Q_NULLPTR);
    explicit PasteLineEdit(const QString &str, QWidget *parent = Q_NULLPTR);
    ~PasteLineEdit();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QMenu *m_menu;
    QTimer *m_timer;
    QPoint m_point;

    void init();

private slots:
    void popup();
};
#endif

#endif // PASTELINEEDIT_H
