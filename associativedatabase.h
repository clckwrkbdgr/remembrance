#ifndef ASSOCIATIVEDATABASE_H
#define ASSOCIATIVEDATABASE_H

#include <QObject>
#include <QMultiHash>
#include <QSet>
#include <QStringList>

#define aDB (AssociativeDatabase::db())

class AssociativeDatabase : public QObject
{
	QHash<int,QString> _keywords;
	QHash<int,QPair<QString,QString> > _notes;
	QMultiHash<int,int> _links; //note,keyword

	QHash<int,QSet<int> > _extracts;
	QString _lastError;
	double _version;
	bool _modified;
public:
    AssociativeDatabase();

	bool save(const QString &fileName);
	bool load(const QString &fileName);
	double version();
	const QString& lastError() const;

	//extracts
	int createEmptyExtracts();
	bool removeExtracts(int extractsKey);
	QHash<int,QString> stringsOfExtracts(int extractsKey);
	bool addExtracts(int extractsKey,int addition);
	bool subtractExtracts(int extractsKey,int subtraction);

	int search(int extractsKey,const QString &partOfWord);

	//keywords
	bool fillExtractsWithAllKeywords(int extractsKey);
	bool fillExtractsWithKeys(int extractsKey,const QSet<int> &keys);

	int addNewKeyword(const QString &text);
	void removeUnusedKeywords();

	//notes
	QSet<int> getNotesByKeywords(int extractsKey);

	bool removeNote(int note);
	int addNewNote(const QString &newTitle,const QString &newText);

	//note properties
	QString title(int note);
	QString text(int note);
	bool setTitle(int note,const QString &newTitle);
	bool setText(int note,const QString &newText);
	bool fillExtractsWithNoteAssociation(int note,int extractsKey);
	bool setNoteAssociationFromExtracts(int note,int extractsKey);

	//one and only
	static AssociativeDatabase* db();
};

#endif // ASSOCIATIVEDATABASE_H
