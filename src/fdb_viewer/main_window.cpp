#include "main_window.h"
#include "file_browser.h"
#include "netdevil/database/fdb/fdb_reader.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QApplication>
#include <QTemporaryDir>

namespace fdb_viewer {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("FDB Viewer");
    resize(1100, 700);

    // Central: table view with row filter
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    rowFilter_ = new QLineEdit;
    rowFilter_->setPlaceholderText("Filter rows (SQL WHERE clause, e.g. name LIKE '%sword%')...");
    rowFilter_->setClearButtonEnabled(true);
    layout->addWidget(rowFilter_);

    tableView_ = new QTableView;
    tableView_->setAlternatingRowColors(true);
    tableView_->setSortingEnabled(true);
    tableView_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(tableView_);

    setCentralWidget(central);

    // Table list dock
    auto* tableDock = new QDockWidget("Tables", this);
    tableDock->setObjectName("FdbTables");
    auto* dockWidget = new QWidget;
    auto* dockLayout = new QVBoxLayout(dockWidget);
    dockLayout->setContentsMargins(2, 2, 2, 2);

    tableFilter_ = new QLineEdit;
    tableFilter_->setPlaceholderText("Filter tables...");
    tableFilter_->setClearButtonEnabled(true);
    dockLayout->addWidget(tableFilter_);

    tableList_ = new QListWidget;
    tableList_->setAlternatingRowColors(true);
    dockLayout->addWidget(tableList_);

    tableDock->setWidget(dockWidget);
    addDockWidget(Qt::LeftDockWidgetArea, tableDock);

    // Menu bar
    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open...", QKeySequence::Open,
                        this, &MainWindow::onFileOpen);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", QKeySequence::Quit,
                        this, &QMainWindow::close);

    // Status bar
    statusLabel_ = new QLabel("No database loaded");
    statusBar()->addPermanentWidget(statusLabel_);

    // Connections
    connect(tableList_, &QListWidget::currentTextChanged,
            this, &MainWindow::onTableSelected);
    connect(rowFilter_, &QLineEdit::returnPressed,
            this, [this]() { onSearchChanged(rowFilter_->text()); });
    connect(tableFilter_, &QLineEdit::textChanged,
            this, &MainWindow::onTableFilterChanged);
}

MainWindow::~MainWindow() {
    if (db_.isOpen()) {
        db_.close();
    }
    QSqlDatabase::removeDatabase("fdb_viewer");
    if (!tempDbPath_.isEmpty()) {
        QFile::remove(tempDbPath_);
    }
}

bool MainWindow::openFile(const QString& path) {
    // Close previous database
    if (db_.isOpen()) {
        db_.close();
        QSqlDatabase::removeDatabase("fdb_viewer");
    }
    if (!tempDbPath_.isEmpty()) {
        QFile::remove(tempDbPath_);
        tempDbPath_.clear();
    }

    QString dbPath = path;

    // If it's an FDB file, convert to temp SQLite first
    if (path.endsWith(".fdb", Qt::CaseInsensitive)) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Error",
                QString("Could not open file:\n%1").arg(path));
            return false;
        }

        QByteArray raw = file.readAll();
        std::vector<uint8_t> data(raw.begin(), raw.end());

        // Convert to temp SQLite
        static QTemporaryDir tempDir;
        if (!tempDir.isValid()) return false;

        tempDbPath_ = tempDir.path() + "/" + QFileInfo(path).baseName() + ".sqlite";

        try {
            lu::assets::fdb_to_sqlite_direct(
                std::span<const uint8_t>(data.data(), data.size()),
                tempDbPath_.toStdString());
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error",
                QString("Failed to convert FDB:\n%1").arg(e.what()));
            return false;
        }

        dbPath = tempDbPath_;
    }

    // Open SQLite database
    db_ = QSqlDatabase::addDatabase("QSQLITE", "fdb_viewer");
    db_.setDatabaseName(dbPath);
    if (!db_.open()) {
        QMessageBox::warning(this, "Error",
            QString("Failed to open database:\n%1").arg(db_.lastError().text()));
        return false;
    }

    QFileInfo fi(path);
    setWindowTitle(QString("FDB Viewer - %1").arg(fi.fileName()));

    loadTableList();

    // Select first table
    if (tableList_->count() > 0) {
        tableList_->setCurrentRow(0);
    }

    return true;
}

void MainWindow::onFileOpen() {
    QSettings settings;
    QString lastDir = settings.value("fdb_last_open_dir").toString();

    QString path = qt_common::FileBrowserDialog::getOpenFileName(this,
        "Open FDB/SQLite Database", lastDir,
        "Databases (*.fdb *.sqlite *.db);;FDB Files (*.fdb);;SQLite (*.sqlite *.db);;All Files (*)");

    if (!path.isEmpty()) {
        settings.setValue("fdb_last_open_dir", QFileInfo(path).absolutePath());
        openFile(path);
    }
}

void MainWindow::loadTableList() {
    tableList_->clear();

    QStringList tables = db_.tables();
    tables.sort(Qt::CaseInsensitive);

    for (const auto& table : tables) {
        tableList_->addItem(table);
    }

    statusLabel_->setText(QString("Tables: %1").arg(tables.size()));
}

void MainWindow::onTableSelected(const QString& tableName) {
    if (tableName.isEmpty() || !db_.isOpen()) return;

    delete model_;
    model_ = new QSqlTableModel(this, db_);
    model_->setTable(tableName);
    model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model_->select();

    // Fetch all rows
    while (model_->canFetchMore()) {
        model_->fetchMore();
    }

    tableView_->setModel(model_);
    tableView_->resizeColumnsToContents();

    rowFilter_->clear();

    statusLabel_->setText(
        QString("Table: %1 | Rows: %2 | Columns: %3")
            .arg(tableName)
            .arg(model_->rowCount())
            .arg(model_->columnCount()));
}

void MainWindow::onSearchChanged(const QString& text) {
    if (!model_) return;

    if (text.isEmpty()) {
        model_->setFilter("");
    } else {
        model_->setFilter(text);
    }
    model_->select();

    while (model_->canFetchMore()) {
        model_->fetchMore();
    }

    statusLabel_->setText(
        QString("Table: %1 | Rows: %2 (filtered) | Columns: %3")
            .arg(model_->tableName())
            .arg(model_->rowCount())
            .arg(model_->columnCount()));
}

void MainWindow::onTableFilterChanged(const QString& text) {
    QString lower = text.toLower();
    for (int i = 0; i < tableList_->count(); ++i) {
        auto* item = tableList_->item(i);
        item->setHidden(!lower.isEmpty() &&
                        !item->text().toLower().contains(lower));
    }
}

} // namespace fdb_viewer
