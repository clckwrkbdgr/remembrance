#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QMultiHash>
#include <QSet>

#include "associativedatabase.h"

class QStringListModel;
class QSortFilterProxyModel;
class QListView;
class QItemSelection;
class QModelIndex;
class QToolButton;
class QLineEdit;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	int _chosenKeywords,_allKeywords;

	QAction *actionExit,*actionAddKeywords,*actionRemoveKeywords,
		*actionAddNote,*actionEditNote,*actionRemoveNote,*actionHelp;

	QToolButton *buttonAddKeywords,*buttonRemoveKeywords;

	QList<int> keysForModelChosenKeywords,keysForModelFoundNotes,
		keysForModelAllKeywords;
	QStringListModel *modelChosenKeywords,*modelFoundNotes,*modelAllKeywords;
	QSortFilterProxyModel *proxyChosenKeywords,*proxyFoundNotes,*proxyAllKeywords;
	QListView *listChosenKeywords,*listFoundNotes,*listAllKeywords;

	QLineEdit *editSearch;

	void createActions();
	void createToolbar();

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
