#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QListView>
#include <QMessageBox>
#include <QToolBar>
#include <QSplitter>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QtDebug>
#include <QBitmap>
#include <QToolButton>
#include <QLineEdit>

#include "keywordslistdialog.h"
#include "notedialog.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_chosenKeywords=-1;
	_allKeywords=-1;

	//settings - size and pos
	QSettings settings;
	resize(settings.value("mainwindow/size",size()).toSize());
	move(settings.value("mainwindow/pos",pos()).toPoint());
	if(settings.value("mainwindow/maximized",false).toBool())
		setWindowState(Qt::WindowMaximized);

	//controls
	createActions();

	modelChosenKeywords=new QStringListModel;
	ui.listChosenKeywords->setModel(modelChosenKeywords);
	connect(ui.listChosenKeywords->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelectionForChosenKeywords(const QItemSelection &,
					  const QItemSelection &)));
	connect(ui.listChosenKeywords,SIGNAL(doubleClicked(const QModelIndex &)),
		this,SLOT(chosenKeywordsDoubleClick(const QModelIndex &)));

	modelAllKeywords=new QStringListModel;
	proxyAllKeywords = new QSortFilterProxyModel;
	proxyAllKeywords->setDynamicSortFilter(true);
	proxyAllKeywords->sort(0);
	proxyAllKeywords->setSourceModel(modelAllKeywords);
	proxyAllKeywords->setFilterCaseSensitivity(Qt::CaseInsensitive);
	ui.listAllKeywords->setModel(proxyAllKeywords);
	connect(ui.listAllKeywords->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelectionForAllKeywords(const QItemSelection &,
					  const QItemSelection &)));
	connect(ui.listAllKeywords,SIGNAL(doubleClicked(const QModelIndex &)),
		this,SLOT(allKeywordsDoubleClick(const QModelIndex &)));

	modelFoundNotes=new QStringListModel;
	proxyFoundNotes = new QSortFilterProxyModel;
	proxyFoundNotes->setDynamicSortFilter(true);
	proxyFoundNotes->sort(0);
	proxyFoundNotes->setSourceModel(modelFoundNotes);
	ui.listFoundNotes->setModel(proxyFoundNotes);
	connect(ui.listFoundNotes->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelectionForFoundNotes(const QItemSelection &,
					  const QItemSelection &)));
	connect(ui.listFoundNotes,SIGNAL(doubleClicked(const QModelIndex &)),
		this,SLOT(noteDoubleClick(const QModelIndex &)));

	ui.buttonAddKeywords->setDefaultAction(ui.actionAddKeywords);
	ui.buttonRemoveKeywords->setDefaultAction(ui.actionRemoveKeywords);

	connect(ui.editSearch,SIGNAL(textChanged(const QString&)),
			this,SLOT(search(const QString&)));

	//base
	if(!aDB->load("database"))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	_chosenKeywords=aDB->createEmptyExtracts();
	_allKeywords=aDB->createEmptyExtracts();
	if(!aDB->fillExtractsWithAllKeywords(_allKeywords))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	repaintList();
}

void MainWindow::search(const QString &text)
{
	proxyAllKeywords->setFilterFixedString(text);
}

MainWindow::~MainWindow()
{
	//settings - size and pos
	QSettings settings;
	settings.setValue("mainwindow/maximized",
					  windowState().testFlag(Qt::WindowMaximized));
	if(!windowState().testFlag(Qt::WindowMaximized))
	{
		settings.setValue("mainwindow/size",size());
		settings.setValue("mainwindow/pos",pos());
	}

	//base
	if(!aDB->removeExtracts(_chosenKeywords))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	if(!aDB->save("database"))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
}

void MainWindow::createActions()
{
	connect(ui.actionExit,SIGNAL(triggered()), qApp,SLOT(quit()));

	ui.actionAddKeywords->setStatusTip(tr("Add more words to current filter"));
	ui.actionAddKeywords->setEnabled(false);
	connect(ui.actionAddKeywords,SIGNAL(triggered()), this,SLOT(addKeywords()));

	ui.actionRemoveKeywords->setStatusTip(tr("Remove selected words from filter"));
	ui.actionRemoveKeywords->setEnabled(false);
	connect(ui.actionRemoveKeywords,SIGNAL(triggered()),
			this,SLOT(removeKeywords()));

	connect(ui.actionAddNote,SIGNAL(triggered()), this,SLOT(addNote()));

	connect(ui.actionEditNote,SIGNAL(triggered()), this,SLOT(editNote()));

	connect(ui.actionRemoveNote,SIGNAL(triggered()), this,SLOT(removeNote()));

	connect(ui.actionHelp,SIGNAL(triggered()), this,SLOT(help()));
}

