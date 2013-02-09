#include <QListView>
#include <QPushButton>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtDebug>
#include <QMessageBox>
#include <QStringListModel>

#include "keywordslistdialog.h"

KeywordListDialog::KeywordListDialog(QWidget *parent)
		: QDialog(parent)
{
	_allKeywords=-1;
	setWindowTitle(tr("Keyword list"));

	//controls
	model=new QStringListModel;
	view=new QListView;
	view->setModel(model);
	view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	view->setEditTriggers(QListView::NoEditTriggers);

	buttonAddKeyword=new QPushButton(QIcon(":/icons/addkeywords"),
									 tr("Add keyword"));
	connect(buttonAddKeyword,SIGNAL(clicked()), this,SLOT(addKeyword()));
	buttonRemoveUnused=new QPushButton(QIcon(":/icons/removeunused"),
									 tr("Remove unused"));
	connect(buttonRemoveUnused,SIGNAL(clicked()), this,SLOT(removeUnused()));

	buttonPick=new QPushButton(QIcon(":/icons/ok"),tr("Pick"));
	connect(buttonPick,SIGNAL(clicked()), this,SLOT(accept()));
	buttonPick->setEnabled(false);
	buttonCancel=new QPushButton(QIcon(":/icons/cancel"),tr("Cancel"));
	connect(buttonCancel,SIGNAL(clicked()), this,SLOT(reject()));

	connect(view->selectionModel(),
			SIGNAL(selectionChanged(const QItemSelection &,
									const QItemSelection &)),
			this,SLOT(changeSelection(const QItemSelection &,
						  const QItemSelection &)));

	//layout
	QVBoxLayout *vbox=new QVBoxLayout;
	vbox->addWidget(buttonAddKeyword);
	vbox->addWidget(buttonRemoveUnused);
	vbox->addWidget(view);
		QHBoxLayout *hbox=new QHBoxLayout;
		hbox->addStretch(1);
		hbox->addWidget(buttonPick);
		hbox->addWidget(buttonCancel);
		hbox->addStretch(1);
	vbox->addLayout(hbox);
	setLayout(vbox);

	//list
	_allKeywords=aDB->createEmptyExtracts();
	if(!aDB->fillExtractsWithAllKeywords(_allKeywords))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	repaintList();
}

KeywordListDialog::~KeywordListDialog()
{
	if(!aDB->removeExtracts(_allKeywords))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
}

void KeywordListDialog::repaintList()
{
	if(_allKeywords<0)
		return;

	QHash<int,QString> list=aDB->stringsOfExtracts(_allKeywords);
	QList<int> keys=list.keys();

	model->removeRows(0,model->rowCount());
	keysForModel.clear();
	foreach(int key,keys)
	{
		model->insertRow(model->rowCount());
		model->setData(
				model->index(model->rowCount()-1,0),
				list[key],Qt::DisplayRole);
		keysForModel<<key;
	}
}

QSet<int> KeywordListDialog::selection()
{
	QModelIndexList list=view->selectionModel()->selectedIndexes();

	QSet<int> result;
	foreach(QModelIndex index,list)
		result<<keysForModel.value(index.row());

	return result;
}

void KeywordListDialog::addKeyword()
{
	bool ok=false;
	QString newKeyword=QInputDialog::getText(this,tr("New keyword"),
			tr("Input new keyword:"),QLineEdit::Normal,QString(),&ok);
	if(ok)
	{
		if(!aDB->addNewKeyword(newKeyword))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		if(!aDB->fillExtractsWithAllKeywords(_allKeywords))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		repaintList();
	}
}

void KeywordListDialog::removeUnused()
{
	aDB->removeUnusedKeywords();
	repaintList();
}

void KeywordListDialog::changeSelection(const QItemSelection &selected,
						  const QItemSelection &deselected)
{
	buttonPick->setEnabled(
			view->selectionModel()->selectedIndexes().count()>0);
}