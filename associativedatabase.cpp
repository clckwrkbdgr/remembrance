#include <QFile>
#include <QtDebug>
#include "associativedatabase.h"

//-------------------------- global declaration --------------------------------
AssociativeDatabase associativeDatabase;

AssociativeDatabase* AssociativeDatabase::db()
{
	return &associativeDatabase;
}

//--------------------------------- methods ------------------------------------
AssociativeDatabase::AssociativeDatabase()
		: QObject(), _modified(false)
{
}

bool AssociativeDatabase::save(const QString &fileName)
{
	if(!_modified)
		return true;

	//open file
	QFile file(fileName);
	if(!file.open(QFile::WriteOnly))
	{
		_lastError=tr("Cannot open file <%1> for write.").arg(fileName);
		return false;
	}
	QDataStream out(&file);

	//write
	out<<QString("REMEMBRANCE")<<0.9<<out.version(); //write header
	out<<_keywords<<_notes<<_links; //read data
	return true;
}

bool AssociativeDatabase::load(const QString &fileName)
{
	QFile file(fileName);
	if(file.exists())
	{
		//open file
		if(!file.open(QFile::ReadOnly))
		{
			_lastError=tr("Cannot open file <%1> for read.").arg(fileName);
			return false;
		}
		QDataStream in(&file);

		//read header
		QString magicWord;
		quint32 dataStreamVersion;
		in>>magicWord>>_version>>dataStreamVersion;
		if(magicWord!=QString("REMEMBRANCE"))
		{
			_lastError=tr("File <%1> has wrong format!").arg(fileName);
			return false;
		}
		if(_version!=0.9)
		{
			_lastError=tr("File <%1> has unknown version %2!").arg(fileName).
					   arg(_version);
			return false;
		}

		//read data
		in.setVersion(dataStreamVersion);
		in>>_keywords>>_notes>>_links;

		_modified = false;
		return true;
	}
	return true; //if not exists
}

double AssociativeDatabase::version()
{
	return _version;
}

const QString& AssociativeDatabase::lastError() const
{
	return _lastError;
}

//------------------------------ extractsKey --------------------------------------

int AssociativeDatabase::createEmptyExtracts()
{
	//get max key
	QList<int> keys=_extracts.keys();
	int maxKey=0;
	foreach(int key,keys)
		if(maxKey<key)
			maxKey=key;

	//add extractsKey
	maxKey++; //first free
	_extracts[maxKey]=QSet<int>();

	return maxKey;
}

bool AssociativeDatabase::removeExtracts(int extractsKey)
{
	//remove and validate
	if(_extracts.remove(extractsKey)<1)
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}

	return true;
}

QHash<int,QString> AssociativeDatabase::stringsOfExtracts(int extractsKey)
{
	QHash<int,QString> result;

	//validate
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return result;
	}

	//compute
	foreach(int key,_extracts[extractsKey])
		result[key]=_keywords[key];

	return result;
}

bool AssociativeDatabase::addExtracts(int extractsKey,int addition)
{
	//validate
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}
	if(!_extracts.contains(addition))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(addition);
		return false;
	}

	//do
	_extracts[extractsKey]+=_extracts[addition];
	return true;
}

bool AssociativeDatabase::subtractExtracts(int extractsKey,int subtraction)
{
	//validate
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}
	if(!_extracts.contains(subtraction))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(subtraction);
		return false;
	}

	//do
	_extracts[extractsKey]-=_extracts[subtraction];
	return true;
}

int AssociativeDatabase::search(int extractsKey,const QString &partOfWord)
{
	foreach(int key,_extracts[extractsKey])
	{
		if(_keywords[key].contains(partOfWord, Qt::CaseInsensitive))
			return key;
	}
	return -1;
}

//------------------------------- keywords -------------------------------------

bool AssociativeDatabase::fillExtractsWithAllKeywords(int extractsKey)
{
	//validate
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}

	//fill
	_extracts[extractsKey]=QSet<int>::fromList(_keywords.keys());
	return true;
}