void MainWindow::repaintList()
{
	//keywords
	QHash<int,QString> list=aDB->stringsOfExtracts(_chosenKeywords);
	QList<int> keys=list.keys();

	modelChosenKeywords->removeRows(0,modelChosenKeywords->rowCount());
	keysForModelChosenKeywords.clear();
	foreach(int key,keys)
	{
		modelChosenKeywords->insertRow(modelChosenKeywords->rowCount());
		modelChosenKeywords->setData(
				modelChosenKeywords->index(modelChosenKeywords->rowCount()-1,0),
				list[key],Qt::DisplayRole);
		keysForModelChosenKeywords<<key;
	}

	//keywords
	if(!aDB->fillExtractsWithAllKeywords(_allKeywords))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	if(!aDB->subtractExtracts(_allKeywords,_chosenKeywords))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	QHash<int,QString> listAll=aDB->stringsOfExtracts(_allKeywords);
	QList<int> keysAll=listAll.keys();

	modelAllKeywords->removeRows(0,modelAllKeywords->rowCount());
	keysForModelAllKeywords.clear();
	foreach(int key,keysAll)
	{
		modelAllKeywords->insertRow(modelAllKeywords->rowCount());
		modelAllKeywords->setData(
				modelAllKeywords->index(modelAllKeywords->rowCount()-1,0),
				listAll[key],Qt::DisplayRole);
		keysForModelAllKeywords<<key;
	}

	//note list
	QSet<int> foundNotes=aDB->getNotesByKeywords(_chosenKeywords);

	modelFoundNotes->removeRows(0,modelFoundNotes->rowCount());
	keysForModelFoundNotes.clear();
	foreach(int key,foundNotes)
	{
		modelFoundNotes->insertRow(modelFoundNotes->rowCount());
		modelFoundNotes->setData(
				modelFoundNotes->index(modelFoundNotes->rowCount()-1,0),
				aDB->title(key),Qt::DisplayRole);
		keysForModelFoundNotes<<key;
	}
}

QSet<int> MainWindow::selectionForChosen()
{
	QModelIndexList list=ui.listChosenKeywords->selectionModel()->
						 selectedIndexes();

	QSet<int> result;
	foreach(QModelIndex index,list)
		result<<keysForModelChosenKeywords.value(index.row());

	return result;
}

QSet<int> MainWindow::selectionForAll()
{
	QModelIndexList sourceList=ui.listAllKeywords->selectionModel()->
						 selectedIndexes();
	QModelIndexList list;
	foreach(const QModelIndex & index, sourceList) {
		list << proxyAllKeywords->mapToSource(index);
	}

	QSet<int> result;
	foreach(QModelIndex index,list)
		result<<keysForModelAllKeywords.value(index.row());

	return result;
}

void MainWindow::allKeywordsDoubleClick(const QModelIndex &/*index*/)
{
	addKeywords();
}

void MainWindow::chosenKeywordsDoubleClick(const QModelIndex &/*index*/)
{
	removeKeywords();
}

