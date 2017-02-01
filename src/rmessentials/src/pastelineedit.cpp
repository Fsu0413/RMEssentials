#include "pastelineedit.h"

#ifdef MOBILE_DEVICES

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPoint>
#include <QTimer>

PasteLineEdit::PasteLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_menu(nullptr)
    , m_timer(nullptr)
{
    init();
}

PasteLineEdit::PasteLineEdit(const QString &str, QWidget *parent)
    : QLineEdit(str, parent)
    , m_menu(nullptr)
    , m_timer(nullptr)
{
    init();
}

PasteLineEdit::~PasteLineEdit()
{
}

void PasteLineEdit::mousePressEvent(QMouseEvent *event)
{
    QLineEdit::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        m_point = event->pos();
        m_timer->start();
    }
}

void PasteLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
    QLineEdit::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        m_timer->stop();
        m_point = QPoint();
    }
}

void PasteLineEdit::mouseMoveEvent(QMouseEvent *event)
{
    QLineEdit::mouseMoveEvent(event);

    if (!m_point.isNull() && (event->buttons() & Qt::LeftButton)) {
        if ((event->pos() - m_point).manhattanLength() > 10) {
            m_timer->stop();
            m_point = QPoint();
        }
    }
}

void PasteLineEdit::init()
{
    m_menu = new QMenu(this);
    QAction *paste = m_menu->addAction(tr("Paste"));
    connect(paste, &QAction::triggered, this, &PasteLineEdit::paste);

    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    m_timer->setSingleShot(true);

    connect(m_timer, &QTimer::timeout, this, &PasteLineEdit::popup);
}

void PasteLineEdit::popup()
{
    QClipboard *clipboard = qApp->clipboard();
    const QMimeData *mime = clipboard->mimeData();
    if (mime->hasHtml() || mime->hasText())
        m_menu->popup(mapToGlobal(m_point));
}

#endif
