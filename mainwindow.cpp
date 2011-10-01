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
	createToolbar();
	statusBar();

	modelChosenKeywords=new QStringListModel;
	listChosenKeywords=new QListView;
	listChosenKeywords->setModel(modelChosenKeywords);
	listChosenKeywords->setSelectionMode(
			QAbstractItemView::ExtendedSelection);
	listChosenKeywords->setEditTriggers(QListView::NoEditTriggers);
	connect(listChosenKeywords->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelectionForChosenKeywords(const QItemSelection &,
					  const QItemSelection &)));
	connect(listChosenKeywords,SIGNAL(doubleClicked(const QModelIndex &)),
		this,SLOT(chosenKeywordsDoubleClick(const QModelIndex &)));

	modelAllKeywords=new QStringListModel;
	listAllKeywords=new QListView;
	listAllKeywords->setModel(modelAllKeywords);
	listAllKeywords->setSelectionMode(
			QAbstractItemView::ExtendedSelection);
	listAllKeywords->setEditTriggers(QListView::NoEditTriggers);
	connect(listAllKeywords->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelectionForAllKeywords(const QItemSelection &,
					  const QItemSelection &)));
	connect(listAllKeywords,SIGNAL(doubleClicked(const QModelIndex &)),
		this,SLOT(allKeywordsDoubleClick(const QModelIndex &)));

	modelFoundNotes=new QStringListModel;
	listFoundNotes=new QListView;
	listFoundNotes->setModel(modelFoundNotes);
	listFoundNotes->setSelectionMode(
				QAbstractItemView::SingleSelection);
	listFoundNotes->setEditTriggers(QListView::NoEditTriggers);
	connect(listFoundNotes->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelectionForFoundNotes(const QItemSelection &,
					  const QItemSelection &)));
	connect(listFoundNotes,SIGNAL(doubleClicked(const QModelIndex &)),
		this,SLOT(noteDoubleClick(const QModelIndex &)));

	buttonAddKeywords=new QToolButton;
	buttonAddKeywords->setDefaultAction(actionAddKeywords);
	buttonAddKeywords->setAutoRaise(true);
	buttonRemoveKeywords=new QToolButton;
	buttonRemoveKeywords->setDefaultAction(actionRemoveKeywords);
	buttonRemoveKeywords->setAutoRaise(true);

	editSearch=new QLineEdit;
	connect(editSearch,SIGNAL(textChanged(const QString&)),
			this,SLOT(search(const QString&)));

	//layout
	QWidget *widget=new QWidget;
	QVBoxLayout *vbox=new QVBoxLayout;
		QHBoxLayout *hbox=new QHBoxLayout;
			QVBoxLayout *vboxAllKeywords=new QVBoxLayout;
			vboxAllKeywords->addWidget(new QLabel(tr("All words:")));
			vboxAllKeywords->addWidget(listAllKeywords);
				QHBoxLayout *hboxSearch=new QHBoxLayout;
				hboxSearch->addWidget(new QLabel(tr("Search:")));
				hboxSearch->addWidget(editSearch);
			vboxAllKeywords->addLayout(hboxSearch);
		hbox->addLayout(vboxAllKeywords);
			QVBoxLayout *vboxButtons=new QVBoxLayout;
			vboxButtons->addStretch(1);
			vboxButtons->addWidget(buttonAddKeywords);
			vboxButtons->addWidget(buttonRemoveKeywords);
			vboxButtons->addStretch(1);
		hbox->addLayout(vboxButtons);
			QVBoxLayout *vboxChosenKeywords=new QVBoxLayout;
			vboxChosenKeywords->addWidget(new QLabel(tr("Chosen words:")));
			vboxChosenKeywords->addWidget(listChosenKeywords);
		hbox->addLayout(vboxChosenKeywords);
	vbox->addLayout(hbox);
	vbox->addWidget(new QLabel(tr("Found notes:")));
	vbox->addWidget(listFoundNotes);

	widget->setLayout(vbox);
	setCentralWidget(widget);

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
	int key=aDB->search(_allKeywords,text);
	if(key<0) return;

	int index=keysForModelAllKeywords.indexOf(key);
	if(index<0) return;

	listAllKeywords->setCurrentIndex(modelAllKeywords->index(index,0));
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
	actionExit=new QAction(QIcon(":/icons/app"),tr("Exit"),this);
	actionExit->setShortcut(tr("Ctrl+Q","action-quit"));
	actionExit->setStatusTip(tr("Exits the application"));
	actionExit->setToolTip(tr("This will be exit the application"));
	connect(actionExit,SIGNAL(triggered()), qApp,SLOT(quit()));

	actionAddKeywords=new QAction(QIcon(":/icons/buttonaddkeywords"),
								  tr("Add words"),this);
	actionAddKeywords->setShortcut(tr("Plus","action-add words"));
	actionAddKeywords->setStatusTip(tr("Add more words to current filter"));
	actionAddKeywords->setToolTip(tr("Add words to current filter and redo "
								  "search"));
	actionAddKeywords->setEnabled(false);
	connect(actionAddKeywords,SIGNAL(triggered()), this,SLOT(addKeywords()));

	actionRemoveKeywords=new QAction(QIcon(":/icons/buttonremovekeywords"),
									 tr("Delete words"),this);
	actionRemoveKeywords->setShortcut(tr("Minus","action-remove words"));
	actionRemoveKeywords->setStatusTip(tr("Remove selected words from filter"));
	actionRemoveKeywords->setToolTip(tr("Remove selected words from filter and"
									 " redo search"));
	actionRemoveKeywords->setEnabled(false);
	connect(actionRemoveKeywords,SIGNAL(triggered()),
			this,SLOT(removeKeywords()));

	actionAddNote=new QAction(QIcon(":/icons/addnote"),tr("Add note"),this);
	actionAddNote->setShortcut(tr("Ctrl+N","action-add note"));
	actionAddNote->setStatusTip(tr("Add new note"));
	actionAddNote->setToolTip(tr("Add new note and associate it with "
								 "keywords"));
	connect(actionAddNote,SIGNAL(triggered()), this,SLOT(addNote()));

	actionEditNote=new QAction(QIcon(":/icons/editnote"),tr("Edit note"),this);
	actionEditNote->setShortcut(tr("Enter","action-edit note"));
	actionEditNote->setStatusTip(tr("Edit selected note"));
	actionEditNote->setToolTip(tr("Edit note that selected in the list of "
								  "found notes"));
	actionEditNote->setEnabled(false);
	connect(actionEditNote,SIGNAL(triggered()), this,SLOT(editNote()));

	actionRemoveNote=new QAction(QIcon(":/icons/removenote"),tr("Remove note"),
								 this);
	actionRemoveNote->setShortcut(tr("Del","action-remove note"));
	actionRemoveNote->setStatusTip(tr("Remove selected note"));
	actionRemoveNote->setToolTip(tr("Remove selected note and all its keywords "
									"associations"));
	actionRemoveNote->setEnabled(false);
	connect(actionRemoveNote,SIGNAL(triggered()), this,SLOT(removeNote()));

	actionHelp=new QAction(QIcon(":/icons/help"),tr("Help"),this);
	actionHelp->setShortcut(tr("F1","action-help"));
	actionHelp->setStatusTip(tr("Show small help info"));
	actionHelp->setToolTip(tr("Show small help info"));
	connect(actionHelp,SIGNAL(triggered()), this,SLOT(help()));
}