void MainWindow::noteDoubleClick(const QModelIndex &index)
{
	//get selected note
	int selectedNote=keysForModelFoundNotes.value(proxyFoundNotes->mapToSource(index).row());

	//dialog
	NoteDialog dialog(this);

	int tmp=aDB->createEmptyExtracts();
	if(!aDB->fillExtractsWithNoteAssociation(selectedNote,tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	dialog.setAssociations(tmp);
	dialog.setTitle(aDB->title(selectedNote));
	dialog.setText(aDB->text(selectedNote));

	if(dialog.exec())
	{
		if(!aDB->setNoteAssociationFromExtracts(selectedNote,tmp))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		if(!aDB->setTitle(selectedNote,dialog.title()))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		if(!aDB->setText(selectedNote,dialog.text()))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	}

	if(!aDB->removeExtracts(tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	//repaint
	int lastIndex=proxyFoundNotes->mapToSource(index).row();
	repaintList();
	ui.listFoundNotes->setCurrentIndex(proxyFoundNotes->mapFromSource(modelFoundNotes->index(lastIndex,0)));
}

void MainWindow::addKeywords()
{
	int tmp=aDB->createEmptyExtracts();
	if(!aDB->fillExtractsWithKeys(tmp,selectionForAll()))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	aDB->stringsOfExtracts(tmp);
	if(!aDB->subtractExtracts(_allKeywords,tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	if(!aDB->addExtracts(_chosenKeywords,tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	repaintList();
	if(!aDB->removeExtracts(tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
/*	KeywordListDialog dialog(this);
	if(dialog.exec())
	{
		int tmp=aDB->createEmptyExtracts();
		if(!aDB->fillExtractsWithKeys(tmp,dialog.selection()))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		aDB->stringsOfExtracts(tmp);
		if(!aDB->addExtracts(_chosenKeywords,tmp))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		repaintList();
		if(!aDB->removeExtracts(tmp))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	}*/
}

void MainWindow::removeKeywords()
{
	int tmp=aDB->createEmptyExtracts();
	if(!aDB->fillExtractsWithKeys(tmp,selectionForChosen()))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	aDB->stringsOfExtracts(tmp);
	if(!aDB->subtractExtracts(_chosenKeywords,tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	if(!aDB->addExtracts(_allKeywords,tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	repaintList();
	if(!aDB->removeExtracts(tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
}

void MainWindow::editNote()
{
	noteDoubleClick(proxyFoundNotes->mapToSource(ui.listFoundNotes->currentIndex()));
}

void MainWindow::removeNote()
{
	//get selected note
	int selectedNote=keysForModelFoundNotes.value(
			proxyFoundNotes->mapToSource(ui.listFoundNotes->currentIndex()).row());

	//dialog
	if(QMessageBox::question(this,tr("Note removing"),tr("Are you sure want to "
			"remove this note?"),QMessageBox::Yes,QMessageBox::Cancel)==
		QMessageBox::Yes)
	{
		if(!aDB->removeNote(selectedNote))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	}

	//repaint
	int lastIndex=proxyFoundNotes->mapToSource(ui.listFoundNotes->currentIndex()).row();
	repaintList();
	ui.listFoundNotes->setCurrentIndex(proxyFoundNotes->mapFromSource(modelFoundNotes->index(lastIndex,0)));
}

void MainWindow::addNote()
{
	NoteDialog dialog(this);
	int tmp=aDB->createEmptyExtracts();

	dialog.setAssociations(tmp);
	if(dialog.exec())
	{
		int newNote=aDB->addNewNote(dialog.title(),dialog.text());
		aDB->setNoteAssociationFromExtracts(newNote,tmp);
	}

	if(!aDB->removeExtracts(tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	repaintList();
}

void MainWindow::help()
{
	QMessageBox::about(this,tr("Small help"),
					   tr("<i>Small help must be there</i>"));
}

void MainWindow::changeSelectionForChosenKeywords(
		const QItemSelection &/*selected*/,
		const QItemSelection &/*deselected*/)
{
	ui.actionRemoveKeywords->setEnabled(
			ui.listChosenKeywords->selectionModel()->selectedIndexes().count()>0);
	ui.listAllKeywords->selectionModel()->clear();
}

void MainWindow::changeSelectionForFoundNotes(
		const QItemSelection &/*selected*/,
		const QItemSelection &/*deselected*/)
{
	ui.actionEditNote->setEnabled(
			ui.listFoundNotes->selectionModel()->selectedIndexes().count()>0);
	ui.actionRemoveNote->setEnabled(
			ui.listFoundNotes->selectionModel()->selectedIndexes().count()>0);
}

void MainWindow::changeSelectionForAllKeywords(
		const QItemSelection &/*selected*/,
		const QItemSelection &/*deselected*/)
{
	ui.actionAddKeywords->setEnabled(
			ui.listAllKeywords->selectionModel()->selectedIndexes().count()>0);
	ui.listChosenKeywords->selectionModel()->clear();
}

