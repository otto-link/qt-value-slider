/**
 * MIT License
 *
 * Copyright (c) 2023 Niels Bugel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QLineEdit>
#include <QElapsedTimer>

class IntegerSliderWidget : public QProgressBar {
public:
    IntegerSliderWidget() = default;

    explicit IntegerSliderWidget(QString name);

    IntegerSliderWidget(QString name, int value);

    IntegerSliderWidget(QString name, int value, int min, int max, bool allowOutside = false);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    [[nodiscard]] QString text() const override;

    void paintEvent(QPaintEvent *event) override;

    void focusOutEvent(QFocusEvent *event) override;

private:
    const int padding_ = 12;
    const int blinkerInterval_ = 500;

    QColor oldBase_;

    bool allowOutside_ = true;
    bool blinkerVisible_ = false;
    bool typing_ = false;
    QTimer *blinkerTimer_ = nullptr;
    QString typeInput_ = "";

    QString name_ = "value";
    int value_ = 50;
    int min_ = 0;
    int max_ = 100;

    void updateValueByPosition(int x);

    void init();

    void toggleBlinkerVisibility();

    void startTyping();

    void stopTyping();

    void select();

    void unselect();

    bool mouseMoved_ = false;
};
