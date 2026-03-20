#include "toolsettingsdialog.h"

#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

ToolSettingsDialog::ToolSettingsDialog(qtllm::toolsstudio::ToolStudioSettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_themeCombo(new QComboBox(this))
    , m_uiFontCombo(new QFontComboBox(this))
    , m_uiFontSizeSpin(new QSpinBox(this))
    , m_editorFontCombo(new QFontComboBox(this))
    , m_editorFontSizeSpin(new QSpinBox(this))
    , m_showBuiltinCheck(new QCheckBox(QStringLiteral("Show built-in tools"), this))
    , m_showDisabledCheck(new QCheckBox(QStringLiteral("Show disabled tools"), this))
    , m_includeDescendantsCheck(new QCheckBox(QStringLiteral("Include descendants by default"), this))
{
    setWindowTitle(QStringLiteral("Tool Studio Settings"));

    m_themeCombo->addItem(QStringLiteral("System"), QStringLiteral("system"));
    m_themeCombo->addItem(QStringLiteral("Light"), QStringLiteral("light"));
    m_themeCombo->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));
    m_themeCombo->addItem(QStringLiteral("Sepia"), QStringLiteral("sepia"));

    m_uiFontSizeSpin->setRange(8, 24);
    m_editorFontSizeSpin->setRange(8, 24);

    if (m_settings) {
        const int themeIndex = m_themeCombo->findData(m_settings->theme());
        m_themeCombo->setCurrentIndex(themeIndex >= 0 ? themeIndex : 0);
        if (!m_settings->uiFontFamily().isEmpty()) {
            m_uiFontCombo->setCurrentFont(QFont(m_settings->uiFontFamily()));
        }
        m_uiFontSizeSpin->setValue(m_settings->uiFontPointSize());
        if (!m_settings->editorFontFamily().isEmpty()) {
            m_editorFontCombo->setCurrentFont(QFont(m_settings->editorFontFamily()));
        }
        m_editorFontSizeSpin->setValue(m_settings->editorFontPointSize());
        m_showBuiltinCheck->setChecked(m_settings->showBuiltinTools());
        m_showDisabledCheck->setChecked(m_settings->showDisabledTools());
        m_includeDescendantsCheck->setChecked(m_settings->includeDescendantsByDefault());
    }

    auto *form = new QFormLayout();
    form->addRow(QStringLiteral("Theme"), m_themeCombo);
    form->addRow(QStringLiteral("UI Font"), m_uiFontCombo);
    form->addRow(QStringLiteral("UI Font Size"), m_uiFontSizeSpin);
    form->addRow(QStringLiteral("Editor Font"), m_editorFontCombo);
    form->addRow(QStringLiteral("Editor Font Size"), m_editorFontSizeSpin);
    form->addRow(QString(), m_showBuiltinCheck);
    form->addRow(QString(), m_showDisabledCheck);
    form->addRow(QString(), m_includeDescendantsCheck);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &ToolSettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ToolSettingsDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void ToolSettingsDialog::accept()
{
    if (m_settings) {
        m_settings->setTheme(m_themeCombo->currentData().toString());
        m_settings->setUiFontFamily(m_uiFontCombo->currentFont().family());
        m_settings->setUiFontPointSize(m_uiFontSizeSpin->value());
        m_settings->setEditorFontFamily(m_editorFontCombo->currentFont().family());
        m_settings->setEditorFontPointSize(m_editorFontSizeSpin->value());
        m_settings->setShowBuiltinTools(m_showBuiltinCheck->isChecked());
        m_settings->setShowDisabledTools(m_showDisabledCheck->isChecked());
        m_settings->setIncludeDescendantsByDefault(m_includeDescendantsCheck->isChecked());
    }
    QDialog::accept();
}
