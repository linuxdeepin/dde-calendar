// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "colorpickerWidget.h"
#include "tabletconfig.h"
#include "commondef.h"

#include <DFontSizeManager>
#include <DFrame>
#include <DListView>
#include <DTitlebar>
#include <DSuggestButton>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDebug>

DGUI_USE_NAMESPACE

CColorPickerWidget::CColorPickerWidget(QWidget *parent)
    : DAbstractDialog(parent)
    , m_colorLabel(new ColorLabel(this))
    , m_colorSlider(new ColorSlider(this))
    , m_colHexLineEdit(new DLineEdit(this))
    , m_wordLabel(new DLabel(this))
    , m_cancelBtn(new DPushButton(this))
    , m_enterBtn(new DSuggestButton(this))
{
    qCDebug(ClientLogger) << "CColorPickerWidget constructor initialized";
    initUI();
    setColorHexLineEdit();
    moveToCenter();

    connect(m_cancelBtn, &DPushButton::clicked, this, &CColorPickerWidget::slotCancelBtnClicked);
    connect(m_enterBtn, &DPushButton::clicked, this, &CColorPickerWidget::slotEnterBtnClicked);
    connect(m_colorLabel, &ColorLabel::signalpickedColor, this, &CColorPickerWidget::slotUpdateColor);
    connect(m_colHexLineEdit, &DLineEdit::textChanged, this, &CColorPickerWidget::slotHexLineEditChange);
    connect(m_colorSlider, &ColorSlider::valueChanged, this, [this](int val) {
        qCDebug(ClientLogger) << "Color slider value changed to:" << val;
        m_colorLabel->setHue(val);
    });
}

CColorPickerWidget::~CColorPickerWidget()
{
    qCDebug(ClientLogger) << "CColorPickerWidget destructor called";
}

void CColorPickerWidget::setColorHexLineEdit()
{
    qCDebug(ClientLogger) << "CColorPickerWidget::setColorHexLineEdit - Setting up hex line edit";
    m_colHexLineEdit->setText("");
    //确认按钮初始置灰
    m_enterBtn->setDisabled(true);
    //输入框输入限制
    QRegularExpression reg("^[0-9A-Fa-f]{6}$");
    QValidator *validator = new QRegularExpressionValidator(reg, m_colHexLineEdit->lineEdit());
    m_colHexLineEdit->lineEdit()->setValidator(validator);
    setFocusProxy(m_colHexLineEdit);
}
/**
 * @brief CColorPickerDlg::initUI      初始化
 */
void CColorPickerWidget::initUI()
{
    qCDebug(ClientLogger) << "CColorPickerWidget::initUI - Initializing UI components";
    QFont mlabelTitle;
    QFont mlabelContext;
    mlabelTitle.setWeight(QFont::Bold);
    mlabelContext.setWeight(QFont::Medium);
    setFixedSize(314, 276);
    m_colorLabel->setFixedSize(294, 136);
    m_colorSlider->setFixedSize(294, 14);

    QVBoxLayout *mLayout = new QVBoxLayout(this);
    mLayout->setSpacing(12);
    mLayout->setContentsMargins(10, 10, 10, 8);
    mLayout->addWidget(m_colorLabel);
    mLayout->addWidget(m_colorSlider);

    m_wordLabel->setText(tr("Color"));
    m_strColorLabel = m_wordLabel->text();
    m_wordLabel->setFixedWidth(40);
    m_wordLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_colHexLineEdit->setClearButtonEnabled(false); //不显示清空按钮
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(6);
    inputLayout->addWidget(m_wordLabel);
    inputLayout->addWidget(m_colHexLineEdit, 1);
    mLayout->addLayout(inputLayout);
    mLayout->addSpacing(4);

    m_cancelBtn->setText(tr("Cancel", "button"));
    m_cancelBtn->setFixedSize(140, 36);
    m_enterBtn->setText(tr("Save", "button"));
    m_enterBtn->setFixedSize(140, 36);
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(5);
    btnLayout->addWidget(m_cancelBtn);
    DVerticalLine *line = new DVerticalLine(this);
    line->setObjectName("VLine");
    line->setFixedSize(3, 28);
    btnLayout->addWidget(line);
    line->show();
    btnLayout->addWidget(m_enterBtn);
    mLayout->addLayout(btnLayout);
    this->setLayout(mLayout);
    setLabelText();
    this->setFocusPolicy(Qt::TabFocus);
    setTabOrder(m_colHexLineEdit, m_cancelBtn);
    setTabOrder(m_cancelBtn, m_enterBtn);
    setTabOrder(m_enterBtn, m_colorSlider);
    qCDebug(ClientLogger) << "UI initialization completed";
}

