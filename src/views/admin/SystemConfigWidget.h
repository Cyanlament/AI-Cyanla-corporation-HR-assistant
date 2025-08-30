#ifndef SYSTEMCONFIGWIDGET_H
#define SYSTEMCONFIGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QSlider>
#include <QDialog>

class SystemConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemConfigWidget(QWidget *parent = nullptr);

private slots:
    void onSaveConfig();
    void onResetConfig();
    void onImportConfig();
    void onExportConfig();
    void onAddFAQ();
    void onEditFAQ();
    void onDeleteFAQ();
    void onAddDepartment();
    void onEditDepartment();
    void onDeleteDepartment();

private:
    void setupUI();
    void setupGeneralTab();
    void setupAIConfigTab();
    void setupFAQTab();
    void setupDepartmentTab();
    void loadConfig();
    void saveConfig();

private:
    QVBoxLayout* m_mainLayout;
    
    // 顶部按钮
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_btnSave;
    QPushButton* m_btnReset;
    QPushButton* m_btnImport;
    QPushButton* m_btnExport;
    
    // 选项卡
    QTabWidget* m_tabWidget;
    
    // 常规设置选项卡
    QWidget* m_generalTab;
    QLineEdit* m_systemName;
    QLineEdit* m_systemVersion;
    QSpinBox* m_maxUsers;
    QSpinBox* m_sessionTimeout;
    QCheckBox* m_enableLogging;
    QCheckBox* m_enableBackup;
    QComboBox* m_logLevel;
    
    // AI配置选项卡
    QWidget* m_aiConfigTab;
    QSlider* m_aiConfidence;
    QSpinBox* m_maxResponse;
    QCheckBox* m_enableAutoReply;
    QCheckBox* m_enableLearning;
    QTextEdit* m_aiPrompt;
    QLineEdit* m_apiEndpoint;
    QLineEdit* m_apiKey;
    
    // FAQ管理选项卡
    QWidget* m_faqTab;
    QTableWidget* m_faqTable;
    QPushButton* m_btnAddFAQ;
    QPushButton* m_btnEditFAQ;
    QPushButton* m_btnDeleteFAQ;
    
    // 部门管理选项卡
    QWidget* m_departmentTab;
    QTableWidget* m_departmentTable;
    QPushButton* m_btnAddDept;
    QPushButton* m_btnEditDept;
    QPushButton* m_btnDeleteDept;
     QPushButton* m_btnSearchDept;
    QLineEdit* m_nameEdit;
    QLineEdit* m_descriptionEdit;
};

// FAQ编辑对话框
class FAQEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FAQEditDialog(QWidget *parent = nullptr);
    void setFAQData(const QString& question, const QString& answer, const QString& category);
    
    QString getQuestion() const;
    QString getAnswer() const;
    QString getCategory() const;

private:
    void setupUI();

private:
    QVBoxLayout* m_layout;
    QLineEdit* m_editQuestion;
    QTextEdit* m_editAnswer;
    QComboBox* m_comboCategory;
    QPushButton* m_btnOk;
    QPushButton* m_btnCancel;
};

// 部门编辑对话框
class DepartmentEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DepartmentEditDialog(QWidget *parent = nullptr);
    void setDepartmentData(const QString& name, const QString& location, 
                          const QString& phone, const QString& description);
    
    QString getName() const;
    QString getLocation() const;
    QString getPhone() const;
    QString getDescription() const;

private:
    void setupUI();

private:
    QVBoxLayout* m_layout;
    QLineEdit* m_editName;
    QLineEdit* m_editLocation;
    QLineEdit* m_editPhone;
    QTextEdit* m_editDescription;
    QPushButton* m_btnOk;
    QPushButton* m_btnCancel;
};

#endif // SYSTEMCONFIGWIDGET_H 
