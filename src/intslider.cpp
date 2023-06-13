#include "intslider.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QStyleOptionProgressBar>
#include <QElapsedTimer>
#include <utility>

ValueSliders::IntSlider::IntSlider(QString name)
        : name_(std::move(name)) {
    init();
}

ValueSliders::IntSlider::IntSlider(QString name, int value)
        : name_(std::move(name)),
          value_(value) {
    init();
}


ValueSliders::IntSlider::IntSlider(QString name, int value, int min, int max, bool allowOutside)
        : allowOutside_(allowOutside),
          name_(std::move(name)),
          value_(value),
          min_(min),
          max_(max) {
    init();
}

void ValueSliders::IntSlider::init() {
    setFocusPolicy(Qt::StrongFocus);
    setMinimum(min_);
    setMaximum(max_);
    setValue(value_);

    blinkerTimer_ = std::make_shared<QTimer>(this);
    connect(blinkerTimer_.get(), &QTimer::timeout, this, &ValueSliders::IntSlider::toggleBlinkerVisibility);
    oldBase_ = palette().color(QPalette::Base);
    oldSheet_ = styleSheet();
    setStyleSheet(QString("QProgressBar::chunk:disabled { background-color: %1; }").arg(oldBase_.name()));
}

void ValueSliders::IntSlider::toggleBlinkerVisibility() {
    blinkerVisible_ = !blinkerVisible_;
    update();
}

QString ValueSliders::IntSlider::text() const {
    return "";
}

void ValueSliders::IntSlider::startTyping() {
    setFocus();
    grabMouse();
    grabKeyboard();
    select();
    setEnabled(true);
    typeInput_ = "";
    typing_ = true;
    blinkerTimer_->start(blinkerInterval_);
    setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(oldBase_.name()));
    update();
}

void ValueSliders::IntSlider::stopTyping() {
    releaseKeyboard();
    releaseMouse();
    blinkerTimer_->stop();
    typing_ = false;
    setVal(value_);
    setStyleSheet(oldSheet_);
    unselect();
    update();
}

void ValueSliders::IntSlider::paintEvent(QPaintEvent *event) {
    QProgressBar::paintEvent(event);

    QPainter painter(this);
    QStyleOptionProgressBar option;
    initStyleOption(&option);
    if (typing_) {
        QRect rect = style()->subElementRect(QStyle::SE_ProgressBarContents, &option, this);
        rect.setX(rect.x() + padding_);

        bool start = typeInput_ == "";
        QFontMetrics fontMetrics(font());

        painter.save();
        QString nameText = start ? QString::number(value_) : typeInput_;
        QSize textSize = fontMetrics.size(Qt::TextSingleLine, nameText);

        QRect textRect = rect;
        textRect.setWidth(textSize.width() + padding_);
        textRect.moveLeft(padding_ / 2);
        QColor highlightColor = palette().color(QPalette::Highlight);

        painter.setBrush(QBrush(highlightColor));
        painter.setPen(Qt::NoPen);
        if (start) {
            painter.drawRect(textRect);
        }

        if (blinkerVisible_) {
            QRect blinkerRect(textRect.x() + textRect.width() - padding_ / 2, textRect.y(), 2, textRect.height());
            painter.fillRect(blinkerRect, palette().color(QPalette::WindowText));
        }

        painter.restore();

        painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, nameText);
    } else {
        QRect rect = style()->subElementRect(QStyle::SE_ProgressBarContents, &option, this);
        rect.setX(rect.x() + padding_);
        QString nameText = name_;
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, nameText);

        QString valueText = QString::number(value_);
        QRect valueRect = rect.adjusted(QFontMetrics(font()).horizontalAdvance(nameText), 0, -padding_, 0);
        painter.drawText(valueRect, Qt::AlignRight | Qt::AlignVCenter, valueText);
    }

}

void ValueSliders::IntSlider::select() {
    QPalette curPalette = QProgressBar::palette();
    curPalette.setColor(QPalette::Base, palette().color(QPalette::AlternateBase));
    QProgressBar::setPalette(curPalette);
}

void ValueSliders::IntSlider::unselect() {
    QPalette curPalette = QProgressBar::palette();
    curPalette.setColor(QPalette::Base, oldBase_);
    QProgressBar::setPalette(curPalette);
}

void ValueSliders::IntSlider::mousePressEvent(QMouseEvent *event) {
    event->accept();
    setFocus();
    if (typing_) {
        stopTyping();
    }
    select();
    mouseMoved_ = false;
}

void ValueSliders::IntSlider::mouseMoveEvent(QMouseEvent *event) {
    if (typing_) {
        event->ignore();
        return;
    }
    if (event->buttons() & Qt::LeftButton) {
        updateValueByPosition(event->pos().x());
        mouseMoved_ = true;
        event->accept();
        return;
    }
}

void ValueSliders::IntSlider::mouseReleaseEvent(QMouseEvent *event) {
    if (typing_) {
        return;
    }
    if (mouseMoved_) {
        if (event->button() == Qt::LeftButton) {
            updateValueByPosition(event->pos().x());
            unselect();
        }
    } else {
        startTyping();
    }
    event->accept();
}

void ValueSliders::IntSlider::updateValueByPosition(int x) {
    double ratio = static_cast<double>(x) / width();
    double val = minimum() + ratio * (maximum() - minimum());
    setVal(int(std::round(val)));
    setEnabled(true);
}

void ValueSliders::IntSlider::mouseDoubleClickEvent(QMouseEvent *event) {
    startTyping();
    event->accept();
}

void ValueSliders::IntSlider::keyPressEvent(QKeyEvent *event) {
    if (typing_) {
        event->accept();
        if (event->key() == Qt::Key_Escape) {
            stopTyping();
            setEnabled(true);
            return;
        }
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            bool ok;
            double newVal = typeInput_.toDouble(&ok);
            if (ok) {
                setVal(int(std::round(newVal)));
            }
            stopTyping();
            setEnabled(true);
            return;
        }
        if (event->key() == Qt::Key_Backspace) {
            typeInput_.chop(1);
            update();
            return;
        }
        typeInput_ += event->text();
        update();
    }
}

void ValueSliders::IntSlider::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    stopTyping();
}

void ValueSliders::IntSlider::setVal(int value) {
    if (value_ == value) {
        return;
    }
    if (allowOutside_) {
        value_ = value;
    } else {
        value_ = std::clamp(value, min_, max_);
    }
    setValue(std::clamp(value, minimum(), maximum()));
    update();
    emit valueChanged(value_);
}

int ValueSliders::IntSlider::getVal() const {
    return value_;
}
