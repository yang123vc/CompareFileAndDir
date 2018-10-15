#ifndef COMPAREROOLREGEXP_H
#define COMPAREROOLREGEXP_H

#include <QRegExp>

class CompareRoolRegExp : public QRegExp
{
public:
	CompareRoolRegExp(){}
	explicit CompareRoolRegExp(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive,
		PatternSyntax syntax = RegExp):QRegExp(pattern,cs,syntax){}
	//CompareRoolRegExp(const CompareRoolRegExp &rx)
	//{
	//	this->QRegExp(rx);
	//};
	~CompareRoolRegExp(){};

	bool exactMatch ( const QString & str )
	{
		return !QRegExp::exactMatch(str);
	}
private:
	
};

#endif // COMPAREROOLREGEXP_H
