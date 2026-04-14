#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QTableView>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QTemporaryFile>

namespace fdb_viewer {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool openFile(const QString& path);

private slots:
    void onFileOpen();
    void onTableSelected(const QString& tableName);
    void onSearchChanged(const QString& text);
    void onTableFilterChanged(const QString& text);

private:
    void loadTableList();

    QListWidget* tableList_ = nullptr;
    QLineEdit* tableFilter_ = nullptr;
    QTableView* tableView_ = nullptr;
    QLineEdit* rowFilter_ = nullptr;
    QLabel* statusLabel_ = nullptr;

    QSqlDatabase db_;
    QSqlTableModel* model_ = nullptr;
    QString tempDbPath_;
};

} // namespace fdb_viewer