bool AssociativeDatabase::fillExtractsWithKeys(int extractsKey,
											   const QSet<int> &keys)
{
	//validate
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}

	//fill
	_extracts[extractsKey]=keys;
	return true;
}

int AssociativeDatabase::addNewKeyword(const QString &text)
{
	//get max key
	QList<int> keys=_keywords.keys();
	int maxKey=0;
	foreach(int key,keys)
		if(maxKey<key)
			maxKey=key;

	//add extractsKey
	maxKey++; //first free
	_keywords[maxKey]=text;
	_modified = true;

	return maxKey;
}

void AssociativeDatabase::editKeyword(int keywordIndex, const QString & newText)
{
	if(!_keywords.keys().contains(keywordIndex)) {
		return;
	}
	_keywords[keywordIndex] = newText;
}

void AssociativeDatabase::removeUnusedKeywords()
{
	QSet<int> keys=_keywords.keys().toSet();
	QList<int> values=_links.values();
	foreach(int key,keys)
	{
		//remove if not used in links
		if(!values.contains(key))
		{
			_keywords.remove(key);
			_modified = true;

			//remove from working extracts
			QList<int> extractsKeys=_extracts.keys();
			foreach(int extractsKey,extractsKeys)
				_extracts[extractsKey].remove(key);
		}
	}
}

//-------------------------------- notes ---------------------------------------

QSet<int> AssociativeDatabase::getNotesByKeywords(int extractsKey)
{
	//validate
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return QSet<int>();
	}

	QSet<int> result;
	if(_extracts[extractsKey].isEmpty()) {
		return _links.keys().toSet();
		//return result; //for empty request there is empty result
	}

	QList<int> keys=_links.keys();
	foreach(int key,keys)
	{ //note match, if it contains all the extracts components
		if((_extracts[extractsKey] - _links.values(key).toSet()).isEmpty())
			result<<key;
	}

	return result;
}

bool AssociativeDatabase::removeNote(int note)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return false;
	}

	//remove
	_notes.remove(note);
	_links.remove(note);
	_modified = true;
	return true;
}

int AssociativeDatabase::addNewNote(const QString &newTitle,
									const QString &newText)
{
	//get max key
	QList<int> keys=_notes.keys();
	int maxKey=0;
	foreach(int key,keys)
		if(maxKey<key)
			maxKey=key;

	//add extractsKey
	maxKey++; //first free
	_notes[maxKey]=qMakePair(newTitle,newText);
	_modified = true;

	return maxKey;
}

//--------------------------- note properties ----------------------------------

QString AssociativeDatabase::title(int note)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return QString();
	}

	//do
	return _notes[note].first;
}

QString AssociativeDatabase::text(int note)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return QString();
	}

	//do
	return _notes[note].second;
}

bool AssociativeDatabase::setTitle(int note,const QString &newTitle)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return false;
	}

	//do
	_notes[note].first=newTitle;
	_modified = true;
	return true;
}

bool AssociativeDatabase::setText(int note,const QString &newText)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return false;
	}

	//do
	_notes[note].second=newText;
	_modified = true;
	return true;
}

bool AssociativeDatabase::fillExtractsWithNoteAssociation(int note,
														  int extractsKey)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return false;
	}
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}

	//do
	_extracts[extractsKey]=_links.values(note).toSet();
	return true;
}

bool AssociativeDatabase::setNoteAssociationFromExtracts(int note,
														 int extractsKey)
{
	//validate
	if(!_notes.contains(note))
	{
		_lastError=tr("Invalid note key <%1>").arg(note);
		return false;
	}
	if(!_extracts.contains(extractsKey))
	{
		_lastError=tr("Invalid extracts key <%1>").arg(extractsKey);
		return false;
	}

	//do
	_links.remove(note);
	_modified = true;
	foreach(int key,_extracts[extractsKey])
		_links.insert(note,key);
	return true;
}

