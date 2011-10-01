#ifndef NOTEDIALOG_H
#define NOTEDIALOG_H

#include <QDialog>
#include <QSet>

#include "associativedatabase.h"

class QStringListModel;
class QListView;
class QLineEdit;
class QTextEdit;
class QItemSelection;

class NoteDialog : public QDialog
{
	Q_OBJECT

	int _associations;

	QPushButton *buttonAddKeywords,*buttonRemoveKeywords;

	QList<int> keysForModel;
	QStringListModel *modelKeywords;
	QListView *viewKeywords;
	QLineEdit *editTitle;
	QTextEdit *editNote;

	QPushButton *buttonColor;

	QPushButton *buttonClose,*buttonSave;

	void repaintList();
	QSet<int> selection();
	void setModified();
private slots:
	void changeText();
	void editText(const QString &text);
	void changeSelection(const QItemSelection &selected,
						  const QItemSelection &deselected);
	void setColor();
public:
    NoteDialog(QWidget *parent=0);
    ~NoteDialog();

	void setTitle(const QString &newTitle);
	void setText(const QString &newText);
	void setAssociations(int newAssociations);
	QString title();
	QString text();
public slots:
	void addKeywords();
	void removeKeywords();
};

#endif // NOTEDIALOG_H
