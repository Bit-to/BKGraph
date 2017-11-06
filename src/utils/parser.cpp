#include "engine/world.h"

bool Parser::isDigit() {
	return getc() >= '0' && getc() <= '9';
}

bool Parser::isOperator() {
	return getc() == '+' || getc() == '-' || getc() == '*' || getc() == '/' || getc() == '^';
}

template <typename T>
bool Parser::isWord(const map<string, T>& words) {
	for (const pair<string, T>& it : words)
		if (getc() == it.first[0])
			return true;
	return false;
}

bool Parser::cmpWord(const string& word) {
	uint i = 0;
	do {
		if (str->at(id+i) != word[i])
			return false;
	} while (word[i++] != '\0');
	return true;
}

// CHECKER

bool Parser::check(string& formula) {
	// remove whitespaces
	for (sizt i=0; i!=formula.size(); i++)
		if (formula[i] == ' ') {
			formula.erase(i);
			i--;
		}

	// reset values
	str = &formula;
	id = 0;
	pcnt = 0;

	// do the checking
	try {
		readFirst();
		if (pcnt != 0)
			throw id;
	} catch (sizt e) {
		cerr << "syntax error at " << e << endl;
		return false;
	}
	return true;
}

void Parser::checkFirst() {
	if (isDigit())
		checkNumber();
	else if (isWord(Default::parserConsts) || isWord(World::program()->getVariables()))
		checkVar();
	else if (isWord(Default::parserFuncs))
		checkFunc();
	else if (getc() == '-')
		checkOperator();
	else if (getc() == '(')
		checkParOpen();
	else
		throw id;
}

void Parser::checkNumber() {
	id++;
	while (isDigit())
		id++;

	if (getc() == '.') {
		id++;
		if (!isDigit())
			throw 1;

		while (isDigit())
			id++;
	}

	if (isOperator())
		checkOperator();
	else if (getc() == ')')
		checkParClose();
	else if (getc() != '\0')
		throw id;
}

void Parser::checkVar() {
	id++;
	if (!checkWord(Default::parserConsts))
		checkWord(World::program()->getVariables());

	if (isOperator())
		checkOperator();
	else if (getc() == ')')
		checkParClose();
	else if (getc() != '\0')
		throw id;
}

void Parser::checkFunc() {
	id++;
	checkWord(Default::parserFuncs);

	if (getc() == '(')
		checkParOpen();
	else
		throw id;
}

template<typename T>
bool Parser::checkWord(const map<string, T>& words) {
	for (const pair<string, T>& it : words)
		if (cmpWord(it.first)) {
			id += it.first.length();
			return true;
		}
	return false;
}

void Parser::checkOperator() {
	id++;
	if (isDigit())
		checkNumber();
	else if (getc() == '(')
		checkParOpen();
	else if (isWord(Default::parserConsts) || isWord(World::program()->getVariables()))
		checkVar();
	else if (isWord(Default::parserFuncs))
		checkFunc();
	else
		throw id;
}

void Parser::checkParOpen() {
	id++;
	pcnt++;

	readFirst();
}

void Parser::checkParClose() {
	id++;
	pcnt--;

	if (isOperator())
		checkOperator();
	else if (getc() == ')')
		checkParClose();
	else if (getc() != '\0')
		throw id;
}

// SOLVER

double Parser::solve(string& formula) {
	// reset values
	str = &formula;
	id = 0;

	// do the solving
	try {
		return readAddSub();
	} catch (sizt e) {
		cerr << "solving error at " << e << endl;
		return 0.0;
	}
}

double Parser::readAddSub() {
	double res = readMulDiv();
	while (getc() == '+' || getc() == '-') {
		if (getc() == '+') {
			id++;
			res += readMulDiv();
		} else {
			id++;
			res -= readMulDiv();
		}
	}
	return res;
}

double Parser::readMulDiv() {
	double res = readPower();
	while (getc() == '*' || getc() == '/') {
		if (getc() == '*') {
			id++;
			res *= readPower();
		} else {
			id++;
			res /= readPower();
		}
	}
	return res;
}

double Parser::readPower() {
	double res = readFirst();
	while (getc() == '^') {
		id++;
		res = std::pow(res, readFirst());
	}
	return res;
}

double Parser::readFirst() {
	if (isDigit())
		return readNumber();
	else if (getc() == '(') {
		id++; // '('
		double result = readAddSub();
		id++; // ')'
		return result;
	} else if (isWord(Default::parserConsts) || isWord(World::program()->getVariables()))
		return readVar();
	else if (isWord(Default::parserFuncs))
		return readFunc();
	else if (getc() == '-') {
		id++;
		return -readFirst();
	}
	throw id;
}

double Parser::readNumber() {
	double res = getc() - '0';
	id++;
	while (isDigit()) {
		res = res*10.0 + double(getc() - '0');
		id++;
	}

	if (getc() == '.') {
		id++;
		double fact = 1.0;
		while (isDigit()) {
			res = res*10.0 + double(getc() - '0');
			fact /= 10.0;
			id++;
		}
		res *= fact;
	}
	return res;
}

double Parser::readVar() {
	for (const pair<string, double>& it : Default::parserConsts)
		if (cmpWord(it.first)) {
			id += it.first.length();
			return it.second;
		}
	for (const pair<string, double>& it : World::program()->getVariables())
		if (cmpWord(it.first)) {
			id += it.first.length();
			return it.second;
		}
	throw id;
}

double Parser::readFunc() {
	for (const pair<string, mfptr>& it : Default::parserFuncs)
		if (cmpWord(it.first)) {
			id += it.first.length() +1;	// skip '('
			return it.second(readFirst());
		}
	throw id;
}