void CColorPickerWidget::setLabelText() {
    qCDebug(ClientLogger) << "CColorPickerWidget::setLabelText - Setting label text based on locale";
    QLocale local;
    if(local.language() == QLocale::Chinese) {
        qCDebug(ClientLogger) << "Chinese locale detected, keeping original text";
        return;
    }
    QString str = m_strColorLabel;
    QFontMetrics fontMetrice(m_wordLabel->font());
    if(fontMetrice.horizontalAdvance(str) > (m_wordLabel->width()+4)) {
        qCDebug(ClientLogger) << "Text too long, applying elision";
        str = fontMetrice.elidedText(str, Qt::ElideRight, m_wordLabel->width());
    }
    m_wordLabel->setText(str);
}

void CColorPickerWidget::changeEvent(QEvent *e) {
    // qCDebug(ClientLogger) << "CColorPickerWidget::changeEvent";
    QWidget::changeEvent(e);
    if(e->type() == QEvent::FontChange) {
        // qCDebug(ClientLogger) << "CColorPickerWidget::changeEvent - Font changed, updating label text";
        setLabelText();
    }
}

void CColorPickerWidget::slotHexLineEditChange(const QString &text)
{
    qCDebug(ClientLogger) << "CColorPickerWidget::slotHexLineEditChange - Text changed to:" << text;
    QString lowerText = text.toLower();
    if (lowerText == text) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QRegularExpression rx("^[0-9a-f]{6}$");
        m_enterBtn->setDisabled(!rx.match(lowerText).hasMatch());
#else
        QRegExp rx("^[0-9a-f]{6}$");
        m_enterBtn->setDisabled(rx.indexIn(lowerText) == -1);
#endif

    } else {
        qCDebug(ClientLogger) << "Converting text to lowercase";
        m_colHexLineEdit->setText(lowerText);
    }
}

QColor CColorPickerWidget::getSelectedColor()
{
    // qCDebug(ClientLogger) << "CColorPickerWidget::getSelectedColor";
    return QColor("#" + m_colHexLineEdit->text());
}

void CColorPickerWidget::slotUpdateColor(const QColor &color)
{
    qCDebug(ClientLogger) << "CColorPickerWidget::slotUpdateColor - Updating color to:" << color;
    if (color.isValid()) {
        QString colorName = color.name().remove("#");
        qCDebug(ClientLogger) << "Setting hex edit text to:" << colorName;
        this->m_colHexLineEdit->setText(colorName);
    } else {
        qCDebug(ClientLogger) << "Invalid color provided";
    }
}

void CColorPickerWidget::slotCancelBtnClicked()
{
    qCDebug(ClientLogger) << "CColorPickerWidget::slotCancelBtnClicked - Cancel button clicked, rejecting dialog";
    reject();
}

void CColorPickerWidget::slotEnterBtnClicked()
{
    qCDebug(ClientLogger) << "CColorPickerWidget::slotEnterBtnClicked - Enter button clicked, accepting dialog";
    accept();
}

void CColorPickerWidget::keyPressEvent(QKeyEvent *e)
{
    // qCDebug(ClientLogger) << "CColorPickerWidget::keyPressEvent - Key pressed:" << e->key();
    //键盘有两个Enter按键，一个为Enter一个为Return
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        // qCDebug(ClientLogger) << "Enter/Return key pressed, not handling";
        //暂时不处理回车事件
    } else {
        DAbstractDialog::keyPressEvent(e);
    }
}
