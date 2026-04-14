/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "codeeditor.h"

#include <QApplication>
#include <QPainter>
#include <QTextBlock>
#include <QStyle>
#include <QTextCursor>
#include <QIcon>
#include <QtGlobal>
#include <QScrollBar>

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QScreen>
#else
#include <QDesktopWidget>
#endif
#include <QIcon>
#include <QFile>

//![constructor]

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent),
    m_search(nullptr),searchBar(nullptr), searchLabel(nullptr),
    searchPrev(nullptr),searchNext(nullptr)
{
    setWindowIcon(QIcon(":logfile"));
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    int WIDTH = 1024;
    int HEIGHT = 768;
//    setGeometry(0,0,1024,768);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QScreen *pscreen = QApplication::primaryScreen();
    int x = (pscreen->geometry().width() - WIDTH) / 2;
    int y = (pscreen->geometry().height() - HEIGHT) / 2;
#else
    QDesktopWidget *desktop = QApplication::desktop();
    int x = (desktop->width() - WIDTH) / 2;
    int y = (desktop->height() - HEIGHT) / 2;
#endif
    setGeometry(x,y,WIDTH, HEIGHT);

    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setWindowModality(Qt::WindowModal);
//    setWindowFlags( Qt::Window | Qt::CustomizeWindowHint
//                               | Qt::WindowTitleHint
//                               | Qt::WindowCloseButtonHint );
    // Connect scrollbar to our custom loader
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeEditor::updateVisibleText);
}

//![constructor]

//![extraAreaWidth]

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

bool CodeEditor::load(QString filename)
{
    m_filename = filename;
    setWindowTitle(filename);

    if (true){
        QFile file(filename);
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)){
            QString content = QString::fromUtf8(file.readAll());
            this->setPlainText(content);
            file.close();
        }
    }else{
        //TODO: "Lazy Loading" Strategy, do not load the file at once when the file is big!!
        if (file.isOpen()) file.close();

        file.setFileName(filename);
        if (!file.open(QIODevice::ReadOnly)) return false;

        fileSize = file.size();
        // Memory map the file for instant access without reading into a QByteArray
        mappedFile = file.map(0, fileSize);
        updateVisibleText();
    }
    return true;
}

void CodeEditor::closeEvent(QCloseEvent *event)
{
    emit Closing(m_filename); // send signal before closing.
    QPlainTextEdit::closeEvent(event);
}

//![extraAreaWidth]

//![slotUpdateExtraAreaWidth]

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::showSearchBar()
{
    if (!m_search) {
    //TODO: "Lazy Loading" Strategy
        m_search = new QWidget(this);
        m_search->setStyleSheet("background-color:#ebedf0;");
        m_hlsearch = new QHBoxLayout(this);
        #if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        m_hlsearch->layout()->setContentsMargins(1,1,1,1);
        #else
        m_hlsearch->layout()->setMargin(1);
        #endif
        m_search->setLayout(m_hlsearch);
        // Create a QLabel for the search label
        searchLabel = new QLabel("Search:", this);
        searchLabel->setStyleSheet("font-weight: bold;");
        m_hlsearch->addWidget(searchLabel);
        // Create a QLineEdit for searching
        searchBar = new QLineEdit(this);
        searchBar->setText(this->textCursor().selectedText());
        searchBar->setPlaceholderText("Search...");
        connect(searchBar, &QLineEdit::returnPressed, this, &CodeEditor::performSearch);
        m_hlsearch->addWidget(searchBar, 1);

        searchPrev = new QPushButton(QIcon(":/prev"), "", this);
        connect(searchPrev, &QPushButton::clicked, this, &CodeEditor::pervSearch);
        m_hlsearch->addWidget(searchPrev);

        searchNext = new QPushButton(QIcon(":/next"), "",this);
        connect(searchNext, &QPushButton::clicked, this, &CodeEditor::nextSearch);
        m_hlsearch->addWidget(searchNext);

        // Position and style the search bar and label
        m_search->setGeometry(30, 5, this->width() - 90, 25);
        //searchLabel->setGeometry(10, 5, 50, 25);
        //searchBar->setGeometry(70, 5, this->width() - 80, 25);
        m_search->show();
        // searchLabel->show();
        // searchBar->show();
        searchBar->setFocus();
    }else{
        m_search->show();
        searchBar->setFocus();
    }
}