void MainWindow::createToolbar()
{
	QToolBar *mainBar=addToolBar(tr("Main"));
	mainBar->setMovable(false);
	mainBar->setFloatable(false);
	mainBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	mainBar->addAction(actionExit);
	mainBar->addSeparator();
	mainBar->addAction(actionAddNote);
	mainBar->addAction(actionEditNote);
	mainBar->addAction(actionRemoveNote);
	mainBar->addSeparator();
	mainBar->addAction(actionHelp);
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
	QModelIndexList list=listChosenKeywords->selectionModel()->
						 selectedIndexes();

	QSet<int> result;
	foreach(QModelIndex index,list)
		result<<keysForModelChosenKeywords.value(index.row());

	return result;
}

QSet<int> MainWindow::selectionForAll()
{
	QModelIndexList list=listAllKeywords->selectionModel()->
						 selectedIndexes();

	QSet<int> result;
	foreach(QModelIndex index,list)
		result<<keysForModelAllKeywords.value(index.row());

	return result;
}

void MainWindow::allKeywordsDoubleClick(const QModelIndex &index)
{
	addKeywords();
}

void MainWindow::chosenKeywordsDoubleClick(const QModelIndex &index)
{
	removeKeywords();
}

void MainWindow::noteDoubleClick(const QModelIndex &index)
{
	//get selected note
	int selectedNote=keysForModelFoundNotes.value(index.row());

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
	int lastIndex=index.row();
	repaintList();
	listFoundNotes->setCurrentIndex(modelFoundNotes->index(lastIndex,0));
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
	noteDoubleClick(listFoundNotes->currentIndex());
}

void MainWindow::removeNote()
{
	//get selected note
	int selectedNote=keysForModelFoundNotes.value(
			listFoundNotes->currentIndex().row());

	//dialog
	if(QMessageBox::question(this,tr("Note removing"),tr("Are you sure want to "
			"remove this note?"),QMessageBox::Yes,QMessageBox::Cancel)==
		QMessageBox::Yes)
	{
		if(!aDB->removeNote(selectedNote))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	}

	//repaint
	int lastIndex=listFoundNotes->currentIndex().row();
	repaintList();
	listFoundNotes->setCurrentIndex(modelFoundNotes->index(lastIndex,0));
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
		const QItemSelection &selected,
		const QItemSelection &deselected)
{
	actionRemoveKeywords->setEnabled(
			listChosenKeywords->selectionModel()->selectedIndexes().count()>0);
	listAllKeywords->selectionModel()->clear();
}

void MainWindow::changeSelectionForFoundNotes(
		const QItemSelection &selected,
		const QItemSelection &deselected)
{
	actionEditNote->setEnabled(
			listFoundNotes->selectionModel()->selectedIndexes().count()>0);
	actionRemoveNote->setEnabled(
			listFoundNotes->selectionModel()->selectedIndexes().count()>0);
}

void MainWindow::changeSelectionForAllKeywords(
		const QItemSelection &selected,
		const QItemSelection &deselected)
{
	actionAddKeywords->setEnabled(
			listAllKeywords->selectionModel()->selectedIndexes().count()>0);
	listChosenKeywords->selectionModel()->clear();
}

