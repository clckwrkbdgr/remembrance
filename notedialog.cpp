#include <QtDebug>
#include <QPushButton>
#include <QListView>
#include <QLineEdit>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include <QStringListModel>
#include <QColorDialog>

#include "keywordslistdialog.h"
#include "notedialog.h"

NoteDialog::NoteDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowFlags((windowFlags() ^ Qt::Dialog) | Qt::Window | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint);
	_associations=-1;
	setWindowTitle(tr("Note"));

	//controls
	buttonAddKeywords=new QPushButton(QIcon(":/icons/addkeywords"),
									  tr("Add keywords"));
	connect(buttonAddKeywords,SIGNAL(clicked()), this,SLOT(addKeywords()));
	buttonRemoveKeywords=new QPushButton(QIcon(":/icons/removekeywords"),
										 tr("Remove keywords"));
	buttonRemoveKeywords->setEnabled(false);
	connect(buttonRemoveKeywords,SIGNAL(clicked()), this,SLOT(removeKeywords()));

	modelKeywords=new QStringListModel;
	viewKeywords=new QListView;
	viewKeywords->setModel(modelKeywords);
	viewKeywords->setSelectionMode(QAbstractItemView::ExtendedSelection);
	viewKeywords->setEditTriggers(QListView::NoEditTriggers);
	connect(viewKeywords->selectionModel(),
		SIGNAL(selectionChanged(const QItemSelection &,
								const QItemSelection &)),
		this,SLOT(changeSelection(const QItemSelection &,
					  const QItemSelection &)));
	editTitle=new QLineEdit;
	connect(editTitle,SIGNAL(textEdited(const QString&)),
			this,SLOT(editText(const QString&)));
	editNote=new QTextEdit;
	connect(editNote,SIGNAL(textChanged()),	this,SLOT(changeText()));

	buttonClose=new QPushButton(QIcon(":/icons/cancel"),
								tr("Close"));
	connect(buttonClose,SIGNAL(clicked()), this,SLOT(reject()));
	buttonSave=new QPushButton(QIcon(":/icons/ok"),tr("Save and close"));
	buttonSave->setVisible(false);
	connect(buttonSave,SIGNAL(clicked()), this,SLOT(accept()));

	buttonColor=new QPushButton(tr("Set color..."));
	connect(buttonColor,SIGNAL(clicked()), this,SLOT(setColor()));

	//layout
	QVBoxLayout *vboxMain=new QVBoxLayout;
		QSplitter *hboxMain=new QSplitter;
			QFrame *frameKeywords=new QFrame;
			QVBoxLayout *vboxKeywords=new QVBoxLayout;
				QHBoxLayout *hboxToolbar=new QHBoxLayout;
				hboxToolbar->addWidget(buttonAddKeywords);
				hboxToolbar->addWidget(buttonRemoveKeywords);
			vboxKeywords->addLayout(hboxToolbar);
			vboxKeywords->addWidget(new QLabel(tr("Associated keywords:")));
			vboxKeywords->addWidget(viewKeywords);
			vboxKeywords->setContentsMargins(0,0,0,0);
			frameKeywords->setLayout(vboxKeywords);
		hboxMain->addWidget(frameKeywords);
		hboxMain->setStretchFactor(0,2);
			QFrame *frameText=new QFrame;
			QVBoxLayout *vboxText=new QVBoxLayout;
			vboxText->addWidget(new QLabel(tr("Title (are shown in list):")));
			vboxText->addWidget(editTitle);
			vboxText->addWidget(new QLabel(tr("Text of note:")));
			vboxText->addWidget(editNote);
			vboxText->addWidget(buttonColor);
			vboxText->setContentsMargins(0,0,0,0);
			frameText->setLayout(vboxText);
		hboxMain->addWidget(frameText);
		hboxMain->setStretchFactor(1,5);
	vboxMain->addWidget(hboxMain);
		QHBoxLayout *hboxButtons=new QHBoxLayout;
		hboxButtons->addStretch(1);
		hboxButtons->addWidget(buttonSave);
		hboxButtons->addWidget(buttonClose);
		hboxButtons->addStretch(1);
	vboxMain->addLayout(hboxButtons);
	setLayout(vboxMain);
}

NoteDialog::~NoteDialog()
{
	aDB->removeExtracts(_associations);
}

void NoteDialog::setColor()
{
	QColor newColor=QColorDialog::getColor(editNote->textColor(),this);
	if(newColor.isValid())
		editNote->setTextColor(newColor);

	QTextCursor tc=editNote->textCursor();
	tc.clearSelection();
	editNote->setTextCursor(tc);
}

void NoteDialog::setTitle(const QString &newTitle)
{
	editTitle->setText(newTitle);
}

void NoteDialog::setText(const QString &newText)
{
	editNote->setText(newText);
	buttonSave->setVisible(false);
}

void NoteDialog::setAssociations(int newAssociations)
{
	_associations=newAssociations;
	repaintList();
}

QString NoteDialog::title()
{
	return editTitle->text();
}

QString NoteDialog::text()
{
	return editNote->toHtml();
}

void NoteDialog::repaintList()
{
	if(_associations<0)
		return;

	QHash<int,QString> list=aDB->stringsOfExtracts(_associations);
	QList<int> keys=list.keys();

	modelKeywords->removeRows(0,modelKeywords->rowCount());
	keysForModel.clear();
	foreach(int key,keys)
	{
		modelKeywords->insertRow(modelKeywords->rowCount());
		modelKeywords->setData(
				modelKeywords->index(modelKeywords->rowCount()-1,0),
				list[key],Qt::DisplayRole);
		keysForModel<<key;
	}
}

QSet<int> NoteDialog::selection()
{
	QModelIndexList list=viewKeywords->selectionModel()->selectedIndexes();

	QSet<int> result;
	foreach(QModelIndex index,list)
		result<<keysForModel.value(index.row());

	return result;
}

void NoteDialog::setModified()
{
	buttonSave->setVisible(!keysForModel.isEmpty());
	buttonClose->setText(tr("Close without saving"));
}

void NoteDialog::changeText()
{
	setModified();
}

void NoteDialog::editText(const QString &text)
{
	setModified();
}

void NoteDialog::changeSelection(const QItemSelection &selected,
					  const QItemSelection &deselected)
{
	buttonRemoveKeywords->setEnabled(
			viewKeywords->selectionModel()->selectedIndexes().count()>0);
}

void NoteDialog::addKeywords()
{
	KeywordListDialog dialog(this);
	if(dialog.exec())
	{
		int tmp=aDB->createEmptyExtracts();
		if(!aDB->fillExtractsWithKeys(tmp,dialog.selection()))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		aDB->stringsOfExtracts(tmp);
		if(!aDB->addExtracts(_associations,tmp))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());
		repaintList();
		if(!aDB->removeExtracts(tmp))
			QMessageBox::warning(this,tr("Database error"),aDB->lastError());

		setModified();
	}
}

void NoteDialog::removeKeywords()
{
	int tmp=aDB->createEmptyExtracts();
	if(!aDB->fillExtractsWithKeys(tmp,selection()))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	aDB->stringsOfExtracts(tmp);
	if(!aDB->subtractExtracts(_associations,tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());
	repaintList();
	if(!aDB->removeExtracts(tmp))
		QMessageBox::warning(this,tr("Database error"),aDB->lastError());

	setModified();
}
