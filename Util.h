#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <time.h>
#include <random>
#include <algorithm>
#include <execution>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include "string.h"
#include "antlr4-runtime.h"
#include "parser/rpqLexer.h"
#include "parser/rpqParser.h"
#include "parser/rpqBaseVisitor.h"
#include "omp.h"

/**
	Listener for errors during SPARQL query parsing, which throws a 
	corresponding exception when an error arises.
*/
class RpqErrorListener: public antlr4::BaseErrorListener
{
public:
	void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
		size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e);
};