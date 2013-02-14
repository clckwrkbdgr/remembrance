#ifndef KEYWORDSLISTDIALOG_H
#define KEYWORDSLISTDIALOG_H

#include <QDialog>
#include <QHash>

#include "associativedatabase.h"

class QStringListModel;
class QListView;
class QItemSelection;
class QModelIndex;

class KeywordListDialog : public QDialog
{
	Q_OBJECT

	int _allKeywords;

	QList<int> keysForModel;
	QStringListModel *model;
	QListView *view;

	QPushButton *buttonAddKeyword,*buttonRemoveUnused;
	QPushButton *buttonPick,*buttonCancel;

	void repaintList();
private slots:
	void changeSelection(const QItemSelection &selected,
						  const QItemSelection &deselected);
	void pickSelectedItem(const QModelIndex & index);
	void addKeyword();
	void removeUnused();
public:
    KeywordListDialog(QWidget *parent=0);
    ~KeywordListDialog();

	QSet<int> selection();
};

#endif // KEYWORDSLISTDIALOG_H