void CodeEditor::hideSearchBar()
{
    if (!m_search) return;
    if (m_search->isVisible()){
        m_search->hide();
    }
}

void CodeEditor::pervSearch()
{
    if (!searchBar) return;
    const QString searchText = searchBar->text();
    if (!searchText.isEmpty()) {
        // Use QPlainTextEdit's find method to search
        if (!find(searchText, QTextDocument::FindBackward)) {
            searchBar->setStyleSheet("border: 1px solid red;");  // Highlight the search bar in red if not found
        } else {
            searchBar->setStyleSheet("");  // Clear the red border on success
        }
    }
}

void CodeEditor::nextSearch()
{
    if (!searchBar) return;
    const QString searchText = searchBar->text();
    if (!searchText.isEmpty()) {
        // Use QPlainTextEdit's find method to search
        if (!find(searchText)) {
            searchBar->setStyleSheet("border: 1px solid red;");  // Highlight the search bar in red if not found
        } else {
            searchBar->setStyleSheet("");  // Clear the red border on success
        }
    }
}

void CodeEditor::updateVisibleText()
{
    // 1. Calculate the target offset using a stable range.
    // Instead of using the widget's dynamic max, use a fixed virtual scale.
    int scrollValue = verticalScrollBar()->value();
    int virtualMax = 10000; // A large constant or total line count

    verticalScrollBar()->setRange(0, virtualMax);

    double pct = (double)scrollValue / virtualMax;
    qint64 targetOffset = static_cast<qint64>(pct * (fileSize - CHUNK_SIZE));

    // 2. Snap to nearest line boundary (Crucial for UTF-8 and readability)
    // You should scan backwards from targetOffset to the nearest '\n'
    // to avoid starting a view in the middle of a line.

    // 3. Update the view
    currentOffset = qBound(0LL, targetOffset, fileSize - CHUNK_SIZE);

    QByteArray chunk = QByteArray::fromRawData(
        reinterpret_cast<const char*>(mappedFile + currentOffset),
        qMin((qint64)CHUNK_SIZE, fileSize - currentOffset)
        );

    // Use a flag to prevent the scroll event from triggering itself
    m_isUpdating = true;
    setPlainText(QString::fromUtf8(chunk));
    m_isUpdating = false;

    // 4. Force the scrollbar to stay where the user put it
    verticalScrollBar()->setValue(scrollValue);
}

void CodeEditor::performSearch()
{
    if (!searchBar) return;

    const QString searchText = searchBar->text();
    if (!searchText.isEmpty()) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);  // Start from the beginning of the document
        setTextCursor(cursor);

        // Use QPlainTextEdit's find method to search
        if (!find(searchText)) {
            searchBar->setStyleSheet("border: 1px solid red;");  // Highlight the search bar in red if not found
        } else {
            searchBar->setStyleSheet("");  // Clear the red border on success
        }
    }
}

//![slotUpdateRequest]

//![resizeEvent]

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F && event->modifiers() == Qt::ControlModifier){
        // ctrl+F
        showSearchBar();
    }else if (event->key() == Qt::Key_Escape){
        //
        hideSearchBar();
    }else{
        QPlainTextEdit::keyPressEvent(event);
    }
}

void CodeEditor::wheelEvent(QWheelEvent *e)
{
    // Standard scroll behavior will trigger valueChanged -> updateVisibleText
    QPlainTextEdit::wheelEvent(e);
}

//![resizeEvent]

//![cursorPositionChanged]

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

//![cursorPositionChanged]

//![extraAreaPaintEvent_0]

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

//![extraAreaPaintEvent_0]

//![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
//![extraAreaPaintEvent_1]

//![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
//![extraAreaPaintEvent_2]

