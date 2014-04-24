#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMultiHash>
#include <QSet>

#include "associativedatabase.h"
#include "ui_mainwindow.h"

class QStringListModel;
class QSortFilterProxyModel;
class QItemSelection;
class QModelIndex;

class MainWindow : public QMainWindow
{
	Q_OBJECT
	Ui::MainWindow ui;

	int _chosenKeywords,_allKeywords;

	QList<int> keysForModelChosenKeywords,keysForModelFoundNotes,
		keysForModelAllKeywords;
	QStringListModel *modelChosenKeywords,*modelFoundNotes,*modelAllKeywords;
	QSortFilterProxyModel *proxyChosenKeywords,*proxyFoundNotes,*proxyAllKeywords;

	void createActions();

	void repaintList();
	QSet<int> selectionForChosen();
	QSet<int> selectionForAll();
private slots:
	void changeSelectionForChosenKeywords(const QItemSelection &selected,
						  const QItemSelection &deselected);
	void changeSelectionForFoundNotes(const QItemSelection &selected,
						  const QItemSelection &deselected);
	void changeSelectionForAllKeywords(const QItemSelection &selected,
						  const QItemSelection &deselected);

	void noteDoubleClick(const QModelIndex &index);
	void allKeywordsDoubleClick(const QModelIndex &index);
	void chosenKeywordsDoubleClick(const QModelIndex &index);

	void search(const QString &);
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
	void addKeywords();
	void removeKeywords();
	void editNote();
	void addNote();
	void removeNote();
	void help();
};

#endif // MAINWINDOW_H
